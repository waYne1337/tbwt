/*
 * mtf-coder.hpp for bwt tunneling
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

#ifndef _MTF_CODER_HPP
#define _MTF_CODER_HPP

#include <limits>
#include <stdexcept>
#include <vector>

//! class for mtf-transformations, requires a string type
/*! template parameter string_t should support random access [], as well as
  resize() - function, empty construction and size()-function.
 */
template<class string_t> 
class mtf_coder {
	public:
		typedef typename string_t::value_type char_type;
		typedef typename string_t::size_type size_type;
	private:
		string_t alph;
	public:
		//! constructs an mtf coder, expects an alphabet of the underlying source.
		mtf_coder (string_t _alph) : alph(_alph) {};

		//! encodes a single character, and returns the coding for the character
		char_type encode_char( char_type c ) {
			char_type r = 0; //rank of S[i] in alph
			while (alph[0] != c) { //move S[i] to front
				++r;

				char_type tmp = alph[0];
				alph[0] = alph[r];
				alph[r] = tmp;
			}
			return r;
		};

		//! decodes a single encoded character and returns its decoded value.
		/*! throws invalid_argument if ranks in S are bigger than alphabet size.
		 */
		char_type decode_char( char_type c ) {
			if (c >= alph.size())
				throw std::invalid_argument("MTF Retransform failed");

			while (c > 0) { //move alph[c] to front
				char_type tmp = alph[c-1];
				alph[c-1] = alph[c];
				alph[c] = tmp;

				--c;
			}
			return (char_type)alph[0];
		};

		//! computes alphabet from underlying string S.
		/*! alphabet must consist of elements in [0..maxsigma-1],
		   depending on the type of S (e.g., if S is a vector of 1-byte-characters,
		   it's suitable to choose maxsigma = 256)
		   Note that alphabet order is equal to the order of the first appearance
		   of the characters in S.
		*/
		static string_t compute_alph( const string_t &S,
		                              size_type maxsigma = std::numeric_limits<char_type>::max()+1 ) {
			//set up alphabet and bitmap
			std::vector<bool> charUsed( maxsigma );
			string_t alph;  alph.reserve( maxsigma );

			//compute alphabet
			for (size_type i = 0; i < S.size(); i++) {
				size_type ch = S[i];
				if (!charUsed[ch]) {
					alph.push_back(ch);
					charUsed[ch] = true;
				}
			}
			return alph;
		};

		//! transforms a string S using Move-To-Front Transformation.
		/*! alph must be a list of the alphabet used in S, e.g. as computed
		   by function compute_alph (alph should be a copy, as it gets modified
		   during execution)
		*/
		static void transform( string_t &S, string_t alph ) {
			mtf_coder coder( std::move( alph ) );
			for (size_type i = 0; i < S.size(); i++) {
				S[i] = coder.encode_char( S[i] );
			}
		};

		//! retransforms a Move-To-Front transformed string S using alph.
		/*! this function thus is the inverse operation of mtf_transform.
		   Note that for correct reconstruction, alph must be same as
		   given to mtf_transform.
		   throws invalid_argument if ranks in S are bigger than alphabet size
		*/
		static void retransform( string_t &S, string_t alph ) {
			mtf_coder coder( std::move( alph ) );
			for (size_type i = 0; i < S.size(); i++) {
				S[i] = coder.decode_char( S[i] );
			}
		};
};

#endif
