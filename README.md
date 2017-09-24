# TBWT
This repository contains an implementation and a benchmark for the such-called
Tunneled BWT, which is, a compression improvement for compressors using the 
Burrows-Wheeler Transform like [bzip2]. The tunneled BWT is described in

	On undetected Redundancy in the Burrows-Wheeler Transform

	by Uwe Baier (hopefully to appear in STACS 2018)

## What is contained
This bundle of files consist of the following parts:
1. The algorithms required to construct, compress and decompress a Tunneled BWT,
   contained in the `include`- and `lib`-directory
2. External resources in the `external`-directory, namely
   - a library for suffix array construction [divsufsort](https://github.com/y-256/libdivsufsort)
   - a library containing different entropy coders [Entropy Coders by Sachin Garg](http://www.sachingarg.com/compression/entropy_coding/64bit)
3. A benchmark to test the given compressor against common other lossless
   data compressors, see `benchmark` - directory.

## Requirements
To compile the compressor(s), you need a modern c++11 ready compiler such as 
[gcc](https://gcc.gnu.org/) version 4.7 or newer.

## Installation
Just call the command `make`. It should produce two executables:
- `bwzip.x`: a compressor similar to [bzip2], but without memory limitation
- `tbwzip.x`: a compressor using the Tunneled BWT

## Usage
Both compiled compressors use the same user interface, just call one of them
without a parameter to get a detailed description.
