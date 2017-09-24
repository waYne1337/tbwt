/*
 * aux-encoding.hpp for bwt tunneling
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

#ifndef _AUX_ENCODING_HPP
#define _AUX_ENCODING_HPP

#include "twobitvector.hpp"

//! namespace gathering constants for interpretation of the auxiliary data structure
namespace aux_encoding {
	typedef twobitvector::value_type value_type;
	//! regular bwt entry
	const value_type REG = 0;
	//! entry indicating the end of a tunnel
	const value_type SKP_F = 1;
	//! entry indicating the start of a tunnel
	const value_type IGN_L = 2;
	//! entry to be removed
	const value_type REM = SKP_F | IGN_L;
	//! alphabet size in auxiliary data structure
	const value_type SIGMA = 3;
};

#endif
