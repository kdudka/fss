#ifndef SATPROBLEMIMPL_H
#define SATPROBLEMIMPL_H

#include <string>
#include "SatSolver.h"

namespace FastSatSolver {

  enum EToken {
  };


  struct Token
  {
      EToken        m_token;
      int           m_number;
      std::string   m_text;
  };


  class IScanner
  {
    public:
      virtual ~IScanner() { }

      /**
       * @return int
       * @param  token
       */
      virtual int readNext (Token* token ) = 0;
  };

  class IFormulaBuilder
  {
    public:
      virtual ~IFormulaBuilder() { }

      /**
       * @param  token
       */
      virtual void parse (Token token ) = 0;

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
      virtual void eval (ISatItem *data ) = 0;
  };


  class Formula : public IFormulaBuilder, public IFormulaEvaluator
  {
    public:
      /**
       * @param  token
       */
      void parse (Token token );

      /**
       * @return bool
       */

      bool isValid ( );
      /**
       * @param  data
       */

      void eval (ISatItem *data );
  };


  class RawScanner : public IScanner
  {
    public:
      /**
       * @param  fd
       */
      RawScanner (FILE* fd );

      /**
       * @return int
       * @param  token
       */
      virtual int readNext (Token* token );
  };


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
  };


  class ScannerStringHandler : public IScanner
  {
    public:
      /**
       * @param  scanner
       * @param  vc
       */
      ScannerStringHandler (IScanner *scanner, VariableContainer *vc );

      /**
       * @return int
       * @param  token
       */
      virtual int readNext (Token* token );
  };


  class FormulaContainer
  {
    public:
      /**
       * @return int
       */
      int getlength ( );

      /**
       * @param  index
       */
      void getItem (int index );

      /**
       * @return int
       * @param  data
       */
      int evalAll (ISatItem *data );

      /**
       * @param  formula
       */
      void addFormula (IFormulaEvaluator *formula );
  };


  class ScannerFormulaHandler : public IScanner
  {
    public:

      /**
       * @param  scanner
       * @param  fc
       */
      ScannerFormulaHandler (IScanner *scanner, FormulaContainer *fc );

      /**
       * @return int
       * @param  token
       */
      virtual int readNext (Token* token );
  };


  class SatProblemImpl : public SatProblem
  {
    public:
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
       * @return int
       * @param  data
       */
      virtual int getSatsCount (ISatItem *data);

      /**
       * @return bool
       */
      virtual bool hasError ( );

  };

} // namespace FastSatSolver


#endif // SATPROBLEMIMPL_H
