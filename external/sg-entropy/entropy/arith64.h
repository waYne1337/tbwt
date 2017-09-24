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

#ifndef	__sg_entropy_arith64
#define	__sg_entropy_arith64

#include "stdx/define.h"
#include "stdx/bit.h"
#include "io/bit_stream.h"

namespace SG
{
	namespace Entropy
	{
		/*	Code for arithmetic coding, derived from work by Mark Nelson, Tom st Denis, Charles Bloom
			Modified to use 64-bit integer maths, for increased precision
            
			author : Sachin Garg
		*/
		class ArithmeticCoder64
		{
		public:
			static const SG::DWord MaxRange;

		protected:

			ArithmeticCoder64();
			SG::QWord	High,Low,UnderflowCount;
			SG::QWord	TempRange;
		};

		class ArithmeticEncoder64 : public ArithmeticCoder64
		{
		private:
			SG::Boolean Flushed;
			SG::io::BitOutputStream &Output;

		public:
			ArithmeticEncoder64(SG::io::BitOutputStream &BitOStream);
			~ArithmeticEncoder64();

			void EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
			void Flush();
		};

		class ArithmeticDecoder64 : public ArithmeticCoder64
		{
		private:
			SG::QWord Code;
			SG::io::BitInputStream &Input;

		public:
			ArithmeticDecoder64(SG::io::BitInputStream &BitIStream);

			SG::DWord GetCurrentCount(SG::DWord TotalRange);
			void RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
		};
	}
}
#endif
