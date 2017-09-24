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

#ifndef	__sg_entropy_range64
#define	__sg_entropy_range64

#include "stdx/define.h"
#include "io/stream.h"

namespace SG
{
	namespace Entropy
	{
		/*	Code for range coding, derived from public domain work by Dmitry Subbotin
			Modified to use 64-bit integer maths, for increased precision

			Note :	Cannot be used at full 'capacity' as the interface still takes DWord parameters (not QWord)
					This is done to maintain uniformity in interface across all entropy coders, feel free to 
					change this.
            
			author : Sachin Garg
		*/
		class RangeCoder64
		{
		public:
			static const SG::QWord MaxRange;

		protected:

			RangeCoder64();
			static const SG::QWord Top,Bottom;
			SG::QWord Low,Range;
		};

		class RangeEncoder64:public RangeCoder64
		{
		private:
			SG::Boolean Flushed;
			SG::io::OutputStream &Output;

		public:
			RangeEncoder64(SG::io::OutputStream &OStream);
			~RangeEncoder64();

			void EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
			void Flush();
		};

		class RangeDecoder64:public RangeCoder64
		{
		private:
			SG::QWord Code;
			SG::io::InputStream &Input;

		public:
			RangeDecoder64(SG::io::InputStream &IStream);

			SG::DWord GetCurrentCount(SG::DWord TotalRange);
			void RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
		};
	}
}

#endif
