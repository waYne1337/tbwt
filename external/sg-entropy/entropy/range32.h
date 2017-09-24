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

#ifndef	__sg_entropy_range32
#define	__sg_entropy_range32

#include "stdx/define.h"
#include "stdx/bit.h"
#include "io/stream.h"

namespace SG
{
	namespace Entropy
	{
		/*	Code for range coding, derived from public domain work by Dmitry Subbotin
			Using 32-bit integer maths
            
			author : Sachin Garg
		*/
		class RangeCoder32
		{
		public:
			static const SG::DWord MaxRange;
		protected:

			RangeCoder32();
			static const SG::DWord Top,Bottom;
			SG::DWord Low,Range;
		};

		class RangeEncoder32:public RangeCoder32
		{
		private:
			SG::Boolean Flushed;
			SG::io::OutputStream &Output;

		public:
			RangeEncoder32(SG::io::OutputStream &OStream);
			~RangeEncoder32();

			void EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
			void Flush();
		};

		class RangeDecoder32:public RangeCoder32
		{
		private:
			SG::DWord Code;
			SG::io::InputStream &Input;

		public:
			RangeDecoder32(SG::io::InputStream &IStream);

			SG::DWord GetCurrentCount(SG::DWord TotalRange);
			void RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
		};
	}
}

#endif
