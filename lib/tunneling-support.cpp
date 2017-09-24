/*
 * tunneling-config.cpp for bwt tunneling
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

#include "tunneling-support.hpp"

using namespace std;

//// CONSTRUCTION /////////////////////////////////////////////////////////////

void tunneling_support::init_support_structures() {
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

t_size_t tunneling_support::blockstate( t_idx_t b ) {
	t_size_t s = bstate[b];
	bstate[b] = tunneling_support::UNCHANGED;
	return s;
}

void tunneling_support::tunnel_block_symbolic( t_idx_t b ) {
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

//// INVERTING A TUNNELED BWT /////////////////////////////////////////////////

void tunneling_support::invert_tunneled_bwt( const t_string_t &tbwt, const twobitvector &aux,
                                             t_idx_t tbwt_idx, t_string_t &S, t_size_t maxalphval ) {
	if (tbwt_idx >= tbwt.size()) {
		throw invalid_argument("tbwt index is invalid");
	}
	//count character frequencies
	vector<t_size_t> C( maxalphval+1 );
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
				throw invalid_argument("auxiliary structure is invalid");
			}				
			if (aux[j] != aux_encoding::SKP_F) { //skip empty positions
				--cnt;
			}
		}
	}

	//NOTE: the following code requires that aux[tbwt.size()] == aux_encoding::REG

	//compute final LF - Mapping
	vector<t_idx_t> LF( tbwt.size() );
	t_idx_t l = 0; //position of the last regular entry 
	for (t_idx_t i = 0; i < tbwt.size(); i++) {
		if (aux[i] == aux_encoding::IGN_L) { //save distance to previous regular entry
			LF[i] = i - l;
		} else {
			l = i;
			//compute LF, depending on primary index
			j = C[tbwt[i]];
			if (j < tbwt_idx) {
				do {	//skip empty positions
					if (++j >= tbwt.size()) {
						throw invalid_argument("auxiliary structure is invalid");
					}
				} while (aux[j] == aux_encoding::SKP_F);

				LF[i] = j;
			} else {
				LF[i] = j;

				do {	//skip empty positions
					if (++j >= aux.size()) {
						throw invalid_argument("auxiliary structure is invalid");
					}
				} while (aux[j] == aux_encoding::SKP_F);
			}
			C[tbwt[i]] = j;
		}
	}

	//invert tunneled bwt using a stack
	stack<t_idx_t> stck;
	t_idx_t i = S.size();
	j = 0;
	while (i-- != 0) { //invert from back to front
		S[i] = tbwt[j];
		if ( aux[j+1] == aux_encoding::SKP_F ) { //end of a tunnel
			if (stck.empty()) {
				throw invalid_argument("missing start of a tunnel");
			} else {
				j += stck.top();
				stck.pop();
			}
		}
		else if (aux[j] == aux_encoding::IGN_L) { //start of a tunnel
			stck.push( LF[j] ); //save distance to uppermost row of tunnel
			j -= LF[j]; //move j to uppermost row of block
		} else if (aux[j+1] == aux_encoding::IGN_L) { //start of a tunnel, being at the uppermost row
			stck.push(0);
		}
		j = LF[j]; //go to next suffix
	}
	if (!stck.empty()) {
		throw invalid_argument("missing end of a tunnel");
	}
}
