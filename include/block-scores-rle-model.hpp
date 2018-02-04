/*
 * block-scores-rle-model.hpp for bwt tunneling
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

#ifndef BLOCK_SCORES_RLE_MODEL_HPP
#define BLOCK_SCORES_RLE_MODEL_HPP

#include "aux-encoding.hpp"
#include "bwt-config.hpp"
#include "bwt-run-support.hpp"
#include "twobitvector.hpp"

#include <algorithm>
#include <array>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <utility>
#include <vector>

#include <iostream> //TODO

//! class managing block scores for bwt-compressors using run-length encoding.
class block_scores_rle_model {
private:
	std::vector<t_size_t> width; //width of each block
	std::vector<t_size_t> brc;   //score for each block (removed characters of block)

	//variables for entropy measurements
	t_size_t n;
	t_size_t runs;
	t_size_t runsn1; //number of runs with height  > 1 in BWT
	t_size_t rc; //overall amount of run characters in BWT
	t_size_t tc; //amount of run characters removed by symbolic tunneling
	t_size_t t;  //number of currently used tunnels	

	//constant needed for logarithm computations
	double ln_2; //natural logarithm of 2, i.e. ln(2)

	//returns position of highest set bit in x, starting at zero [undefined value if x is zero]
	inline t_size_t hibit(t_size_t x) const {
		//TODO: offer an alternative if __builtin_clz is not available
		return std::numeric_limits<t_size_t>::digits - __builtin_clz(x) - 1;
	}
public:
	//! initializes the upper variables, EXCEPT FOR width and brc
	void init_enc_information( const bwt_run_support &bwtrs ) {
		runs = bwtrs.runs;
		runsn1 = 0;
		tc = 0;
		t = 0;
		ln_2 = log1p(1);

		//compute the number of run characters
		rc = 0;
		for (t_idx_t i = 0; i < runs; i++) {
			if (bwtrs.height(i) > 1u) {
				++runsn1;
				rc += hibit(bwtrs.height(i));
			}
		}
		n = rc + runs; //length of rlencoded BWT

		//create width and brc, each initialized to zero
		width.resize( runs );
		brc.resize( runs );
	}

	//! adds a column to the given block, thus encreasing its score
	void add_block_column( const bwt_run_support &bwtrs, t_idx_t b, t_idx_t b_col ) {
		width[b]++;
		brc[b] += hibit( bwtrs.height(b_col) ) - hibit( bwtrs.height(b_col) - bwtrs.height(b) + 1 );
	}

	//! returns a score for each block. The higher the score, the more it's
	//  worth to tunnel the block.
	t_bitsize_t blockscore( t_idx_t b ) const {
		return (brc[b] - brc[b] * 2 / width[b]);      //brc[b] - symbols in first and last column
	}

	//! function symbolically tunnels block b, what changes the overall encoding size.
	void tunnel_block_symbolic( t_idx_t b ) {
		t++;
		tc += brc[b] - brc[b]*2/width[b];
	}

	//! reduces the score of the inner block ic_b caused by collision with outer block b.
	void reduce_score_of_inner_block( t_idx_t b, t_idx_t ic_b ) {
	
		//amount of information removed by block b on the shortened width of ic_b
		auto ic_ocb_rc = brc[b] * width[ic_b] / width[b]; 

		//height-reduce inner block
		brc[ic_b] = (brc[ic_b] > 0u && ic_ocb_rc > brc[ic_b])
			  ?  ic_ocb_rc - brc[ic_b] //remaining information
			  :  0; //clear blockscore, not enough benefit
	}

	//! reduces the score of the outer block oc_b caused by collision with inner block b.
	void reduce_score_of_outer_block( t_idx_t b, t_idx_t oc_b ) {
		//weight-reduce inner block
		auto oc_b_nw = width[oc_b] - width[b]; //new width of block oc_b

		//linearly reduce width
		brc[oc_b] = brc[oc_b] * oc_b_nw / width[oc_b];

		//adapt weight of block
		width[oc_b] = oc_b_nw;
	}

	//! returns the estimated size benefit (in bits) current tbwt (without aux).
	t_bitsize_t current_tbwt_gross_benefit() const {
		// n * H(rlencode(BWT)) − (n − tc) * H(rlencode(TBWT))
		if (t == 0)	return 0;

		return (t_bitsize_t)(
			(
				  n *  (log1p( tc / (double)(n - tc) ))
				- rc * (log1p( tc / (double)(rc - tc) ))
				+ tc * (log1p( runs / (double)(rc - tc) ) + ln_2)
			) / ln_2);
	}

	//! returns the estimated size (in bits) to encode the current auxiliary data structure.
	t_bitsize_t current_aux_tax() const {
		// |rlencode(aux)| * H(rlencode(aux))
		if (t == 0)	return 0;
		t_size_t h = std::max( (t_size_t)2, (t_size_t)( hibit( runsn1 - 2*t ) - hibit( 2*t ) ) );
		return (t_bitsize_t)(
			(
				  2 * t     * (log( h*h - 1 ) + ln_2)
				+ 2 * t * h * (log1p( 2 / (double)( h - 1 ) ))
			) / ln_2);
	}

	//// AUX TRANSFORMATION ///////////////////////////////////////////////////////////

	//! transforms an auxiliary structure to be run-based
	static void transform_aux( const t_string_t &tbwt, t_idx_t tbwt_idx, twobitvector &aux ) {
		if (tbwt.size() == 0)	return;
		assert( tbwt_idx > 0u && tbwt_idx < tbwt.size() ); //a valid index must be greater zero

		//transfer aux to a run-based representation
		t_idx_t j = 0;
		std::array<t_idx_t,3> bounds = {(t_idx_t)0u,
			                   tbwt_idx,
			                   (t_idx_t)tbwt.size()}; //don't forget primary index run
		for (t_idx_t ib = 0; ib != 2; ib++) {
			t_idx_t i = bounds[ib];
			t_idx_t e = bounds[ib+1];

			bool newrun = true;
			while (++i < e) {
				if (tbwt[i] != tbwt[i-1]) { //new run detected
					newrun = true;
				}
				else if (newrun) { //first run-character of new run
					aux[j++] = aux[i]; //copy aux-value of runs with height > 1
					newrun = false;
				}
			}
		}
		aux.resize( j );
	}

	//! retransforms a run-based auxiliary structure to its original structure
	static void retransform_aux( const t_string_t &tbwt, t_idx_t tbwt_idx, twobitvector &aux ) {
		if (tbwt.size() == 0)	return;
		assert( tbwt_idx > 0u && tbwt_idx < tbwt.size() ); //a valid index must be greater zero

		//decode run-based aux
		std::array<t_idx_t,3> bounds = {
			(t_idx_t)tbwt.size(),
			tbwt_idx,
			(t_idx_t)0u }; //don't forget primary index run

		t_idx_t j = aux.size();
		aux.resize( tbwt.size() + 1 );
		aux[tbwt.size()] = aux_encoding::REG;

		for (t_idx_t ib = 0; ib != 2; ib++) {
			t_idx_t i = bounds[ib];
			t_idx_t e = bounds[ib+1];

			bool newrun = true;
			while (--i > e) {
				if (tbwt[i] != tbwt[i-1]) { //start of a run
					aux[i] = aux_encoding::REG;
					newrun = true;
				}
				else {
					if (newrun) {
						if (j-- == 0u) throw std::invalid_argument("invalid aux encoding");
						newrun = false;
					}
					aux[i] = aux[j];
				}
			}
			aux[i] = aux_encoding::REG; //set flag for start of last run
		}
	}
};

#endif
