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

#ifndef __sg_stdx_bit
#define __sg_stdx_bit

#include "stdx/define.h"

namespace SG
{
	namespace stdx
	{
		//To ease operating on bits.

		//Checks if BitNo of data is set(1) or not. Returns 1 if yes, 0 otherwise.
		SG::Binary GetBit(SG::Byte BitNo,SG::Byte Data);

		//Sets BitNo of data as X (X = 0 or 1)
		void SetBit(SG::Byte BitNo,SG::Byte &Data,SG::Binary X);

		//Similar to above functions - for vector<SG::Byte>
		//inline SG::Binary GetArr(SG::Counter BitNo,std::vector<Byte> &Data);
		//inline void SetArr(SG::Counter BitNo,std::vector<Byte> &Data,SG::stdx::Type::Binary X);
	}
}

#endif
