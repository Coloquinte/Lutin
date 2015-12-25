
#ifndef LUTIN_LUT_H
#define LUTIN_LUT_H

#include "LutRef.h"

#include <vector>

class Lut : public LutRef {
    public:
    static Lut Gnd  (unsigned inputs);
    static Lut Vcc  (unsigned inputs);
    static Lut And  (unsigned inputs);
    static Lut Or   (unsigned inputs);
    static Lut Nand (unsigned inputs);
    static Lut Nor  (unsigned inputs);
    static Lut Xor  (unsigned inputs);
    static Lut Exor (unsigned inputs);
    static Lut Wire (unsigned wireInput, unsigned inputs, bool invert);
    static Lut Buf  (unsigned wireInput, unsigned inputs);
    static Lut Inv  (unsigned wireInput, unsigned inputs);

    // Operations on same-size Luts with a common input set
    static Lut Not  (LutRef const & a);
    static Lut And  (LutRef const & a, LutRef const & b);
    static Lut Or   (LutRef const & a, LutRef const & b);
    static Lut Nand (LutRef const & a, LutRef const & b);
    static Lut Nor  (LutRef const & a, LutRef const & b);
    static Lut Xor  (LutRef const & a, LutRef const & b);
    static Lut Exor (LutRef const & a, LutRef const & b);

    static Lut Cofactor (LutRef const & a, unsigned input, bool value);
    static Lut FromCofactors (LutRef const & neg, LutRef const & pos, unsigned input);
    static Lut CompactCofactor (LutRef const & a, unsigned input, bool value);
    static Lut FromCompactCofactors (LutRef const & neg, LutRef const & pos, unsigned input);
    static Lut PseudoRepresentant (LutRef const & a);
    static Lut SwappedInputs (LutRef const & a, unsigned i1, unsigned i2);

    void setInputCount(unsigned inputCnt);

    Lut getCofactor (unsigned input, bool value) const;
    Lut getCompactCofactor (unsigned input, bool value) const;
    Lut getPseudoRepresentant() const;

    // Contructors (default is the ground lut)
    Lut(unsigned inputCnt = 0);
    Lut(unsigned inputCnt, unsigned long mask);
    Lut(std::string const & init);

    // Since the memory is managed internally, we need a copy constructor
    Lut(Lut const & o);
    // Allow resizing, contrary to LutRef
    Lut& operator=(LutRef const & o);

    ~Lut();
};

inline Lut operator|(LutRef const & a, LutRef const & b) {
  return Lut::Or(a, b);
}
inline Lut operator&(LutRef const & a, LutRef const & b) {
  return Lut::And(a, b);
}
inline Lut operator^(LutRef const & a, LutRef const & b) {
  return Lut::Xor(a, b);
}
inline Lut operator~(LutRef const & a) {
  return Lut::Not(a);
}

inline Lut::Lut(unsigned inputCount) 
: LutRef(inputCount, new LutMask[arraySize(inputCount)]) {
}

inline Lut::Lut(Lut const & o) : Lut(o.inputCount()){
  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = o._lut[i];
  }
}

inline Lut& Lut::operator=(LutRef const & o){
  setInputCount(o.inputCount());
  LutRef::operator=(o);
  return *this;
}

inline Lut::~Lut(){
  delete[] _lut;
}

inline void Lut::setInputCount(unsigned inputCnt){
  LutMask* tmp = new LutMask[arraySize(inputCnt)];
  for(unsigned i=0; i<arraySize(std::min(inputCnt, inputCount())); ++i){
    tmp[i] = _lut[i];
  }
  delete[] _lut;
  _lut = tmp;
  _inputCnt = inputCnt;
}

#endif

