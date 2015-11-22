
#include "Lut.h"
#include "LutUtils.h"

#include <cassert>
#include <stdexcept>

using namespace std;

bool Lut::equal(Lut const & b) const{
  if(inputCount() != b.inputCount()) return false;
  if(inputCount() < 6){ // Safe while I don't force the unused bits to 0
    return ((_lut[0] ^ b._lut[0]) & lutSizeMask[inputCount()]) == 0;
  }
  else{
    return _lut == b._lut;
  }
}

bool Lut::evaluate(unsigned inputValues) const{
  unsigned lutChunk = inputValues >> 6;
  assert(lutChunk < _lut.size());
  unsigned chunkInd = inputValues & 0x003f;
  return ((_lut[lutChunk] >> chunkInd) & 0x01) != 0;
}

void Lut::invert(){
  for(LutMask & cur : _lut){
    cur = ~cur;
  }
}

void Lut::setVal(unsigned inputValues, bool val){
  unsigned lutChunk = inputValues >> 6;
  assert(lutChunk < _lut.size());
  unsigned chunkInd = inputValues & 0x003f;
  if(val) _lut[lutChunk] |=  (one << chunkInd);
  else    _lut[lutChunk] &= ~(one << chunkInd);
}

bool Lut::isGeneralizedAnd() const{
  // Check if either only one bit is set or only one bit is unset
  if(inputCount() == 0){
    return true;
  }
  else if(inputCount() <= 6){
    uint64_t pv =  _lut[0] & lutSizeMask[inputCount()];
    uint64_t nv = ~_lut[0] & lutSizeMask[inputCount()];
    bool pand = pv != allZero && (pv & (pv-1)) == allZero;
    bool nand = nv != allZero && (nv & (nv-1)) == allZero;
    return pand || nand;
  }
  else{
    size_t zeros=0, ones=0, pands=0, nands=0;
    for(uint64_t cur : _lut){
      if(cur == allOne) ++ones;
      else if((~cur & (~cur-1)) == allZero) ++nands;
      if(cur == allZero) ++zeros;
      else if((cur & (cur-1)) == allZero) ++pands;
    }
    return (ones  == 0 && nands == 0 && pands == 1)
        || (zeros == 0 && pands == 0 && nands == 1);
  }
}

bool Lut::isGeneralizedXor() const{
  return equal(Xor(inputCount())) || equal(Exor(inputCount()));
}

void Lut::invertInput(unsigned input){
  if(input < 6){
    for(LutMask & cur : _lut){
      LutMask lowerPart = cur & ~lutInputMask[input]; 
      LutMask upperPart = cur &  lutInputMask[input];
      int shiftAmount = 1<<input;
      cur = lowerPart << shiftAmount | upperPart >> shiftAmount;
    }
  }
  else{
    size_t const stride = 1<<(input-6);
    for(size_t i=0; i<_lut.size(); i += 2*stride){
      for(size_t j=i; j<i+stride; ++j){
        std::swap(_lut[j], _lut[j+stride]);
      }
    }
  }
}

void Lut::swapInputs(unsigned i1, unsigned i2){
  if(i1 == i2) return;
  if(i1 >= inputCount() && i2 >= inputCount()) throw std::logic_error("Inputs to swap must be valid inputs");

  Lut ret(inputCount());
  for(unsigned inMask=0; inMask < 1u<<inputCount(); ++inMask){
    unsigned bit1Sel = 1u << i1;
    unsigned bit2Sel = 1u << i2;
    unsigned swappedMask = inMask & ~bit1Sel & ~bit2Sel;
    if((bit1Sel & inMask) != 0u) swappedMask |= bit2Sel;
    if((bit2Sel & inMask) != 0u) swappedMask |= bit1Sel;
    ret.setVal(swappedMask, evaluate(inMask));
  }
  std::swap(ret._lut, _lut);
}

void Lut::swapToEnd(unsigned input){
  swapInputs(inputCount()-1, input);
}

Lut Lut::getCofactor(unsigned input, bool value) const{
  assert(input < inputCount());
  Lut ret = *this;
  if(input < 6){
    unsigned shiftAmount = 1 << input;
    LutMask inputMask = value? lutInputMask[input] : ~lutInputMask[input];
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
    return (acc & lutInputMask[input]) == allZero;
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
    return (acc | ~lutInputMask[input]) == allOne;
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
    LutMask inputMask = inVal? lutInputMask[input] : ~lutInputMask[input];
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

