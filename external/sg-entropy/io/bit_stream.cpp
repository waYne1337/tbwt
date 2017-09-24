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
#include "io/bit_stream.h"

SG::io::BitInputStream::BitInputStream(SG::io::InputStream &Under) :
	_Under(Under),
	_Position(0)
{
    _Buffer=_Under.ReadByte(); 
}

SG::Binary SG::io::BitInputStream::ReadBit()
{
	SG::Binary Result=stdx::GetBit(_Position,_Buffer);

	if(_Position==7)
	{
		_Position=0;
		_Buffer=_Under.ReadByte();
	}
	else
	{
        ++_Position;
	}

    return Result;
}

SG::io::BitOutputStream::BitOutputStream(SG::io::OutputStream &Under) :
	_Under(Under),
	_Position(0)
{
}

SG::io::BitOutputStream::~BitOutputStream()
{
	Flush();
}

void SG::io::BitOutputStream::WriteBit(SG::Binary Value)
{
	stdx::SetBit(_Position,_Buffer,Value);

	if(_Position==7)
	{
		_Position=0;
		_Under.WriteByte(_Buffer);
	}
	else
	{
		++_Position;
	}
}

void SG::io::BitOutputStream::Flush()
{
	if(_Position!=0)
	{
		_Under.WriteByte(_Buffer);
		_Position=0;
		_Buffer=0;
	}
}
