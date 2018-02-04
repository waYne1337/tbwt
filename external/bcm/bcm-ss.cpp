// This is the implementation of the second stage BWT transformation of BCM,
// invented by Ilya Muravyov
#include "bcm-ss.hpp"

using namespace bcm;

//// ENCODER IMPLEMENTATION ////

Encoder::Encoder()
  {
    low=0;
    high=uint(-1);
    code=0;
  }

void Encoder::EncodeBit0(uint p, std::ostream &out)
  {
#ifdef _WIN64
    low+=((ulonglong(high-low)*p)>>18)+1;
#else
    low+=((ulonglong(high-low)*(p<<(32-18)))>>32)+1;
#endif
    while ((low^high)<(1<<24))
    {
      out.put(low>>24);
      low<<=8;
      high=(high<<8)+255;
    }
  }

void Encoder::EncodeBit1(uint p, std::ostream &out)
  {
#ifdef _WIN64
    high=low+((ulonglong(high-low)*p)>>18);
#else
    high=low+((ulonglong(high-low)*(p<<(32-18)))>>32);
#endif
    while ((low^high)<(1<<24))
    {
      out.put(low>>24);
      low<<=8;
      high=(high<<8)+255;
    }
  }

void Encoder::Flush(std::ostream &out)
  {
    for (int i=0; i<4; ++i)
    {
      out.put(low>>24);
      low<<=8;
    }
  }

void Encoder::Init(std::istream &in)
  {
    for (int i=0; i<4; ++i)
      code=(code<<8)+in.get();
  }

int Encoder::DecodeBit(uint p, std::istream &in)
  {
#ifdef _WIN64
    const uint mid=low+((ulonglong(high-low)*p)>>18);
#else
    const uint mid=low+((ulonglong(high-low)*(p<<(32-18)))>>32);
#endif
    const int bit=(code<=mid);
    if (bit)
      high=mid;
    else
      low=mid+1;

    while ((low^high)<(1<<24))
    {
      low<<=8;
      high=(high<<8)+255;
      code=(code<<8)+in.get();
    }

    return bit;
  }

//// BWT ENCODER IMPLEMENTATION ////

CM::CM()
  {
    c1=0;
    c2=0;
    run=0;

    for (int i=0; i<2; ++i)
    {
      for (int j=0; j<256; ++j)
      {
        for (int k=0; k<17; ++k)
          counter2[i][j][k].p=(k<<12)-(k==16);
      }
    }
  }

void CM::Encode32(uint n, std::ostream &out)
  {
    for (int i=0; i<32; ++i)
    {
      if (n&(1<<31))
        Encoder::EncodeBit1(1<<17, out);
      else
        Encoder::EncodeBit0(1<<17, out);
      n+=n;
    }
  }

uint CM::Decode32(std::istream &in)
  {
    uint n=0;
    for (int i=0; i<32; ++i)
      n+=n+Encoder::DecodeBit(1<<17, in);

    return n;
  }

void CM::Encode(int c, std::ostream &out)
  {
    if (c1==c2)
      ++run;
    else
      run=0;
    const int f=(run>2);

    int ctx=1;
    while (ctx<256)
    {
      const int p0=counter0[ctx].p;
      const int p1=counter1[c1][ctx].p;
      const int p2=counter1[c2][ctx].p;
      const int p=((p0+p1)*7+p2+p2)>>4;

      const int j=p>>12;
      const int x1=counter2[f][ctx][j].p;
      const int x2=counter2[f][ctx][j+1].p;
      const int ssep=x1+(((x2-x1)*(p&4095))>>12);

      const int bit=c&128;
      c+=c;

      if (bit)
      {
        Encoder::EncodeBit1(ssep*3+p, out);
        counter0[ctx].UpdateBit1();
        counter1[c1][ctx].UpdateBit1();
        counter2[f][ctx][j].UpdateBit1();
        counter2[f][ctx][j+1].UpdateBit1();
        ctx+=ctx+1;
      }
      else
      {
        Encoder::EncodeBit0(ssep*3+p, out);
        counter0[ctx].UpdateBit0();
        counter1[c1][ctx].UpdateBit0();
        counter2[f][ctx][j].UpdateBit0();
        counter2[f][ctx][j+1].UpdateBit0();
        ctx+=ctx;
      }
    }

    c2=c1;
    c1=ctx&255;
  }

int CM::Decode(std::istream &in)
  {
    if (c1==c2)
      ++run;
    else
      run=0;
    const int f=(run>2);

    int ctx=1;
    while (ctx<256)
    {
      const int p0=counter0[ctx].p;
      const int p1=counter1[c1][ctx].p;
      const int p2=counter1[c2][ctx].p;
      const int p=((p0+p1)*7+p2+p2)>>4;

      const int j=p>>12;
      const int x1=counter2[f][ctx][j].p;
      const int x2=counter2[f][ctx][j+1].p;
      const int ssep=x1+(((x2-x1)*(p&4095))>>12);

      const int bit=Encoder::DecodeBit(ssep*3+p, in);

      if (bit)
      {
        counter0[ctx].UpdateBit1();
        counter1[c1][ctx].UpdateBit1();
        counter2[f][ctx][j].UpdateBit1();
        counter2[f][ctx][j+1].UpdateBit1();
        ctx+=ctx+1;
      }
      else
      {
        counter0[ctx].UpdateBit0();
        counter1[c1][ctx].UpdateBit0();
        counter2[f][ctx][j].UpdateBit0();
        counter2[f][ctx][j+1].UpdateBit0();
        ctx+=ctx;
      }
    }

    c2=c1;
    return c1=ctx&255;
  }
