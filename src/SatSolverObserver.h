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

#ifndef SATSOLVEROBSERVER_H
#define SATSOLVEROBSERVER_H

/**
 * @file SatSolverObserver.h
 * @brief Set of useful observers attachable to AbstractSatSolver or its base
 * classes.
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
 * @ingroup SatSolver
 */

#include <iostream>
#include "SatSolver.h"

namespace FastSatSolver {

  /**
   * @brief Observer which stops process after specified time.
   * @ingroup SatSolver
   */
  class TimedStop: public IObserver {
    public:
      /**
       * @param process Observed process.
       * @param msec Time in milliseconds to stop process after.
       */
      TimedStop(AbstractProcessWatched *process, long msec);
      virtual ~TimedStop();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief Observer which write out progress percentage when it is changed.
   * @ingroup SatSolver
   */
  class ProgressWatch: public IObserver {
    public:
      /**
       * @param process Observed process.
       * @param stepsTotal Count of steps corresponding to 100% of progress.
       * @param streamTo Standard output stream to write to.
       */
      ProgressWatch(
                    AbstractProcess   *process,
                    int               stepsTotal,
                    std::ostream      &streamTo);
      
      virtual ~ProgressWatch();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief Observer which stop solver after specified count of solutions is
   * found.
   * @ingroup SatSolver
   */
  class SolutionsCountStop: public IObserver {
    public:
      /**
       * @param solver Observed solver.
       * @param minCountOfSolutions Count of solutions to stop solver after.
       */
      SolutionsCountStop(AbstractSatSolver *solver, int minCountOfSolutions);
      virtual ~SolutionsCountStop();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief Observer which write out message when maxFitness value is increased.
   * @ingroup SatSolver
   */
  class FitnessWatch: public IObserver {
    public:
      /**
       * @param solver Observed solver.
       * @param streamTo Standard output stream to write to.
       */
      FitnessWatch(AbstractSatSolver *solver, std::ostream &streamTo);
      virtual ~FitnessWatch();
      virtual void notify();
      void reset();
    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief Observer which write out message when solution is found.
   * @ingroup SatSolver
   */
  class ResultsWatch: public IObserver {
    public:
      /**
       * @param solver Observed solver.
       * @param streamTo Standard output stream to write to.
       */
      ResultsWatch(AbstractSatSolver *solver, std::ostream &streamTo);
      virtual ~ResultsWatch();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver

#endif // SATSOLVEROBSERVER_H
