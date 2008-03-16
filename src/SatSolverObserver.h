#ifndef SATSOLVEROBSERVER_H
#define SATSOLVEROBSERVER_H

/**
 * @file SatSolverObserver.h
 * @brief Set of useful observers attachable to AbstractSatSolver or its base
 * classes.
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-03-16
 * @ingroup SatSolver
 */

#include <iostream>
#include "SatSolver.h"

namespace FastSatSolver {

  class TimedStop: public IObserver {
    public:
      TimedStop(AbstractProcessWatched *process, long msec);
      virtual ~TimedStop();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };


  class SolutionsCountStop: public IObserver {
    public:
      SolutionsCountStop(AbstractSatSolver *solver, int minCountOfSolutions);
      virtual ~SolutionsCountStop();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };


  class FitnessWatch: public IObserver {
    public:
      FitnessWatch(AbstractSatSolver *solver, std::ostream &streamTo);
      virtual ~FitnessWatch();
      virtual void notify();
      void reset();
    private:
      struct Private;
      Private *d;
  };

  class ResultsWatch: public IObserver {
    public:
      ResultsWatch(AbstractSatSolver *solver, std::ostream &streamTo);
      virtual ~ResultsWatch();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

  class ProgressWatch: public IObserver {
    public:
      ProgressWatch(AbstractProcess *process, int stepsTotal, std::ostream &streamTo);
      virtual ~ProgressWatch();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver

#endif // SATSOLVEROBSERVER_H
