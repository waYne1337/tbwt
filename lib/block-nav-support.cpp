/*
 * block-nav-support.cpp for bwt tunneling
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

#include "block-nav-support.hpp"

#include <stack>

using namespace std;

//// CONSTRUCTION /////////////////////////////////////////////////////////////

void block_nav_support::compute_blocks() {
	//create helpful arrays
	vector<t_idx_t> bend; //block end
	vector<t_idx_t> &pbend = m_end; //perfect block end

	//initialize arrays and a stack
	bend.resize( blocks );
	pbend.resize( blocks );
	for (t_idx_t b = 0; b < blocks; b++) {
		bend[b] = bwtrs.run_lf(b);
		pbend[b] = bwtrs.run_lf(b);
	}
	stack<t_idx_t> s;

	//run algorithm (without collision computation)
	for (t_idx_t b = 0; b < blocks; b++) {
		if (bwtrs.height(b) >= mbh) {
			s.push( b );
			do {
				auto b_ = bwtrs.run_of( bend[s.top()] );
				if (bend[s.top()] + bwtrs.height(s.top()) <= bwtrs.end(b_)) {
					//stacktop block can be extended
					s.push( b_ );
				} else {
					//block on stacktop is maximal
					b_ = s.top();
					s.pop();

					if (!s.empty()) {
						//adapt end of new stacktop block
						bend[s.top()] = bend[b_] + (bend[s.top()] - bwtrs.start(b_));

						//adapt end of perfect blocks
						if (bwtrs.height(b_) == bwtrs.height(s.top())) {
							pbend[s.top()] = pbend[b_];
							//set b_'s perfect width to 1 to avoid overlapping blocks
							pbend[b_] = bwtrs.run_lf(b_);
						}
					}
				}
			} while (!s.empty());
		}
	}
}

void block_nav_support::init_empty_collision_map() {
	//create collision map of "big-enough" size, initialized with a "virtual" 0
	collisions.resize( bwtrs.n / mbh, blocks );
	//create a counter at the end position of each's run, indicating
	//how much collisions a block has
	for (t_idx_t b = 0; b < blocks; b++) {
		if (bwtrs.height(b) >= mbh) {
			t_idx_t b_last_col = bwtrs.end(b) / mbh - 1;
			collisions[b_last_col] += b_last_col - bwtrs.start(b) / mbh;
		}
	}
}

//// COLLISION HANDLING ///////////////////////////////////////////////////////

void block_nav_support::add_collision( t_idx_t ic_b, t_idx_t oc_b ) {
	//add oc_b to ic_b's collision list
	t_idx_t b_last_col = bwtrs.end(ic_b) / mbh - 1;
	t_idx_t b_new_col = b_last_col - (collisions[b_last_col] - blocks);
	//decrement counter and save entry
	--collisions[b_last_col];
	collisions[b_new_col] = oc_b;
}

void block_nav_support::set_end( t_idx_t b, t_idx_t e ) {
	m_end[b] = e;
}

vector<t_idx_t> block_nav_support::get_inner_collisions( t_idx_t b ) const {
	//create vector for storing block indices
	vector<t_idx_t> b_cols; //TODO: reserve a good number in future
	b_cols.push_back( b );
	//use navigation to compute all colliding blocks
	t_idx_t i = bwtrs.run_lf(b);

	while (i != end[b]) {
		//get block where i points in
		auto b_ = bwtrs.run_of(i);
		//check if blocks collide (simple check, see function below)
		if (collisions[bwtrs.start(b_)/mbh] == blocks) { //no collision
			i = end[b_] + (i - bwtrs.start(b_)); //skip block
		} else { //collision
			b_cols.push_back( b_ );
			//advance i
			i = bwtrs.run_lf(b_) + (i - bwtrs.start(b_));
		}
	}
	return b_cols;
}

vector<t_idx_t> block_nav_support::get_outer_collisions( t_idx_t b ) const {
	//create vector for storing block indices
	vector<t_idx_t> b_cols; //TODO: reserve a good number in future
	b_cols.push_back( b );
	//use collision map to compute all colliding blocks "recursive"
	for (t_idx_t i = 0; i < b_cols.size(); i++) {
		auto b_ = b_cols[i];
		for (t_idx_t j = bwtrs.start(b_)/mbh; j < bwtrs.end(b_)/mbh 
		     && collisions[j] != blocks; j++) {
			
			b_cols.push_back(collisions[j]);
		}
	}
	return b_cols;
}

void block_nav_support::remove_inner_outer_collisions( t_idx_t b ) {
	//clear first entry in collision map
	collisions[bwtrs.start(b)/mbh] = blocks;
}
