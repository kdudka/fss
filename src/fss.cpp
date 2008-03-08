#include "SatSolver.h"

using FastSatSolver::SatSolver;
using FastSatSolver::SatProblem;

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
  SatProblemWrapper sp;

  sp.instance()->loadFromInput();
  if (sp.instance()->hasError())
    return 1;

  SatSolverWrapper(sp.instance());

  return 0;
}

