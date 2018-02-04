/*
 * wt-compressor.hpp for bwt tunneling
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

#ifndef BW94_COMPRESSOR_HPP
#define BW94_COMPRESSOR_HPP

#include "bwt-compressor.hpp"
#include "tbwt-compressor.hpp"

#include "sdsl/bit_vectors.hpp"
#include "sdsl/wavelet_trees.hpp"

#include "block-scores-rle-model.hpp"

#include <istream>
#include <ostream>

//! class which encodes a BWT with a wavelet tree (and hybrid bitvectors) as second stage
class BW_SS_WT : public block_scores_rle_model {
public:
	//! encodes the transform t using a wavelet tree
	template<class T>
	static void encode( T &t, std::ostream &out ) {
		sdsl::wt_huff<sdsl::hyb_vector<>> wt( t, t.size() );
		wt.serialize( out );
	}

	//! decodes the transform and stores it in t
	template<class T>
	static void decode( std::istream &in, T &t ) {
		sdsl::wt_huff<sdsl::hyb_vector<>> wt;
		wt.load( in );
		for (t_idx_t i = 0; i < t.size(); i++) {
			t[i] = wt[i];
		}
	}
};

//typedefs defining compressors
typedef bwt_compressor<BW_SS_WT> bwt_compressor_wt;
typedef tbwt_compressor<BW_SS_WT> tbwt_compressor_wt;

#endif
