#include <iostream>
#include <iomanip>
#include <string>
#include <ga/GAParameter.h>
#include <ga/GAStatistics.h>
#include <ga/GASimpleGA.h>
#include "fss.h"
#include "SatProblem.h"
#include "SatSolver.h"

using std::string;
using FastSatSolver::GenericException;
using FastSatSolver::SatProblem;
using FastSatSolver::AbstractSatSolver;
using FastSatSolver::GASatSolver;
using FastSatSolver::SolutionsCountStop;
using FastSatSolver::TimedStop;
using FastSatSolver::FitnessWatch;
using FastSatSolver::SatItemVector;

namespace {

  inline void error(string szMsg) {
    std::cerr << "fss: " << szMsg << std::endl;
  }

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

  class SatSolverDestructor {
    public:
      SatSolverDestructor(AbstractSatSolver *solver):
        solver_(solver)
    {
    }
      ~SatSolverDestructor() {
        delete solver_;
      }
    private:
      AbstractSatSolver *solver_;
  };

}

int main(int argc, char *argv[]) {
  try {
    SatProblemWrapper spWrapper;
    SatProblem *problem= spWrapper.instance();

    std::cerr << "Loading SAT problem...\n";
    problem->loadFromInput();
    //problem->loadFromFile("input.txt");
    if (problem->hasError())
      throw GenericException("SatProblem::hasError() returned true)");

    // Parse cmd-line parameters
    GAParameterList params;
    // GASatSolver-specific parameters
    GASatSolver::registerDefaultParameters(params);

    // Default values of parameters
    const bool FALSE = false;
    const int DEF_MIN_COUNT_OF_SOLUTIONS =  1;
    const int DEF_MAX_COUNT_OF_SOLUTIONS =  8;
    const int DEF_MAX_COUNT_OF_RUNS =       8;
    const int DEF_MAX_TIME_PER_RUN =        0;
    const int DEF_STEP_WIDTH =              16;

    // Register extra parameters
    params.add("blind_solver",            "blind",    GAParameter::BOOLEAN,     &FALSE);
    params.add("min_count_of_solutions",  "minslns",  GAParameter::INT,         &DEF_MIN_COUNT_OF_SOLUTIONS);
    params.add("max_count_of_solutions",  "maxslns",  GAParameter::INT,         &DEF_MAX_COUNT_OF_SOLUTIONS);
    params.add("max_count_of_runs",       "maxruns",  GAParameter::INT,         &DEF_MAX_COUNT_OF_RUNS);
    params.add("max_time_per_run",        "maxtime",  GAParameter::INT,         &DEF_MAX_TIME_PER_RUN);
    params.add("step_width",              "stepw",    GAParameter::INT,         &DEF_STEP_WIDTH);

    // parse using GAParameterList class
    params.parse(argc, argv, gaTrue);
    // TODO: Remove next line
    std::cout << params;

    // true for blind solver, false for GA solver
    bool useBlindSolver= false;
    params.get("blind_solver", &useBlindSolver);

    // Minimum of solutions (to declare as solution)
    int minSlns= DEF_MIN_COUNT_OF_SOLUTIONS;
    params.get("min_count_of_solutions", &minSlns);
    if (minSlns <= 0) {
      error("min_count_of_solutions out of range, using default");
      minSlns = DEF_MIN_COUNT_OF_SOLUTIONS;
    }

    // Maximum number of solutions (solver stop when reached)
    int maxSlns= DEF_MAX_COUNT_OF_SOLUTIONS;
    params.get("max_count_of_solutions", &maxSlns);
    if (maxSlns <= 0) {
      error("max_count_of_solutions out of range, using default");
      maxSlns = DEF_MAX_COUNT_OF_SOLUTIONS;
    }

    // Maximum nuber of runs (if satisfaction is not reachable)
    int maxRuns= DEF_MAX_COUNT_OF_RUNS;
    params.get("max_count_of_runs", &maxRuns);
    if (maxRuns <= 0) {
      error("max_count_of_runs out of range, using default");
      maxRuns = DEF_MAX_COUNT_OF_RUNS;
    }

    // Maximum time elapsed for one run, 0 means infinity
    int maxTime= DEF_MAX_TIME_PER_RUN;
    params.get("max_time_per_run", &maxTime);
    if (maxTime < 0) {
      error("max_time_per_run out of range, using default");
      maxTime = DEF_MAX_TIME_PER_RUN;
    }

    // Step width (only for blind solver)
    int stepWidth= DEF_STEP_WIDTH;
    params.get("step_width", &stepWidth);
    if (stepWidth <= 0) {
      error("step_width out of range, using default");
      stepWidth = DEF_STEP_WIDTH;
    }

    if (useBlindSolver) {
      // exclude parameters for blind solver
      if (maxRuns != DEF_MAX_COUNT_OF_RUNS) {
        error("Parameter 'max_count_of_runs' is irrelevant for blind solver");
      }
      maxRuns = 1;
    } else {
      // exclude parameters for GA solver
      if (stepWidth != DEF_STEP_WIDTH) {
        error("Parameter 'step_width' is irrelevant for GA solver");
        stepWidth = DEF_STEP_WIDTH;
      }
    }

    std::cerr << "Creating solver...\n";
    AbstractSatSolver *solver= 0;
    FitnessWatch *fitnessWatch= 0;
    /*if (useBlindSolver) {
      solver = new BlindSatSolver(problem, stepWidth);
      }  else*/ {
      GASatSolver *gaSolver= GASatSolver::create(problem, params);
      fitnessWatch = new FitnessWatch(gaSolver, std::cout);
      gaSolver->addObserver(fitnessWatch);
      solver = gaSolver;
    }
    SatSolverDestructor solverDestructor(solver);

    SolutionsCountStop *maxSlnsStop= new SolutionsCountStop(solver, maxSlns);
    solver->addObserver(maxSlnsStop);

    TimedStop *timedStop = 0;
    if (maxTime) {
      timedStop = new TimedStop(solver, maxTime);
      solver->addObserver(timedStop);
    }

    SatItemVector *results= 0;
    int nSolutions = 0;
    for(int i=0; i<maxRuns; i++) {
      if (1<maxRuns)
        std::cout << ">>> Run" << std::setw(4) << i+1 << " of" << std::setw(4) << maxRuns << std::endl;
      solver->reset();
      if (fitnessWatch)
        fitnessWatch->reset();
      solver->start();
      delete results;
      results= solver->getSolutionVector();
      nSolutions= results->getLength();
      const float timeElapsed= solver->getTimeElapsed()/1000.0;
      std::cout
        << "<<< Found " << nSolutions << " solutions"
        << " in " << std::fixed << std::setw(5) << std::setprecision(2) << timeElapsed << " s"
        << std::endl;
      if (nSolutions >= minSlns)
        break;
      std::cout << std::endl;
    }
    if (nSolutions >= minSlns)
      results->writeOut(solver->getProblem(), std::cout);

    if (!useBlindSolver) {
      GASatSolver *gaSolver= dynamic_cast<GASatSolver *>(solver);
      GAStatistics stats= gaSolver->getStatistics();
      std::cout << "--- GA Statistics" << std::endl << stats << std::endl;
    }

    delete results;
    delete fitnessWatch;
    delete timedStop;
    delete maxSlnsStop;

    return 0;
  }
  catch (GenericException e) {
    error(e.getText());
    return 1;
  }
}

