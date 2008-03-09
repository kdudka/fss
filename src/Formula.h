#ifndef FOROMULA_H
#define FOROMULA_H

#include "SatProblemImpl.h"


namespace FastSatSolver {

  class Formula : public IFormulaBuilder, public IFormulaEvaluator
  {
    public:
      /**
       * @param  token
       */
      int parse (Token token );

      /**
       * @return bool
       */

      bool isValid ( );
      /**
       * @param  data
       */

      bool eval (ISatItem *data );
  };


} // namespace FastSatSolver


#endif // FOROMULA_H
