#ifndef BLINDSATSOLVER_H
#define BLINDSATSOLVER_H

#include "SatSolver.h"

namespace FastSatSolver {

  class LongSatItem: public ISatItem {
    public:
      LongSatItem(int length, long fromNumber);
      virtual ~LongSatItem();
      virtual int getLength() const;
      virtual bool getBit(int index) const;
      virtual LongSatItem* clone() const;
    private:
      int  length_;
      long lNumber_;
  };


  class BlindSatSolver: public AbstractSatSolver
  {
    public:
      BlindSatSolver(SatProblem *problem, int stepWidth);
      virtual ~BlindSatSolver();

      /**
       * @return SatProblemSolver*
       * @param  problem
       */
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

