
#include "LutPrint.h"

#include <ostream>

std::ostream& operator<<(std::ostream& os, Lut const & lut){
  os << lut.str();
  return os;
}



