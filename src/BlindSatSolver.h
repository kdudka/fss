#ifndef BLINDSATSOLVER_H
#define BLINDSATSOLVER_H

/**
 * @file BlindSatSolver.h
 * @brief BlindSatSolver class using brute force method to solve SAT problem.
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-03-16
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
       * @param lenth Item length responds the count of variables.
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

