/*
 * tunneling-support.hpp for bwt tunneling
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

#ifndef _TUNNELING_SUPPORT_HPP
#define _TUNNELING_SUPPORT_HPP

#include <stack>
#include <stdexcept>
#include <vector>

#include "aux-encoding.hpp"
#include "block-nav-support.hpp"
#include "bwt-config.hpp"
#include "bwt-run-support.hpp"
#include "tunnel-enc-support.hpp"
#include "twobitvector.hpp"

//! support structure for bwt tunneling.
/*! structure offers methods to symbolically tunnel the BWT to see benefits
   (which means that the BWT is not tunneled), as well as methods for tunneling
   and inverting the tunneled BWT directly to the original string.
*/
class tunneling_support {
	private:
		const bwt_run_support &bwtrs;
		twobitvector bstate; //state of each block, see lower constants
		block_nav_support m_bns;
		tunnel_enc_support m_tes;

		void init_support_structures();
	public:
		//! navigation support for blocks
		const block_nav_support &bns = m_bns;

		//! score and encoding support for blocks
		const tunnel_enc_support &tes = m_tes;

		//! constructor
		tunneling_support( const bwt_run_support &bwsupport ) : bwtrs{ bwsupport },
				m_bns{ bwtrs } {
			init_support_structures();
		};

		//! indicates that the block score did not change
		const static t_idx_t UNCHANGED = 0;

		//! indicates that the block score was decreased
		const static t_idx_t DECREASED = 1;

		//! indicates that the block score was set to zero, i.e.
		//  the block can not be used for tunneling
		const static t_idx_t CLEARED = 2;

		//! returns whether the block score of the corresponding block changed since the last block score query.
		/*! Note that calling this function changes the state of the blockscore to "unchanged".
		*/
		t_idx_t blockstate( t_idx_t b );

		//! function symbolically tunnels block b.
		/*! Symbolically tunneling means that scores of
		    colliding blocks are reduced, and the information required to 
		    encode all run characters is updated.
		*/
		void tunnel_block_symbolic( t_idx_t b );

//// REGION FOR TUNNELING ITSELF //////////////////////////////////////////////

	public:
		//! tunnels the given BWT using the blocks given by the identifiers placed between first and last.
		/*! Note that this object modified by this operation,
		    and therefore shouldn't be used afterwards. aux should be an empty vector which is filled
		    by this function. Function returns the new primary index of the tunneled bwt.
		    Note that aux is one character longer than
		    the tunneled BWT, because it always is terminated by aux_encoding::REG.
		*/
		template<class Iterator>
		t_idx_t tunnel_bwt( t_string_t &bwt, twobitvector &aux,
		                 Iterator first, Iterator last );

		//! inverts the given tunneled bwt.
		/*! This means that this function not only recomputes
		    the original BWT; it instead rebuilds the original text from which the BWT was
		    created from. S must have same size as that of the original text, additionally the
		    primary index of the tunneled bwt as well as the maximal character value are required.
		*/
		static void invert_tunneled_bwt( const t_string_t &tbwt, const twobitvector &aux,
		                                 t_idx_t tbwt_idx, t_string_t &S, t_size_t maxalphval );
};

template<class Iterator>
t_idx_t tunneling_support::tunnel_bwt( t_string_t &bwt, twobitvector &aux, Iterator first, Iterator last ) {

	//resize auxiliary bit vector to cover enough space
	aux.resize( bwtrs.idx_n+1 );			

	//mark each tunnel in auxiliary structure
	std::vector<t_idx_t> intervals;
	while (first != last) {
		auto b = *(first++);

		//save which rows of block were not tunneled yet
		intervals.clear();
		auto lastaux = aux_encoding::REM;
		for (t_idx_t i = bwtrs.log_to_idx(bwtrs.start(b)+1);
		             i < bwtrs.log_to_idx(bwtrs.end(b)); i++) {
			if (aux[i] != lastaux) {
				lastaux = aux[i];
				intervals.push_back( i - bwtrs.log_to_idx(bwtrs.start(b)) );
			}
		}
		intervals.push_back( bwtrs.height(b) );

		//prepare marking in aux
		auto cur = bwtrs.run_lf(b);    //current run
		auto last = bwtrs.start(b); //previous run (seen in text order)
		while (cur != m_bns.end[b]) {
			//clear cntL for all intervals in previous run
			for (t_idx_t i = 1; i < intervals.size(); i += 2 ) {
				t_idx_t i_s = bwtrs.log_to_idx(last + intervals[i-1]); //interval start
				t_idx_t i_e = bwtrs.log_to_idx(last + intervals[i]  ); //interval end
				do {
					aux[i_s] = aux[i_s] | aux_encoding::IGN_L;
				} while (++i_s < i_e);
			}

			//move on cur and last by 1 column
			last = cur;
			t_idx_t cur_r = bwtrs.run_of( cur );
			cur = (aux[bwtrs.log_to_idx( cur + intervals.front() )] == aux_encoding::IGN_L) //block of run cur_r was tunneled already
			    ? m_bns.end[cur_r]    + (cur - bwtrs.start(cur_r))  //jump over block if tunneled already
			    : bwtrs.run_lf(cur_r) + (cur - bwtrs.start(cur_r)); //otherwise proceed stepwise

			//clear cntF for all intervals in current run (before both pointers were moved)
			for (t_idx_t i = 1; i < intervals.size(); i += 2 ) {
				t_idx_t i_s = bwtrs.log_to_idx(last + intervals[i-1]);
				t_idx_t i_e = bwtrs.log_to_idx(last + intervals[i]  );
				do {
					aux[i_s] = aux[i_s] | aux_encoding::SKP_F;
				} while (++i_s < i_e);
			}
		}
		//set end of block b one position to right (i.e. one application of inverse LF),
		// such that block jumping as shown above works correct
		m_bns.set_end(b, last);
	}

	//remove doubly marked entries from BWT and auxiliary structure
	t_idx_t bwt_idx;
	t_idx_t borders[] = { bwtrs.bwt_idx, bwtrs.idx_n };
	t_idx_t p = 0; //position in bwt and auxiliary data structure
	t_idx_t i = 0; //position in original bwt
	for (auto b : borders) {
		bwt_idx = p; //also, compute new position of primary index
		while (i < b) {
			if (aux[i] != aux_encoding::REM) { //copy entries which won't be removed
				bwt[p] = bwt[i];
				aux[p++] = aux[i];
			}
			++i;
		}
	}	
	//trim both bwt and aux to correct sizes and add a terminator to aux
	bwt.resize( p );
	aux[p++] = aux_encoding::REG;	aux.resize( p );
	return bwt_idx;
}

#endif
