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

#include "stdx/exception.h"

SG::stdx::Exception::Exception(std::string Description,std::string Location)
{
	Exception::Description	= Description;
	Exception::Location		= Location;
}
