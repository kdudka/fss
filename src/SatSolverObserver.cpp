#include <iostream>
#include <iomanip>
#include <ga/GAStatistics.h>
#include "fssIO.h"
#include "SatSolver.h"
#include "GaSatSolver.h"
#include "SatSolverObserver.h"

namespace FastSatSolver {

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SolutionsCountStop implementation
  struct SolutionsCountStop::Private {
    AbstractSatSolver *solver;
    int               minCountOfSolutions;
  };
  SolutionsCountStop::SolutionsCountStop(AbstractSatSolver *solver, int minCountOfSolutions):
    d(new Private)
  {
    d->solver = solver;
    d->minCountOfSolutions = minCountOfSolutions;
  }
  SolutionsCountStop::~SolutionsCountStop() {
    delete d;
  }
  void SolutionsCountStop::notify() {
    const int nSolutions= d->solver->getSolutionsCount();
    if (nSolutions >= d->minCountOfSolutions)
      d->solver->stop();
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // TimedStop implementation
  struct TimedStop::Private {
    AbstractProcessWatched *process;
    long msec;
  };
  TimedStop::TimedStop(AbstractProcessWatched *process, long msec):
    d(new Private)
  {
    d->process = process;
    d->msec = msec;
  }
  TimedStop::~TimedStop() {
    delete d;
  }
  void TimedStop::notify() {
    long elapsed = d->process->getTimeElapsed();
    if (elapsed > d->msec)
      d->process->stop();
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // FitnessWatch implementation
  struct FitnessWatch::Private {
    AbstractSatSolver   *solver;
    std::ostream        &stream;
    float               maxFitness;

    Private(std::ostream &streamTo): stream(streamTo) { }
  };
  FitnessWatch::FitnessWatch(AbstractSatSolver *solver, std::ostream &streamTo):
    d(new Private(streamTo))
  {
    d->solver = solver;
    d->maxFitness = 0.0;
  }
  FitnessWatch::~FitnessWatch() {
    delete d;
  }
  void FitnessWatch::notify() {
    AbstractSatSolver *solver= d->solver;
    float maxFitness = solver->maxFitness();
    if (maxFitness <= d->maxFitness)
      // Fitness not changed
      return;

    int generation = 0;
    GaSatSolver *gaSolver= dynamic_cast<GaSatSolver *>(solver);
    if (gaSolver) {
      GAStatistics stats= gaSolver->getStatistics();
      generation = stats.generation();
      if (0 == generation)
        return;
    }

    // Save maxFitness for next call
    d->maxFitness = maxFitness;

    // Read other statistics
    const float minFitness= solver->minFitness();
    const float avgFitness= solver->avgFitness();
    const float timeElapsed= d->solver->getTimeElapsed()/1000.0;

    using StreamDecorator::FixedFloat;
    d->stream
      << "--- satisfaction:" << FixedFloat(3,1) << maxFitness*100.0 << "%"
      << "(avg:" << FixedFloat(3,1) << avgFitness*100.0
      << ", min:" << FixedFloat(3,1) << minFitness*100.0 << ")"
      << ", generation " << std::setw(5) << generation
      << ", time elapsed: " << FixedFloat(5,2) << timeElapsed << " s"
      << std::endl;
  }
  void FitnessWatch::reset() {
    d->maxFitness = 0.0;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // ResultsWatch implementation
  struct ResultsWatch::Private {
    AbstractSatSolver   *solver;
    std::ostream        &stream;
    int                 nResults;

    Private(std::ostream &streamTo): stream(streamTo) { }
  };
  ResultsWatch::ResultsWatch(AbstractSatSolver *solver, std::ostream &streamTo):
    d(new Private(streamTo))
  {
    d->solver = solver;
    d->nResults = 0;
  }
  ResultsWatch::~ResultsWatch() {
    delete d;
  }
  void ResultsWatch::notify() {
    using namespace StreamDecorator;
    AbstractSatSolver *solver= d->solver;
    const int nResults= solver->getSolutionsCount();
    if (nResults <= d->nResults)
      return;
    d->nResults = nResults;

    const float timeElapsed= solver->getTimeElapsed()/1000.0;
    d->stream
      << Color(C_LIGHT_BLUE) << "--- solution #" << nResults
      << " found in " << FixedFloat(5,2) << timeElapsed << " s"
      << Color() << std::endl;
  }

} // namespace FastSatSolver

