
#ifndef LUTIN_LUT
#define LUTIN_LUT

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
    static Lut Not  (LutRef const a);
    static Lut And  (LutRef const a, LutRef const b);
    static Lut Or   (LutRef const a, LutRef const b);
    static Lut Nand (LutRef const a, LutRef const b);
    static Lut Nor  (LutRef const a, LutRef const b);
    static Lut Xor  (LutRef const a, LutRef const b);
    static Lut Exor (LutRef const a, LutRef const b);

    void setInputCount(unsigned inputCnt);

    Lut getCofactor (unsigned input, bool value) const;
    Lut getPseudoRepresentant() const;

    // Contructors (default is the ground lut)
    Lut(unsigned inputCnt = 0);
    Lut(std::string const & init);

    // Since the memory is managed internally, we need a copy constructor
    Lut(Lut const & o);
    Lut(Lut&& o);
    // Allow resizing, contrary to LutRef
    Lut& operator=(LutRef const o);
    Lut& operator=(Lut&& o);

    ~Lut();
};

#endif

