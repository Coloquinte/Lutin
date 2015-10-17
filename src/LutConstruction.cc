
/*
 * Basic Lut manipulation and construction functions
 *
 *
 *
 */

#include "Lut.h"
#include "LutUtils.h"

#include <cassert>

using namespace std;

void Lut::invert(){
  for(LutMask & cur : _lut){
    cur = ~cur;
  }
}

void Lut::invertInput(unsigned input){
  if(input < 6){
    for(LutMask & cur : _lut){
      LutMask lowerPart = cur & ~mask[input]; 
      LutMask upperPart = cur &  mask[input];
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
  Lut ret(inputs);
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


