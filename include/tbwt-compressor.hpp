/*
 * tbwt-compressor.hpp for bwt tunneling
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

#ifndef _TBWT_COMPRESSOR_HPP
#define _TBWT_COMPRESSOR_HPP

#include "block-compressor.hpp"

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

//! a tunneled-bwt-based generic compressor
template<class t_2st_encoder>
class tbwt_compressor : public block_compressor {
	protected:
		virtual void compress_block( std::istream &in, std::streampos end, std::ostream &out ) const;
		virtual void decompress_block( std::istream &in, std::streampos end, std::ostream &out ) const;
	public:
		//! constructor
		tbwt_compressor() : block_compressor( t_max_size ) {};
};

//// COMPRESSION //////////////////////////////////////////////////////////////

template<class t_ss_e>
void tbwt_compressor<t_ss_e>::compress_block( std::istream &in, std::streampos end, std::ostream &out ) const {
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

	//read string from input and reverse it
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

	//// SET UP BWT NAVIGATION ////////////////////////////////////////////

	start = timer::now();
	bwt_run_support bwtrs( S.data(), n, bwt_idx );

	//// COMPUTE BLOCKS AND COLLISIONS ////////////////////////////////////

	tunneling_support<t_ss_e> ts( bwtrs );
	stop = timer::now();
	print_info("block computation time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );

	//// SET UP A HEAP CONTAINING BLOCKS //////////////////////////////////

	//a mapper between block states and heap states
	struct _bs_lhvs_mapper {
		array<int,aux_encoding::SIGMA> map;
		_bs_lhvs_mapper() {
			map[tunneling_support<t_ss_e>::UNCHANGED] = lheap_vstate::unchanged;
			map[tunneling_support<t_ss_e>::DECREASED] = lheap_vstate::decreased;
			map[tunneling_support<t_ss_e>::CLEARED  ] = lheap_vstate::empty;
		};
	} bs_lhvs_mapper;

	start = timer::now();

	vector<t_idx_t> H; //heap space
	H.reserve( ts.bns.blocks ); //put all blocks to a heap which are worth to be tunneled
	for (t_idx_t b = 0; b < ts.bns.blocks; b++) {
		if (ts.blockstate(b) != tunneling_support<t_ss_e>::CLEARED) {
			H.push_back( b );
		}
	}
	//create function for block comparison
	auto blockcmp = [&ts]( t_idx_t b1, t_idx_t b2 ) {
		return ts.tes.blockscore(b1) < ts.tes.blockscore(b2);
	};
	//create function for block score state
	auto blockstate = [&ts,&bs_lhvs_mapper]( t_idx_t b ) {
		return bs_lhvs_mapper.map[ts.blockstate(b)];
	};
	//create the initial heap
	make_lheap( H.begin(), H.end(), blockcmp );

	//// SEARCH FOR AN OPTIMAL BLOCK CHOICE ///////////////////////////////

	auto bc_tbwt_benefit = ts.tes.current_tbwt_gross_benefit(); //best choice tbwt run encoding benefit
	auto bc_aux_tax = ts.tes.current_aux_tax(); //best choice aux size
	t_size_t bc_tunnel_cnt = 0; //number of tunnels under best choice

	auto SB = H.rbegin(); //array to store sorted blocks

	for (auto e = H.end(); e != H.begin(); ) {

		auto b = H.front(); //get block with maximal score
		ts.tunnel_block_symbolic( b ); //symbolically tunnel block with best score

		//remove block from heap, and store it in SB (similar to heapsort)
		e = pop_lheap_nomove(H.begin(), e, blockstate, blockcmp);
		*(SB++) = b;

		//check if new encoding is smaller
		auto current_tbwt_benefit = ts.tes.current_tbwt_gross_benefit();
		auto current_aux_tax = ts.tes.current_aux_tax();
		if (current_tbwt_benefit - bc_tbwt_benefit >=
		    current_aux_tax - bc_aux_tax) {

			bc_tbwt_benefit = current_tbwt_benefit;
			bc_aux_tax = current_aux_tax;
			bc_tunnel_cnt = distance( H.rbegin(), SB );
		}
	}

	stop = timer::now();
	print_info("block choice time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );
	print_info("block count", H.size());
	print_info("tunneled block count", bc_tunnel_cnt);
	print_info("expected tbwt gross benefit size", bc_tbwt_benefit / numeric_limits<t_uchar_t>::digits );
	print_info("expected aux tax size", bc_aux_tax / numeric_limits<t_uchar_t>::digits );

	//// TUNNEL BEST CHOICE OF BLOCKS /////////////////////////////////////

	start = timer::now();
	twobitvector aux; //auxiliary structure for tunneling
	auto tbwt_idx = ts.tunnel_bwt( S, aux, H.rbegin(), (H.rbegin()+bc_tunnel_cnt) );
	move( ts ); move( bwtrs ); move( H ); //get rid of some structures
	stop = timer::now();
	print_info("tunneling time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );	

	//// WRITE ENCODING ///////////////////////////////////////////////////

	start = timer::now();
	t_ss_e::transform_aux( S, tbwt_idx, aux );

	write_primitive<t_size_t>( n, out );
	write_primitive<t_size_t>( S.size(), out );
	write_primitive<t_size_t>( aux.size(), out );
	write_primitive<t_idx_t>(  tbwt_idx, out );

	auto tbwencstartpos = out.tellp();
	t_ss_e::encode( S, out );
	auto auxencstartpos = out.tellp();
	t_ss_e::encode( aux, out );

	stop = timer::now();
	print_info("encoding time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );
	print_info("tbwt encoding size", auxencstartpos - tbwencstartpos );
	print_info("aux encoding size", out.tellp() - auxencstartpos );
}

//// DECOMPRESSION ////////////////////////////////////////////////////////////

template<class t_ss_e>
void tbwt_compressor<t_ss_e>::decompress_block( std::istream &in, std::streampos end, std::ostream &out ) const {
	using namespace std;
	using namespace std::chrono;
	typedef typename ostream::char_type schar_t;
	typedef high_resolution_clock timer;
	static_assert( is_same<
	                    typename make_unsigned<schar_t>::type,
	                    typename make_unsigned<t_uchar_t>::type
	               >::value,
	               "character types must be compatible" );

	//// READ HEADER //////////////////////////////////////////////////////

	auto start = timer::now();
	auto n = read_primitive<t_size_t>( in );
	auto tbwt_size = read_primitive<t_size_t>( in );
	auto aux_size = read_primitive<t_size_t>( in );
	auto tbwt_idx = read_primitive<t_idx_t>( in );
	//do some checks
	if (n > t_max_size) {
		throw invalid_argument("text(part) is too long to be decoded!");
	}
	if (tbwt_size != 0 && (tbwt_idx >= tbwt_size || tbwt_idx == 0)) {
		throw invalid_argument("invalid bwt index");
	}
	if (aux_size > tbwt_size+1) {
		throw invalid_argument("aux size is longer than tbwt size");
	}

	//// DECODE TUNNELED BWT USING ENCODING SUPPORT ///////////////////////                                    

	t_string_t tbwt; tbwt.resize( tbwt_size );
	twobitvector aux; aux.resize( aux_size );
	t_ss_e::decode( in, tbwt );
	t_ss_e::decode( in, aux );

	t_ss_e::retransform_aux( tbwt, tbwt_idx, aux );
	auto stop = timer::now();
	print_info("decoding time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );

	//// INVERT TUNNELED BWT //////////////////////////////////////////////

	start = timer::now();
	tunneling_support<t_ss_e>::invert_tunneled_bwt( move(tbwt), move(aux), n, tbwt_idx,
	                                        numeric_limits<t_uchar_t>::max(), out );
	stop = timer::now();
	print_info("tbwt inversion time", (uint64_t)duration_cast<milliseconds>( stop - start ).count() );
}

#endif
