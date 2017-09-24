/*
 * bwt-config.hpp for bwt tunneling
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

#ifndef _BWT_CONFIG_HPP
#define _BWT_CONFIG_HPP

#include <limits>
#include <stdint.h>
#include <vector>

typedef uint8_t  t_uchar_t;
typedef uint32_t t_size_t;
typedef uint32_t t_idx_t;
typedef int64_t  t_bitsize_t;
typedef typename std::vector<t_uchar_t> t_string_t;

const t_size_t t_max_size = (1024ul + 512ul)*1024ul*1024ul; //maximal size of input (1,5 GB)

#include "divsufsort.h"

//do some type assertions
static_assert( std::numeric_limits<saidx_t>::max() > t_max_size,
               "saidx_t is too small" );
static_assert( std::numeric_limits<t_idx_t>::max() > t_max_size,
               "t_idx_t is too small" );
static_assert( std::numeric_limits<t_size_t>::max() > t_max_size,
               "t_size_t is too small" );
static_assert( std::numeric_limits<t_bitsize_t>::max() > 8ul * t_max_size,
               "t_bitsize_t is too small" );

#endif
