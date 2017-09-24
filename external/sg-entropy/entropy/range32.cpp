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

#include "entropy/range32.h"

const SG::DWord	SG::Entropy::RangeCoder32::Top=(SG::DWord)1<<24;
const SG::DWord	SG::Entropy::RangeCoder32::Bottom=(SG::DWord)1<<16;
const SG::DWord	SG::Entropy::RangeCoder32::MaxRange=Bottom;

SG::Entropy::RangeCoder32::RangeCoder32() :
	Low(0),
	Range((SG::DWord)-1)
{
}

SG::Entropy::RangeEncoder32::RangeEncoder32(SG::io::OutputStream &OStream) :
	Flushed(false),
	Output(OStream)
{
}

SG::Entropy::RangeEncoder32::~RangeEncoder32()
{
	if (!Flushed) Flush();
}

void SG::Entropy::RangeEncoder32::EncodeRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord TotalRange)
{
	Low += SymbolLow*(Range/=TotalRange);
	Range *= SymbolHigh-SymbolLow;

	while ((Low ^ (Low+Range))<Top || Range<Bottom && ((Range= -Low & (Bottom-1)),1))
	{
		Output.WriteByte(Low>>24), Range<<=8, Low<<=8;
	}
}

void SG::Entropy::RangeEncoder32::Flush()
{
	if(!Flushed)
	{
		for(SG::FastInt i=0;i<4;i++)
		{
			Output.WriteByte(Low>>24);
			Low<<=8;
		}

		Flushed=true;
	}
}

SG::Entropy::RangeDecoder32::RangeDecoder32(SG::io::InputStream &IStream) :
	Code(0),
	Input(IStream)
{
	for(SG::FastInt i=0;i<4;i++)
	{
		Code = (Code << 8) | Input.ReadByte();
	}
}

SG::DWord SG::Entropy::RangeDecoder32::GetCurrentCount(SG::DWord TotalRange)
{
	return (Code-Low)/(Range/=TotalRange);
}

void SG::Entropy::RangeDecoder32::RemoveRange(SG::DWord SymbolLow,SG::DWord SymbolHigh,SG::DWord /*TotalRange*/)
{
	Low += SymbolLow*Range;
	Range *= SymbolHigh-SymbolLow;

	while ((Low ^ Low+Range)<Top || Range<Bottom && ((Range= -Low & Bottom-1),1))
	{
		Code= Code<<8 | Input.ReadByte(), Range<<=8, Low<<=8;
	}
}
