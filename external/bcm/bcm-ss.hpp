// This is a header file to cover the second stage BWT transformation of BCM,
// invented by Ilya Muravyov

#ifndef BCM_SS_HPP
#define BCM_SS_HPP

#include <istream>
#include <ostream>

namespace bcm {

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int uint;
typedef unsigned long long ulonglong;

//basic encoder

struct Encoder
{
  uint low;
  uint high;
  uint code;

  Encoder();
  void EncodeBit0(uint p, std::ostream &out);
  void EncodeBit1(uint p, std::ostream &out);
  void Flush(std::ostream &out);
  void Init(std::istream &in);
  int DecodeBit(uint p, std::istream &in);
};

//counter
template<int RATE>
struct Counter
{
  word p;
  Counter()
  {
    p=1<<15;
  }
  void UpdateBit0()
  {
    p-=p>>RATE;
  }
  void UpdateBit1()
  {
    p+=(p^65535)>>RATE;
  }
};

//BWT encoder
struct CM: Encoder
{
  Counter<2> counter0[256];
  Counter<4> counter1[256][256];
  Counter<6> counter2[2][256][17];
  int c1;
  int c2;
  int run;

  CM();

  void Encode32(uint n, std::ostream &out);
  uint Decode32(std::istream &in);
  void Encode(int c, std::ostream &out);
  int Decode(std::istream &in);
};

//// EXAMPLES OF USE //////////////////////////////////////////////////////////
/*
  //ENCODING OF A BWT
  CM cm;
  cm.Encode32(n, out);
  for (int i=0; i<n; ++i)
    cm.Encode(bwt[i], out);

  cm.Flush(out);

  //DECODING OF A BWT
  CM cm;
  cm.Init(in);
  int n = cm.Decode32(in);
  byte *bwt = new byte[n];
  for (int i=0; i<n; ++i)
    bwt[i]=cm.Decode(in);
*/
};

#endif
