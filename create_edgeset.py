# Below code assumes that ./build.sh has been run
import os
from numba import jit, int32, uint32, uint64, void, float64, boolean
import numpy as np

# https://stackoverflow.com/a/35317443/1779853
from cffi import FFI
from numba import prange

# from buffer returns a pointer to the beginning of the
# py buffer, and numba doesn't support pointer arithmatic
# so we need an offset capability in the C code.

ffi = FFI()
ffi.cdef('void u8_sort_offset(unsigned long long *a, const unsigned long long offset, const long sz);')
C = ffi.dlopen('u8_sort.so')
C_u8_sort_offset = C.u8_sort_offset

ffi2 = FFI()
ffi2.cdef('void u4_sort_offset(unsigned *a, const unsigned long long offset, const long sz);')
C = ffi2.dlopen('u4_sort.so')
C_u4_sort_offset = C.u4_sort_offset

class NullContextManager(object):
    def __init__(self, total=None):
        self.dummy_resource = total
    def __enter__(self):
        return self.dummy_resource
    def __exit__(self, *args):
        pass

@jit([uint32(uint64[:]), uint32(uint32[:])], nopython=True)
def uniquify(x):
    ctr = 1
    for i in range(1, len(x)):
        if x[i] != x[i - 1]:
            x[ctr] = x[i]
            ctr += 1
    return min(ctr, len(x))

@jit(void(uint32[::1], uint64, int32), nopython=True)
def sort4(x, offset, buflen):
    C_u4_sort_offset(ffi2.from_buffer(x), offset, buflen)

@jit(uint32(uint64[::1], uint64, int32), nopython=True)
def dirty_unique(x, offset, buflen):
    C_u8_sort_offset(ffi.from_buffer(x), offset, buflen)
    return uniquify(x[offset:offset+buflen])

# https://stackoverflow.com/a/28663374/1779853
from parallelSort import numpyParallelSort

def merge(c):
    numpyParallelSort(c)
    lenc = uniquify(c)
    return c[:lenc]

# edgestores is really a matrix of row length 'rowlen'
# but numba static analysis buckles on matrix inputs
@jit(
    void(uint64[::1], uint32, int32[:], int32[:], uint32[:], uint32[:], uint32[:]),
    nopython=True,
    parallel=True
)
def pairs_into(edgestores, rowlen, indptr, indices, starts, stops, out):
    SKIP = rowlen
    nthreads = len(starts)
    for t in prange(nthreads):
        ctr = 0
        for i in range(starts[t], stops[t]):
            lo, hi = indptr[i], indptr[i+1]
            nnz = hi - lo
            for j, d in enumerate(indices[lo:hi]):
                nk = nnz - j - 1
                # we assume int32s are positive all over
                edgestores[t * SKIP + ctr:t * SKIP + ctr + nk] = uint64(d) << 32
                for k in range(nk):
                    edgestores[t * SKIP + ctr + k] |= uint64(indices[lo + j + k + 1])
                ctr += nk
        uq = dirty_unique(edgestores, t * SKIP, ctr)
        out[t] = uq

def create_edgeset_u64(Xbinary_csr, edgebufsz, tqdm=None, nthreads=16):
    """
    NOTE: this doesn't actually set the number of threads.
    You should have done that at the beginning of your program for
    OMP_NUM_THREADS
    and
    NUMBA_NUM_THREADS
    These can't be re-initialized.
    """
    nnzr = np.diff(Xbinary_csr.indptr)
    edges_per_row = nnzr * (nnzr - 1) // 2
    m = edges_per_row.max()
    assert edges_per_row.max() < edgebufsz
    edges_so_far = np.cumsum(np.insert(edges_per_row, 0, 0))
    edges_so_far = np.insert(edges_so_far, -1, edges_so_far[-1])

    edgebuf = np.zeros(nthreads * edgebufsz, np.uint64)

    parent = np.zeros((0,), np.uint64)

    nrows = Xbinary_csr.shape[0]
    row_start = 0
    pbar_gen = tqdm if tqdm else NullContextManager
    with pbar_gen(total=nrows) as pbar:
        while row_start < nrows:
            lens = np.zeros(nthreads, np.uint32)

            edgebufsz_lb = edgebufsz - m
            # assert edgebufsz_lb > 0
            target_stop_values = (np.arange(nthreads, dtype=np.uint64) + 1) * edgebufsz_lb
            target_stop_values += edges_so_far[row_start]
            ixs = np.searchsorted(edges_so_far[row_start:-1], target_stop_values).astype(np.uint32)
            row_stops = ixs.astype(np.uint32) + row_start
            row_starts = np.roll(row_stops.copy(), 1)
            row_starts[0] = row_start
            # assert (edges_so_far[row_stops] - edges_so_far[row_starts]).max() <= edgebufsz

            pairs_into(
                edgebuf, edgebufsz,
                Xbinary_csr.indptr,
                Xbinary_csr.indices,
                row_starts, row_stops, lens)

            cat = np.concatenate([parent] + [edgebuf[t*edgebufsz:t*edgebufsz + l] for t, l in enumerate(lens)])
            parent = merge(cat)

            if tqdm:
                pbar.update(min(row_stops[-1], nrows) - row_start)
            row_start = row_stops[-1]

    return parent
