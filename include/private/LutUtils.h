
#ifndef LUTIN_LUTUTILS_H
#define LUTIN_LUTUTILS_H

#include "LutRef.h"

#include <stdexcept>

namespace{

// Avoid integer conversion problems by using these
const std::uint64_t allZero =  0;
const std::uint64_t allOne  = -1;
const std::uint64_t one     =  1;

// Output positions for positive values of this input
const std::uint64_t lutInputMask[7] = {
  0xaaaaaaaaaaaaaaaa,
  0xcccccccccccccccc,
  0xf0f0f0f0f0f0f0f0,
  0xff00ff00ff00ff00,
  0xffff0000ffff0000,
  0xffffffff00000000,
  allOne
};

// Masks when the Lut has less than 6 inputs
const std::uint64_t lutSizeMask[7] = {
  0x01,
  0x03,
  0x0f,
  0xff,
  0xffff,
  0xffffffff,
  0xffffffffffffffff
};

// The 64-bit Lut for Xor
const std::uint64_t xorMask = 0x6996966996696996;

inline void checkInputMask(LutRef const & lut, unsigned inputValues) {
  if(inputValues >= (1u<<lut.inputCount())){
    throw std::logic_error("Out of range bits are set in the given input mask");
  }
}

inline LutRef::LutMask getSizeMask(unsigned inputCount){
  return inputCount >= 6 ? lutSizeMask[6] : lutSizeMask[inputCount];
}

inline void checkInputCounts(LutRef const & a, LutRef const & b){
  if(a.inputCount() != b.inputCount()){
    throw std::logic_error("Luts have different input counts");
  }
}

}

#endif

