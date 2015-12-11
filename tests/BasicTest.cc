
#include "Lut.h"
#include "LutPrint.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;

void testCofactors(Lut const & lut){
  bool forceMismatch = false;
  for(unsigned in=0; in<lut.inputCount(); ++in){
    if(!lut.getCofactor(in, true).isDC(in) || !lut.getCofactor(in, false).isDC(in)){
      cerr << lut.inputCount() << "-input lut failed DC test" << in << std::endl;
      abort();
    }
    if(lut.toggles(in) != (lut.getCofactor(in, false) == Lut::Not(lut.getCofactor(in, true)))) forceMismatch = true;
    if(lut.forcesValue(in, false, false) != lut.getCofactor(in, false).isConstant(false)) forceMismatch = true;
    if(lut.forcesValue(in, false, true ) != lut.getCofactor(in, false).isConstant(true )) forceMismatch = true;
    if(lut.forcesValue(in, true , false) != lut.getCofactor(in, true ).isConstant(false)) forceMismatch = true;
    if(lut.forcesValue(in, true , true ) != lut.getCofactor(in, true ).isConstant(true )) forceMismatch = true;
  }
  if(forceMismatch){
    cerr << lut.inputCount() << "-input lut failed forcer test" << endl;
  }
}

void testSaveReload(Lut const & lut){
  if(lut.inputCount() >= 2){
    string strRepr = lut.str();
    Lut rebuilt(strRepr);
    if(rebuilt != lut){
      cerr << "Luts <" << lut << "> and <" << rebuilt << "> should be equal but are not" << endl;
      abort();
    }
  }
}

void testSwapInputs(Lut const & lut){
  Lut dup = lut;
  if(lut.inputCount() <= 1) return;

  for(unsigned i=0; i<lut.inputCount()-1; ++i){
    for(unsigned j=i+1; j<lut.inputCount(); ++j){
      dup.swapInputs(i,j);
      dup.swapInputs(i,j);
      if(dup != lut){
        cerr << "Input swapping error: <" << dup << "> vs <" << lut << ">" << endl;
        abort();
      }
    }
  }
}

void testInvertInput(Lut const & lut){
  Lut dup = lut;
  for(unsigned i=0; i<lut.inputCount()-1; ++i){
    dup.invertInput(i);
    dup.invertInput(i);
    if(dup != lut){
      cerr << "Input inversion error: <" << dup << "> vs <" << lut << ">" << endl;
      abort();
    }
  }
}

Lut getGeneralizedAnd(unsigned inputCnt, unsigned inputValues, bool inverted){
    Lut ret = Lut::Gnd(inputCnt);
    ret.setVal(inputValues & ((1u << inputCnt)-1), true);
    if(inverted) ret.invert();
    return ret;
}

void testGeneralizedAnd(unsigned inputCnt, unsigned inputValues, bool inverted){
  Lut lut = getGeneralizedAnd(inputCnt, inputValues, inverted);
  if(lut.inputCount() != inputCnt){
    cerr << "Input counts don't match" << std::endl;
  }
  testSaveReload(lut);
  testInvertInput(lut);
  if(inputCnt <= 10) testSwapInputs(lut);
  for(unsigned in=0; in<inputCnt; ++in){
    bool forcingIn = ((inputValues >> in) & 0x1) == 0, forcedVal = inverted;
    if(!lut.forcesValue(in, forcingIn, forcedVal)){
      cerr << inputCnt << "-input generalized And gate forcing failed on input " << in << std::endl;
      abort();
    }
    if(lut.forcesValue(in, forcingIn, !forcedVal) || lut.forcesValue(in, !forcingIn, true) || lut.forcesValue(in, !forcingIn, false)){
      cerr << inputCnt << "-input generalized And gate false positive on input " << in << std::endl;
      abort();
    }
  }
  testCofactors(lut);
  if(!lut.isGeneralizedAnd()){
    cerr << "Generalized And check failed for " << inputCnt << " input" << std::endl;
    abort();
  }
  Lut pseudoRepr = lut.getPseudoRepresentant();
  if(lut.isPseudoRepresentant() && lut != pseudoRepr){
    cerr << "Failed pseudo-representant stability check" << endl;
  }
  if(pseudoRepr != getGeneralizedAnd(inputCnt, 0u, true)){
    cerr << "Unexpected pseudo-representant of generalized and" << endl;
  }
}

void testGeneralizedAnds(){
  for(unsigned inCnt=2; inCnt<15u; ++inCnt){
    for(unsigned inMask=0; inMask < (1u<<inCnt); ++inMask){
      testGeneralizedAnd(inCnt, inMask, false);
      testGeneralizedAnd(inCnt, inMask, true);
    }
  }
  cout << "Generalized And test OK" << std::endl;
}

void testAnd(){
  for(unsigned i=0; i<15u; ++i){
    Lut lut = Lut::And(i);
    Lut reference = getGeneralizedAnd(i, -1, false);
    if(reference != lut){
      cerr << "And check failed for " << lut << std::endl;
      abort();
    }
  }

  cout << "And test OK" << std::endl;
}

void testOr(){
  for(unsigned i=0; i<15u; ++i){
    Lut lut = Lut::Or(i);
    Lut reference = getGeneralizedAnd(i, 0, true);
    if(reference != lut){
      cerr << "Or check failed for " << lut << std::endl;
      abort();
    }
  }
  cout << "Or test OK" << std::endl;
}

void testNor(){
  for(unsigned i=0; i<15u; ++i){
    Lut lut = Lut::Nor(i);
    Lut reference = getGeneralizedAnd(i, 0, false);
    if(reference != lut){
      cerr << "Nor check failed for " << lut << std::endl;
      abort();
    }
  }
  cout << "Nor test OK" << std::endl;
}

void testNand(){
  for(unsigned i=0; i<15u; ++i){
    Lut lut = Lut::Nand(i);
    Lut reference = getGeneralizedAnd(i, -1, true);
    if(reference != lut){
      cerr << "Nand check failed for " << lut << std::endl;
      abort();
    }
  }
  cout << "Nand test OK" << std::endl;
}

void testXor(){
  for(unsigned i=0; i<15; ++i){
    Lut lut = Lut::Xor(i);
    Lut reference = Lut::Exor(i);
    if(Lut::Not(reference) != lut){
      cerr << "Xor check failed for " << lut << std::endl;
      abort();
    }
    testSaveReload(lut);
    for(unsigned in=0; in<i; ++in){
      if(!lut.toggles(in)){
        cerr << i << " inputs Xor gate failed for input " << in << std::endl;
        abort();
      }
    }
    testCofactors(lut);
    if(!lut.isGeneralizedXor()){
      cerr << "Generalized And check failed" << std::endl;
      abort();
    }
  }
  cout << "Xor test OK" << std::endl;
}

void testBufCofactors(Lut const & wire, unsigned j){
  for(unsigned k=0; k<wire.inputCount(); ++k){
    if(k != j){
      if(wire.getCofactor(k, false) != wire.getCofactor(k, true)){
        cerr << "Buffer cofactor test failed" << endl;
        abort();
      }
      if(!wire.isDC(k)){
        cerr << "Buffer DC check failed" << endl;
        abort();
      }
    }
  }
  if(wire.isDC(j)){
    cerr << "Buffer sensitivity check failed" << endl;
    abort();
  }
}

void testBuf(){
  for(unsigned i=1; i<15; ++i){
    for(unsigned j=0; j<i; ++j){
      Lut buf = Lut::Buf(j, i);
      Lut inv = Lut::Inv(j, i);
      if(buf != Lut::Not(inv)){
        cerr << "Buffer test failed" << endl;
        abort();
      }
      testBufCofactors(buf, j);
      testBufCofactors(inv, j);
    }
  }
  cout << "Buf test OK" << std::endl;
}

int main(){
  testBuf();
  testAnd();
  testOr();
  testNand();
  testNor();
  testXor();
  testGeneralizedAnds();

  return 0;
}


