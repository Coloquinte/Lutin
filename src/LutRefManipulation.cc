
#include "LutRef.h"
#include "Lut.h"
#include "LutUtils.h"

#include <cassert>
#include <bitset>

using namespace std;

bool LutRef::equal(LutRef const & b) const{
  if(inputCount() != b.inputCount()) return false;
  if(inputCount() <= 6){ // Safe while I don't force the unused bits to 0
    return ((_lut[0] ^ b._lut[0]) & lutSizeMask[inputCount()]) == 0;
  }
  else{
    for(unsigned i=0; i<arraySize(); ++i){
      if(_lut[i] != b._lut[i]) return false;
    }
    return true;
  }
}

bool LutRef::evaluate(unsigned inputValues) const{
  checkInputMask(*this, inputValues);

  unsigned lutChunk = inputValues >> 6;
  assert(lutChunk < arraySize());

  unsigned chunkInd = inputValues & 0x003f;
  return ((_lut[lutChunk] >> chunkInd) & 0x01) != 0;
}

void LutRef::invert(){
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = ~_lut[i];
  }
}

void LutRef::setVal(unsigned inputValues, bool val){
  checkInputMask(*this, inputValues);

  unsigned lutChunk = inputValues >> 6;
  assert(lutChunk < arraySize());

  unsigned chunkInd = inputValues & 0x003f;
  if(val) _lut[lutChunk] |=  (one << chunkInd);
  else    _lut[lutChunk] &= ~(one << chunkInd);
}

bool LutRef::isGeneralizedAnd() const{
  // Check if either only one bit is set or only one bit is unset
  if(inputCount() == 0){
    return true;
  }
  else if(inputCount() <= 6){
    LutMask pv =  _lut[0] & lutSizeMask[inputCount()];
    LutMask nv = ~_lut[0] & lutSizeMask[inputCount()];
    bool pand = pv != allZero && (pv & (pv-1)) == allZero;
    bool nand = nv != allZero && (nv & (nv-1)) == allZero;
    return pand || nand;
  }
  else{
    unsigned zeros=0, ones=0, pands=0, nands=0;
    for(unsigned i=0; i<arraySize(); ++i){
      LutMask cur = _lut[i];
      if(cur == allOne) ++ones;
      else if((~cur & (~cur-1)) == allZero) ++nands;
      if(cur == allZero) ++zeros;
      else if((cur & (cur-1)) == allZero) ++pands;
    }
    return (ones  == 0 && nands == 0 && pands == 1)
        || (zeros == 0 && pands == 0 && nands == 1);
  }
}

bool LutRef::isGeneralizedXor() const{
  return equal(Lut::Xor(inputCount())) || equal(Lut::Exor(inputCount()));
}

void LutRef::invertInput(unsigned input){
  checkInput(*this, input);

  if(input < 6){
    for(unsigned i=0; i<arraySize(); ++i){
      LutMask lowerPart = _lut[i] & ~lutInputMask[input]; 
      LutMask upperPart = _lut[i] &  lutInputMask[input];
      int shiftAmount = 1<<input;
      _lut[i] = lowerPart << shiftAmount | upperPart >> shiftAmount;
    }
  }
  else{
    unsigned const stride = 1<<(input-6);
    for(unsigned i=0; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        std::swap(_lut[j], _lut[j+stride]);
      }
    }
  }
}

void LutRef::setToSwappedInputs(LutRef const & a, unsigned i1, unsigned i2){
  checkInputCounts(*this, a);

  if(i1 == i2){
    operator=(a);
  }
  checkInput(*this, i1);
  checkInput(*this, i2);

  for(unsigned inMask=0; inMask < 1u<<inputCount(); ++inMask){
    unsigned bit1Sel = 1u << i1;
    unsigned bit2Sel = 1u << i2;
    unsigned swappedMask = inMask & ~bit1Sel & ~bit2Sel;
    if((bit1Sel & inMask) != 0u) swappedMask |= bit2Sel;
    if((bit2Sel & inMask) != 0u) swappedMask |= bit1Sel;
    setVal(swappedMask, a.evaluate(inMask));
  }
}

void LutRef::swapInputs(unsigned i1, unsigned i2){
  Lut ret(inputCount());
  ret.setToSwappedInputs(*this, i1, i2);
  operator=(ret);
}

void LutRef::swapToBegin(unsigned input){
  checkInput(*this, input);

  Lut reference = Lut::SwappedInputs(*this, 0, input);

  // Four possibilities for each bit: it is shifted differently depending on the two cofactors it is in
  if(input >= 6){
    unsigned const stride = 1<<(input-6);
    for(unsigned i=0; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        LutMask f00 = _lut[j       ] & ~lutInputMask[0];
        LutMask f01 = _lut[j       ] &  lutInputMask[0];
        LutMask f10 = _lut[j+stride] & ~lutInputMask[0];
        LutMask f11 = _lut[j+stride] &  lutInputMask[0];
        _lut[j       ] = f00 | (f10 << 1);
        _lut[j+stride] = f11 | (f01 >> 1);
      }
    }
  }
  else{
    unsigned shiftAmount = 1u << input;
    for(unsigned i=0; i<arraySize(); ++i){
      LutMask f00 = _lut[i] & (~lutInputMask[input] & ~lutInputMask[0]);
      LutMask f01 = _lut[i] & (~lutInputMask[input] &  lutInputMask[0]);
      LutMask f10 = _lut[i] & ( lutInputMask[input] & ~lutInputMask[0]);
      LutMask f11 = _lut[i] & ( lutInputMask[input] &  lutInputMask[0]);

      LutMask nf01 = (f10 >> (shiftAmount-1));
      LutMask nf10 = (f01 << (shiftAmount-1));

      _lut[i] = f00 | f11 | nf01 | nf10;

      assert((nf01 & nf10) == 0); 
      assert((nf01 & f11 ) == 0); 
      assert((nf01 & f00 ) == 0); 
      assert((nf10 & f11 ) == 0); 
      assert((nf10 & f00 ) == 0); 
    }
  }
  assert(equal(reference));
}

void LutRef::setToCofactor(LutRef const & o, unsigned input, bool value){
  operator=(o);
  setToCofactor(input, value);
}

void LutRef::setToCofactor(unsigned input, bool value){
  assert(input < inputCount());
  if(input < 6){
    unsigned shiftAmount = 1 << input;
    LutMask inputMask = value? lutInputMask[input] : ~lutInputMask[input];
    for(unsigned i=0; i<arraySize(); ++i){
      LutMask maskedVal = _lut[i] & inputMask;
      // The shift is different depending on the input mask
      if(value) _lut[i] = maskedVal | (maskedVal >> shiftAmount);
      else      _lut[i] = maskedVal | (maskedVal << shiftAmount);
    }
  }
  else{
    unsigned const stride = 1<<(input-6);
    for(unsigned i=0; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        if(value) _lut[j] = _lut[j+stride];
        else      _lut[j+stride] = _lut[j];
      }
    }
  }
}

void LutRef::setFromCofactors(LutRef const & neg, LutRef const & pos, unsigned input){
  assert(input < inputCount());
  if(input < 6){
    for(unsigned i=0; i<arraySize(); ++i){
      _lut[i] = (neg._lut[i] & ~lutInputMask[input]) | (pos._lut[i] & lutInputMask[input]);
    }
  }
  else{
    unsigned const stride = 1<<(input-6);
    for(unsigned i=0; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        _lut[j] = neg._lut[j];
      }
      for(unsigned j=i+stride; j<i+2*stride; ++j){
        _lut[j] = pos._lut[j];
      }
    }
  } 
}

void LutRef::setToCompactCofactor(LutRef const & o, unsigned input, bool value){
  checkSizeMinusOne(o, *this);
  checkInput(o, input);

  setGnd();
  Lut tmp = Lut::SwappedInputs(o, input, o.inputCount()-1);
  size_t offs = value ? bitCount() : 0;
  for(size_t i=0; i<bitCount(); ++i){
    setVal(i, tmp.evaluate(offs + i));
  }
}

void LutRef::setFromCompactCofactors(LutRef const & neg, LutRef const & pos, unsigned input){
  checkSizeMinusOne(*this, pos);
  checkSizeMinusOne(*this, neg);
  checkInput(*this, input);
 
  setGnd();
  for(size_t i=0; i<bitCount(inputCount()-1); ++i){
    setVal(i, neg.evaluate(i));
    setVal(i + bitCount(inputCount()-1), pos.evaluate(i));
  }
  swapInputs(inputCount()-1, input);
}

// The 3 following algorithms use a bitmask accumulator rather than an early exit: it is believed - but not tested - to be faster, with fewer instructions and better branch prediction

bool LutRef::isDC(unsigned input) const{
  assert(input < inputCount());
  if(input < 6){
    LutMask acc = allZero;
    for(unsigned i=0; i<arraySize(); ++i){
      acc |= (_lut[i] << (1<<input)) ^ _lut[i];
    }
    return (acc & lutInputMask[input]) == allZero;
  }
  else{
    LutMask acc = allZero;
    unsigned const stride = 1<<(input-6);
    for(unsigned i=0; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        acc |= (_lut[j] ^ _lut[j+stride]);
      }
    }
    return acc == allZero;
  }
}

bool LutRef::toggles(unsigned input) const{
  assert(input < inputCount());
  if(input < 6){
    LutMask acc = allOne;
    for(unsigned i=0; i<arraySize(); ++i){
      acc &= (_lut[i] << (1<<input)) ^ _lut[i];
    }
    LutMask excludeMask = ~lutInputMask[input] | ~getSizeMask(inputCount());
    return (acc | excludeMask) == allOne;
  }
  else{
    LutMask acc = allOne;
    unsigned const stride = 1<<(input-6);
    for(unsigned i=0; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        acc &= (_lut[j] ^ _lut[j+stride]);
      }
    }
    return acc == allOne;
  }
}

bool LutRef::forcesValue(unsigned input, bool inVal, bool outVal) const{
  assert(input < inputCount());
  LutMask comp = outVal? allOne : allZero;
  if(input<6){
    LutMask inputMask = inVal? lutInputMask[input] : ~lutInputMask[input];
    LutMask excludeMask = inputMask & getSizeMask(inputCount());
    LutMask acc = allZero;
    for(unsigned i=0; i<arraySize(); ++i){
      acc |= (_lut[i] ^ comp);
    }
    return (acc & excludeMask) == allZero;
  }
  else{
    LutMask acc = allZero;
    unsigned const stride = 1<<(input-6);
    for(unsigned i=inVal? stride : 0; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        acc |= (_lut[j] ^ comp);
      }
    }
    return acc == allZero;
  }
}

bool LutRef::hasDC() const {
  for(unsigned i=0; i<inputCount(); ++i){
    if(isDC(i)) return true;
  }
  return false;
}

bool LutRef::isUnate(unsigned input) const {
  return isUnate(input, false) || isUnate(input, true);
}

bool LutRef::isBinate(unsigned input) const {
  return !isUnate(input);
}

bool LutRef::isUnate(unsigned input, bool polarity) const {
  Lut negCofactor = Lut::Cofactor(*this, input, false);
  Lut posCofactor = Lut::Cofactor(*this, input, true );
  if(polarity){
    return (~negCofactor | posCofactor).isVcc();
  }
  else{
    return (negCofactor | ~posCofactor).isVcc();
  }
}

std::size_t LutRef::countUnate(unsigned input) const{
  return std::max(countUnate(input, true), countUnate(input, false));
}

std::size_t LutRef::countUnate(unsigned input, bool polarity) const{
  Lut negCofactor = Lut::Cofactor(*this, input, false);
  Lut posCofactor = Lut::Cofactor(*this, input, true );
  if(polarity){
    return (~negCofactor | posCofactor).countSetBits();
  }
  else{
    return (negCofactor | ~posCofactor).countSetBits();
  }
}

bool LutRef::hasSingleInputFactorization() const {
  for(unsigned i=0; i<inputCount(); ++i){
    if(toggles(i)) return true;
    if(forcesValue(i, false, false)) return true;
    if(forcesValue(i, false, true )) return true;
    if(forcesValue(i, true , false)) return true;
    if(forcesValue(i, true , true )) return true;
  }
  return false;
}

bool LutRef::hasTwoInputFactorization() const {
  Lut pos(inputCount());
  Lut neg(inputCount());
  Lut pospos(inputCount());
  Lut posneg(inputCount());
  Lut negpos(inputCount());
  Lut negneg(inputCount());
  for(unsigned i=0; i+1<inputCount(); ++i){
    pos.setToCofactor(*this, i, true);
    neg.setToCofactor(*this, i, false);
    for(unsigned j=i+1; j<inputCount(); ++j){
      pospos.setToCofactor(pos, j, true );
      posneg.setToCofactor(pos, j, false);
      negpos.setToCofactor(neg, j, true );
      negneg.setToCofactor(neg, j, false);
      // Now if there are only two possible cofactors we can factor it away
      // Just forget about the DC cases
      // First the And/Or type
      if(pos == negneg || pos == negpos) return true;
      if(neg == posneg || neg == pospos) return true;
      // Then the Xor type
      if(posneg == negpos && pospos == negneg) return true;
    }
  }
  return false;
}

bool LutRef::isPseudoRepresentant() const{
  Lut tmp1(inputCount()), tmp2(inputCount());

  // A pseudo-representant has half the bits or more set
  if(2*countSetBits() < bitCount()){
    return false;
  }

  // For each input, the 1 cofactor has at least as many bit sets than the 0 cofactor
  for(unsigned input=0; input<inputCount(); ++input){
    tmp1.setToCofactor(*this, input, false);
    tmp2.setToCofactor(*this, input, true);
    if(tmp1.countSetBits() > tmp2.countSetBits()){
      return false;
    }
  }

  // The inputs are sorted in increasing order of set bits in the 1 cofactor
  for(unsigned input=0; input+1<inputCount(); ++input){
    tmp1.setToCofactor(*this, input, true);
    tmp2.setToCofactor(*this, input+1, true);
    std::size_t cnt1 = tmp1.countSetBits(), cnt2 = tmp2.countSetBits();
    if(cnt1 > cnt2){
      return false;
    }
    else if(cnt1 == cnt2 && countUnate(input, true) < countUnate(input+1, true)){
      return false;
    }
  }

  return true;
}

void LutRef::setToPseudoRepresentant(LutRef const & o){
  operator=(o);
  setToPseudoRepresentant();
}

void LutRef::setToPseudoRepresentant(){
  Lut tmp1(inputCount()), tmp2(inputCount());

  // Invert the output if necessary to obtain majority of bit sets
  if(2*countSetBits() < bitCount()){
    invert();
  }

  // Invert the inputs to obtain majority of bit sets on each input
  for(unsigned input=0; input<inputCount(); ++input){
    tmp1.setToCofactor(*this, input, false);
    tmp2.setToCofactor(*this, input, true);
    if(tmp1.countSetBits() > tmp2.countSetBits()){
      invertInput(input);
    }
  }

  // Quadratic sort of the inputs in increasing order of set bits
  for(unsigned end=inputCount(); end>=2; --end){
    for(unsigned input=0; input+1<end; ++input){
      tmp1.setToCofactor(*this, input, true);
      tmp2.setToCofactor(*this, input+1, true);
      std::size_t cnt1 = tmp1.countSetBits(), cnt2 = tmp2.countSetBits();
      if(cnt1 > cnt2){
        swapInputs(input, input+1);
      }
      else if(cnt1 == cnt2 && countUnate(input, true) < countUnate(input+1, true)){
        swapInputs(input, input+1);
      }
    }
  }
  assert(isPseudoRepresentant());
}

size_t LutRef::countSetBits() const{
  size_t ret = 0;
  LutMask szMask = getSizeMask(inputCount());
  for(unsigned i=0; i<arraySize(); ++i){
    bitset<64> bs(_lut[i] & szMask);
    ret += bs.count();
  }
  return ret;
}

std::size_t LutRef::getHash() const {
  std::size_t ret = 0;
  LutMask szMask = getSizeMask(inputCount());
  for(unsigned i=0; i<arraySize(); ++i){
    ret ^= (_lut[i] & szMask);
  }
  return ret;
}

