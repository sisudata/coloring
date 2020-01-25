# Sisu Blog Post Materials

Please find Sisu Data blog post materials in this repository. Python notebooks require some environment set up, specified below.

## Python Environment Setup for Notebooks

First, install [Anaconda 3](https://www.anaconda.com/distribution/) and add it to your path.


**To be done when you clone or move this repo**:
```
conda env create -f environment.yaml
```

**Should be done once per session:**
```
source activate sisu-env
```

**To save new dependencies**:
```
conda env export --no-builds | grep -v "prefix: " > environment.yaml
```

**To update out-of-date local environment with new dependencies in the `environment.yaml` file**:
```
conda env update -f environment.yaml --prune
```
