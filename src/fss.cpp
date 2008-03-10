#include <iostream>
#include "fss.h"
#include "SatProblem.h"
#include "SatSolver.h"

using FastSatSolver::GenericException;
using FastSatSolver::SatProblem;
using FastSatSolver::SatSolver;
using FastSatSolver::SatSolverParameters;
using FastSatSolver::TimedStop;
using FastSatSolver::FitnessWatch;
using FastSatSolver::SatSolverStatsProxy;

// RAII object
class SatProblemWrapper {
  public:
    SatProblemWrapper() {
      this->ptr = new (SatProblem);
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
  //sp.instance()->loadFromFile(argv[1]);
  sp.instance()->loadFromInput();
  if (sp.instance()->hasError())
    throw GenericException("SatProblem::hasError() returned true)");

  std::cerr << "SatSolver::create(...)\n";
  SatSolverWrapper satSolverWrapper(sp.instance(), 0);
  SatSolver *satSolver = satSolverWrapper.instance();

  TimedStop *timedStop = new TimedStop(satSolver, 60*1000);
  FitnessWatch *fitnessWatch = new FitnessWatch(satSolver, std::cout);
  satSolver->addObserver(timedStop);
  satSolver->addObserver(fitnessWatch);
  satSolver->start();
  delete fitnessWatch;
  delete timedStop;

  SatSolverStatsProxy *statsProxy= satSolver->getStatsProxy();
  std::cout << *statsProxy << std::endl;
  delete statsProxy;

  return 0;
  }
  catch (GenericException e) {
    std::cerr << "fss: " << e.getText() << std::endl;
    return 1;
  }
}

