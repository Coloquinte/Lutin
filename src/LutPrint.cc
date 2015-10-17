
#include "Lut.h"

#include <ostream>

std::ostream& operator<<(std::ostream& stream, Lut const & lut){
  for(unsigned msk=0; msk < 1u<<lut.inputCount(); ++msk){
    stream.put(lut.evaluate(msk) ? '1' : '0');
  }
  return stream;
}



