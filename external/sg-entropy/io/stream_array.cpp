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

#include "io/stream_array.h"
#include "stdx/exception.h"

SG::io::ArrayInputStream::ArrayInputStream(SG::Byte *Array,SG::Counter Size,SG::Counter InitialOffset) :
	_Array(Array),
	_Size(Size),
	_Offset(InitialOffset)
{
}

int SG::io::ArrayInputStream::ReadByte()
{
	int Result=_Offset<_Size ? _Array[_Offset]:(-1);
	++_Offset;
	return Result;
}

SG::Boolean SG::io::ArrayInputStream::Ended()
{
	return _Offset>=_Size;
}

SG::io::ArrayOutputStream::ArrayOutputStream(SG::Byte *Array,SG::Counter Size,SG::Counter InitialOffset) :
	_Array(Array),
	_Size(Size),
	_Offset(InitialOffset)
{
}

void SG::io::ArrayOutputStream::WriteByte(SG::Byte Value)
{
	if(_Offset>=_Size) throw stdx::Exception("Buffer Overflow","SG::io::ArrayOutputStream::WriteByte");
	_Array[_Offset]=Value;
	++_Offset;
}

