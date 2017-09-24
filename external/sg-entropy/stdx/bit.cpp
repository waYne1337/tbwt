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

#include "stdx/bit.h"


SG::Binary SG::stdx::GetBit(SG::Byte BitNo,SG::Byte Data)
{
	return (1<<BitNo)&Data?1:0;
}

void SG::stdx::SetBit(SG::Byte BitNo,SG::Byte &Data,SG::Binary X)
{
	if(X)
		Data=Data|(1<<BitNo);
	else
		Data=Data&(~(1<<BitNo));
}

//SG::stdx::Type::Binary SG::stdx::Bit::GetArr(SG::stdx::Type::Counter BitNo,SG::stdx::Type::Byte *Data)
//{
//	return Get((SG::stdx::Type::Byte)(BitNo%8),Data[BitNo/8]);
//}
//
//void SG::stdx::Bit::SetArr(SG::stdx::Type::Counter BitNo,SG::stdx::Type::Byte *Data,SG::stdx::Type::Binary X)
//{
//	Set((SG::stdx::Type::Byte)(BitNo%8),Data[BitNo/8],X);
//}
