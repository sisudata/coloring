"""
Utilities for coloring

Here, take this:

PYTHONMALLOC=malloc valgrind --tool=memcheck --error-limit=no python ...
"""

from numba import jit, int32, uint32, uint64, void, float64, boolean, prange
import numpy as np
import scipy.sparse as sps

@jit(
    void(float64[:], int32[:], uint32[:]),
    nopython=True
)
def _uniques_and_counts_compiled(
    data, indptr, counts):
    for i, (start, stop) in enumerate(zip(indptr, indptr[1:])):
        data[start:stop].sort()

        unique_ix = start
        for scan_ix in range(start + 1, stop):
            if data[unique_ix] == data[scan_ix]:
                continue
            unique_ix += 1
            data[unique_ix] = data[scan_ix]

        counts[i] = min(stop, 1 + unique_ix) - start

def get_uniques_and_counts(X):
    """
    Accepts a CSR or CSC sparse matrix over float64.

    Returns (uniques, uniques_offsets, nunique), where
    uniques[uniques_offsets[i]:unique_offsets[i]+nunique[i]]
    contains the sorted, unique values for the i-th
    row (for CSR) or column (for CSC).
    """
    assert sps.issparse(X), type(X)
    assert X.getformat() in ['csc', 'csr']
    assert X.dtype == np.float64

    n = len(X.indptr) - 1
    uniques = X.data.copy()
    nunique = np.zeros(n, np.uint32)
    _uniques_and_counts_compiled(uniques, X.indptr, nunique)
    offsets = X.indptr[:-1]
    return uniques, offsets, nunique

@jit(
    void(
        float64[:], int32[:], uint32[:],
        float64[:], int32[:], uint32[:]),
    nopython=True
)
def _remap_floats_compiled(
    data, indptr, data_out,
    uniques, offsets, nuniques):
    for i, (start, stop, ustart, ulen) in enumerate(zip(
        indptr, indptr[1:], offsets, nuniques)):
        ustop = ustart + ulen
        data_out[start:stop] = 1 + np.searchsorted(uniques[ustart:ustop], data[start:stop])

def remap_floats(X, uniques, offsets, nunique):
    """
    Accepts a CSR or CSC sparse matrix X over float64, and
    its get_uniques_and_counts(X) output.

    Remaps the i-th smallest float in each row (for CSR) or
    column (for CSC) to the value i, returning a corresponding
    sparse matrix over uint32, starting the indexing from
    1 (since 0 is our sparse fill value).
    """
    assert sps.issparse(X), type(X)
    assert X.getformat() in ['csc', 'csr']
    assert X.dtype == np.float64

    data = np.empty(len(X.data), np.uint32)
    _remap_floats_compiled(
        X.data, X.indptr, data,
        uniques, offsets, nunique
    )

    constructor = sps.csc_matrix if X.getformat() else sps.csr_matrix
    return constructor((data, X.indices, X.indptr))

from create_edgeset import uniquify, sort4, create_edgeset_u64 as create_edgeset_u64

def onehot(Xcategorical_csc_remapped, nunique):
    # accepts CSC remapped values (contiguous ints in each column)
    # return CSR, CSC for onehot.
    nnzc = np.diff(Xcategorical_csc_remapped.indptr)

    # Xcategorical - csc u32 sparse matrix with contiguous categorical values
    # nunique - number unique vals in each col, excl 0
    # nnzc - number nonzeros in each col
    col_count_base = np.roll(np.cumsum(nunique, dtype=np.uint32), 1)
    col_count_base[0] = 0

    # column i can be recoded to have integers
    # from col_count_base[i] + 1 (incl) to
    #   col_count_base[i] + nunique[i] + 1 (excl)
    X = Xcategorical_csc_remapped
    data = X.data + np.repeat(col_count_base, nnzc).astype(np.uint32)

    # now that every categorical value is unique,
    # make set the indices to the data values (one-hotting, essentially)
    assert data.max() < 2 ** 31
    data = data.astype(np.int32)

    # uniqued original matrix, where value = its target column in binary form
    X = sps.csc_matrix((data, X.indices, X.indptr), shape=X.shape)
    X = X.tocsr()

    new_data = np.ones_like(X.data, dtype=bool)
    new_indices = X.data

    nrows = X.shape[0]
    ncols = col_count_base[-1] + nunique[-1] + 1
    Xbinary = sps.csr_matrix((new_data, new_indices, X.indptr), shape=(nrows, ncols))
    # creates a useless dummy column 0 with no hot values
    return Xbinary

# create an inline adjacency list

@jit(uint32(uint64), nopython=True)
def left(x):
    return x >> 32

@jit(uint32(uint64), nopython=True)
def right(x):
    return x & (2 ** 32 - 1)

@jit(void(uint64[:], uint32[:]), nopython=True)
def count_degree(edges, degree):
    for e64 in edges:
        l, r = left(e64), right(e64)
        degree[l] += 1
        degree[r] += 1

@jit(void(uint64[:], uint32[::1], uint64[:], uint64[:]), nopython=True, parallel=True)
def fill_edges(edges, bidir_edges, start_offsets, start_offsets_immutable):
    for e64 in edges:
        l, r = left(e64), right(e64)
        bidir_edges[start_offsets[l]] = r
        bidir_edges[start_offsets[r]] = l
        start_offsets[l] += 1
        start_offsets[r] += 1

    noffsets = len(start_offsets_immutable) - 1
    for i in prange(noffsets):
        start = start_offsets_immutable[i]
        stop = start_offsets_immutable[i + 1]
        sort4(bidir_edges, start, stop - start)

u32max = np.iinfo(np.uint32).max
@jit(
    uint32(uint32[:], uint32[:], uint64[:],
           uint32[:], boolean[:]),
    nopython=True
)
def _color_graph_compiled(
    vertex_order, adjacency, vertex_offsets,
    color_map, adjacent_colors):
    ncolors = 0
    for v in vertex_order:
        vstart, vend = vertex_offsets[v], vertex_offsets[v + 1]
        for n in adjacency[vstart:vend]:
            if color_map[n] != u32max:
                adjacent_colors[color_map[n]] = True

        color = ncolors
        for i in range(ncolors):
            if not adjacent_colors[i]:
                color = i
                break

        ncolors = max(color + 1, ncolors)
        color_map[v] = color

        if vend - vstart > ncolors:
            adjacent_colors[:ncolors] = False
        else:
            for n in adjacency[vstart:vend]:
                if color_map[n] != u32max:
                    adjacent_colors[color_map[n]] = False
    return ncolors

def color_graph(degree, bidir_edges, vertex_offsets, color_ub=2 ** 16):
    nverts = len(degree)
    smallest_first = np.argsort(degree).astype(np.uint32)
    largest_first = smallest_first[::-1]

    color_map = np.full(int(nverts), u32max, dtype=np.uint32)
    # will segfault if you have >2**10 colors
    adjacent_colors = np.zeros(color_ub, dtype=bool)

    ncolors = _color_graph_compiled(
        largest_first, bidir_edges, vertex_offsets,
        color_map, adjacent_colors)

    return ncolors, color_map

@jit(void(uint32[:], uint32[:, ::1], uint32[:]), nopython=True)
def _color_remap_compiled(
        remap_map,
        color_coded_T,
        color_cards):
    ncolors, nrows = color_coded_T.shape
    for col in range(ncolors):
        column = color_coded_T[col]
        sort4(color_coded_T.ravel(), col * nrows, nrows)
        ucol = uniquify(column)
        color_cards[col] = ucol - 1
        for i, c in enumerate(column[:ucol]):
            remap_map[c] = i


def color_remap(Xbinary_csr, ncolors, color_map, nnzr):
    nverts = Xbinary_csr.shape[1]
    nrows = Xbinary_csr.shape[0]
    color_coded = np.zeros((nrows, ncolors), dtype=np.uint32)
    color_coded_T = np.zeros((ncolors, nrows), dtype=np.uint32)

    row_ix = np.repeat(np.arange(0, nrows, dtype=np.uint32), nnzr)
    active_columns = Xbinary_csr.indices
    colors = color_map[active_columns]
    color_coded[row_ix, colors] = active_columns
    color_coded_T[colors, row_ix] = active_columns

    # convert unique factors back into compact intervals
    remap_map = np.zeros(int(nverts), np.uint32)
    color_cards = np.zeros(ncolors, np.uint32)
    _color_remap_compiled(
        remap_map,
        color_coded_T,
        color_cards)

    Xcategorical_color = np.take(remap_map, color_coded)

    return Xcategorical_color, color_cards

import time
from contextlib import contextmanager
import sys

class _timeit:
    def __init__(self):
        self.seconds = 0

    def set_seconds(self, x):
        self.seconds = x

@contextmanager
def timeit(name=None, name_pad=32):
    if name:
        print(('{:>' + str(name_pad) + '}').format(name), end='')
    sys.stdout.flush()
    x = _timeit()
    t = time.time()
    yield x
    x.set_seconds(time.time() - t)
    if name:
        print("  ...took {:10.2f} sec ".format(x.seconds))


from joblib import Memory
memory = Memory('urls.coloring.cache')

from sklearn.datasets import load_svmlight_file

def read_svmlight(filename):
    X, y = load_svmlight_file('url_svmlight/' + filename)
    assert X.shape[0] == len(y)
    return X, y

import os
datafiles = [f for f in os.listdir('url_svmlight') if f.startswith('Day') and f.endswith('.svm')]
datafiles = list(sorted((f for f in datafiles), key=lambda x: int(x[len('Day'):x.index('.svm')])))

from multiprocessing import Pool, cpu_count

@memory.cache
def read_all_svmlight():
    Xs, ys = [], []
    nrows = 0
    ncols = 0
    with Pool(cpu_count()) as p:
        for X, y in p.map(read_svmlight, datafiles):
            nrows += len(y)
            ncols = max(ncols, X.shape[1])
            Xs.append(X)
            ys.append(y)
    return Xs, ys, nrows, ncols

def pad_columns(Xs, ncols):
    # TODO: csc-specialized version of this should be really fast
    for i in range(len(Xs)):
        r, c = Xs[i].shape
        if ncols > c:
            Xs[i] = sps.hstack([Xs[i], sps.lil_matrix((r, ncols - c), dtype=Xs[i].dtype)], 'csc')

@memory.cache
def extract_continuous():
    continuous_feature_ixs = []
    with open('url_svmlight/FeatureTypes') as f:
        for line in f:
            continuous_feature_ixs.append(int(line))

    return continuous_feature_ixs

@memory.cache
def extract_sparse():
    with timeit('load all svmlight files'):
        Xs, ys, nrows, ncols = read_all_svmlight()

    with timeit('pad columns'):
        pad_columns(Xs, ncols)

    with timeit('gather feature type ixs'):
        continuous_feature_ixs = extract_continuous()
        cat_feature_ixs = [i for i in range(Xs[0].shape[1]) if i not in set(continuous_feature_ixs)]

    with timeit('extract continuous'):
        Xcontinuous = np.concatenate([X[:, continuous_feature_ixs].todense() for X in Xs])

    with timeit('extract categorical'):
        # TODO: csc-specialized version of this should be really fast
        Xcategorical_csc = sps.vstack([X[:, cat_feature_ixs] for X in Xs], 'csc')

    return Xcontinuous, Xcategorical_csc, ys, nrows, ncols

@memory.cache
def get_all_data():
    Xcontinuous, Xcategorical_csc, ys, nrows, ncols = extract_sparse()

    with timeit('cat label'):
        y = np.concatenate(ys) == 1
        y = y.astype(float)

    with timeit('unique column values'):
        uniques, offsets, nunique = get_uniques_and_counts(Xcategorical_csc)

    with timeit('remap categorical floats'):
        Xcategorical_csc = remap_floats(Xcategorical_csc, uniques, offsets, nunique)
    # coloring works just fine with categorical input, since you can create
    # a new vertex for categorical values
    with timeit('onehot'):
        Xbinary_csr = onehot(Xcategorical_csc, nunique)
        
    with timeit('csc'):
        Xcsc = Xbinary_csr.tocsc()

    ncols = Xbinary_csr.shape[1] + Xcontinuous.shape[1]

    return Xcontinuous, Xbinary_csr, Xcsc, y, nrows, ncols

# messing with env for C lib parallelism here...
# can't use this with other jobs with diff nthreads
def set_parallelism(nthreads):
    if 'NUMBA_NUM_THREADS' in os.environ:
        assert os.environ['NUMBA_NUM_THREADS'] == str(nthreads), "once set, can't override"
    else:
        os.environ['NUMBA_NUM_THREADS'] = str(nthreads)
    if 'OMP_NUM_THREADS' in os.environ:
        assert os.environ['OMP_NUM_THREADS'] == str(nthreads), "once set, can't override"
    else:
        os.environ['OMP_NUM_THREADS'] = str(nthreads)

def sums_to_means_precondition(offsets, means, counts):
    assert np.all(0 <= offsets)
    assert np.all(offsets <= len(means))
    assert np.all(offsets[-1] == len(means))
    assert len(means) == len(counts)

@jit(void(uint32[:], float64[:], uint32[:]), nopython=True)
def sums_to_means(offsets, means, counts):
    for start, stop in zip(offsets, offsets[1:]):
        net_sum = means[start:stop].sum()
        net_count = counts[start:stop].sum()
        for i in range(start, stop):
            if counts[i]:
                means[i] = means[i] / counts[i]
            else:
                means[i] = net_sum / net_count if net_count else 0

def fit_target_encode_csc_precondition(
    indptr, data, indices, y,
    offsets, counts, means):
    sums_to_means_precondition(offsets, means, counts)
    assert len(indptr) == len(offsets)
    assert indptr[-1] == len(data)

    for col, (start, stop) in enumerate(zip(indptr, indptr[1:])):
        card = offsets[col + 1] - offsets[col]
        assert card >= 0
        assert np.all(0 <= data[start:stop] -1)
        assert np.all(data[start:stop] - 1 < card)
        assert np.all(0 <= offsets[col] + data[start:stop] - 1)
        assert np.all(offsets[col] + data[start:stop] - 1 < len(means))
        assert np.all(0 <= indices)
        assert np.all(indices < len(y))

@jit(void(int32[:], uint32[:], int32[:], float64[:],
          uint32[:], uint32[:], float64[:]),
     nopython=True)
def fit_target_encode_csc(
    indptr, data, indices, y,
    offsets, counts, means):
    for col, (start, stop) in enumerate(zip(indptr, indptr[1:])):
        for nnz_ix in range(start, stop):
            value_ix = offsets[col] + data[nnz_ix] - 1
            means[value_ix] += y[indices[nnz_ix]]
            counts[value_ix] += 1

    sums_to_means(offsets, means, counts)

def fit_target_encode_dense_precondition(
    Xcat, y,
    offsets, counts, means):
    sums_to_means_precondition(offsets, means, counts)
    nrows, ncols = Xcat.shape
    assert ncols + 1 == len(offsets)

    for col in range(ncols):
        card = offsets[col + 1] - offsets[col]
        assert card > 0
        assert np.all(0 <= Xcat[:, col] - 1)
        assert np.all(Xcat[:, col] - 1 < card)
        assert np.all(0 <= offsets[col] + Xcat[:, col] - 1)
        assert np.all(offsets[col] + Xcat[:, col] - 1 < len(means))

@jit(void(uint32[:, :], float64[:],
          uint32[:], uint32[:], float64[:]),
    nopython=True)
def fit_target_encode_dense(
    Xcat, y,
    offsets, counts, means):
    nrows, ncols = Xcat.shape
    for row in range(nrows):
        # TODO invert, then vectorize this loop
        for col in range(ncols):
            value_ix = offsets[col] + Xcat[row, col] - 1
            means[value_ix] += y[row]
            counts[value_ix] += 1

    sums_to_means(offsets, means, counts)

@jit(void(int32[:], uint32[:],
          uint32[:], float64[:],
          float64[:]),
    nopython=True)
def transform_target_encode_csc(
    indptr, data,
    offsets, means,
    data_out):
    for col, (start, stop) in enumerate(zip(indptr, indptr[1:])):
        data_out[start:stop] = means[offsets[col] + data[start:stop] - 1]

@jit(void(uint32[:, :],
          uint32[:], float64[:],
          float64[:, :]),
    nopython=True)
def transform_target_encode_dense(
    Xcat,
    offsets, means,
    data_out):
    nrows, ncols = Xcat.shape
    for row in range(nrows):
        # TODO: invert, then vectorize this loop
        for col in range(ncols):
            value_ix = offsets[col] + Xcat[row, col] - 1
            data_out[row, col] = means[value_ix]

class TargetEncoder:
    """
    Should be initialized with

    cards - cardinalities for categorical columns, in order,
            excluding zeros from cardinality
    is_sparse - whether to expect sparse categorical inputs or dense ones

    It's OK to know this cardinality info ahead of time since
    values unseen in training are filled with the average
    target value from the training set.
    """

    def __init__(self, *, cards, is_sparse, debug):
        self.cards = cards.astype(np.uint32)
        self.is_sparse = is_sparse
        self.debug = debug

        # means with imputation values for non-zero entries
        self.means = np.zeros(np.sum(cards), np.float64)
        self.counts = np.zeros(len(self.means), np.uint32)
        self.offsets = np.cumsum(np.insert(cards, 0, 0), dtype=np.uint32)

    def check_sparse(self, Xcat):
        assert self.is_sparse == sps.issparse(Xcat), (self.is_sparse, type(Xcat))
        if self.is_sparse:
            assert Xcat.getformat() == 'csc', Xcat.getformat()

    def fit(self, X, y=None):
        """
        Assumes csc for is_sparse, assumes all continuous columns are first
        expects input X to be a tuple (Xcont, Xcat) of design matrics
        for continuous, categorical features.

        Xcont should be over float64, Xcat should be over uint32.

        Xcat may be csc sparse or dense. If it is sparse, then
        the transformation of this operator will (by necessity of
        classifier interfaces) generate a sparse matrix with the
        categorical values.
        """
        Xcont, Xcat = X
        self.check_sparse( Xcat)

        if self.is_sparse:
            args = (
                Xcat.indptr, Xcat.data, Xcat.indices, y,
                self.offsets, self.counts, self.means)
            if self.debug:
                fit_target_encode_csc_precondition(*args)
            fit_target_encode_csc(*args)
        else:
            args = (
                Xcat, y,
                self.offsets, self.counts, self.means)
            if self.debug:
                fit_target_encode_dense_precondition(*args)
            fit_target_encode_dense(*args)

        return self

    def transform(self, X, y=None):
        """
        See self.fit() documentation for expected X input.

        Ignores y argument.

        Returns a single matrix, the new design matrix
        after categorical encoding, which will be sparse
        iff self.is_sparse
        """
        Xcont, Xcat = X
        self.check_sparse(Xcat)

        if self.is_sparse:
            data_out = np.empty(Xcat.data.shape, np.float64)
            transform_target_encode_csc(
                    Xcat.indptr, Xcat.data,
                    self.offsets, self.means,
                    data_out)
            Xcat_encoded = sps.csc_matrix((data_out, Xcat.indices, Xcat.indptr))
            return sps.hstack([Xcont, Xcat_encoded], 'csc')
        else:
            data_out = np.zeros(Xcat.shape, np.float64)
            transform_target_encode_dense(
                    Xcat,
                    self.offsets, self.means,
                    data_out
                )
            return np.hstack([Xcont, Xcat])

    def fit_transform(self, X, y=None):
        return self.fit(X, y).transform(X, y)
