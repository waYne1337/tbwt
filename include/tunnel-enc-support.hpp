/*
 * tunneling-enc-support.hpp for bwt tunneling
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

#ifndef _TUNNEL_ENC_SUPPORT_HPP
#define _TUNNEL_ENC_SUPPORT_HPP

#include "bwt-config.hpp"
#include "bwt-run-support.hpp"
#include "twobitvector.hpp"

#include <istream>
#include <ostream>
#include <utility>
#include <vector>

//! support structure for tunnel encoding.
/*! structure offers methods to estimate the overall encoding size
   of a tunneled BWT, as well as information about benefits when
   tunneling a specific block.
*/
class tunnel_enc_support {
	private:
		std::vector<t_size_t> width; //width of each block
		std::vector<t_size_t> brc;   //score for each block (removed characters of block)

		//variables for entropy measurements
		t_size_t runs;
		t_size_t runsn1; //number of runs with height  > 1 in BWT
		t_size_t rc; //overall amount of run characters in BWT
		t_size_t tc; //amount of run characters removed by symbolic tunneling
		t_size_t t;  //number of currently used tunnels	

		//variables for the average bit size of a single run character
		// as bit size normally is no natural number, it's presented as a fraction here
		t_bitsize_t avg_rlc_bits_numer; //numerator
		//denominator is a constant and needs not to be listed here

	public:
		//! initializes the upper variables, EXCEPT FOR width and brc
		void init_enc_information( const bwt_run_support &bwtrs );

		//! adds a column to the given block, thus encreasing its score
		void add_block_column( const bwt_run_support &bwtrs, t_idx_t b, t_idx_t b_col );

		//! returns a score for each block. The higher the score, the more it's
		//  worth to tunnel the block.
		t_bitsize_t blockscore( t_idx_t b ) const;

		//! function symbolically tunnels block b, what changes the overall encoding size.
		void tunnel_block_symbolic( t_idx_t b );

		//! reduces the score of the inner block ic_b caused by collision with outer block b.
		void reduce_score_of_inner_block( t_idx_t b, t_idx_t ic_b );

		//! reduces the score of the outer block oc_b caused by collision with inner block b.
		void reduce_score_of_outer_block( t_idx_t b, t_idx_t oc_b );

		//! returns the size (in bits) required to encode the current tunneled BWT (aux included).
		t_bitsize_t current_rlc_encoding_size() const;

		//! returns the size (in bits) necessary to encode the auxiliary data structure needed for tunneling.
		t_bitsize_t current_aux_enc_size() const;

		//! encodes a tunneled bwt such that the given approximations are achieved.
		/*! returns a pair containing the the sizes required to encode the tbwt and the auxiliary data structure.
		*/
		static std::pair<std::streamsize,std::streamsize> encode_tunneled_bwt(
		                                                       t_string_t &&tbwt, t_idx_t tbwt_idx,
		                                                       twobitvector &&aux, std::ostream &out );

		//! decodes a tunneled bwt. 
		/*! function resizes tbwt and aux to correct sizes, and returns the tbwt_idx.
		   Function throws invalid_argument exception if instream represents no proper
		   encoding, or throws any of the underlying stream exceptions.
		*/
		static t_idx_t decode_tunneled_bwt( t_string_t &tbwt, twobitvector &aux,
		                                    std::istream &in, std::streampos end,
		                                    t_size_t max_tbwt_size = t_max_size );
};

#endif
