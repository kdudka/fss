#ifndef FSS_H
#define FSS_H

#include <string>

namespace FastSatSolver {

  class GenericException {
    public:
      GenericException(std::string text) {
        this->text_ = text;
      }

      std::string getText() {
        return this->text_;
      }

    private:
      std::string text_;
  };

} // namespace FastSatSolver

#endif // FSS_H
