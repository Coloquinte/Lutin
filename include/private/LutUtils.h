
#ifndef LUT_UTILS
#define LUT_UTILS

namespace{
// Output positions for positive values of this input
const std::uint64_t mask[6] = {
  0xaaaaaaaaaaaaaaaa,
  0xcccccccccccccccc,
  0xf0f0f0f0f0f0f0f0,
  0xff00ff00ff00ff00,
  0xffff0000ffff0000,
  0xffffffff00000000
};

// Masks when the Lut has less than 6 inputs
const std::uint64_t sizeMask[7] = {
  0x00,
  0x01,
  0x0f,
  0xff,
  0xffff,
  0xffffffff,
  0xffffffffffffffff
};

// Avoid integer conversion problems by using these
const std::uint64_t allZero =  0;
const std::uint64_t allOne  = -1;
const std::uint64_t one     =  1;

// The 64-bit Lut for Xor
const std::uint64_t xorMask = 0x6996966996696996;

}

#endif

