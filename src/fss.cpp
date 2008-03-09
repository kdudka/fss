#include <iostream>
#include "SatSolver.h"

using FastSatSolver::SatSolver;
using FastSatSolver::SatProblem;
using FastSatSolver::GenericException;

// RAII object
class SatProblemWrapper {
  public:
    SatProblemWrapper() {
      this->ptr = SatProblem::create();
    }
    ~SatProblemWrapper() {
      delete this->ptr;
    }
    SatProblem* instance() {
      return ptr;
    }
  private:
    SatProblem *ptr;
};

// RAII object
class SatSolverWrapper {
  public:
    SatSolverWrapper(SatProblem *sp) {
      this->ptr = SatSolver::create(sp);
    }
    ~SatSolverWrapper() {
      delete this->ptr;
    }
    SatSolver* instance() {
      return ptr;
    }
  private:
    SatSolver *ptr;
};

int main(int argc, char *argv[]) {
  try {
  SatProblemWrapper sp;

  std::cerr << "SatProblem::loadFromInput()...\n";
  //sp.instance()->loadFromFile(argv[1]);
  sp.instance()->loadFromInput();
  if (sp.instance()->hasError())
    throw GenericException("Unhandled error in main()");

  /*std::cerr << "SatSolver::create(...)\n";
  SatSolverWrapper(sp.instance());*/

  return 0;
  }
  catch (GenericException e) {
    std::cerr << "fss: " << e.getText() << std::endl;
    return 1;
  }
}

