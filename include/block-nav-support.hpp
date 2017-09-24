/*
 * block-nav-support.hpp for bwt tunneling
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

#ifndef _BLOCK_NAV_SUPPORT_HPP
#define _BLOCK_NAV_SUPPORT_HPP

#include "bwt-run-support.hpp"
#include "bwt-config.hpp"

#include <vector>

//! support class for blocks and block navigation in a bwt.
/*! class offers methods to compute blocks, as well as methods
   to store, enumerate and remove block collisions.
*/
class block_nav_support {
	private:
		const bwt_run_support &bwtrs; //navigation
		const t_size_t mbh = 2; //minimal block height

		std::vector<t_idx_t> m_end; //end position of blocks (see below)
		std::vector<t_idx_t> collisions; //map for collisions

		void compute_blocks();
		void init_empty_collision_map();

	public:
		//! number of blocks (always equal to number of runs)
		const t_size_t& blocks;

		//! exclusive end position (upper left position in BWT) of block
		const std::vector<t_idx_t> &end = m_end;

		//! constructor, expects a navigation and a minimal block height.
		/*! note that collisions will NOT be computed by this function,
		   use function add_collision for this purpose.
		*/
		block_nav_support( const bwt_run_support &bwsupport )
			: bwtrs( bwsupport ), blocks( bwtrs.runs ) {
			compute_blocks();
			init_empty_collision_map();
		};

		//! adds a collision between inner block ic_b and outer block oc_b.
		void add_collision( t_idx_t ic_b, t_idx_t oc_b );

		//! sets end of a block b to the given value
		void set_end( t_idx_t b, t_idx_t e );

		//! computes all inner colliding blocks of the given one (array is ordered in text order).
		//! Note that first block always is block b.
		std::vector<t_idx_t> get_inner_collisions( t_idx_t b ) const;

		//! computes all outer colliding blocks of the given one. Note that first block always is block b.
		std::vector<t_idx_t> get_outer_collisions( t_idx_t b ) const;

		//! removes all collisions between colliding inner and outer blocks of b
		void remove_inner_outer_collisions( t_idx_t b );
};

#endif
