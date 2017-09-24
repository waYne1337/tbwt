/*
 * tbwt-compressor.hpp for bwt tunneling
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

#ifndef _TBWT_COMPRESSOR_HPP
#define _TBWT_COMPRESSOR_HPP

#include "block-compressor.hpp"

//! a tunneled-bwt-based compressor. Uses TBWT + MTF + RLE0 + Entropy.
class tbwt_compressor : public block_compressor {
	protected:
		virtual void compress_block( std::istream &in, std::streampos end, std::ostream &out ) const;
		virtual void decompress_block( std::istream &in, std::streampos end, std::ostream &out ) const;
	public:
		//! constructor
		tbwt_compressor();
};

#endif
