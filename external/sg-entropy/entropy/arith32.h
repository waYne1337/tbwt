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

#ifndef	__sg_entropy_arith32
#define	__sg_entropy_arith32

#include "stdx/define.h"
#include "stdx/bit.h"
#include "io/bit_stream.h"

namespace SG
{
	namespace Entropy
	{
		/*	Code for arithmetic coding
			derived from work by Mark Nelson, Tom st Denis, Charles Bloom
            
			author : Sachin Garg
		*/
		class ArithmeticCoder32
		{
		public:
			static const SG::DWord MaxRange;

		protected:

			ArithmeticCoder32();
			SG::DWord High,Low,UnderflowCount;
			SG::DWord TempRange;
		};

		class ArithmeticEncoder32 : public ArithmeticCoder32
		{
		private:
			SG::Boolean Flushed;
			SG::io::BitOutputStream &Output;

		public:
			ArithmeticEncoder32(SG::io::BitOutputStream &BitOStream);
			~ArithmeticEncoder32();

			void EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
			void Flush();
		};

		class ArithmeticDecoder32 : public ArithmeticCoder32
		{
		private:
			SG::DWord Code;
			SG::io::BitInputStream &Input;
			
		public:
            ArithmeticDecoder32(SG::io::BitInputStream &BitIStream);

			SG::DWord GetCurrentCount(SG::DWord TotalRange);
			void RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange);
		};
	}
}

#endif
