
#include "LutRef.h"
#include "Lut.h"
#include "LutUtils.h"

#include <cassert>
#include <bitset>

using namespace std;

void LutRef::invert(){
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = ~_lut[i];
  }
}

void LutRef::setToInverted(LutRef const & o){
  checkInputCounts(*this, o);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = ~o._lut[i];
  }
}

bool LutRef::isEqual(LutRef const & b) const{
  if(inputCount() != b.inputCount()) return false;
  for(unsigned i=0; i<arraySize(); ++i){
    LutMask diff = (_lut[i] ^ b._lut[i]) & getSizeMask(inputCount());
    if(diff != allZero) return false;
  }
  return true;
}

bool LutRef::isInverse(LutRef const & b) const{
  if(inputCount() != b.inputCount()) return false;
  for(unsigned i=0; i<arraySize(); ++i){
    LutMask diff = (_lut[i] ^ b._lut[i]) | ~getSizeMask(inputCount());
    if(diff != allOne) return false;
  }
  return true;
}

bool LutRef::evaluate(std::size_t inputValues) const{
  checkInputMask(*this, inputValues);

  unsigned lutChunk = inputValues >> 6;
  assert(lutChunk < arraySize());

  unsigned chunkInd = inputValues & 0x003f;
  return ((_lut[lutChunk] >> chunkInd) & 0x01) != 0;
}

void LutRef::setVal(std::size_t inputValues, bool val){
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
  return isEqual(Lut::Xor(inputCount())) || isEqual(Lut::Exor(inputCount()));
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

void LutRef::setToInvertedInput(LutRef const & o, unsigned input){
  checkInputCounts(*this, o);
  operator=(o);
  invertInput(input);
}

void LutRef::setToSwappedInputs(LutRef const & a, unsigned i1, unsigned i2){
  checkInputCounts(*this, a);

  operator=(a);
  swapInputs(i1, i2);
}

void LutRef::swapInputs(unsigned i1, unsigned i2){
  checkInput(*this, i1);
  checkInput(*this, i2);

  if(i1 == i2){
    return;
  }
  else if(i1 == 0){
    swapToBegin(i2);
  }
  else if(i2 == 0){
    swapToBegin(i1);
  }
  else{
    swapToBegin(i1);
    swapToBegin(i2);
    swapToBegin(i1);
  }
}

void LutRef::swapToBegin(unsigned input){
  checkInput(*this, input);

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

bool LutRef::hasDC() const {
  for(unsigned i=0; i<inputCount(); ++i){
    if(getSimplification(i) == Simplification::OneInput::DC) return true;
  }
  return false;
}

bool LutRef::hasSingleInputFactorization() const {
  using namespace Simplification;
  for(unsigned i=0; i<inputCount(); ++i){
    OneInput cur = getSimplification(i);
    if(cur != OneInput::None && cur != OneInput::DC){
      return true;
    }
  }
  return false;
}

bool LutRef::hasTwoInputFactorization() const {
  using namespace Simplification;
  for(unsigned i=0; i+1<inputCount(); ++i){
    for(unsigned j=i+1; j<inputCount(); ++j){
      TwoInput cur = getSimplification(i, j);
      if(cur != TwoInput::None && cur != TwoInput::Symm && cur != TwoInput::SymmInv){
        return true;
      }
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

Simplification::OneInput LutRef::getSimplification(unsigned input) const {
  checkInput(*this, input);

  Lut posCofactor = Lut::Cofactor(*this, input, true);
  Lut negCofactor = Lut::Cofactor(*this, input, false);

  if(posCofactor.isEqual(negCofactor))         return Simplification::OneInput::DC;
  else if(posCofactor.isInverse(negCofactor)) return Simplification::OneInput::Toggles;
  else if(negCofactor.isGnd())                return Simplification::OneInput::F00;
  else if(negCofactor.isVcc())                return Simplification::OneInput::F01;
  else if(posCofactor.isGnd())                return Simplification::OneInput::F10;
  else if(posCofactor.isVcc())                return Simplification::OneInput::F11;
  else                                        return Simplification::OneInput::None;
}

Simplification::TwoInput LutRef::getSimplification(unsigned i1, unsigned i2) const {
  Lut pos = Lut::Cofactor(*this, i1, true);
  Lut neg = Lut::Cofactor(*this, i1, false);

  Lut pospos = Lut::Cofactor(pos, i2, true );
  Lut posneg = Lut::Cofactor(pos, i2, false);
  Lut negpos = Lut::Cofactor(neg, i2, true );
  Lut negneg = Lut::Cofactor(neg, i2, false);

  // And factorization
  if(pos == negpos) return Simplification::TwoInput::SAnd00;
  if(pos == negneg) return Simplification::TwoInput::SAnd01;
  if(neg == pospos) return Simplification::TwoInput::SAnd10;
  if(neg == posneg) return Simplification::TwoInput::SAnd11;
  // Xor factorization
  if(posneg == negpos && pospos == negneg) return Simplification::TwoInput::SXor;
  // No factorization, but some symmetry
  if(posneg == negpos) return Simplification::TwoInput::Symm;
  if(pospos == negneg) return Simplification::TwoInput::SymmInv;
  // Nothing interesting
  return Simplification::TwoInput::None;
}



