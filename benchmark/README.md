# Tunneled BWT Compression Benchmark
Experiments for compression of different compressors.

## What is contained
This bundle of files consist of the following parts:
1. The `cp`-directory contains foreach compressor a standardized interface
   for installation, compression and decompression with an arbitrary compressor.
2. A benchmark measuring compression and resource usage of each compressor
3. A benchmark measuring the estimator quality of estimators in the tunneled bwt
4. A visualization for the benchmark data
5. A set of test files (which need to be downloaded first, see below), contained
   in the `rcrdata`-directory

## Requirements
To run the benchmark, you need the following:
- a modern c++11 ready compiler such as [gcc](https://gcc.gnu.org/) version 4.7 or newer
- [awk]
- [bc]
- [sed]
- [tr]

To visualize your results, the following programs are needed:
- [pdflatex], especially supporting pgf and pgfplotstable

To download and set up the test files, the following programs are needed:
- [curl](https://curl.haxx.se/)
- [gzip]
- [bzip2]

## Installation
-  To install the required compressors, call `sudo make install`. Superuser-rights
  are required to download the other compressors using [apt-get], which can be 
  avoided by downloading the compressors yourself, see the scripts in the
  `cp`-directory.
- To download the test data, switch into the `rcrdata` - directory, and call `make`.
  This will download and extract all of the test data using [curl].

## Usage

### Benchmark
To run the benchmark, configure the files `testcases.config` and `compressors.config`
as you require it, an example is already listed. Afterwards, call

	make

After the benchmark has finished, 4 files are generated:
- `result.dat`: a file containing the benchmark results of all test files on all compressors.
  Every speed measurement is measured in MB/s, every size is measured in bits per symbol
  (both with the size of the original file as borderline)
- `estquality.dat`: a file containing the measured relative errors of estimators
  used for BWT Tunneling.
- `result.tex`: a file ready to be compiled with [latex], displaying the results
  in a better readable format
- `result.pdf`: a presentation of all measurements

All of the 4 above mentioned files can be generated seperately by calling `make FILE`.
A rule of thumb for the memory usage is that the compressors will need 12 times input
size or less.

### Replicating Computational Results
The most straightforward way to use this benchmark is by just calling

	make rcr

This command will automatically download the test data, sets up the benchmark
properly (Warning: the .config - files will be overwritten), execute the
benchmark and generate all resulting files. Your machine should contain 16 GB
of memory to ensure no swapping takes place.
