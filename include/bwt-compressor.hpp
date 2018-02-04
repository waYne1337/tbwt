/*
 * bwt-compressor.hpp for bwt tunneling
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

#ifndef _BWT_COMPRESSOR_HPP
#define _BWT_COMPRESSOR_HPP

#include "block-compressor.hpp"
#include "bwt-config.hpp"
#include "divsufsort.h"

#include <assert.h>
#include <chrono>
#include <ios>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <type_traits>
#include <utility>

//! a bwt-based compressor with second stage transform as defined in t_2st_encoder
template<class t_2st_encoder>
class bwt_compressor : public block_compressor {
	public:
		//! constructor
		bwt_compressor() : block_compressor( t_max_size ) {};
	protected:
		virtual void compress_block( std::istream &in, std::streampos end, std::ostream &out ) const;
		virtual void decompress_block( std::istream &in, std::streampos end, std::ostream &out ) const;
};

//// COMPRESSION //////////////////////////////////////////////////////////////

template<class t_ss_e>
void bwt_compressor<t_ss_e>::compress_block( std::istream &in, std::streampos end, std::ostream &out ) const {
	using namespace std;
	using namespace std::chrono;
	typedef typename istream::char_type schar_t;
	typedef high_resolution_clock timer;
	static_assert( is_same<
	                    typename make_unsigned<schar_t>::type,
	                    typename make_unsigned<t_uchar_t>::type
	               >::value,
	               "character types must be compatible" );

	//// GET INPUT ////////////////////////////////////////////////////////

	//get length of input
	t_size_t n = (t_size_t)(end - in.tellg());
	assert(n <= t_max_size );
	print_info("input size", n);

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
	print_info("bwt construction time", (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>( stop - start ).count() );

	//// WRITE HEADER AND ENCODING TO STREAM //////////////////////////////
	start = timer::now();

	write_primitive<t_size_t>( S.size(), out );
	write_primitive<t_idx_t>( bwt_idx , out );
	auto bwencstartpos = out.tellp();
	t_ss_e::encode( S, out );	

	stop = timer::now();
	print_info("encoding time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );
	print_info("bwt encoding size", out.tellp() - bwencstartpos );
}

//// DECOMPRESSION ////////////////////////////////////////////////////////////

template<class t_ss_e>
void bwt_compressor<t_ss_e>::decompress_block( std::istream &in, std::streampos end, std::ostream &out ) const {
	using namespace std;
	using namespace std::chrono;
	typedef typename ostream::char_type schar_t;
	typedef high_resolution_clock timer;
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
	if (n != 0 && (bwt_idx >= n || bwt_idx == 0)) {
		throw invalid_argument("invalid bwt index");
	}

	//set up string for result (required to invert BWT)
	t_string_t S( n );
	t_ss_e::decode( in, S );

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

#endif
