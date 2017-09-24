/*
 * tbwt-compressor.cpp for bwt tunneling
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

#include "tbwt-compressor.hpp"

#include "aux-encoding.hpp"
#include "bwt-config.hpp"
#include "bwt-run-support.hpp"
#include "divsufsort.h"
#include "lheap.hpp"
#include "tunneling-support.hpp"

#include <array>
#include <assert.h>
#include <chrono>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace std::chrono;

typedef high_resolution_clock timer;

//a mapper between block states and heap states
struct _bs_lhvs_mapper {
	array<int,aux_encoding::SIGMA> map;
	_bs_lhvs_mapper() {
		map[tunneling_support::UNCHANGED] = lheap_vstate::unchanged;
		map[tunneling_support::DECREASED] = lheap_vstate::decreased;
		map[tunneling_support::CLEARED  ] = lheap_vstate::empty;
	};
} bs_lhvs_mapper;

//implementations
tbwt_compressor::tbwt_compressor()
	: block_compressor( t_max_size ) {};


void tbwt_compressor::compress_block( istream &in, streampos end, ostream &out ) const {
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
	print_info("bwt index", bwt_idx);
	print_info("bwt construction time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );

	//// SET UP BWT NAVIGATION ////////////////////////////////////////////

	start = timer::now();
	bwt_run_support bwtrs( S.data(), n, bwt_idx );

	//// COMPUTE BLOCKS AND COLLISIONS ////////////////////////////////////

	tunneling_support ts( bwtrs );
	stop = timer::now();
	print_info("block computation time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );

	//// SET UP A HEAP CONTAINING BLOCKS //////////////////////////////////

	start = timer::now();

	vector<t_idx_t> H; //heap space
	H.reserve( ts.bns.blocks ); //put all blocks to a heap which are worth to be tunneled
	for (t_idx_t b = 0; b < ts.bns.blocks; b++) {
		if (ts.blockstate(b) != tunneling_support::CLEARED) {
			H.push_back( b );
		}
	}
	//create function for block comparison
	auto blockcmp = [&ts]( t_idx_t b1, t_idx_t b2 ) {
		return ts.tes.blockscore(b1) < ts.tes.blockscore(b2);
	};
	//create function for block score state
	auto blockstate = [&ts]( t_idx_t b ) {
		return bs_lhvs_mapper.map[ts.blockstate(b)];
	};
	//create the initial heap
	make_lheap( H.begin(), H.end(), blockcmp );

	//// SEARCH FOR AN OPTIMAL BLOCK CHOICE ///////////////////////////////

	auto     min_rlc_enc_size = ts.tes.current_rlc_encoding_size(); //best choice of tunnels for input
	auto     normal_rlc_enc_size = min_rlc_enc_size;
	t_size_t min_rlc_tunnel_cnt = 0; //number of tunnels under current minimal encoding size
	auto     min_rlc_aux_size = ts.tes.current_aux_enc_size();
	auto SB = H.rbegin(); //array to store sorted blocks

	for (auto e = H.end(); e != H.begin(); ) {

		auto b = H.front(); //get block with maximal score
		ts.tunnel_block_symbolic( b ); //symbolically tunnel block with best score

		//remove block from heap, and store it in SB (similar to heapsort)
		e = pop_lheap_nomove(H.begin(), e, blockstate, blockcmp);
		*(SB++) = b;

		//check if new encoding is smaller
		auto new_rlc_enc_size = ts.tes.current_rlc_encoding_size();
		if (new_rlc_enc_size <= min_rlc_enc_size) {
			min_rlc_enc_size = new_rlc_enc_size;
			min_rlc_aux_size = ts.tes.current_aux_enc_size();
			min_rlc_tunnel_cnt = distance( H.rbegin(), SB );
		}
	}

	stop = timer::now();
	print_info("block choice time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );
	print_info("block count", H.size());
	print_info("tunneled block count", min_rlc_tunnel_cnt);
	print_info("expected size benefit", (normal_rlc_enc_size - min_rlc_enc_size)
	            / numeric_limits<make_unsigned<schar_t>::type>::digits );
	print_info("expected aux size", min_rlc_aux_size
	            / numeric_limits<make_unsigned<schar_t>::type>::digits );
		
	//// TUNNEL BEST CHOICE OF BLOCKS /////////////////////////////////////

	start = timer::now();
	twobitvector aux; //auxiliary structure for tunneling
	auto tbwt_idx = ts.tunnel_bwt( S, aux, H.rbegin(), (H.rbegin()+min_rlc_tunnel_cnt) );
	move( ts ); move( bwtrs ); move( H ); //get rid of some structures
	stop = timer::now();
	print_info("tunneling time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );	

	//// WRITE HEADER /////////////////////////////////////////////////////

	start = timer::now();
	write_primitive<t_size_t>( n, out );	

	//// ENCODE TUNNELED BWT USING ENCODING SUPPORT ///////////////////////

	auto p = tunnel_enc_support::encode_tunneled_bwt( move(S), tbwt_idx, move(aux), out );
	stop = timer::now();
	print_info("encoding time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );
	print_info("tbwt size", p.first );
	print_info("aux size", p.second );
}

void tbwt_compressor::decompress_block( istream &in, streampos end, ostream &out ) const {
	typedef typename ostream::char_type schar_t;
	static_assert( is_same<
	                    typename make_unsigned<schar_t>::type,
	                    typename make_unsigned<t_uchar_t>::type
	               >::value,
	               "character types must be compatible" );

	//// READ HEADER //////////////////////////////////////////////////////

	auto start = timer::now();
	t_size_t n = read_primitive<t_size_t>( in );
	print_info("block size", n);

	//// DECODE TUNNELED BWT USING ENCODING SUPPORT ///////////////////////                                       

	t_string_t tbwt;
	twobitvector aux;
	auto tbwt_idx = tunnel_enc_support::decode_tunneled_bwt( tbwt, aux, in, end, n );
	auto stop = timer::now();
	print_info("decoding time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );

	//// INVERT TUNNELED BWT //////////////////////////////////////////////

	start = timer::now();
	t_string_t S( n );
	tunneling_support::invert_tunneled_bwt( move(tbwt), move(aux), tbwt_idx,
	                                        S, numeric_limits<t_uchar_t>::max() );
	stop = timer::now();
	print_info("tbwt inversion time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );

	//// WRITE S TO OUTPUTSTREAM //////////////////////////////////////////
	out.write( (const schar_t *)S.data(), n );
}
