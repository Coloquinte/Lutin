
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
  void checkInputCounts(LutRef const & a, LutRef const & b){
    if(a.inputCount() != b.inputCount()){
      throw std::logic_error("Luts have different input counts");
    }
  }
}

LutRef::LutRef(unsigned inputs, LutMask* pt)
: _inputCnt(inputs)
, _lut(pt) {
}

// Quick and dirty for now
bool LutRef::isConstant() const { return isGnd() || isVcc(); }
bool LutRef::isConstant(bool val) const { return val ? isVcc() : isGnd(); }
bool LutRef::isGnd  () const { return equal(Lut::Gnd(inputCount())); }
bool LutRef::isVcc  () const { return equal(Lut::Vcc(inputCount())); }
bool LutRef::isAnd  () const { return equal(Lut::And(inputCount())); }
bool LutRef::isOr   () const { return equal(Lut::Or(inputCount())); }
bool LutRef::isNand () const { return equal(Lut::Nand(inputCount())); }
bool LutRef::isNor  () const { return equal(Lut::Nor(inputCount())); }
bool LutRef::isXor  () const { return equal(Lut::Xor(inputCount())); }
bool LutRef::isExor () const { return equal(Lut::Exor(inputCount())); }

void LutRef::setGnd(){
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = allZero;
  }
}
void LutRef::setVcc(){
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = allOne;
  }
}
void LutRef::setAnd(){
  if(inputCount() <= 6){
    _lut[0] = one << ((1<<inputCount())-1);
  }
  else{
    setGnd();
    _lut[arraySize()-1] = one<<63;
  }
}
void LutRef::setOr(){
  setVcc();
  _lut[0] = ~one;
}
void LutRef::setNand(){
  setAnd();
  invert();
}
void LutRef::setNor(){
  setOr();
  invert();
}
void LutRef::setXor(){
  _lut[0] = xorMask;
  if(inputCount()>6){
    for(unsigned i=0; i<inputCount()-6; ++i){
      unsigned curMaskSize = 1<<i;
      for(unsigned j=0; j<curMaskSize; ++j){
        _lut[curMaskSize+j] = ~_lut[j];
      }
    }
  }
}
void LutRef::setExor(){
  setXor();
  invert();
}

void LutRef::setWire(unsigned wireInput, bool invert){
  if(wireInput >= inputCount()){
    throw std::logic_error("The specified wire input is not in the Lut's input range");
  }

  if(wireInput < 6){
    LutMask szMask = inputCount() >= 6 ? lutSizeMask[6] : lutSizeMask[inputCount()];
    LutMask msk = invert ? ~lutInputMask[wireInput] : lutInputMask[wireInput];
    for(unsigned i=0; i<arraySize(); ++i){
      _lut[i] = msk & szMask;
    }
  }
  else{
    setGnd();
    unsigned const stride = 1<<(wireInput-6);
    for(unsigned i= invert ? 0 : stride; i<arraySize(); i += 2*stride){
      for(unsigned j=i; j<i+stride; ++j){
        _lut[j] = allOne;
      }
    }
  }
}

void LutRef::setBuf(unsigned wireInput){
  setWire(wireInput, false);
}

void LutRef::setInv(unsigned wireInput){
  setWire(wireInput, true );
}

LutRef& LutRef::operator=(LutRef const & a){
  checkInputCounts(*this, a);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = a._lut[i];
  }
  return *this;
}

void LutRef::setNot(LutRef const & a){
  checkInputCounts(*this, a);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = ~a._lut[i];
  }
}

void LutRef::setAnd(LutRef const & a, LutRef const & b){
  checkInputCounts(*this, a);
  checkInputCounts(a, b);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = a._lut[i] & b._lut[i];
  }
}

void LutRef::setOr(LutRef const & a, LutRef const & b){
  checkInputCounts(*this, a);
  checkInputCounts(a, b);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = a._lut[i] | b._lut[i];
  }
}

void LutRef::setNor(LutRef const & a, LutRef const & b){
  checkInputCounts(*this, a);
  checkInputCounts(a, b);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = ~(a._lut[i] | b._lut[i]);
  }
}

void LutRef::setNand(LutRef const & a, LutRef const & b){
  checkInputCounts(*this, a);
  checkInputCounts(a, b);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = ~(a._lut[i] & b._lut[i]);
  }
}

void LutRef::setXor(LutRef const & a, LutRef const & b){
  checkInputCounts(*this, a);
  checkInputCounts(a, b);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = a._lut[i] ^ b._lut[i];
  }
}

void LutRef::setExor(LutRef const & a, LutRef const & b){
  checkInputCounts(*this, a);
  checkInputCounts(a, b);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = ~(a._lut[i] ^ b._lut[i]);
  }
}

void LutRef::operator&=(LutRef const & o){
  checkInputCounts(*this, o);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] &= o._lut[i];
  }
}

void LutRef::operator|=(LutRef const & o){
  checkInputCounts(*this, o);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] |= o._lut[i];
  }
}

void LutRef::operator^=(LutRef const & o){
  checkInputCounts(*this, o);
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] ^= o._lut[i];
  }
}

void LutRef::initFromStr(string const & init){
  unsigned s = 2;
  while( (1lu << (s-2)) < init.size()) ++s;
  if(init.size() != (1lu << (s-2))) throw std::logic_error("Bad string size as init of the Lut: not a power of two");
  if(s != inputCount()) throw std::logic_error("Cannot initialize LutRef from a string of different size");

  if(_inputCnt != s) throw std::logic_error("Init size is not compatible with the Lut's size");

  for(unsigned i=0; i<arraySize(); ++i){
    LutMask val = 0ul;
    for(unsigned j=i*16; j<i*16+16 && j<init.size(); ++j){
      LutMask cur = 0ul;
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
    _lut[arraySize()-1-i] = val;
  }
}

string LutRef::str() const{
  if(inputCount() == 0) return string();
  stringstream ret;
  ret << std::hex;

  int fillWidth;
  if(inputCount() >= 6) fillWidth = 16;
  else if(inputCount() <= 2) fillWidth = 1;
  else fillWidth = (1 << (inputCount()-2));

  LutMask mask = inputCount() >= 6 ? lutSizeMask[6] : lutSizeMask[inputCount()];
  
  for(unsigned i=arraySize(); i>0; --i){
    ret << setfill('0') << setw(fillWidth);
    ret << (mask & _lut[i-1]);
  }
  return ret.str();
}


