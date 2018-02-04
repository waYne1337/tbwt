/*
 * bcm-compressor.hpp for bwt tunneling
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

#ifndef BCM_COMPRESSOR_HPP
#define BCM_COMPRESSOR_HPP

#include "bwt-compressor.hpp"
#include "tbwt-compressor.hpp"
#include "bcm-ss.hpp"

#include "block-scores-rle-model.hpp"

#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>

//! class which encodes a BWT with second stage by Ilya Muravyov
class BW_SS_BCM : public block_scores_rle_model {
	
public:
	//! encodes the transform t using MTF + RLE0 + Entropy
	template<class T>
	static void encode( T &t, std::ostream &out ) {
		bcm::CM cm;
		for (t_idx_t i = 0; i < t.size(); i++) {
			cm.Encode( t[i], out );
		}
		cm.Flush(out);
	}

	//! decodes the transform and stores it in t using MTF + RLE0 + Entropy (t must have length of output)
	template<class T>
	static void decode( std::istream &in, T &t ) {
		bcm::CM cm;
		cm.Init(in);
		for (t_idx_t i = 0; i < t.size(); i++) {
			t[i] = cm.Decode(in);
		}
	}
};

//typedefs defining compressors
typedef bwt_compressor<BW_SS_BCM> bwt_compressor_bcm;
typedef tbwt_compressor<BW_SS_BCM> tbwt_compressor_bcm;

#endif
