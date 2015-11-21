
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
#include <iomanip>

using namespace std;

namespace{
  size_t mapSize(unsigned inputCnt) {
    return inputCnt >= 6 ? 1ul << (inputCnt-6) : 1ul;
  }

  void checkInputCounts(Lut const & a, Lut const & b){
    if(a.inputCount() != b.inputCount()){
      throw std::logic_error("Luts have different input counts");
    }
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

Lut Lut::Wire(unsigned wireInput, unsigned inputs, bool invert){
  if(wireInput >= inputs){
    throw std::logic_error("The specified wire input is not in the Lut's input range");
  }

  Lut ret(inputs);
  if(wireInput < 6){
    uint64_t szMask = inputs >= 6 ? lutSizeMask[6] : lutSizeMask[inputs];
    uint64_t msk = invert ? ~lutInputMask[wireInput] : lutInputMask[wireInput];
    for(uint64_t & cur : ret._lut){
      cur = msk & szMask;
    }
  }
  else{
    size_t const stride = 1<<(wireInput-6);
    for(size_t i= invert ? 0 : stride; i<ret._lut.size(); i += 2*stride){
      for(size_t j=i; j<i+stride; ++j){
        ret._lut[j] = allOne;
      }
    }
  }

  return ret;
}

Lut Lut::Buf(unsigned wireInput, unsigned inputs){
  return Wire(wireInput, inputs, false);
}

Lut Lut::Inv(unsigned wireInput, unsigned inputs){
  return Wire(wireInput, inputs, true );
}

Lut Lut::Not(Lut const & a){
  Lut ret = a;
  ret.invert();
  return ret;
}

Lut Lut::And(Lut const & a, Lut const & b){
  checkInputCounts(a, b);
  Lut ret(a.inputCount());
  assert(ret._lut.size() == a._lut.size() && ret._lut.size() == b._lut.size());
  for(size_t i=0; i<a._lut.size(); ++i){
    ret._lut[i] = a._lut[i] & b._lut[i];
  }
  return ret;
}

Lut Lut::Or(Lut const & a, Lut const & b){
  checkInputCounts(a, b);
  Lut ret(a.inputCount());
  assert(ret._lut.size() == a._lut.size() && ret._lut.size() == b._lut.size());
  for(size_t i=0; i<a._lut.size(); ++i){
    ret._lut[i] = a._lut[i] | b._lut[i];
  }
  return ret;
}

Lut Lut::Nor(Lut const & a, Lut const & b){
  checkInputCounts(a, b);
  Lut ret(a.inputCount());
  assert(ret._lut.size() == a._lut.size() && ret._lut.size() == b._lut.size());
  for(size_t i=0; i<a._lut.size(); ++i){
    ret._lut[i] = ~(a._lut[i] | b._lut[i]);
  }
  return ret;
}

Lut Lut::Nand(Lut const & a, Lut const & b){
  checkInputCounts(a, b);
  Lut ret(a.inputCount());
  assert(ret._lut.size() == a._lut.size() && ret._lut.size() == b._lut.size());
  for(size_t i=0; i<a._lut.size(); ++i){
    ret._lut[i] = ~(a._lut[i] & b._lut[i]);
  }
  return ret;
}

Lut Lut::Xor(Lut const & a, Lut const & b){
  checkInputCounts(a, b);
  Lut ret(a.inputCount());
  assert(ret._lut.size() == a._lut.size() && ret._lut.size() == b._lut.size());
  for(size_t i=0; i<a._lut.size(); ++i){
    ret._lut[i] = a._lut[i] ^ b._lut[i];
  }
  return ret;
}

Lut Lut::Exor(Lut const & a, Lut const & b){
  checkInputCounts(a, b);
  Lut ret(a.inputCount());
  assert(ret._lut.size() == a._lut.size() && ret._lut.size() == b._lut.size());
  for(size_t i=0; i<a._lut.size(); ++i){
    ret._lut[i] = ~(a._lut[i] ^ b._lut[i]);
  }
  return ret;
}

Lut::Lut(string const & init){
  unsigned s = 2;
  while( (1lu << (s-2)) < init.size()) ++s;
  if(init.size() != (1lu << (s-2))) throw std::logic_error("Bad string size as init of the Lut");

  _inputCnt = s;
  _lut.resize(mapSize(_inputCnt));

  for(size_t i=0; i<_lut.size(); ++i){
    uint64_t val = 0ul;
    for(size_t j=i*16; j<i*16+16 && j<init.size(); ++j){
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
    _lut[_lut.size()-1-i] = val;
  }
}

string Lut::str() const{
  if(inputCount() == 0) return string();
  stringstream ret;
  ret << std::hex;

  int fillWidth;
  if(inputCount() >= 6) fillWidth = 16;
  else if(inputCount() <= 2) fillWidth = 1;
  else fillWidth = (1 << (inputCount()-2));

  uint64_t mask = inputCount() >= 6 ? lutSizeMask[6] : lutSizeMask[inputCount()];
  
  for(auto it=_lut.rbegin(); it!=_lut.rend(); ++it){
    ret << setfill('0') << setw(fillWidth);
    ret << (mask & *it);
  }
  return ret.str();
}


