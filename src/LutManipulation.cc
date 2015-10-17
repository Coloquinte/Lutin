
#include "Lut.h"
#include "LutUtils.h"

#include <cassert>

using namespace std;

Lut::Lut(unsigned inputs)
: _inputCnt(inputs)
, _lut(inputs <= 6? 1 : 1 << (inputs-6), allZero)
{
}

bool Lut::equal(Lut const & a, Lut const & b){
  if(a.inputCount() != b.inputCount()) return false;
  if(a.inputCount() < 6){ // Safe while I don't force the unused bits to 0
    return ((a._lut[0] ^ b._lut[0]) & sizeMask[a.inputCount()]) == 0;
  }
  else{
    return a._lut == b._lut;
  }
}

bool Lut::evaluate(unsigned inputValues) const{
  unsigned lutChunk = inputValues >> 6;
  assert(lutChunk < _lut.size());
  unsigned chunkInd = inputValues & 0x003f;
  return ((_lut[lutChunk] >> chunkInd) & 0x01) != 0;
}

Lut Lut::getCofactor(unsigned input, bool value) const{
  assert(input < inputCount());
  Lut ret = *this;
  if(input < 6){
    unsigned shiftAmount = 1 << input;
    LutMask inputMask = value? mask[input] : ~mask[input];
    for(LutMask & cur : ret._lut){
      LutMask maskedVal = cur & inputMask;
      // The shift is different depending on the input mask
      if(value) cur = maskedVal | (maskedVal >> shiftAmount);
      else      cur = maskedVal | (maskedVal << shiftAmount);
    }
  }
  else{
    size_t const stride = 1<<(input-6);
    for(size_t i=0; i<_lut.size(); i += 2*stride){
      for(size_t j=i; j<i+stride; ++j){
        if(value) ret._lut[j] = _lut[j+stride];
        else      ret._lut[j+stride] = _lut[j];
      }
    }
  }
  return ret;
}


// The 3 following algorithms use a bitmask accumulator rather than an early exit: it is believed - but not tested - to be faster, with fewer instructions and better branch prediction

bool Lut::isDC(unsigned input) const{
  assert(input < inputCount());
  if(input < 6){
    LutMask acc = allZero;
    for(LutMask cur : _lut){
      acc |= (cur << (1<<input)) ^ cur;
    }
    return (acc & mask[input]) == allZero;
  }
  else{
    LutMask acc = allZero;
    size_t const stride = 1<<(input-6);
    for(size_t i=0; i<_lut.size(); i += 2*stride){
      for(size_t j=i; j<i+stride; ++j){
        acc |= (_lut[j] ^ _lut[j+stride]);
      }
    }
    return acc == allZero;
  }
}

bool Lut::toggles(unsigned input) const{
  assert(input < inputCount());
  if(input < 6){
    LutMask acc = allOne;
    for(LutMask cur : _lut){
      acc &= (cur << (1<<input)) ^ cur;
    }
    return (acc | ~mask[input]) == allOne;
  }
  else{
    LutMask acc = allOne;
    size_t const stride = 1<<(input-6);
    for(size_t i=0; i<_lut.size(); i += 2*stride){
      for(size_t j=i; j<i+stride; ++j){
        acc &= (_lut[j] ^ _lut[j+stride]);
      }
    }
    return acc == allOne;
  }
}

bool Lut::forcesValue(unsigned input, bool inVal, bool outVal) const{
  assert(input < inputCount());
  LutMask comp = outVal? allOne : allZero;
  if(input<6){
    LutMask inputMask = inVal? mask[input] : ~mask[input];
    LutMask acc = allZero;
    for(LutMask cur : _lut){
      acc |= (cur ^ comp);
    }
    return (acc & inputMask) == allZero;
  }
  else{
    LutMask acc = allZero;
    size_t const stride = 1<<(input-6);
    for(size_t i=inVal? stride : 0; i<_lut.size(); i += 2*stride){
      for(size_t j=i; j<i+stride; ++j){
        acc |= (_lut[j] ^ comp);
      }
    }
    return acc == allZero;
  }
}

