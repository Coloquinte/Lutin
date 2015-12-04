
#ifndef LUTIN_LUTREF
#define LUTIN_LUTREF

#include <cstdint>
#include <string>

class LutRef{
    public:
    typedef std::uint64_t LutMask;

    public:
    // Contructors (default is the ground lut)
    LutRef(unsigned inputs = -1, LutMask* pt = nullptr);

    // No copy, but assignment possible
    LutRef& operator=(LutRef const o);

    public:
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

    // Operations on same-size Luts with a common input set
    void setNot  (LutRef const a);
    void setAnd  (LutRef const a, LutRef const b);
    void setOr   (LutRef const a, LutRef const b);
    void setNand (LutRef const a, LutRef const b);
    void setNor  (LutRef const a, LutRef const b);
    void setXor  (LutRef const a, LutRef const b);
    void setExor (LutRef const a, LutRef const b);

    void operator&=(LutRef const o);
    void operator|=(LutRef const o);
    void operator^=(LutRef const o);

    // Basic modifiers: invert one input or the output
    void invertInput(unsigned input);
    void invert();
    void setVal(unsigned inputValues, bool val);
    void swapInputs(unsigned i1, unsigned i2);
    void setSwappedInputs(LutRef const o, unsigned i1, unsigned i2);

    // Basic queries
    unsigned inputCount() const { return _inputCnt; }
    bool evaluate(unsigned inputValues) const;

    bool isConstant() const;
    bool isGnd  () const;
    bool isVcc  () const;
    bool isAnd  () const;
    bool isOr   () const;
    bool isNand () const;
    bool isNor  () const;
    bool isXor  () const;
    bool isExor () const;

    // And/Xor with some input and output inversions
    bool isGeneralizedAnd() const;
    bool isGeneralizedXor() const;

    // Logic comparison
    bool operator==(LutRef const b){ return  equal(b); }
    bool operator!=(LutRef const b){ return !equal(b); }

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

    // Get the cofactors, but keep the inputs in position (make the corresponding input DC)
    void setToCofactor(LutRef const o, unsigned input, bool value);
    void setToCofactor(unsigned input, bool value);

    // Get a pseudo representant with some input/output inversions and input permutations; it is not unique but is a good approximation for a unique representant
    bool isPseudoRepresentant() const;
    void setToPseudoRepresentant();
    void setToPseudoRepresentant(LutRef const o);

    private:
    // Helper functions
    bool equal(LutRef const o) const; // Defines equality operators
    void swapToEnd(unsigned input); // Used in optimized swapInputs implementations
    unsigned countSetBits() const; // Used to compute a pseudorepresentant

    protected:
    static unsigned arraySize(unsigned inputCount) {
      return inputCount >= 6 ? 1ul << (inputCount-6) : 1ul;
    }
    unsigned arraySize() const{
      return arraySize(inputCount());
    }

    protected:
    // Need to new/delete if managed, or increment if iterator
    unsigned _inputCnt;
    LutMask* _lut;
};

#endif
