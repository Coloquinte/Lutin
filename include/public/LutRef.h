
#ifndef LUTIN_LUTREF_H
#define LUTIN_LUTREF_H

#include <cstdint>
#include <string>
#include <algorithm>

namespace Simplification{
  enum struct OneInput{
    None,
    DC,
    F00,
    F01,
    F10,
    F11,
    Toggles
  };

  enum struct TwoInput{
    // Trivial symmetries (one input is DC) are ignored
    None,
    // Symmetry with only two cofactors equal
    Symm,
    SymmInv,
    // Symmetry with three cofactors equal (can factor an And gate out)
    SAnd00,
    SAnd01,
    SAnd10,
    SAnd11,
    // Symmetry with 2x2 equal cofactors (can factor a Xor gate out)
    SXor
  };

  inline bool isSymm(TwoInput symm){
    return symm == TwoInput::Symm
        || symm == TwoInput::SAnd00
        || symm == TwoInput::SAnd11
        || symm == TwoInput::SXor;
  }
  inline bool isSymmInv(TwoInput symm){
    return symm == TwoInput::SymmInv
        || symm == TwoInput::SAnd01
        || symm == TwoInput::SAnd10
        || symm == TwoInput::SXor;
  }
}

class LutRef{
    public:
    typedef std::uint64_t LutMask;

    public:
  // Constructors/basic operators

    // Contructors (default is the ground lut)
    LutRef(unsigned inputs = -1, LutMask* pt = nullptr);

    // No copy, but assignment possible
    LutRef(LutRef const & o) = delete;
    LutRef& operator=(LutRef const & o);

  // Size of the Lut ands its internal structures

    // Number of inputs
    unsigned inputCount() const { return _inputCnt; }
    // Size of the internal array (in uint64_t)
    static unsigned arraySize(unsigned inputCnt);
    unsigned arraySize() const;
    // Bit-level size (2^n)
    static std::size_t bitCount(unsigned inputCnt);
    std::size_t bitCount() const;

    public:

  // Basic lut creation

    // As basic functions
    void setGnd  ();
    void setVcc  ();
    void setAnd  ();
    void setOr   ();
    void setNand ();
    void setNor  ();
    void setXor  ();
    void setExor ();
    void setWire (unsigned wireInput, bool invert);
    void setBuf  (unsigned wireInput);
    void setInv  (unsigned wireInput);

    // From other Luts
    void setNot  (LutRef const & a);
    void setAnd  (LutRef const & a, LutRef const & b);
    void setOr   (LutRef const & a, LutRef const & b);
    void setNand (LutRef const & a, LutRef const & b);
    void setNor  (LutRef const & a, LutRef const & b);
    void setXor  (LutRef const & a, LutRef const & b);
    void setExor (LutRef const & a, LutRef const & b);

  // In-place modifications
    void operator&=(LutRef const & o);
    void operator|=(LutRef const & o);
    void operator^=(LutRef const & o);
    void invert();
    void invertInput(unsigned input);
    void swapInputs(unsigned i1, unsigned i2);

  // Set from another Lut
    void setToInverted(LutRef const & o);
    void setToInvertedInput(LutRef const & o, unsigned input);
    void setToSwappedInputs(LutRef const & o, unsigned i1, unsigned i2);

  // Bit-level queries
    void setVal(std::size_t inputValues, bool val);
    bool evaluate(std::size_t inputValues) const;

  // Simple logic queries
    bool isConstant() const;
    bool isConstant(bool val) const;
    bool isGnd  () const;
    bool isVcc  () const;
    bool isAnd  () const;
    bool isOr   () const;
    bool isNand () const;
    bool isNor  () const;
    bool isXor  () const;
    bool isExor () const;
    // n-input And/Xor with some input and output inversions
    bool isGeneralizedAnd() const;
    bool isGeneralizedXor() const;

  // Cofactor manipulations keeping the inputs in position (make the corresponding input DC)
    void setToCofactor(LutRef const & o, unsigned input, bool value);
    void setToCofactor(unsigned input, bool value);
    void setFromCofactors(LutRef const & neg, LutRef const & pos, unsigned input);

  // Decompositions for one and two inputs
    Simplification::OneInput getSimplification(unsigned input) const;
    Simplification::TwoInput getSimplification(unsigned i1, unsigned i2) const;

  // Logic comparison + hash
    bool isEqual(LutRef const & o) const;
    bool isInverse(LutRef const & o) const;
    bool operator==(LutRef const & b) const { return  isEqual(b); }
    bool operator!=(LutRef const & b) const { return !isEqual(b); }
    struct Hash;
    std::size_t getHash() const;

  // To/from string represented as an hexadecimal init; the init is ordered high-bit first, contrary to internal storage
    std::string str() const;
    void initFromStr(std::string const & init);

    /*
     * Logic queries
     *   * Is an input a don't care (no influence on the output result)?
     *   * Does the input toggle the output value?
     *   * Does a particular value at one input force the output value?
     *   * Test if the Lut can be simplified in smaller disjoint Luts
     */

    bool isDC(unsigned input) const;
    bool toggles(unsigned input) const;
    bool forcesValue(unsigned input, bool inVal, bool outVal) const;

    // Possible simplifications/decompositions
    bool hasDC() const;
    bool hasSingleInputFactorization() const;
    bool hasTwoInputFactorization() const;

    // Is a function unate for a given input, and if not how many bits are against the trend
    bool isUnate(unsigned input) const;
    bool isBinate(unsigned input) const;
    bool isUnate(unsigned input, bool polarity) const;
    std::size_t countUnate(unsigned input, bool polarity) const;
    std::size_t countUnate(unsigned input) const;

    // Get the cofactors, but reduce the number of inputs (the last input replaces the one that is removed)
    void setToCompactCofactor(LutRef const & o, unsigned input, bool value);
    void setFromCompactCofactors(LutRef const & neg, LutRef const & pos, unsigned input);

    // Get a pseudo representant with some input/output inversions and input permutations; it is not unique but is a good approximation for a unique representant
    bool isPseudoRepresentant() const;
    void setToPseudoRepresentant();
    void setToPseudoRepresentant(LutRef const & o);

    public:
    // Exception throwing
    void throwOutOfRange(unsigned input) const;
    void throwIncompatibleSize(LutRef const & o) const;
    void throwBadMask(std::size_t inputMask) const;

    private:
    // Helper functions
    void swapToBegin(unsigned input); // Used in optimized swapInputs implementations
    std::size_t countSetBits() const; // Used to compute a pseudorepresentant

    protected:
    // Protected only: Need to new/delete if managed, or increment if iterator
    unsigned _inputCnt;
    LutMask* _lut;
};

struct LutRef::Hash{
  std::size_t operator()(LutRef const & lut) const{
    return lut.getHash();
  }
};

inline unsigned LutRef::arraySize(unsigned inputCnt) {
  return inputCnt >= 6 ? 1ul << (inputCnt-6) : 1ul;
}
inline unsigned LutRef::arraySize() const{
  return arraySize(inputCount());
}
inline std::size_t LutRef::bitCount(unsigned inputCnt) {
  return 1ul << inputCnt;
}
inline std::size_t LutRef::bitCount() const{
  return bitCount(inputCount());
}

inline LutRef::LutRef(unsigned inputs, LutMask* pt)
: _inputCnt(inputs)
, _lut(pt) {
}

inline LutRef& LutRef::operator=(LutRef const & a){
  if(inputCount() != a.inputCount()) throwIncompatibleSize(a);

  for(unsigned i=0; i<arraySize(); ++i){
    _lut[i] = a._lut[i];
  }
  return *this;
}



#endif

