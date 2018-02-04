/*
 * entropy-coder.hpp for bwt tunneling
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

#ifndef _ENTROPY_CODER_HPP
#define _ENTROPY_CODER_HPP

#include <math.h>
#include <stdexcept>
#include <type_traits>
#include <vector>

//SG entropy includes
#include "entropy/range64.h"
#include "io/bit_stream.h"
#include "io/stream.h"
#include "stdx/define.h"
#include "stdx/exception.h"

//! base class for entropy coding.
class entropy_coder {
	public:
		typedef SG::Counter size_type;
		typedef SG::Counter value_type;
	protected:
		std::vector<size_type> freq{ 1 }; //frequency table

		// rescales frequencies if they get to big
		void rescale_frequencies() {
			for(size_type i = 1; i < freq.size(); i++) {
				freq[i] /= 2;
				if(freq[i] <= freq[i-1]) freq[i] = freq[i-1]+1;
			}
		};

	public:
		//! destructor
		virtual ~entropy_coder() {};

		//! returns sigma (alphabet size) of this coder.
		size_type sigma() const {
			return freq.size()-1;
		};

		//! resets this entropy coder and initializes it to the new sigma (alphabet size).
		void reset( size_type sgm ) {
			freq.resize( sgm+1 );
			for (size_type i = 0; i < freq.size(); i++)
				freq[i] = i;
		};
};

//! class for entropy encoding.
/*! class guarantees that ostream_t only has to support operations put and flush.
 */
template<class ostream_t>
class entropy_encoder : public entropy_coder {
		static_assert( std::is_same<
		                    typename std::make_unsigned<SG::Byte>::type,
		                    typename std::make_unsigned<typename ostream_t::char_type>::type
		               >::value,
		               "stream types must be compatible" );
	private:
		//class for mapping output streams to SG entropy
		class output_stream_mapper : public SG::io::OutputStream {
			private:
				friend class entropy_encoder<ostream_t>;
				ostream_t &s; //underlying stream
				output_stream_mapper( ostream_t &_s ) : s(_s) {};
			public:
				virtual void WriteByte(SG::Byte Value) {
					s.put( Value );
				};
				virtual void Flush() {
					s.flush();
				};
		};

		//range coder
		output_stream_mapper s_mapper;
		SG::Entropy::RangeEncoder64 encoder;
	public:
		//! constructor
		entropy_encoder( ostream_t &s ) : s_mapper(s), encoder( s_mapper ) {};

		//! destructor
		~entropy_encoder() { flush(); };
		
		//! encodes next character.
		/*! The character must be in range [0..sigma-1]. passes through
		  exceptions from the underlying stream
		  , or throws an invalid_argument if range coder has problems
		 */
		void encode_char(value_type c) {
			try {
				encoder.EncodeRange( freq[c], freq[c+1], freq.back() );

				//and adapt frequencies
				while (++c < freq.size()) ++freq[c];
				if (freq.back() >= encoder.MaxRange) {
					rescale_frequencies();
				}
			} catch ( SG::stdx::Exception e ) {
				throw std::invalid_argument( e.Description + " at " + e.Location );
			}
		};

		//! flushes this encoder, important to call after encoding process.
		/*! passes through exceptions from the underlying stream
		  , or throws an invalid_argument if range coder has problems
		 */
		void flush() {
			try {
				encoder.Flush();
				s_mapper.Flush();
			} catch ( SG::stdx::Exception e ) {
				throw std::invalid_argument( e.Description + " at " + e.Location );
			}
		};

		//! returns the maximal size of an encoding for any string of
		//! length n and alphabet size sgm (in bytes).
		static long max_enc_size( size_type n, size_type sgm ) {
			return (n * log2( sgm ) / 8) + 2000000l; //add some offset to be sure
		};
};

//! class for entropy-decoding.
/*! class guarantees that ostream_t only has to support operations get and tellg.
 */
template<class istream_t>
class entropy_decoder : public entropy_coder {
		static_assert( std::is_same<
		                    typename std::make_unsigned<SG::Byte>::type,
		                    typename std::make_unsigned<typename istream_t::char_type>::type
		               >::value,
		               "stream types must be compatible" );
	private:
		
		//class for mapping input streams to SG entropy
		class input_stream_mapper : public SG::io::InputStream {
			private:
				friend class entropy_decoder<istream_t>;
				istream_t &s; //underlying stream
				input_stream_mapper( istream_t &_s ) : s(_s) {};
			public:
				virtual int ReadByte() {
					return (SG::Byte)s.get();
				};
				virtual SG::Boolean Ended() {
					return false; //is ignored either way
				};
		};

		//range coder
		input_stream_mapper s_mapper;
		SG::Entropy::RangeDecoder64 decoder;

		//last character decoded
		value_type ch;
	
	public:
		//! constructor, expects a stream and a end position when to stop reading in stream
		entropy_decoder( istream_t &s ) : s_mapper(s),
			decoder( s_mapper ) {};

		//!reads next character from stream and returns it.
		/*! function should be called only once, character can be
		  queried by get_char() function.
		  IMPORTANT NOTE: between two successive calls of this function,
		  the function next() should be called, thus to decode a string, use
		  following order of execution:
		  decode_char() next() decode_char() next() ... decode_char() next() decode_char()
		*/
		value_type decode_char() {
			ch = sigma();
			try {
				//decode character and adapt frequencies
				size_type cnt = decoder.GetCurrentCount( freq.back() );
				while (freq[ch] > cnt) {
					++freq[ch--];
				}
			} catch ( SG::stdx::Exception e ) {
				throw std::invalid_argument( e.Description + " at " + e.Location );
			}
			return ch;
		}

		//! function returns last decoded character
		value_type get_char() const {
			return ch;
		}

		//! function advances iterator to next character.
		//! IMPORTANT NOTE: after last call of decode_char(),
		//! no further next() - call should be performed.
		void next() {
			try {
				//remove range and rescale if necessary
				decoder.RemoveRange( freq[ch], freq[ch+1]-1, freq.back()-1 );
				if (freq.back() >= decoder.MaxRange) {
					rescale_frequencies();
				}
			} catch ( SG::stdx::Exception e ) {
				throw std::invalid_argument( e.Description + " at " + e.Location );
			}
		}
};

#endif
