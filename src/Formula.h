#ifndef FOROMULA_H
#define FOROMULA_H

#include "SatProblemImpl.h"


namespace FastSatSolver {

  class IFormulaBuilder
  {
    public:
      virtual ~IFormulaBuilder() { }

      /**
       * @param  token
       */
      virtual int parse (Token token ) = 0;

      /**
       * @return bool
       */
      virtual bool isValid ( ) = 0;
  };

  class IFormulaEvaluator
  {
    public:
      virtual ~IFormulaEvaluator() { }

      /**
       * @param  data
       */
      virtual bool eval (ISatItem *data ) = 0;
  };


  class InterpretedFormula : public IFormulaBuilder, public IFormulaEvaluator
  {
    public:
      InterpretedFormula();
      ~InterpretedFormula();

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

    private:
      struct Private;
      Private *const d;
  };


} // namespace FastSatSolver


#endif // FOROMULA_H
