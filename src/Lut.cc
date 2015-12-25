
#include "Lut.h"
#include "LutUtils.h"

#include <stdexcept>

using namespace std;

Lut::Lut(unsigned inputCount, unsigned long mask) : Lut(inputCount) {
  if(inputCount > 6){
    throw std::logic_error("Too many inputs to be constructed from an unsigned long");
  }
  if(inputCount < 6 && mask >= (1lu << bitCount())){
    throw std::logic_error("Out of range bits are set in the mask");
  }

  _lut[0] = mask;
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
Lut Lut::Not  (LutRef const & a) { Lut ret(a.inputCount()); ret.setNot(a); return ret;}
Lut Lut::And  (LutRef const & a, LutRef const & b) { Lut ret(a.inputCount()); ret.setAnd  (a, b); return ret; }
Lut Lut::Or   (LutRef const & a, LutRef const & b) { Lut ret(a.inputCount()); ret.setOr   (a, b); return ret; }
Lut Lut::Nand (LutRef const & a, LutRef const & b) { Lut ret(a.inputCount()); ret.setNand (a, b); return ret; }
Lut Lut::Nor  (LutRef const & a, LutRef const & b) { Lut ret(a.inputCount()); ret.setNor  (a, b); return ret; }
Lut Lut::Xor  (LutRef const & a, LutRef const & b) { Lut ret(a.inputCount()); ret.setXor  (a, b); return ret; }
Lut Lut::Exor (LutRef const & a, LutRef const & b) { Lut ret(a.inputCount()); ret.setExor (a, b); return ret; }

Lut Lut::Cofactor(LutRef const & a, unsigned input, bool value) {
  Lut ret(a.inputCount());
  ret.setToCofactor(a, input, value);
  return ret;
}

Lut Lut::getCofactor(unsigned input, bool value) const {
  return Cofactor(*this, input, value);
}

Lut Lut::CompactCofactor(LutRef const & a, unsigned input, bool value) {
  Lut ret(a.inputCount()-1);
  ret.setToCompactCofactor(a, input, value);
  return ret;
}

Lut Lut::getCompactCofactor(unsigned input, bool value) const {
  return CompactCofactor(*this, input, value);
}

Lut Lut::PseudoRepresentant(LutRef const & a) {
  Lut ret(a.inputCount());
  ret.setToPseudoRepresentant(a);
  return ret;
}

Lut Lut::getPseudoRepresentant() const {
  return PseudoRepresentant(*this);
}

Lut Lut::SwappedInputs(LutRef const & a, unsigned i1, unsigned i2) {
  Lut ret(a.inputCount());
  ret.setToSwappedInputs(a, i1, i2);
  return ret;
}

Lut Lut::FromCofactors(LutRef const & neg, LutRef const & pos, unsigned input){
  checkInputCounts(neg, pos);

  Lut ret(neg.inputCount());
  ret.setFromCofactors(neg, pos, input);
  return ret;
}

Lut Lut::FromCompactCofactors(LutRef const & neg, LutRef const & pos, unsigned input){
  checkInputCounts(neg, pos);

  Lut ret(neg.inputCount()+1);
  ret.setFromCompactCofactors(neg, pos, input);
  return ret;
}

