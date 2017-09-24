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

#include "entropy/arith32.h"

const SG::DWord	SG::Entropy::ArithmeticCoder32::MaxRange=0x3FFF;

SG::Entropy::ArithmeticCoder32::ArithmeticCoder32() :
	High(0xFFFF),
	Low(0),
	UnderflowCount(0),
	TempRange(0)
{
}

SG::Entropy::ArithmeticEncoder32::ArithmeticEncoder32(SG::io::BitOutputStream &BitOStream) :
	Flushed(false),
	Output(BitOStream)
{
}

SG::Entropy::ArithmeticEncoder32::~ArithmeticEncoder32()
{
	if(!Flushed) Flush();
}

void SG::Entropy::ArithmeticEncoder32::EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange)
{
	TempRange=(High-Low)+1;
	High=Low + ((TempRange*SymbolHigh)/TotalRange)-1;
	Low	=Low + ((TempRange*SymbolLow )/TotalRange);

	for(;;)
	{
		if((High & 0x8000)==(Low & 0x8000))
		{
			Output.WriteBit(High>>15);
			while(UnderflowCount)
			{
				Output.WriteBit((High>>15)^1);
				UnderflowCount--;
			}
		}
		else
		{
			if((Low	& 0x4000) && !(High	& 0x4000))
			{
				UnderflowCount++;

				Low	 &=	0x3FFF;
				High |=	0x4000;
			}
			else
				return;
		}

		Low	=(Low<<1) &	0xFFFF;
		High=((High<<1)|1) & 0xFFFF;
	}
}

void SG::Entropy::ArithmeticEncoder32::Flush()
{
	if(!Flushed)
	{
		Output.WriteBit((Low>>14)&1);
		UnderflowCount++;

		while(UnderflowCount)
		{
			Output.WriteBit(((Low>>14)^1)&1);
			UnderflowCount--;
		}

		Output.Flush();
		Flushed=true;
	}
}

SG::Entropy::ArithmeticDecoder32::ArithmeticDecoder32(SG::io::BitInputStream &BitIStream) :
	Code(0),
	Input(BitIStream)
{
	for(SG::FastInt I=0;I<16;I++)
	{
		Code<<=1;
		Code+=Input.ReadBit();;
	}
}

SG::DWord SG::Entropy::ArithmeticDecoder32::GetCurrentCount(SG::DWord TotalRange)
{
	TempRange=(High-Low)+1;
	return (SG::DWord)(((((Code-Low)+1)*(SG::QWord)TotalRange)-1)/TempRange);
}

void SG::Entropy::ArithmeticDecoder32::RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange)
{
	TempRange=(High-Low)+1;
	High=Low+((TempRange*SymbolHigh)/TotalRange)-1;
	Low	=Low+((TempRange*SymbolLow )/TotalRange);

	for(;;)
	{
		if((High & 0x8000) == (Low & 0x8000))
		{
		}
		else
		{
			if((Low	& 0x4000) && !(High	& 0x4000))
			{
				Code ^=	0x4000;
				Low	 &=	0x3FFF;
				High |=	0x4000;
			}
			else
				return;
		}
		Low	 = (Low	<< 1) &	0xFFFF;
		High = ((High<<1) |	1) & 0xFFFF;

		Code <<=1;
		Code|=Input.ReadBit();
		Code &=0xFFFF;
	}
}
