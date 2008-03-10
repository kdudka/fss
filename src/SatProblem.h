#ifndef SATPROBLEMIMPL_H
#define SATPROBLEMIMPL_H

#include <string>
#include "Scanner.h"

namespace FastSatSolver {

  class VariableContainer
  {
    public:
      VariableContainer();
      ~VariableContainer();
      /**
       * @return int
       */
      int getLength ( );

      /**
       * @return string
       * @param  index
       */
      std::string getVarName (int index );

      /**
       * @return int
       * @param  name
       */
      int addVariable (std::string name );

    private:
      struct Private;
      Private *d;
  };


  class ISatItem;
  class IFormulaEvaluator;
  class FormulaContainer
  {
    public:
      FormulaContainer();
      ~FormulaContainer();

      /**
       * @return int
       */
      int getLength ( );

      /**
       * @return int
       * @param  data
       */
      int evalAll (ISatItem *data );

      /**
       * @param  formula
       */
      void addFormula (IFormulaEvaluator *formula );

    private:
      struct Private;
      Private *d;
  };


  class SatProblem {
    public:
      SatProblem();
      ~SatProblem();

      /**
       * @param  fileName
       */
      void loadFromFile (std::string fileName );

      /**
      */
      void loadFromInput ( );

      /**
       * @return int
       */
      int getVarsCount ( );

      /**
       * @return std::string
       * @param  index
       */
      std::string getVarName (int index );

      /**
       * @return
       */
      int getFormulasCount();

      /**
       * @return int
       * @param  data
       */
      int getSatsCount (ISatItem *data);

      /**
       * @return bool
       */
      bool hasError ( );

    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver


#endif // SATPROBLEMIMPL_H
