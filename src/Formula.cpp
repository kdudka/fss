#include <iostream>
#include "Formula.h"

using std::string;

namespace FastSatSolver {

  /**
   * @param  token
   */
  int Formula::parse (Token token ) {
#ifndef NDEBUG
          std::cerr << token << std::endl;
#endif // NDEBUG
  }

  /**
   * @return bool
   */
  bool Formula::isValid ( ) {
    // TODO
  }

  /**
   * @param  data
   */
  bool Formula::eval (ISatItem *data ) {
    // TODO
  }


} // namespace FastSatSolver

