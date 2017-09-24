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

#include <iostream>
#include <fstream>
#include <time.h>
#include "stdx/define.h"

#include "io/bit_stream.h"
#include "io/stream_array.h"

#include "entropy/range32.h"

using namespace std;
using namespace SG;

void Rescale(Counter *Frequency) {
	for(int i=1;i<=256;i++) {
		Frequency[i]/=2;
		if(Frequency[i]<=Frequency[i-1]) Frequency[i]=Frequency[i-1]+1;
	}
}

//A quick test for entropty coders. Uses order-0 model.
int main(int argc,char *argv[])
{
	fstream Fin,Fout;
	Counter FileSizeB;

	Counter Seconds, OutputSize;

	if(argc!=4)
	{
		cerr<<"Usage: c|d InputFileName OutputFileName\n"
			<<"c: compress\n"
			<<"d: decompress\n";
		return 1;
	}

	Fin.open(argv[2],ios::in|ios::binary);
	if(!Fin.good())	{	cerr<<"File not found\n";	return 1;	}

	Fin.seekg(0,ios::end);
	FileSizeB=Fin.tellg();
	Fin.seekg(0,ios::beg);

	Byte *InputFile=new Byte[(DWord)(FileSizeB)];
	Byte *OutputFile=new Byte[(DWord)(FileSizeB+2000000)];
	if(InputFile==NULL||OutputFile==NULL)	{ cerr<<"Memory allocation error\n";	return 1;	}

	Fin.read((char *)InputFile,FileSizeB);

	Fout.open(argv[3],ios::out|ios::binary);
	if(!Fout.good()) {	cerr<<"Error creating file\n";	return 1;	}

	Seconds=clock();

	if(argv[1][0]=='c')
	{
		cout<<"Compressing...\n";

		io::ArrayInputStream ByteStream(InputFile,(DWord)(FileSizeB));
		io::ArrayOutputStream  OutputStream(OutputFile,(DWord)(FileSizeB+2000000));

		for(int i=0;i<sizeof(Counter);i++) OutputStream.WriteByte(((Byte*)&FileSizeB)[i]);

		SG::Entropy::RangeEncoder32 EntropyCoder(OutputStream);

        Counter Freq[257];
		for(int i=0;i<257;i++) Freq[i]=i;

		for(int i=0;i<FileSizeB;i++)
		{
			Byte ch=ByteStream.ReadByte();
			EntropyCoder.EncodeRange(Freq[ch],Freq[ch+1],Freq[256]);

			for(int j=ch+1;j<257;j++) Freq[j]++;	
			if(Freq[256]>=EntropyCoder.MaxRange) Rescale(Freq);
		}
		EntropyCoder.Flush();

		OutputSize = OutputStream.Tell();

		cout<<OutputSize<<"/"<<FileSizeB<<"\n";

		Fout.write((char*)OutputFile,OutputSize);
	}
	else if (argv[1][0]=='d')
	{
		cout<<"Decompressing...\n";

		io::ArrayInputStream InputStream(InputFile,(DWord)(FileSizeB));
		io::ArrayOutputStream ByteStream(OutputFile,(DWord)(FileSizeB+2000000));

		for(int i=0;i<sizeof(Counter);i++) ((Byte *)&OutputSize)[i]=InputStream.ReadByte();

		SG::Entropy::RangeDecoder32 EntropyCoder(InputStream);
		
		Counter Freq[257];
		for(int i=0;i<=256;i++) Freq[i]=i;

		for(int i=0;i<OutputSize;i++)
		{
			Counter Count = EntropyCoder.GetCurrentCount(Freq[256]);

			Byte Symbol;
			for(Symbol=255;Freq[Symbol]>Count;Symbol--);
			//Symbol--;

			ByteStream.WriteByte(Symbol);
			EntropyCoder.RemoveRange(Freq[Symbol],Freq[Symbol+1],Freq[256]);

			for(int j=Symbol+1;j<257;j++) Freq[j]++;
			if(Freq[256]>=EntropyCoder.MaxRange) Rescale(Freq);
		}

		Fout.write((char*)OutputFile,OutputSize);
	}
	else
	{
        cerr<<"Invalid parameter\n";
		return 1;
	}
}
