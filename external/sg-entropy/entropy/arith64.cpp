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

#include "entropy/arith64.h"

const SG::DWord	SG::Entropy::ArithmeticCoder64::MaxRange=0x3FFFFFFF;

SG::Entropy::ArithmeticCoder64::ArithmeticCoder64() :
	High(0xFFFFFFFF),
	Low(0),
	UnderflowCount(0),
	TempRange(0)
{
}

SG::Entropy::ArithmeticEncoder64::ArithmeticEncoder64(SG::io::BitOutputStream &BitOStream) :
	Flushed(false),
	Output(BitOStream)
{
}

SG::Entropy::ArithmeticEncoder64::~ArithmeticEncoder64()
{
	if(!Flushed) Flush();
}

void SG::Entropy::ArithmeticEncoder64::EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange)
{
	TempRange=(High-Low)+1;
	High=Low + ((TempRange*(SG::QWord)SymbolHigh)/TotalRange)-1;
	Low	=Low + ((TempRange*(SG::QWord)SymbolLow )/TotalRange);

	for(;;)
	{
		if((High & 0x80000000)==(Low & 0x80000000))
		{
			Output.WriteBit(High>>31);
			while(UnderflowCount)
			{
				Output.WriteBit((High>>31)^1);
				UnderflowCount--;
			}
		}
		else
		{
			if((Low	& 0x40000000) && !(High	& 0x40000000))
			{
				UnderflowCount++;

				Low	 &=	0x3FFFFFFF;
				High |=	0x40000000;
			}
			else
				return;
		}

		Low	=(Low<<1) &	0xFFFFFFFF;
		High=((High<<1)|1) & 0xFFFFFFFF;
	}
}

void SG::Entropy::ArithmeticEncoder64::Flush()
{
	if(!Flushed)
	{
		Output.WriteBit((Low>>30)&1);
		UnderflowCount++;

		while(UnderflowCount)
		{
			Output.WriteBit(((Low>>30)^1)&1);
			UnderflowCount--;
		}

		Output.Flush();
		Flushed=true;
	}
}

SG::Entropy::ArithmeticDecoder64::ArithmeticDecoder64(SG::io::BitInputStream &BitIStream) :
	Code(0),
	Input(BitIStream)
{
	for(SG::FastInt I=0;I<32;I++)
	{
		Code<<=1;
		Code+=Input.ReadBit();;
	}
}



SG::DWord SG::Entropy::ArithmeticDecoder64::GetCurrentCount(SG::DWord TotalRange)
{
	TempRange=(High-Low)+1;
	return (SG::DWord)(((((Code-Low)+1)*(SG::QWord)TotalRange)-1)/TempRange);
}

void SG::Entropy::ArithmeticDecoder64::RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange)
{
	TempRange=(High-Low)+1;
	High=Low+((TempRange*(SG::QWord)SymbolHigh)/TotalRange)-1;
	Low	=Low+((TempRange*(SG::QWord)SymbolLow )/TotalRange);

	for(;;)
	{
		if((High & 0x80000000) == (Low & 0x80000000))
		{
		}
		else
		{
			if((Low	& 0x40000000) && !(High	& 0x40000000))
			{
				Code ^=	0x40000000;
				Low	 &=	0x3FFFFFFF;
				High |=	0x40000000;
			}
			else
				return;
		}
		Low	 = (Low	<< 1) &	0xFFFFFFFF;
		High = ((High<<1) |	1) & 0xFFFFFFFF;

		Code <<=1;
		Code|=Input.ReadBit();
		Code &=0xFFFFFFFF;
	}
}
