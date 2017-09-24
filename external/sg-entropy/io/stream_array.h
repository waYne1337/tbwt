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

#ifndef __sg_io_streamarray
#define __sg_io_streamarray

#include "stdx/define.h"
#include "stdx/bit.h"
#include "io/stream.h"

namespace SG
{
	namespace io
	{
		class ArrayInputStream : public SG::io::InputStream
		{
		private:
			SG::Counter _Offset;
			SG::Byte *_Array;
			SG::Counter _Size;

		public:
			ArrayInputStream(SG::Byte *Array,SG::Counter Size,SG::Counter InitialOffset=0);
			~ArrayInputStream(){}

			int ReadByte();
			SG::Boolean Ended();

			SG::Counter Tell(){return _Offset;}
			void Seek(SG::Counter Offset){_Offset=Offset;}
		};

		class ArrayOutputStream : public SG::io::OutputStream
		{
		private:
			SG::Counter _Offset;
			SG::Byte *_Array;
			SG::Counter _Size;

		public:
			ArrayOutputStream(SG::Byte *Array,SG::Counter Size,SG::Counter InitialOffset=0);
			~ArrayOutputStream(){}

			void WriteByte(SG::Byte Value);
			void Flush(){};

			SG::Counter Tell(){return _Offset;}
			void Seek(SG::Counter Offset){_Offset=Offset;}
		};
	}
}

#endif
