
#include "Lut.h"

#include <cassert>

using namespace std;

Lut::Lut(unsigned inputCount) 
: LutRef(inputCount, new uint64_t[arraySize(inputCount)]) {
}

Lut::Lut(unsigned inputCount, unsigned long mask) : Lut(inputCount) {
  assert(inputCount <= 6);
  _lut[0] = mask;
}

Lut::Lut(Lut const & o) : Lut(o.inputCount()){
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = o._lut[i];
  }
}

Lut::Lut(Lut&& o) : LutRef(o._inputCnt, o._lut){
}

Lut& Lut::operator=(LutRef const o){
  setInputCount(o.inputCount());
  LutRef::operator=(o);
  return *this;
}

Lut& Lut::operator=(Lut&& o){
  delete[] _lut;
  _inputCnt = o._inputCnt;
  _lut = o._lut;
  return *this;
}

Lut::~Lut(){
  delete[] _lut;
}

void Lut::setInputCount(unsigned inputCnt){
  LutMask* tmp = new LutMask[arraySize(inputCnt)];
  for(unsigned i=0; i<arraySize(min(inputCnt, inputCount())); ++i){
    tmp[i] = _lut[i];
  }
  delete[] _lut;
  _lut = tmp;
  _inputCnt = inputCnt;
}

namespace{
  unsigned getLutSize(string const & init){
    unsigned lutSize = 2;
    while( (1u<<(lutSize-2)) < init.size()){
      ++lutSize;
    }
    return lutSize;
  }
}

Lut::Lut(string const & init) : Lut(getLutSize(init)){
  initFromStr(init);
}

Lut Lut::Gnd  (unsigned inputs) { Lut ret(inputs); ret.setGnd  (); return ret;}
Lut Lut::Vcc  (unsigned inputs) { Lut ret(inputs); ret.setVcc  (); return ret;}
Lut Lut::And  (unsigned inputs) { Lut ret(inputs); ret.setAnd  (); return ret;}
Lut Lut::Or   (unsigned inputs) { Lut ret(inputs); ret.setOr   (); return ret;}
Lut Lut::Nand (unsigned inputs) { Lut ret(inputs); ret.setNand (); return ret;}
Lut Lut::Nor  (unsigned inputs) { Lut ret(inputs); ret.setNor  (); return ret;}
Lut Lut::Xor  (unsigned inputs) { Lut ret(inputs); ret.setXor  (); return ret;}
Lut Lut::Exor (unsigned inputs) { Lut ret(inputs); ret.setExor (); return ret;}
Lut Lut::Wire (unsigned wireInput, unsigned inputs, bool invert) { Lut ret(inputs); ret.setWire(wireInput, invert); return ret; }
Lut Lut::Buf  (unsigned wireInput, unsigned inputs) { Lut ret(inputs); ret.setBuf(wireInput); return ret; }
Lut Lut::Inv  (unsigned wireInput, unsigned inputs) { Lut ret(inputs); ret.setInv(wireInput); return ret; }

// Operations on same-size Luts with a common input set
Lut Lut::Not  (LutRef const a) { Lut ret(a.inputCount()); ret.setNot(a); return ret;}
Lut Lut::And  (LutRef const a, LutRef const b) { Lut ret(a.inputCount()); ret.setAnd  (a, b); return ret; }
Lut Lut::Or   (LutRef const a, LutRef const b) { Lut ret(a.inputCount()); ret.setOr   (a, b); return ret; }
Lut Lut::Nand (LutRef const a, LutRef const b) { Lut ret(a.inputCount()); ret.setNand (a, b); return ret; }
Lut Lut::Nor  (LutRef const a, LutRef const b) { Lut ret(a.inputCount()); ret.setNor  (a, b); return ret; }
Lut Lut::Xor  (LutRef const a, LutRef const b) { Lut ret(a.inputCount()); ret.setXor  (a, b); return ret; }
Lut Lut::Exor (LutRef const a, LutRef const b) { Lut ret(a.inputCount()); ret.setExor (a, b); return ret; }

Lut Lut::getCofactor(unsigned input, bool value) const{
  Lut ret(inputCount());
  ret.setToCofactor(*this, input, value);
  return ret;
}

Lut Lut::getPseudoRepresentant() const{
  Lut ret(inputCount());
  ret.setToPseudoRepresentant(*this);
  return ret;
}

