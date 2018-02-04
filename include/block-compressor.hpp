/*
 * block-compressor.hpp for bwt tunneling
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

#ifndef _BLOCK_COMPRESSOR_HPP
#define _BLOCK_COMPRESSOR_HPP

#include <assert.h>
#include <forward_list>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

//! abstract base class for a block compressor.
/*! a block compressor divides its input into blocks
   (a continous sequence of characters of the input) and compresses
   each block individually.
 */
class block_compressor {
	private:
		//current block size (it is ensured that block size is smaller than maxblocksize)
		std::streamsize blocksize;
		// constant indicating how big a block can maximally be
		const std::streamsize maxblocksize;
		bool quiet = true; //indicates whether compressor is quiet and does not print any additional information
	protected:
		//prototypes for real encoding and decoding. end refers to the end position
		// in the input stream at which the input ends. For compress - function, this
		// class ensures that input to be compressed is smaller or equal to maxblocksize,
		// decompress function may check itself if output is smaller than maxblocksize.
		virtual void compress_block( std::istream &in, std::streampos end, std::ostream &out ) const = 0;
		virtual void decompress_block( std::istream &in, std::streampos end, std::ostream &out ) const = 0;

		//a function to print information during encoding (function will not print
		// something if compressor is set to be quiet, what is the default)
		template<class V>
		void print_info( std::string key, V value ) const {
			if (!quiet) {
				std::cout << "> " << key << "\t\t" << value << std::endl;
			}
		};

	public:
		//! constructor, expects maximal block size possible.
		block_compressor( std::streamsize max_block_size )
		                : blocksize( max_block_size ), maxblocksize( max_block_size ) {};

		//! sets the quiet state of this compressor (quiet=true is default).
		/*! if the compressor is set to quiet, it will not print any extra information,
		   otherwise it will print extra information related to the compression method used.
		 */
		void set_quiet( bool q ) {
			quiet = q;
		};

		//! returns whether this compressor is quiet (see set_quiet).
		bool is_quiet() const {
			return quiet;
		};

		//! returns current block size. Block size initially is set to the maximal
		//! possible block size.
		std::streamsize get_block_size() const {
			return blocksize;
		};

		//! returns maximal possible block size
		std::streamsize get_max_block_size() const {
			return maxblocksize;
		};

		//! can be used to set the block size, value must be smaller
		//! or equal to get_max_block_size().
		void set_block_size( std::streamsize bs ) {
			assert( bs > 0 && bs <= get_max_block_size() );
			blocksize = bs;
		};

		//! compresses input.
		/*! instream and outstream should not point to the same direction.
		  function throws a runtime error if encoding failed, or a stream exception
		  if input or output stream streams make problems.
		 */
		void compress( std::istream &in, std::ostream &out ) const {
			//set exception mask of instream
			in.exceptions( std::istream::badbit | std::istream::eofbit );
			out.exceptions( std::ostream::badbit );

			//get length of input
			in.seekg(0, std::ios_base::end);
			std::streamsize n = in.tellg();
			in.seekg(0, std::ios_base::beg); //jump to start of stream again

			//compute block sizes and store the end of each encoding
			size_t b = n / get_block_size();
			if (n % get_block_size() != 0)	++b;
			
			std::forward_list<std::streampos> blockend;
			for (size_t i = 0; i <= b; i++) //make place for header
				write_primitive<std::streamoff>( 0, out );

			blockend.push_front( out.tellp() ); //store position behind header
			print_info("number of blocks", b );

			//compress blocks
			auto it = blockend.begin();
			while (n > 0) {
				auto bs = std::min(n, get_block_size());
				compress_block( in, in.tellg()+bs, out);
				n -= bs;
				it = blockend.insert_after( it, out.tellp() );
			}

			//write header
			out.seekp(0, std::ios_base::beg); //jump back to start
			for (it = blockend.begin(); it != blockend.end(); ++it) {
				n = *it;
				write_primitive<std::streamoff>( n, out ); //end positions
			}
			
			//put stream to a good state and stop
			out.seekp( n );
			out.flush();				
		};

		//! compresses input.
		/*! function throws a runtime error if encoding failed, or a stream exception
		  if input or output stream streams make problems.
		 */
		std::string compress( const std::string &S ) const {
			std::istringstream in( S );
			std::ostringstream out;
			compress( in, out );
			return out.str();
		};

		//! decompresses a compressed input.
		/*! instream and outstream should not point to the same direction.
		  function throws a runtime error if decoding failed, an invalid 
		  argument exception if encoding was manipulated or a stream exception
		  if input or output stream streams make problems
		  IMPORTANT NOTE: it is not ensured that encoding manipulation is detected,
		  as this is experimental code without checksums
		*/
		void decompress( std::istream &in, std::ostream &out ) const {
			//set exception mask of streams
			in.exceptions( std::istream::badbit | std::istream::eofbit );
			out.exceptions( std::ostream::badbit );

			//read header
			std::forward_list<std::streampos> blockend;
			blockend.push_front( read_primitive<std::streamoff>( in ) );

			auto it = blockend.begin();
			while (in.tellg() != blockend.front()) {
				auto p = read_primitive<std::streamoff>( in );
				if (p < *it)
					throw std::invalid_argument("invalid header end positions");

				it = blockend.insert_after( it, p );
			}
			print_info("number of blocks", std::distance(blockend.begin(), blockend.end())-1 );
			
			//decompress each block
			blockend.pop_front();
			for (auto be : blockend) {
				decompress_block( in, be, out );
				if (in.tellg() != be) {
					throw std::invalid_argument("invalid block decompression");
				}
			}

			//leave streams in good state
			out.flush();
		};

		//! decompresses a compressed input.
		/*! function throws a runtime error if decoding failed, an invalid 
		  argument exception if encoding was manipulated or a stream exception
		  if input or output stream streams make problems
		  IMPORTANT NOTE: it is not ensured that encoding manipulation is detected,
		  as this is experimental code without checksums
		*/
		std::string decompress( const std::string &Enc ) const {
			std::istringstream in( Enc );
			std::ostringstream out;
			decompress( in, out );
			return out.str();
		};

		//! utility for writing POD types to a stream.
		template<class T>
		static void write_primitive( T p, std::ostream &out ) {
			for (size_t i = 0; i < sizeof(T); i++) {
				out.put( (char)(p & std::numeric_limits<unsigned char>::max()) );
				p >>= std::numeric_limits<unsigned char>::digits;
			}
		};

		//! utility for reading POD types from a stream.
		template<class T>
		static T read_primitive( std::istream &in ) {
			T p = (T)0;
			for (size_t i = 0; i < sizeof(T); i++) {
				p |= (T)( (unsigned char)in.get() ) << 
					( i * std::numeric_limits<unsigned char>::digits );
			}
			return p;
		};
};

#endif
