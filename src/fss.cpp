#include <iostream>
#include "SatSolver.h"

using FastSatSolver::GenericException;
using FastSatSolver::SatProblem;
using FastSatSolver::SatSolver;
using FastSatSolver::SatSolverParameters;
using FastSatSolver::TimedStop;

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
    SatSolverWrapper(SatProblem *problem, SatSolverParameters *params) {
      this->ptr = SatSolver::create(problem, params);
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
  sp.instance()->loadFromFile("form.txt");
  //sp.instance()->loadFromFile(argv[1]);
  //sp.instance()->loadFromInput();
  if (sp.instance()->hasError())
    throw GenericException("SatProblem::hasError() returned true)");

  std::cerr << "SatSolver::create(...)\n";
  SatSolverWrapper satSolverWrapper(sp.instance(), 0);
  SatSolver *satSolver = satSolverWrapper.instance();

  TimedStop *timedStop = new TimedStop(satSolver, 2000);
  satSolver->addObserver(timedStop);
  satSolver->start();
  delete timedStop;

  return 0;
  }
  catch (GenericException e) {
    std::cerr << "fss: " << e.getText() << std::endl;
    return 1;
  }
}

