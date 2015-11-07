
#include "Lut.h"

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

void testAnd(){
  for(unsigned i=0; i<15u; ++i){
    Lut lut = Lut::And(i);
    for(unsigned in=0; in<i; ++in){
      if(!lut.forcesValue(in, false, false)){
        cerr << i << " inputs And gate forcing failed on input " << in << std::endl;
      }
      if(lut.forcesValue(in, true, false) || lut.forcesValue(in, true, true) || lut.forcesValue(in, false, true)){
        cerr << i << " inputs And gate false positive on input " << in << std::endl;
      }
    }
    testCofactors(lut);
    for(unsigned mask=0; mask+1 < (1u<<i); ++mask){
      if(lut.evaluate(mask)){
        cerr << i << " inputs And gate ";
        cerr << std::hex << "failed with input mask " << mask << std::endl;
        abort();
      }
    }
    if(!lut.evaluate( (1u<<i)-1 )){
      cerr << i << " inputs And gate ";
      cerr << std::hex << "failed with all bits set" << std::endl;
      abort();
    }
    if(!lut.isGeneralizedAnd()){
      cerr << "Generalized And check failed for " << i << " input" << std::endl;
      abort();
    }
  }
  cout << "And test OK" << std::endl;
}

void testOr(){
  for(unsigned i=0; i<15u; ++i){
    Lut lut = Lut::Or(i);
    for(unsigned in=0; in<i; ++in){
      if(!lut.forcesValue(in, true, true)){
        cerr << i << " inputs Or gate forcing failed on input " << in << std::endl;
      }
      if(lut.forcesValue(in, false, false) || lut.forcesValue(in, false, true) || lut.forcesValue(in, true, false)){
        cerr << i << " inputs And gate false positive on input " << in << std::endl;
      }
    }
    testCofactors(lut);
    for(unsigned mask=1; mask < (1u<<i); ++mask){
      if(!lut.evaluate(mask)){
        cerr << i << " inputs Or gate ";
        cerr << std::hex << "failed with input mask " << mask << std::endl;
        abort();
      }
    }
    if(lut.evaluate(0)){
      cerr << i << " inputs Or gate ";
      cerr << std::hex << "failed with no bit set" << std::endl;
      abort();
    }
    if(!lut.isGeneralizedAnd()){
      cerr << "Generalized And check failed for " << i << " input" << std::endl;
      abort();
    }
  }
  cout << "Or test OK" << std::endl;
}

void testXor(){
  for(unsigned i=0; i<15; ++i){
    Lut lut = Lut::Xor(i);
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
  testXor();

  return 0;
}


