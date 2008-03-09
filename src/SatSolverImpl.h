#ifndef SATSOLVERIMPL_H
#define SATSOLVERIMPL_H

#include "SatSolver.h"

namespace FastSatSolver {

  class SatSolverEngine: public ISatSolverStats {
    public:
      SatSolverEngine(SatProblem *problem, SatSolverParameters *params);
      virtual ~SatSolverEngine();
      virtual SatSolverStatsProxy* getStatsProxy();
      void doStep();
    private:
      struct Private;
      Private *d;
  };
} // namespace FastSatSolver


#endif // SATSOLVERIMPL_H
