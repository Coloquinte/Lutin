
#include "Lut.h"
#include "LutPrint.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;

inline void testVal(bool value){
  if(!value) abort();
}

void testCofactors(Lut const & lut){
  for(unsigned in=0; in<lut.inputCount(); ++in){
    if(!lut.getCofactor(in, true).isDC(in) || !lut.getCofactor(in, false).isDC(in)){
      cerr << lut.inputCount() << " inputs lut failed DC test" << in << std::endl;
      abort();
    }
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
  for(unsigned in=0; in<inputCnt; ++in){
    bool forcingIn = ((inputValues >> in) & 0x1) == 0, forcedVal = inverted;
    if(!lut.forcesValue(in, forcingIn, forcedVal)){
      cerr << inputCnt << " inputs generalized And gate forcing failed on input " << in << std::endl;
      abort();
    }
    if(lut.forcesValue(in, forcingIn, !forcedVal) || lut.forcesValue(in, !forcingIn, true) || lut.forcesValue(in, !forcingIn, false)){
      cerr << inputCnt << " inputs generalized And gate false positive on input " << in << std::endl;
      abort();
    }
  }
  testCofactors(lut);
  if(!lut.isGeneralizedAnd()){
    cerr << "Generalized And check failed for " << inputCnt << " input" << std::endl;
    abort();
  }
}

void testGeneralizedAnds(){
  for(unsigned inCnt=1; inCnt<15u; ++inCnt){
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
      cerr << "Or check failed" << std::endl;
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
      cerr << "Or check failed" << std::endl;
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
      cerr << "Nor check failed" << std::endl;
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
      cerr << "Nand check failed" << std::endl;
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
      cerr << "Nand check failed" << std::endl;
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
      cerr << "Generalized And check failed for " << i << " input" << std::endl;
      abort();
    }
  }
  cout << "Xor test OK" << std::endl;
}


int main(){
  testAnd();
  testOr();
  testNand();
  testNor();
  testXor();
  testGeneralizedAnds();

  return 0;
}


