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

#ifndef BLINDSATSOLVER_H
#define BLINDSATSOLVER_H

/**
 * @file BlindSatSolver.h
 * @brief BlindSatSolver class using brute force method to solve SAT problem.
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
 * @ingroup SatSolver
 */

#include "SatSolver.h"

namespace FastSatSolver {

  /**
   * @brief ISatItem implementation used by BlindSatSolver
   * @ingroup SatSolver
   */
  class LongSatItem: public ISatItem {
    public:
      /**
       * @param length Item length responds the count of variables.
       * @param fromNumber Long number representing item's data (bit by bit).
       */
      LongSatItem(int length, long fromNumber);
      virtual ~LongSatItem();
      virtual int getLength() const;
      virtual bool getBit(int index) const;
      virtual LongSatItem* clone() const;
    private:
      int  length_;
      long lNumber_;
  };

  /**
   * @brief Solver using brute force method to solve SAT problem.
   * @ingroup SatSolver
   */
  class BlindSatSolver: public AbstractSatSolver
  {
    public:
      /**
       * @param problem SatProblem instance containing SAT problem to solve.
       * @param stepWidth Number of bits explored in one step. This influences
       * the granullarity of notifications and process control. Recomended
       * value for ordinary machines is 16.
       */
      BlindSatSolver(SatProblem *problem, int stepWidth);
      virtual ~BlindSatSolver();
      virtual SatProblem* getProblem();
      virtual int getSolutionsCount();
      virtual SatItemVector* getSolutionVector();
      virtual float minFitness();
      virtual float avgFitness();
      virtual float maxFitness();

    protected:
      virtual void initialize();
      virtual void doStep();

    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver

#endif // BLINDSATSOLVER_H

