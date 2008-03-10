#ifndef SATPROBLEMIMPL_H
#define SATPROBLEMIMPL_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include "SatSolver.h"
#include "Scanner.h"

namespace FastSatSolver {

  class VariableContainer
  {
    public:
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
      typedef std::string TVarName;
      typedef std::vector<TVarName> TIndexToName;
      typedef std::map<TVarName, int> TNameToIndex;
      TIndexToName indexToName_;
      TNameToIndex nameToIndex_;
      int currentIndex_;
  };


  class IFormulaEvaluator;
  class FormulaContainer
  {
    public:
      ~FormulaContainer();

      /**
       * @return int
       */
      int getLength ( );

      /**
       * @param  index
       *
      void getItem (int index );*/

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
      typedef std::list<IFormulaEvaluator *> TContainer;
      TContainer container_;
  };


  class SatProblemImpl : public SatProblem
  {
    public:
      SatProblemImpl();

      /**
       * @param  fileName
       */
      virtual void loadFromFile (std::string fileName );

      /**
      */
      virtual void loadFromInput ( );

      /**
       * @return int
       */
      virtual int getVarsCount ( );

      /**
       * @return std::string
       * @param  index
       */
      virtual std::string getVarName (int index );

      /**
       * @return
       */
      virtual int getFormulasCount();

      /**
       * @return int
       * @param  data
       */
      virtual int getSatsCount (ISatItem *data);

      /**
       * @return bool
       */
      virtual bool hasError ( );

    private:
      bool                hasError_;
      VariableContainer   vc_;
      FormulaContainer    fc_;
      std::string         fileName_;

    private:
      void parseFile(FILE *);
      void parserLoop(IScanner *);
      void printError(Token);
  };

} // namespace FastSatSolver


#endif // SATPROBLEMIMPL_H
