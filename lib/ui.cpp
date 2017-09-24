/*
 * ui.cpp for bwt tunneling
 * Copyright (c) 2017 Uwe Baier All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string.h>

#if defined BWT
	#include "bwt-compressor.hpp"
	#define FILESUFFIX ".bwz"
	#define COMPRESSOR bwt_compressor
#elif defined TBWT
	#include "tbwt-compressor.hpp"
	#define FILESUFFIX ".tbwz"
	#define COMPRESSOR tbwt_compressor
#else
	#error unknown block compressor
#endif

using namespace std;

const int MODE_COMPRESS = 0;
const int MODE_DECOMPRESS = 1;

void printUsage(const char *cmd) {
	cerr << "usage: " << cmd << " MODE [INFO] INFILE [OUTFILE]" << endl;
	cerr << "\tMODE: -c (compress) or -d (decompress)" << endl;
	cerr << "\tINFO: -i for extra information about compression, nothing otherwise" << endl;
	cerr << "\tINFILE: if compress mode, file to be compressed" << endl;
	cerr << "\t        if decompress mode, file to be decompressed" << endl;
	cerr << "\tOUTFILE: if compress mode, path to resulting compressed file" << endl;
	cerr << "\t         if decompress mode, path to file to be decompressed" << endl;
}

int main( int argc, char **argv ) {
	//analyse args
	string infile;
	string outfile;
	bool quiet = true;
	int mode = -1;

	for (int i = 1; i < argc-1; i++) {
		if (strcmp(argv[i], "-c") == 0) { //compress mode
			if (mode != -1) {
				printUsage(argv[0]);
				cerr << "Mode defined already!" << endl;
				return 1;
			}
			mode = MODE_COMPRESS;
		}
		else if (strcmp(argv[i], "-d") == 0) { //decompress mode
			if (mode != -1) {
				printUsage(argv[0]);
				cerr << "Mode defined already!" << endl;
				return 1;
			}
			mode = MODE_DECOMPRESS;
		}
		else if (strcmp(argv[i], "-i") == 0) { //information mode
			quiet = false;
		}
		else {
			if (!infile.empty()) {
				printUsage( argv[0] );
				return 1;
			}
			infile = argv[i];
		}
	}
	if (mode == -1) {
		printUsage(argv[0]);
		cerr << "Missing mode!" << endl;
		return 1;
	}
	if (infile.empty()) { //check if outfile is defined
		infile = argv[argc-1];
		outfile = (mode == MODE_COMPRESS)
		        ? infile + FILESUFFIX
		        : infile.substr(0, infile.find_last_of(FILESUFFIX));
	} else {
		outfile = argv[argc-1];
	}

	//open streams for infile and outfile
	ifstream fin{ infile };
	ofstream fout{ outfile, ofstream::out | ofstream::trunc };
	if (!fin) {
		printUsage(argv[0]);
		cerr << "unable to open file \"" << infile << "\"" << endl;
		return 1;
	} else if (!fout) {
		printUsage(argv[0]);
		cerr << "unable to open file \"" << outfile << "\"" << endl;
		return 1;
	}

	//compress or decompress, depending on mode
	COMPRESSOR compressor;
	compressor.set_quiet(quiet);
	try {
		switch (mode) {
		case MODE_COMPRESS:
			compressor.compress( fin, fout );
			break;
		case MODE_DECOMPRESS:
			compressor.decompress( fin, fout );
			break;
		default:
			throw logic_error("Internal fault");
		}
	} catch ( invalid_argument &e ) {
		printUsage( argv[0] );
		cerr << "Invalid argument: " << e.what() << endl;
		return 1;
	} catch ( runtime_error &e ) {
		printUsage( argv[0] );
		cerr << "Runtime error: " << e.what() << endl;
		return 1;
	} catch ( exception &e ) {
		cerr << "Exception: " << e.what() << endl;
		return 1;
	}
	return 0;	
}
