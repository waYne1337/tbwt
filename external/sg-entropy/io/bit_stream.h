//Entropy Coding Source code
//By Sachin Garg, 2006
//
//Includes range coder based upon the carry-less implementation 
//by Dmitry Subbotin, and arithmetic coder based upon Mark Nelson's
//DDJ code.
// 
//Modified to use 64-bit variables for improved performance.
//32-bit reference implementations also included.
//
//For details:
//http://www.sachingarg.com/compression/entropy_coding/64bit

#ifndef __sg_io_bit_stream
#define __sg_io_bit_stream

#include "stdx/define.h"
#include "io/stream.h"

namespace SG
{
	namespace io
	{
		class BitInputStream
		{
		protected:
			SG::Byte _Position;
			SG::Byte _Buffer;
			SG::io::InputStream &_Under;
                        
		public:
			BitInputStream(SG::io::InputStream &Under);

			SG::Binary ReadBit();
			SG::Boolean Ended();
		};

		class BitOutputStream
		{
		protected:

			SG::Byte _Position;
			SG::Byte _Buffer;
			SG::io::OutputStream &_Under;

		public:
			BitOutputStream(SG::io::OutputStream &Under);
			~BitOutputStream();

			void WriteBit(SG::Binary Value);
			void Flush();
		};
	}
}

#endif
