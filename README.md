# TBWT
This repository contains an implementation and a benchmark for the such-called
Tunneled BWT, which is, a compression improvement for compressors using the 
Burrows-Wheeler Transform like [bzip2]. The tunneled BWT is described in

	On Undetected Redundancy in the Burrows-Wheeler Transform

	by Uwe Baier (hopefully to appear in CPM 2018)

## What is contained
This bundle of files consist of the following parts:
1. The algorithms required to construct, compress and decompress a Tunneled BWT,
   contained in the `include`- and `lib`-directory
2. External resources in the `external`-directory, namely
   - a library for suffix array construction [divsufsort](https://github.com/y-256/libdivsufsort)
   - a library containing different entropy coders [Entropy Coders by Sachin Garg](http://www.sachingarg.com/compression/entropy_coding/64bit)
   - a library containing a bundle of succinct data structures [sdsl-lite](https://github.com/simongog/sdsl-lite)
   - the backend of a high-performance file compressor using the BWT [bcm](https://github.com/encode84/bcm)
3. A benchmark to test the given compressor against common other lossless
   data compressors, see `benchmark` - directory.

## Requirements
To compile the compressor(s), you need a modern c++11 ready compiler such as 
[gcc](https://gcc.gnu.org/) version 4.7 or newer.

## Installation
Just call the command `make`. It should produce six executables:
- `bwzip.x`: a compressor similar to [bzip2], but without memory limitation
- `tbwzip.x`: like `bwzip.x`, enhanced with tunneling
- `bcmzip.x`: a compressor similar to [bcm]
- `tbcmzip.x`: like `bcmzip.x`, enhanced with tunneling
- `wtzip.x`: compression of a BWT using a wavelet tree and compressed bitvectors,
  currently not usable for text indexing
- `twtzip.x`: like `wtzip.x`, enhanced with tunneling

## Usage
Both compiled compressors use the same user interface, just call one of them
without a parameter to get a detailed description.
