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

#include "entropy/range64.h"

const SG::QWord SG::Entropy::RangeCoder64::Top=(SG::QWord)1<<56;
const SG::QWord SG::Entropy::RangeCoder64::Bottom=(SG::QWord)1<<48;
const SG::QWord SG::Entropy::RangeCoder64::MaxRange=Bottom;

SG::Entropy::RangeCoder64::RangeCoder64() :
	Low(0),
	Range((SG::QWord)-1)
{
}

SG::Entropy::RangeEncoder64::RangeEncoder64(SG::io::OutputStream &OStream) :
	Flushed(false),
	Output(OStream)
{
}

SG::Entropy::RangeEncoder64::~RangeEncoder64()
{
	if (!Flushed) Flush();
}

void SG::Entropy::RangeEncoder64::EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange)
{
	Low += SymbolLow*(Range/=TotalRange);
	Range *= SymbolHigh-SymbolLow;

	while ((Low ^ (Low+Range))<Top || Range<Bottom && ((Range= -Low & (Bottom-1)),1))
	{
		Output.WriteByte(Low>>56), Range<<=8, Low<<=8;
	}
}

void SG::Entropy::RangeEncoder64::Flush()
{
	if(!Flushed)
	{
		for(SG::FastInt i=0;i<8;i++)
		{
			Output.WriteByte(Low>>56);
			Low<<=8;
		}
		Flushed=true;
	}
}

SG::Entropy::RangeDecoder64::RangeDecoder64(SG::io::InputStream &IStream) :
	Code(0),
	Input(IStream)
{
	for(SG::FastInt i=0;i<8;i++)
	{
		Code = (Code << 8) | Input.ReadByte();
	}
}

SG::DWord SG::Entropy::RangeDecoder64::GetCurrentCount(SG::DWord TotalRange)
{
	return (Code-Low)/(Range/=TotalRange);
}

void SG::Entropy::RangeDecoder64::RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord /*TotalRange*/)
{
	Low += SymbolLow*Range;
	Range *= SymbolHigh-SymbolLow;

	while ((Low ^ Low+Range)<Top || Range<Bottom && ((Range= -Low & Bottom-1),1))
	{
		Code= Code<<8 | Input.ReadByte(), Range<<=8, Low<<=8;
	}
}
