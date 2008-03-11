#include <iostream>
#include <ga/GAParameter.h>
#include <ga/GAStatistics.h>
#include <ga/GASimpleGA.h>
#include "fss.h"
#include "SatProblem.h"
#include "SatSolver.h"

using FastSatSolver::GenericException;
using FastSatSolver::SatProblem;
using FastSatSolver::GASatSolver;
using FastSatSolver::TimedStop;
using FastSatSolver::FitnessWatch;

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
class GASatSolverWrapper {
  public:
    GASatSolverWrapper(SatProblem *problem, const GAParameterList &params) {
      this->ptr = GASatSolver::create(problem, params);
    }
    ~GASatSolverWrapper() {
      delete this->ptr;
    }
    GASatSolver* instance() {
      return ptr;
    }
  private:
    GASatSolver *ptr;
};

int main(int argc, char *argv[]) {
  try {
  SatProblemWrapper sp;

  std::cerr << "SatProblem::loadFromInput()...\n";
  sp.instance()->loadFromInput();
  if (sp.instance()->hasError())
    throw GenericException("SatProblem::hasError() returned true)");

  GAParameterList params;
  std::cerr << "Parsing cmd line:" << std::endl;
  GASatSolver::registerDefaultParameters(params);
  params.parse(argc, argv, gaTrue);
  std::cout << params << std::endl;
  std::cerr << "SatSolver::create(...)\n";
  GASatSolverWrapper satSolverWrapper(sp.instance(), params);
  GASatSolver *satSolver = satSolverWrapper.instance();

  //TimedStop *timedStop = new TimedStop(satSolver, 10*1000);
  FitnessWatch *fitnessWatch = new FitnessWatch(satSolver, std::cout);
  //satSolver->addObserver(timedStop);
  satSolver->addObserver(fitnessWatch);
  satSolver->start();
  delete fitnessWatch;
  //delete timedStop;

  GAStatistics stats= satSolver->getStatistics();
  std::cout << "--- GA Statistics" << std::endl << stats << std::endl;

  return 0;
  }
  catch (GenericException e) {
    std::cerr << "fss: " << e.getText() << std::endl;
    return 1;
  }
}

