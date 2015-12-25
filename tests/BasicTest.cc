
#include "Lut.h"
#include "LutPrint.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

#include <unordered_set>

using namespace std;

void testSymmetry(Lut const & lut){
  for(unsigned i1=0; i1+1<lut.inputCount(); ++i1){
    for(unsigned i2=i1; i2<lut.inputCount(); ++i2){
      if(Lut::SwappedInputs(lut, i1, i2) != lut){
        cerr << "Inputs " << i1 << " and " << i2 << " of Lut " << lut << " are not symmetrical" << endl;
        abort();
      }
    }
  }
}

void testCofactors(Lut const & lut){
  for(unsigned in=0; in<lut.inputCount(); ++in){
    Lut posCofactor = lut.getCofactor(in, true);
    Lut negCofactor = lut.getCofactor(in, false);
    if(Lut::FromCofactors(negCofactor, posCofactor, in) != lut){
      cerr << lut.inputCount() << "-input lut failed cofactor test " << in << endl;
      abort();
    }
  }
}

void testForce(Lut const & lut){
  for(unsigned in=0; in<lut.inputCount(); ++in){
    Lut posCofactor = lut.getCofactor(in, true);
    Lut negCofactor = lut.getCofactor(in, false);
    if(!posCofactor.isDC(in) || !negCofactor.isDC(in)){
      cerr << lut.inputCount() << "-input lut failed DC test" << in << std::endl;
      abort();
    }

    bool forceMismatch = false;

    if(lut.toggles(in) != (negCofactor == Lut::Not(posCofactor))) forceMismatch = true;
    if(lut.forcesValue(in, false, false) != negCofactor.isConstant(false)) forceMismatch = true;
    if(lut.forcesValue(in, false, true ) != negCofactor.isConstant(true )) forceMismatch = true;
    if(lut.forcesValue(in, true , false) != posCofactor.isConstant(false)) forceMismatch = true;
    if(lut.forcesValue(in, true , true ) != posCofactor.isConstant(true )) forceMismatch = true;
    
    if(forceMismatch){
      cerr << lut.inputCount() << "-input lut failed forcer test " << in << endl;
      abort();
    }
  }
}

void testRepresentant(Lut const & lut){
  Lut pseudoRepr = lut.getPseudoRepresentant();
  if(lut.isPseudoRepresentant() && lut != pseudoRepr){
    cerr << "Failed pseudo-representant stability check on Lut" << lut << " with representant " << pseudoRepr << endl;
    abort();
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
  for(unsigned i=0; i<lut.inputCount(); ++i){
    dup.invertInput(i);
    dup.invertInput(i);
    if(dup != lut){
      cerr << "Input inversion error: <" << dup << "> vs <" << lut << ">" << endl;
      abort();
    }
  }
}

void genericTests(Lut const & lut){
  testSaveReload(lut);
  testInvertInput(lut);
  testSwapInputs(lut);
  testCofactors(lut);
  testForce(lut);
  testRepresentant(lut);
}

void testSimpleGate(Lut const & lut){
  if(lut.inputCount() >= 2){
    if(lut.hasDC() || !lut.hasSingleInputFactorization() || !lut.hasTwoInputFactorization()){
      cerr << "Failed algorithm check" << endl;
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
  genericTests(lut);
  testSimpleGate(lut);
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
    if(!lut.isUnate(in)){
      cerr << inputCnt << "-input generalized And gate unate check failed on input " << in << std::endl;
      abort();
    }
  }
  if(!lut.isGeneralizedAnd()){
    cerr << "Generalized And check failed for " << inputCnt << " input" << std::endl;
    abort();
  }

  if(lut.getPseudoRepresentant() != getGeneralizedAnd(inputCnt, 0u, true)){
    cerr << "Unexpected pseudo-representant of generalized and" << endl;
    abort();
  }
}

void testGeneralizedAnds(){
  for(unsigned inCnt=2; inCnt<10u; ++inCnt){
    for(unsigned inMask=0; inMask < Lut::bitCount(inCnt); ++inMask){
      testGeneralizedAnd(inCnt, inMask, false);
      testGeneralizedAnd(inCnt, inMask, true);
    }
  }
  cout << "Generalized And test OK" << std::endl;
}

void testAnd(){
  for(unsigned i=0; i<15u; ++i){
    Lut lut = Lut::And(i);
    genericTests(lut);
    testSimpleGate(lut);
    testSymmetry(lut);
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
    genericTests(lut);
    testSimpleGate(lut);
    testSymmetry(lut);
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
    genericTests(lut);
    testSimpleGate(lut);
    testSymmetry(lut);
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
    genericTests(lut);
    testSimpleGate(lut);
    testSymmetry(lut);
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
    genericTests(lut);
    testSimpleGate(lut);
    testSymmetry(lut);
    Lut reference = Lut::Exor(i);
    if(Lut::Not(reference) != lut){
      cerr << "Xor check failed for " << lut << std::endl;
      abort();
    }
    testSaveReload(lut);
    for(unsigned in=0; in<i; ++in){
      if(!lut.toggles(in)){
        cerr << i << "-input Xor gate toggle check failed for input " << in << std::endl;
        abort();
      }
      if(i >= 2 && lut.isUnate(in)){
        cerr << i << "-input Xor gate unate check failed for input " << in << std::endl;
        abort();
      }
    }
    if(!lut.isGeneralizedXor()){
      cerr << "Generalized Xor check failed" << std::endl;
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

void testRepresentants(){
  for(unsigned inputCount = 2; inputCount <= 4; ++inputCount){
    unordered_set<Lut, Lut::Hash> complexRepresentants, nonTrivialRepresentants;
    unsigned long totCount = (1Lu << Lut::bitCount(inputCount));
    unsigned long nonTrivialCount = 0;
    unsigned long complexCount = 0;
    for(unsigned long i=0; i < totCount; ++i){
      Lut cur(inputCount, i);
      if(!cur.hasDC()){
        ++nonTrivialCount;
        if(!cur.hasSingleInputFactorization() && !cur.hasTwoInputFactorization()){
          complexCount++;
          if(cur.isPseudoRepresentant()){
            complexRepresentants.emplace(cur);
          }
        }
        if(cur.isPseudoRepresentant()){
          if(cur != cur.getPseudoRepresentant()){
            cerr << "Pseudo-representant " << cur << " is different than its calculated pseuo-representant " << cur.getPseudoRepresentant() << endl;
          }
          nonTrivialRepresentants.emplace(cur);
        }
      }
    }

    cout << "For Lut size " << inputCount << ": (total Lut count " << totCount << ")" << endl;
    cout << "\tGot " << nonTrivialCount << " non-trivial Luts totalizing " << nonTrivialRepresentants.size() << " representants" << endl;
    cout << "\tGot " << complexCount << " complex Luts totalizing " << complexRepresentants.size() << " representants" << endl;

    //cout << "Printing complex representants:" << endl;
    //for(auto const cur : complexRepresentants) cout << cur << endl;

    for(unsigned long i=0; i < totCount; ++i){
      Lut cur(inputCount, i);
      if(!cur.hasDC()){
        if(!cur.hasSingleInputFactorization() && !cur.hasTwoInputFactorization()){
          if(complexRepresentants.count(cur.getPseudoRepresentant()) == 0){
            cerr << "The representant of the complex Lut " << cur << " is " << cur.getPseudoRepresentant() << " and wasn't found" << endl;
            abort();
          }
        }
        else if(nonTrivialRepresentants.count(cur.getPseudoRepresentant()) == 0){
          cerr << "The representant of the non-trivial Lut " << cur << " is " << cur.getPseudoRepresentant() << " and wasn't found" << endl;
          abort();
        }
      }
    }
  }
}

int main(){
  testBuf();
  testAnd();
  testOr();
  testNand();
  testNor();
  testXor();
  testGeneralizedAnds();
  testRepresentants();

  return 0;
}


