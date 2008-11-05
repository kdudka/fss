/*
 * Copyright (C) 2008 Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
 *
 * This file is part of fss (Fast SAT Solver).
 *
 * fss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * fss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fss.  If not, see <http://www.gnu.org/licenses/>.
 */

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
      << Color(C_LIGHT_BLUE) << "--- " << nResults
      << ". solution found in " << FixedFloat(5,2) << timeElapsed << " s"
      << Color() << std::endl;
  }

  struct ProgressWatch::Private {
    AbstractProcess *process;
    int             stepsTotal;
    int             last;
    std::ostream    &stream;

    Private(std::ostream &streamTo): stream(streamTo) { }
  };
  ProgressWatch::ProgressWatch(AbstractProcess *process, int stepsTotal, std::ostream &streamTo):
    d(new Private(streamTo))
  {
    d->process = process;
    d->stepsTotal = stepsTotal;
    d->last = 0;
  }
  ProgressWatch::~ProgressWatch() {
    delete d;
  }
  void ProgressWatch::notify() {
    using namespace StreamDecorator;

    // Check percentage value
    const int currentStep= d->process->getStepsCount();
    const int percents=
      currentStep*100 /
      d->stepsTotal;
    if (percents == d->last)
      return;
    d->last = percents;

    // Write out message
    d->stream
      << Color(C_GREEN) << "--- Progress:"
      << std::setw(3) << percents << "%"
      << Color() << std::endl;
  }

} // namespace FastSatSolver

