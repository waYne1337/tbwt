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

#include <ostream>
#include <stack>
#include <stdexcept>
#include <utility>
#include <vector>

#include "aux-encoding.hpp"
#include "block-nav-support.hpp"
#include "bwt-config.hpp"
#include "bwt-run-support.hpp"
#include "twobitvector.hpp"

//! support structure for bwt tunneling.
/*! structure offers methods to symbolically tunnel the BWT to see benefits
   (which means that the BWT is not tunneled), as well as methods for tunneling
   and inverting the tunneled BWT directly to the original string.
*/
template<class t_tunnel_enc_support>
class tunneling_support {
	private:
		const bwt_run_support &bwtrs;
		twobitvector bstate; //state of each block, see lower constants
		block_nav_support m_bns;
		t_tunnel_enc_support m_tes;

		void init_support_structures();
	public:
		//! navigation support for blocks
		const block_nav_support &bns = m_bns;

		//! score and encoding support for blocks
		const t_tunnel_enc_support &tes = m_tes;

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
		    created from. The original length, primary index of the tunneled bwt as well as the maximal
		    character value are required, characters are written in reverse order to out.
		*/
		static void invert_tunneled_bwt( const t_string_t &tbwt, const twobitvector &aux, t_size_t n, 
		                                 t_idx_t tbwt_idx, t_size_t maxalphval, std::ostream &out );
};

//// CONSTRUCTION /////////////////////////////////////////////////////////////

template<class ttec>
void tunneling_support<ttec>::init_support_structures() {
	bstate.resize( bwtrs.runs );
	m_tes.init_enc_information( bwtrs );
	
	for (t_idx_t b = 0; b < m_bns.blocks; b++) {
		//filter out all blocks with width 1
		if (m_bns.end[b] == bwtrs.run_lf(b)) {
			bstate[b] = tunneling_support::CLEARED;
		} else {
			//iterate over full width of block to set up collisions and score
			t_idx_t i = bwtrs.run_lf(b);
			m_tes.add_block_column( bwtrs, b, b ); //add first column manually

			//offset from last collision block start and b's start at lc block, that is, i-start(lc)
			t_idx_t lc_soffset = bwtrs.n;
			//offset from last collision block end and b's end at lc block, that is, end(lc)-(i+height(b))
			t_idx_t lc_eoffset = bwtrs.n;
			do {
				t_idx_t i_b = bwtrs.run_of(i);
				t_size_t ds = i - bwtrs.start(i_b); //start differences
				m_tes.add_block_column( bwtrs, b, i_b );
				if (m_bns.end[i_b] != bwtrs.run_lf(i_b)) { //i_b has a width more than 1
					t_size_t de = bwtrs.end(i_b) - (i + bwtrs.height(b)); //end differences

					//add a collision if block i_b has no outer collision with the last detected collision
					if (ds < lc_soffset || de < lc_eoffset) {
						lc_soffset = ds;
						lc_eoffset = de;
						m_bns.add_collision( i_b, b );
					}
				}
				i = bwtrs.run_lf(i_b) + ds;
			} while (i != m_bns.end[b]);
		}
	}
}

//// SYMBOLIC TUNNELING ///////////////////////////////////////////////////////

template<class ttec>
t_size_t tunneling_support<ttec>::blockstate( t_idx_t b ) {
	t_size_t s = bstate[b];
	bstate[b] = tunneling_support::UNCHANGED;
	return s;
}

template<class ttec>
void tunneling_support<ttec>::tunnel_block_symbolic( t_idx_t b ) {
	//tunnel block in score aproximations
	m_tes.tunnel_block_symbolic( b );

	//reduce score of colliding blocks and remove collisions
	auto ic_blocks = m_bns.get_inner_collisions( b );

	//reduce block score for each outer colliding block
	for (t_idx_t i = 1; i < ic_blocks.size(); i++) {
		auto ic_b = ic_blocks[i];
		//reduce height of oc_b by height of b
		m_tes.reduce_score_of_inner_block( b, ic_b );
		bstate[ic_b] = tunneling_support::DECREASED;
	}

	auto oc_blocks = m_bns.get_outer_collisions( b );
	//reduce block score for each inner colliding block
	for (t_idx_t i = 1; i < oc_blocks.size(); i++) {
		auto oc_b = oc_blocks[i];
		//reduce weight of ic_b by weight of b
		m_tes.reduce_score_of_outer_block( b, oc_b );
		bstate[oc_b] = tunneling_support::DECREASED;
	}

	//remove collisions between all inner colliding blocks and outer colliding blocks
	m_bns.remove_inner_outer_collisions( b );
}

//// TUNNEL A GIVEN BWT ///////////////////////////////////////////////////////

template<class ttec>
template<class Iterator>
t_idx_t tunneling_support<ttec>::tunnel_bwt( t_string_t &bwt, twobitvector &aux, Iterator first, Iterator last ) {

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

//// INVERTING A TUNNELED BWT /////////////////////////////////////////////////

template<class ttec>
void tunneling_support<ttec>::invert_tunneled_bwt( const t_string_t &tbwt, const twobitvector &aux, t_size_t n,
                                             t_idx_t tbwt_idx, t_size_t maxalphval, std::ostream &out ) {
	typedef typename std::ostream::char_type schar_t;
	static_assert( std::is_same<
	                    typename std::make_unsigned<schar_t>::type,
	                    typename std::make_unsigned<t_uchar_t>::type
	               >::value,
	               "character types must be compatible" );

	if (tbwt.size() != 0 && (tbwt_idx >= tbwt.size() || tbwt_idx == 0)) {
		throw std::invalid_argument("tbwt index is invalid");
	}

	//count character frequencies
	std::vector<t_size_t> C( maxalphval+1 );
	for (t_idx_t i = 0; i < tbwt.size(); i++) {
		if (aux[i] != aux_encoding::IGN_L) { //skip entries which are ignored
			++C[tbwt[i]];
		}
	}
	//compute start positions
	t_size_t j = 0;
	for (t_size_t i = 0; i < C.size(); i++) {
		auto cnt = C[i];
		C[i] = j;
		while (cnt > 0) {
			if (++j >= aux.size()) {
				throw std::invalid_argument("auxiliary structure is invalid");
			}				
			if (aux[j] != aux_encoding::SKP_F) { //skip empty positions
				--cnt;
			}
		}
	}

	//NOTE: the following code requires that aux[tbwt.size()] == aux_encoding::REG
	if (aux[tbwt.size()] != aux_encoding::REG) {
		throw std::invalid_argument("auxiliary structure is invalid");
	}

	//// INVERTITION USING PHI ////////////////////////////////////////////
	
	//compute PHI
	std::vector<t_idx_t> PHI( tbwt.size() );
	for (t_idx_t i = 0; i < tbwt.size(); i++) {
		if (aux[i] != aux_encoding::IGN_L) {
			j = C[tbwt[i]];
			if (j < tbwt_idx) {
				//skip empty positions
				for (t_idx_t k = 1; aux[++j] == aux_encoding::SKP_F; k++) {
					PHI[j] = k; //save distance to previous regular entry
				}
				//set PHI
				if (j >= tbwt.size()) {
					throw std::invalid_argument("auxiliary structure is invalid");
				}
				if (j < tbwt_idx)	PHI[j] = i;
				else             	PHI[0] = i; //save start index
			} else {
				//set PHI
				if (j >= tbwt.size()) {
					throw std::invalid_argument("auxiliary structure is invalid");
				}
				PHI[j] = i;
				//skip empty positions
				for (t_idx_t k = 1; aux[++j] == aux_encoding::SKP_F; k++) {
					PHI[j] = k; //save distance to previous regular entry
				}
			}
			C[tbwt[i]] = j;
		}
	}
	//invert tunneled bwt using a stack
	std::stack<t_idx_t> stck;
	j = 0; //start at saved start index
	for (t_idx_t i = 0; i < n; i++) {
		j = PHI[j];
		out.put( (schar_t)tbwt[j] );
		if ( aux[j+1] == aux_encoding::IGN_L ) { //end of a tunnel (reverse order)
			if (stck.empty()) {
				throw std::invalid_argument("missing end of a tunnel");
			}
			else {
				j += stck.top();
				stck.pop();
			}
		}
		else if ( aux[j] == aux_encoding::SKP_F ) { //start of a tunnel (reverse order)
			stck.push( PHI[j] ); //save distance to uppermost row of block
			j -= PHI[j];
		}
		else if ( aux[j+1] == aux_encoding::SKP_F ) { //start of a tunnel, being at the uppermost row (reverse order)
			stck.push( 0 );
		}
	}
	if (!stck.empty()) {
		throw std::invalid_argument("missing start of a tunnel");
	}
	
	//// INVERTITION USING LF (TEXT IS WRITTEN IN REVERSE ORDER TO STREAM) /
	/*
	//compute final LF - Mapping
	std::vector<t_idx_t> LF( tbwt.size() );
	t_idx_t l = 0; //position of the last regular entry 
	for (t_idx_t i = 0; i < tbwt.size(); i++) {
		if (aux[i] == aux_encoding::IGN_L) { //save distance to previous regular entry
			LF[i] = i - l;
		} else {
			l = i;
			//compute LF, depending on primary index
			j = C[tbwt[i]];
			if (j < tbwt_idx) {
				//skip empty positions
				while (aux[++j] == aux_encoding::SKP_F);

				//set LF
				if (j >= tbwt.size()) {
					throw std::invalid_argument("auxiliary structure is invalid");
				}
				LF[i] = j;
			} else {
				//set LF
				if (j >= tbwt.size()) {
					throw std::invalid_argument("auxiliary structure is invalid");
				}
				LF[i] = j;

				//skip empty positions
				while (aux[++j] == aux_encoding::SKP_F);
			}
			C[tbwt[i]] = j;
		}
	}

	//invert tunneled bwt using a stack
	std::stack<t_idx_t> stck;
	t_idx_t i = n;
	j = 0;
	while (i-- != 0) { //invert from back to front
		out.put( (schar_t)tbwt[j] );
		if ( aux[j+1] == aux_encoding::SKP_F ) { //end of a tunnel
			if (stck.empty()) {
				throw std::invalid_argument("missing start of a tunnel");
			} else {
				j += stck.top();
				stck.pop();
			}
		}
		else if (aux[j] == aux_encoding::IGN_L) { //start of a tunnel
			stck.push( LF[j] ); //save distance to uppermost row of tunnel
			j -= LF[j]; //move j to uppermost row of block
		} 
		else if (aux[j+1] == aux_encoding::IGN_L) { //start of a tunnel, being at the uppermost row
			stck.push(0);
		}
		j = LF[j]; //go to next suffix
	}
	if (!stck.empty()) {
		throw std::invalid_argument("missing end of a tunnel");
	}
	*/
}

#endif
