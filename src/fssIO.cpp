#include <string>
#include "fssIO.h"

using std::string;

namespace FastSatSolver {

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // GenericException implementation
  struct GenericException::Private {
    std::string text;
  };
  GenericException::GenericException(string text):
    d(new Private)
  {
    d->text = text;
  }
  GenericException::~GenericException() {
    delete d;
  }
  string GenericException::getText() {
    return d->text;
  }

} // namespace FastSatSolver
