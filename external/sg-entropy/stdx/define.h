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

#ifndef __sg_stdx_define
#define __sg_stdx_define

//Constants
#define True 1
#define False 0
#define Yes 1
#define No 0

#ifndef NULL
#define NULL 0
#endif

#define NotFound -1

namespace SG
{
	//Compiler/implementation dependent typedefs
	typedef unsigned char Byte;
	typedef unsigned short Word;
	typedef unsigned long DWord;
	//typedef unsigned __int64 QWord;		//MS platform
	typedef unsigned long long QWord;	//Linux and other Unices
	typedef long double Real;

	typedef Byte Binary;	//reperesents 0 or 1
	typedef bool Boolean;	//reperesents true or false

	typedef DWord Counter;
	typedef Word SmallCounter;
	typedef QWord BigCounter;

	typedef signed long Num;
	typedef unsigned int FastInt;
}

#endif

