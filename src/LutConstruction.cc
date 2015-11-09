
/*
 * Basic Lut manipulation and construction functions
 *
 *
 *
 */

#include "Lut.h"
#include "LutUtils.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace{
  size_t mapSize(unsigned inputCnt) {
    return inputCnt >= 6 ? 1ul << (inputCnt-6) : 1ul;
  }
}

// Quick and dirty for now
bool Lut::isConstant() const{ return isGnd() || isVcc(); }
bool Lut::isGnd() const{ return equal(Gnd(inputCount())); }
bool Lut::isVcc() const{ return equal(Vcc(inputCount())); }
bool Lut::isAnd() const{ return equal(And(inputCount())); }
bool Lut::isOr() const{ return equal(Or(inputCount())); }
bool Lut::isNand() const{ return equal(Nand(inputCount())); }
bool Lut::isNor() const{ return equal(Nor(inputCount())); }
bool Lut::isXor() const{ return equal(Xor(inputCount())); }
bool Lut::isExor() const{ return equal(Exor(inputCount())); }

Lut::Lut(unsigned inputs)
: _inputCnt(inputs)
, _lut(mapSize(inputs), allZero)
{
}

Lut Lut::Gnd(unsigned inputs){
  Lut ret(inputs);
  return ret;
}
Lut Lut::Vcc(unsigned inputs){
  Lut ret(inputs);
  for(LutMask & cur : ret._lut) cur = allOne;
  return ret;
}
Lut Lut::And(unsigned inputs){
  Lut ret = Gnd(inputs);
  if(inputs < 6){
    ret._lut[0] = one << ((1<<inputs)-1);
  }
  else{
    ret._lut.back() = one<<63;
  }
  return ret;
}
Lut Lut::Or(unsigned inputs){
  Lut ret = Vcc(inputs);
  ret._lut[0] = ~one;
  return ret;
}
Lut Lut::Nand(unsigned inputs){
  Lut ret = And(inputs);
  ret.invert();
  return ret;
}
Lut Lut::Nor(unsigned inputs){
  Lut ret = Or(inputs);
  ret.invert();
  return ret;
}
Lut Lut::Xor(unsigned inputs){
  Lut ret(inputs);
  ret._lut[0] = xorMask;
  if(inputs>6){
    for(size_t i=0; i<inputs-6; ++i){
      size_t curMaskSize = 1<<i;
      for(size_t j=0; j<curMaskSize; ++j){
        ret._lut[curMaskSize+j] = ~ret._lut[j];
      }
    }
  }
  return ret;
}
Lut Lut::Exor(unsigned inputs){
  Lut ret = Xor(inputs);
  ret.invert();
  return ret;
}

Lut Lut::Not(Lut const & a){
  Lut ret = a;
  ret.invert();
  return ret;
}

Lut Lut::And(Lut const & a, Lut const & b){
  assert(a.inputCount() == b.inputCount());
  Lut ret(a.inputCount());
  for(size_t i=0; i<a.inputCount(); ++i){
    ret._lut[i] = a._lut[i] & b._lut[i];
  }
  return ret;
}

Lut Lut::Or(Lut const & a, Lut const & b){
  assert(a.inputCount() == b.inputCount());
  Lut ret(a.inputCount());
  for(size_t i=0; i<a.inputCount(); ++i){
    ret._lut[i] = a._lut[i] | b._lut[i];
  }
  return ret;
}

Lut Lut::Nor(Lut const & a, Lut const & b){
  assert(a.inputCount() == b.inputCount());
  Lut ret(a.inputCount());
  for(size_t i=0; i<a.inputCount(); ++i){
    ret._lut[i] = ~(a._lut[i] | b._lut[i]);
  }
  return ret;
}

Lut Lut::Nand(Lut const & a, Lut const & b){
  assert(a.inputCount() == b.inputCount());
  Lut ret(a.inputCount());
  for(size_t i=0; i<a.inputCount(); ++i){
    ret._lut[i] = ~(a._lut[i] & b._lut[i]);
  }
  return ret;
}

Lut Lut::Xor(Lut const & a, Lut const & b){
  assert(a.inputCount() == b.inputCount());
  Lut ret(a.inputCount());
  for(size_t i=0; i<a.inputCount(); ++i){
    ret._lut[i] = a._lut[i] ^ b._lut[i];
  }
  return ret;
}

Lut Lut::Exor(Lut const & a, Lut const & b){
  assert(a.inputCount() == b.inputCount());
  Lut ret(a.inputCount());
  for(size_t i=0; i<a.inputCount(); ++i){
    ret._lut[i] = ~(a._lut[i] ^ b._lut[i]);
  }
  return ret;
}

Lut::Lut(string const & init){
  unsigned s = 2;
  while( (1lu << (s-2)) < init.size()) ++s;
  if(init.size() != (1lu << s)) throw std::logic_error("Bad string size as init of the Lut");

  _inputCnt = s;
  _lut.resize(mapSize(_inputCnt));

  for(size_t i=0; i<_lut.size(); ++i){
    uint64_t val = 0ul;
    for(size_t j=i*8; j<i*8+8 && j<init.size(); ++j){
      uint64_t cur = 0ul;
      char c = init[j];
      if(c >= '0' && c <= '9'){
        cur = c-'0';
      }
      else if(c >= 'a' && c <= 'f'){
        cur = c-'a'+10;
      }
      else if(c >= 'A' && c <= 'F'){
        cur = c-'A'+10;
      }
      else throw std::logic_error("Invalid character in init of the Lut");
      val = val << 4 | cur;
    }
    _lut[i] = val;
  }
}

string Lut::str() const{
  stringstream ret;
  ret << std::hex;
  for(size_t i=_lut.size(); i>0; --i){
    ret << _lut[i-1];
  }
  return ret.str();
}


