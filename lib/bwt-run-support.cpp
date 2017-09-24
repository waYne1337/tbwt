/*
 * bwt-run-support.cpp for bwt tunneling
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

#include "bwt-run-support.hpp"

#include <algorithm>
#include <limits>

using namespace std;

t_idx_t bwt_run_support::run_of( t_idx_t i ) const {
	//use binary search with runstart - array
	auto it = upper_bound( m_rs.begin(), m_rs.end(), i );
	return (t_idx_t)(it - m_rs.begin()) - 1;
}

bwt_run_support::bwt_run_support( const t_uchar_t *bwt, t_size_t _n, t_idx_t idx ) {
	//init some basic variables
	m_bwt_idx = idx;
	m_idx_n = _n;
	m_idx_runs = 0;

	//build C Array and count runs
	vector<t_size_t> C( numeric_limits<t_uchar_t>::max() + 1 );
	t_idx_t borders[] = {bwt_idx,idx_n};
	t_idx_t i = 0;
	for (t_idx_t b : borders) { //to split runs at primary index
		t_uchar_t lastchar = (i < idx_n) ? bwt[i]+1 : 0;
		while (i < b) {
			if (lastchar != bwt[i]) { //start of a run
				++m_idx_runs;
				lastchar = bwt[i];
			}
			++C[lastchar];
			++i;
		}
	}
	m_n = idx_n + 1;
	m_runs = idx_runs + 1; //for bwt index

	//build cumulative sums of the C array
	t_idx_t l = 1; //for bwt index
	for (t_size_t &c : C) {
		auto tmp = c;
		c = l;
		l += tmp;
	}

	//compute LF
	m_lfr.reserve( m_runs + 1 );
	m_rs.reserve( m_runs + 1 );
	i = 0;
	t_idx_t i_log = 0; //logical position of i
	for (t_idx_t b : borders) { //to split runs at primary index
		t_uchar_t lastchar = (i < n) ? bwt[i]+1 : 0;
		while (i < b) {
			if (lastchar != bwt[i]) { //start of a run
				m_rs.push_back( i_log ); //store start of run

				lastchar = bwt[i];
				m_lfr.push_back( C[lastchar] );
			}
			++C[lastchar];
			++i; ++i_log;
		}
		//add a terminator to both lfr and rs (for both primary index and n)
		m_rs.push_back( i_log++ );
		m_lfr.push_back( 0 );
	}
}
