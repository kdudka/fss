#ifndef SATPROBLEMIMPL_H
#define SATPROBLEMIMPL_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include "SatSolver.h"

namespace FastSatSolver {

  enum EToken {
    T_EOF,
    T_FALSE,
    T_TRUE,
    T_NOT,
    T_AND,
    T_OR,
    T_XOR,
    T_LPAR,
    T_RPAR,
    T_DELIM,
    T_STRING,
    T_ERR_LEX = -1,
    T_ERR_PARSE = -2
  };


  struct Token {
    int           m_line;
    EToken        m_token;
    int           m_ext_number;
    std::string   m_ext_text;
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
      virtual bool eval (ISatItem *data ) = 0;
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

      bool eval (ISatItem *data );
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

    private:
      FILE *fd_;
      enum EState {
        STATE_INIT,
        STATE_BUILDING_STRING
      };
      EState state_;
      int line_;
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

    private:
      typedef std::string TVarName;
      typedef std::vector<TVarName> TIndexToName;
      typedef std::map<TVarName, int> TNameToIndex;
      TIndexToName indexToName_;
      TNameToIndex nameToIndex_;
      int currentIndex_;
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

    private:
      IScanner *scanner_;
      VariableContainer *vc_;
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

    private:
      IScanner *scanner_;
      FormulaContainer *fc_;
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

    private:
      void parseFile(FILE *);
      void parserLoop(IScanner *);
  };

} // namespace FastSatSolver


#endif // SATPROBLEMIMPL_H
