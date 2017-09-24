/*
 * bwt-run-support.hpp for bwt tunneling
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

#ifndef _BWT_RUN_SUPPORT_HPP
#define _BWT_RUN_SUPPORT_HPP

#include <vector>

#include "bwt-config.hpp"

//! support structure for bwt navigation and bwt run support
/*! The support structure distinguishes between positions in
   the logical BWT, that is, the BWT of a nullterminated string,
   and the indexed BWT, which is BWT of a nullterminated string
   where the null-character in the BWT is removed and stored by
   a such-called bwt idx.
   Both versions differ not much, but make a difference in positioning.
   Unless especially stated, this support structure always uses
   logical positioning, but also offers conversion methods to switch
   between logical and indexed positioning.
*/
class bwt_run_support {
	private:
		t_size_t m_runs; //number of logical runs
		t_size_t m_idx_runs; //number of runs (indexed BWT)
		t_idx_t m_bwt_idx; //bwt index
		t_size_t m_n; //logical text length
		t_size_t m_idx_n; //text length (indexed BWT)

		std::vector<t_idx_t> m_lfr; //lf, only for the start of runs
		std::vector<t_idx_t> m_rs; //start positions of all runs, sorted ascending.
		                           //additionally, m_rs[m_runs] = n+1 holds.

	public:
		//! constructor, expects a indexed BWT and its primary index.
		bwt_run_support( const t_uchar_t *bwt, t_size_t _n, t_idx_t _idx );

		//! logical number of runs in BWT
		const t_size_t &runs = m_runs;

		//! number of runs in the indexed BWT
		const t_size_t &idx_runs = m_idx_runs;

		//! primary index of the bwt
		const t_idx_t &bwt_idx = m_bwt_idx;

		//! logical length of text
		const t_size_t &n = m_n;

		//! real text length (also length of indexed BWT)
		const t_size_t &idx_n = m_idx_n;

		//! utility function, returns lf at the start of the given run
		t_idx_t run_lf( t_idx_t r ) const {
			return m_lfr[r];
		};

		//! utility function, returns the start of a run
		t_idx_t start( t_idx_t r ) const {
			return m_rs[r];
		};

		//! function returns the run to which position i belongs,
		//  or a value >= runs if i does not belong to any run (e.g. i < 0 or i >= n)
		t_idx_t run_of( t_idx_t i ) const;

		//! utility function, computes height of a run
		t_size_t height( t_idx_t r ) const {
			return m_rs[r+1]-m_rs[r];
		};

		//! utility function, computes exclusive end of a run
		t_idx_t end( t_idx_t r ) const {
			return m_rs[r+1];
		};

		//! utility function, converts a position in the indexed bwt to
		//! a position in the logical bwt
		t_idx_t idx_to_log( t_idx_t p_idx ) const {
			return (p_idx < bwt_idx) ? p_idx : p_idx + 1;
		};

		//! utility function, converts a logical position in the bwt to
		//! a position in the indexed bwt
		t_idx_t log_to_idx( t_idx_t p_log ) const {
			return (p_log <= bwt_idx) ? p_log : p_log - 1;
		};
};

#endif
