
#ifndef LUTIN_LUT
#define LUTIN_LUT

#include <cstdint>
#include <vector>
#include <string>

class Lut{
    public:
    typedef std::uint64_t LutMask;

    public:
    // Contructors (default is the ground lut)
    Lut(unsigned inputs);

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
    static Lut Not  (Lut const & a);
    static Lut And  (Lut const & a, Lut const & b);
    static Lut Or   (Lut const & a, Lut const & b);
    static Lut Nand (Lut const & a, Lut const & b);
    static Lut Nor  (Lut const & a, Lut const & b);
    static Lut Xor  (Lut const & a, Lut const & b);
    static Lut Exor (Lut const & a, Lut const & b);

    // Basic modifiers: invert one input or the output
    void invertInput(unsigned input);
    void invert();
    void setVal(unsigned inputValues, bool val);
    void swapInputs(unsigned i1, unsigned i2);

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
    bool operator==(Lut const & b){ return  equal(b); }
    bool operator!=(Lut const & b){ return !equal(b); }

    // To/from string represented as an hexadecimal init; the init is ordered high-bit first, contrary to internal storage
    std::string str() const;
    Lut(std::string const & init);

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
    bool hasFactors() const;

    // Get the cofactors, but keep the inputs in position (make the corresponding input DC)
    Lut getCofactor (unsigned input, bool value) const;

    // Get a pseudo representant with some input/output inversions and input permutations; it is not unique but is a good approximation for a unique representant
    bool isPseudoRepresentant() const;
    Lut getPseudoRepresentant() const;

    private:
    // Helper functions
    bool equal(Lut const & b) const; // Defines equality operators
    void swapToEnd(unsigned input); // Used in optimized swapInputs implementations
    unsigned countSetBits() const; // Used to compute a pseudorepresentant

    private:
    unsigned _inputCnt;
    std::vector<LutMask> _lut;
};

#endif

