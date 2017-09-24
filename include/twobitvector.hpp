/*
 * twobitvector.hpp for bwt tunneling
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

#ifndef _TWOBITVECTOR_HPP
#define _TWOBITVECTOR_HPP

#include <assert.h>
#include <stdint.h>
#include <vector>

//! a simple implementation of a vector where each entry requires 2 bits.
class twobitvector {
	public:
		typedef uint8_t                            value_type;
		typedef std::vector<value_type>::size_type size_type;

		//! reference type for twobitvector
		class reference {
			private:
				value_type &val;
				value_type shift;
			
				friend class twobitvector;
				reference( value_type &v, value_type s ) : val{v}, shift{s} {};
			public:
				//! get value
				operator value_type() const {
					return (val >> shift) & 3u;
				};
				//! set value
				reference& operator=(value_type v) {
					val ^= (((val >> shift) ^ v) & 3u) << shift;
					return *this;
				};
				//! set value using another reference
				reference& operator=(const reference& x) {
					return *this=((x.val >> x.shift) & 3u);
				};
		};
	private:
		std::vector<value_type> m_data;
		size_type m_size = 0;
	
	public:	
		//! resize vector to the given size.
		/*! if n is bigger than current size, old contents stay and the end
		   is filled with zeros.
		*/
		void resize( size_type n ) {
			m_data.resize( (n >> 2) + 1 );
			m_size = n;
		};

		//! returns the number of entries in the twobitvector
		size_type size() const {
			return m_size;
		};

		//! returns a pointer to the underlying data field
		const uint8_t *data() const {
			return (const uint8_t *)m_data.data();
		};

		//! length of the underlying data field in bytes
		size_type datasize() const {
			return m_data.size();
		};

		//! random read access to the elements
		value_type operator[]( size_type i ) const {
			assert(i < m_size);
			return (m_data[i >> 2] >> ((i & 3u) << 1)) & 3u;
		};

		//! random read/write access to the elements
		reference operator[]( size_type i ) {
			assert(i < m_size);
			return reference( m_data[i >> 2], (i & 3u) << 1 );
		};

		//TODO: add more functions if required
};

#endif
