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

#ifndef __sg_io_stream
#define __sg_io_stream

#include "stdx/define.h"

namespace SG
{
	namespace io
	{
		class InputStream
		{
		protected:
			
		public:
			InputStream(){}
			virtual ~InputStream(){}

			virtual int ReadByte()=0;
			virtual SG::Boolean Ended()=0;
		};

		class OutputStream
		{
		protected:

		public:
			OutputStream(){}
			virtual ~OutputStream(){Flush();}

			virtual void WriteByte(SG::Byte Value)=0;
			virtual void Flush(){};
		};
	}
}

#endif
