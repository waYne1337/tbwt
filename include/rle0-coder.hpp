/*
 * rle0-coder.hpp for bwt tunneling
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

#ifndef _RLE0_CODER_HPP
#define _RLE0_CODER_HPP

#include <limits>
#include <stdexcept>
#include <type_traits>

//! class for run-length-encoding of zero runs, requires a string type
/*! template parameter string_t should support random access [], as well as
  resize() - function, empty construction and size()-function.
  Note that rle0-coding increases the alphabet size by one (zero runs get encoded
  as a combination of zeros and ones). This means that if the
  maximal value in the alphabet of underlying source is m, then the maximal
  value of the encoded sequence will be m+1.
 */
template<class string_t>
class rle0_encoder {
	public:
		typedef typename string_t::value_type char_type;
		typedef typename string_t::size_type size_type;
	private:
		static_assert( std::is_unsigned<char_type>::value,
		               "character type must be unsigned!" );
		char_type lastchar = 0;
		size_type zrl_p1 = 1; //length of current zero-run plus 1
	public:
		//! encodes the next character.
		/*! returns true if encoder requires the next (if possible) character
		  for a correct encoding, false if the encoder is ready for fetching a
		  couple of encoded character values. (if at the end of input, just ignore the result
		  and fetch the remaining encoding characters to be written).
		 */
		bool encode_char( char_type c ) {
			if (c == 0) { //run-length character
				++zrl_p1;
				return true; //request next run-length-character
			} else {
				lastchar = c;
				return false; //decoding can straightly be written out
			}
		};

		//! returns true if encoder has at least one more character present
		//! for the encoding.
		bool has_next_enc_char() const {
			return lastchar != 0 || zrl_p1 != 1;
		};

		//! returns the next encoded character or one if has_next_enc_char returned false.
		/*! Note that the returned value is from type size_type,
		  because the encoded character is incremented by one, thus possibly falling
		  out of the range of a char_type.
		 */
		size_type next_enc_char() {
			size_type c; //character to be returned
			//write run-length characters first
			if (zrl_p1 != 1) {
				c = zrl_p1 & 1u;
				zrl_p1 >>= 1;
			} else { //otherwise increment last character by one to distinguish between RL and normal chars
				c = (size_type)lastchar + 1;
				lastchar = 0;
			}
			return c;
		};

		//! encodes a given string using rle0-coding, trimming the string to its new size.
		/*! throws an invalid_argument exception if any character's value
		  plus 1 would produce an overflow (e.g. with unsigned char,
		  value 255 + 1 => 256 = 0 (overflow)) which would result in an
		  incorrect coding. (If you got such cases, use the object methods from above)
		 */
		static void encode_string( string_t &s ) {
			rle0_encoder<string_t> encoder;
			size_type j = 0;
			for (size_type i = 0; i < s.size(); ) {

				do {
					if (i >= s.size())	break;
					//check correct range
					if (s[i] == std::numeric_limits<char_type>::max()) {
						throw std::invalid_argument("overflow in rle0-coding");
					}
					//feed encoder with characters until he's ready for more output
				} while (encoder.encode_char(s[i++]));

				//write encoder's builded sequence
				while (encoder.has_next_enc_char()) {
					s[j++] = (char_type)encoder.next_enc_char();
				}
			}
			//trim s to correct size
			s.resize(j);
		};
};

//! class for run-length-decoding of zero runs, requires a string type
/*! template parameter string_t should support random access [], as well as
  resize() - function, empty construction and size()-function.
 */
template<class string_t>
class rle0_decoder {
	public:
		typedef typename string_t::value_type char_type;
		typedef typename string_t::size_type size_type;
	private:
		static_assert( std::is_unsigned<char_type>::value,
		               "character type must be unsigned!" );
		char_type lastchar = 0u;
		size_type rll = 0; //length of current run-length-sequence
		size_type z2write = 0; //number of zeros to write for current run
	public:
		//! decodes the next encoded character.
		/*! after decoding, any decoded sequence must be fetched immediately using
		  has_next_char- and next_char-functions. Function throws an
		  exception if the value range of c exceeds the allowed interval.
		 */
		void decode_char( size_type c ) {
			if (c > std::numeric_limits<char_type>::max()+1) {
				throw std::invalid_argument("illegal value range in RLE0-decoding");
			}
			//analyse character
			if (c <= 1u) { //run character
				z2write += 1ul << (c + rll++);
			} else { //normal character
				lastchar = (char_type)c - 1;
				rll = 0;
			}
		};

		//! returns true if decoder has more output to be fetched.
		bool has_next_char() const {
			return lastchar != 0 || z2write != 0;
		};

		//! can be used to fetch the next character from the decoder
		char_type next_char() {
			char_type c;
			if (z2write != 0) { //fetch zeros first
				--z2write;
				c = 0u;
			} else {
				c = lastchar;
				lastchar = 0u;
			}
			return c;
		};

		//! decodes a rle0-encoded string and returns the size of the decoding.
		/*! the given string must be large enough to cover the whole result.
		  Also, the encoding must be placed at the back of the given string
                  where the encodings start position is indicated by position p.
		  The decoding is written to the front of S, which has no side effects
		  if S is long enough to cover all characters, and if the encoding is placed
		  at the back.
		  Function throws an illegal_argument exception if S is not long enough
		  or potential side effects between encoding and decoding take place.
		 */
		static size_type decode_string( string_t &s, size_type p ) {
			rle0_decoder<string_t> decoder;
			size_type i = 0;
			while (p < s.size()) {
				//feed decoder
				decoder.decode_char( s[p++] );
				//and write result
				while (decoder.has_next_char()) {
					//do side effect check
					if (i >= p) {
						throw std::invalid_argument("distance between front and RLE0-encoding too small");
					}
					//store character
					s[i++] = decoder.next_char();
				}
			}
			return i;
		};
};

#endif
