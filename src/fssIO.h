#ifndef FSSIO_H
#define FSSIO_H

#include <string>

namespace FastSatSolver {

  class GenericException {
    public:
      GenericException(std::string text);
      ~GenericException();
      std::string getText();

    private:
      struct Private;
      Private *d;
  };


} // namespace FastSatSolver

#endif // FSS_H
