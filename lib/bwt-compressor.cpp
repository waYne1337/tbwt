/*
 * bwt-compressor.cpp for bwt tunneling
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

#include "bwt-compressor.hpp"

#include "bwt-config.hpp"
#include "divsufsort.h"
#include "mtf-coder.hpp"
#include "rle0-coder.hpp"
#include "entropy-coder.hpp"

#include <assert.h>
#include <chrono>
#include <ios>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <type_traits>
#include <utility>

using namespace std;
using namespace std::chrono;

typedef high_resolution_clock timer;

bwt_compressor::bwt_compressor()
	: block_compressor( t_max_size ) {};

void bwt_compressor::compress_block( istream &in, streampos end, ostream &out ) const {
	typedef typename istream::char_type schar_t;
	static_assert( is_same<
	                    typename make_unsigned<schar_t>::type,
	                    typename make_unsigned<t_uchar_t>::type
	               >::value,
	               "character types must be compatible" );

	//// GET INPUT ////////////////////////////////////////////////////////

	//get length of input
	t_size_t n = (t_size_t)(end - in.tellg());
	assert(n <= t_max_size );
	print_info("block size", n );

	//read string from input
	t_string_t S( n );
	in.read( (schar_t *)S.data(), n );

	//// BW-TRANSFORM INPUT ///////////////////////////////////////////////

	auto start = timer::now();
	saidx_t bwt_idx = 0;
	if (bw_transform(S.data(), S.data(), NULL, (saidx_t)n, &bwt_idx) < 0) {
		throw runtime_error( string("BW Transformation failed") );
	}
	auto stop = timer::now();
	print_info("bwt construction time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );

	//// WRITE HEADER TO STREAM ///////////////////////////////////////////
	auto outstartpos = out.tellp();
	start = timer::now();

	write_primitive<t_size_t>( S.size(), out );
	write_primitive<t_idx_t>( bwt_idx , out );
	//write alphabet
	auto alph = mtf_coder<t_string_t>::compute_alph( S );
	out.put( (t_uchar_t)alph.size() ); //store alphabet size (note that this stores 0 if full alphabet is used)
	for (t_idx_t i = 0; i < alph.size(); i++) { //and the alphabet itself
		out.put( alph[i] );
	}

	//// PERFORM AND WRITE MTF+RLE0+ENTROPY ///////////////////////////////

	//prepare encoders
	mtf_coder<t_string_t> mtfcoder( alph );
	rle0_encoder<t_string_t> rle0coder;
	entropy_encoder<ostream> entcoder( out );
	entcoder.reset( alph.size() + 1 );

	for (t_idx_t i = 0; i < S.size(); ) { //do encoding
		do {
			if (i >= S.size())	break;
			//feed rle0-encoder with mtf coded input until some contents can be written
		} while (rle0coder.encode_char( mtfcoder.encode_char( S[i++] ) ));

		//move the output of the rle0coder to the entropy coder
		while (rle0coder.has_next_enc_char()) {
			entcoder.encode_char( rle0coder.next_enc_char() );
		}
	}
	entcoder.flush();
	stop = timer::now();
	print_info("encoding time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );
	print_info("encoding size", out.tellp() - outstartpos );
}

void bwt_compressor::decompress_block( istream &in, streampos end, ostream &out ) const {
	typedef typename ostream::char_type schar_t;
	static_assert( is_same<
	                    typename make_unsigned<schar_t>::type,
	                    typename make_unsigned<t_uchar_t>::type
	               >::value,
	               "character types must be compatible" );

	//// READ INPUT ///////////////////////////////////////////////////////

	auto start = timer::now();
	//read header
	auto n       = read_primitive<t_size_t>( in );
	auto bwt_idx = read_primitive<t_idx_t>( in );
	if (n > t_max_size) {
		throw invalid_argument("text(part) is too long to be decoded!");
	}
	if (n != 0 && bwt_idx >= n) {
		throw invalid_argument("bwt index must be smaller than text size");
	}
	print_info("block size", n);
	//read alphabet
	t_uchar_t alphsize = in.get();
	t_string_t alph( (alphsize > 0u) 
	                 ? alphsize
	                 : numeric_limits<t_uchar_t>::max()+1 ); //remember that on full alphabet 0 is stored
	for (t_idx_t i = 0; i < alph.size(); i++) {
		alph[i] = in.get();
	}

	//// READ AND INVERT MTF+RLE0+ENTROPY /////////////////////////////////
	
	//set up string for result (required to invert BWT)
	t_string_t S( n );

	//set up required decodes
	mtf_coder<t_string_t> mtfcoder( alph );
	rle0_decoder<t_string_t> rle0coder;
	entropy_decoder<istream> entcoder( in, end );
	entcoder.reset( alph.size() + 1 );

	//do decoding
	for (t_idx_t i = 0; i < S.size(); ) {
		//feed rle0-decoder with input
		rle0coder.decode_char( entcoder.decode_char() );

		//fetch characters from rle0-decoder and invert mtf
		while (i < S.size() && rle0coder.has_next_char()) {
			S[i++] = mtfcoder.decode_char( rle0coder.next_char() );
		}
	}
	if (rle0coder.has_next_char()) {
		throw invalid_argument("encoded rle0-sequence is longer than text length");
	}

	auto stop = timer::now();
	print_info("decoding time", (uint64_t)duration_cast<milliseconds>( stop - start ).count());

	//// INVERT BWT ///////////////////////////////////////////////////////

	start = timer::now();
	if (inverse_bw_transform(S.data(), S.data(), NULL,
	                         (saidx_t)S.size(), (saidx_t)bwt_idx) < 0) {
		throw invalid_argument( "Inverse BW Transformation failed" );		
	}
	stop = timer::now();
	print_info("bwt inversion time", (uint64_t)duration_cast<milliseconds>( stop - start ).count());

	//// WRITE S TO OUTPUTSTREAM //////////////////////////////////////////
	out.write( (const schar_t *)S.data(), S.size() );
}
