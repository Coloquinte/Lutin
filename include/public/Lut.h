
#ifndef LUTIN_LUT
#define LUTIN_LUT

#include <cstdint>
#include <vector>

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

    // Considering same-size Luts with a common input set
    static Lut And  (Lut const & a, Lut const & b);
    static Lut Or   (Lut const & a, Lut const & b);
    static Lut Nand (Lut const & a, Lut const & b);
    static Lut Nor  (Lut const & a, Lut const & b);
    static Lut Xor  (Lut const & a, Lut const & b);
    static Lut Exor (Lut const & a, Lut const & b);

    public:
    // Basic modifiers: invert one input or the output
    void invertInput(unsigned input);
    void invert();

    public:
    // Basic queries
    unsigned inputCount() const { return _inputCnt; }
    static bool equal(Lut const & a, Lut const & b);

    bool evaluate(unsigned inputValues) const;

    bool isConstant() const;
    bool isGnd() const;
    bool isVcc() const;
    bool isGeneralizedAnd() const;
    bool isGeneralizedXor() const;

    public:
    /*
     * Logic queries
     *   * Is an input a don't care (no influence on the output result)?
     *   * Does the input toggle the output value?
     *   * Does a particular value at one input force the output value?
     *   * Get the cofactors, but keep the inputs in position (make the corresponding input DC)
     */
    bool isDC(unsigned input) const;
    bool toggles(unsigned input) const;
    bool forcesValue(unsigned input, bool inVal, bool outVal) const;

    Lut getCofactor (unsigned input, bool value) const;

    private:
    unsigned _inputCnt;
    std::vector<LutMask> _lut;
};

inline bool operator==(Lut const & a, Lut const & b){ return  Lut::equal(a, b); }
inline bool operator!=(Lut const & a, Lut const & b){ return !Lut::equal(a, b); }

#endif

