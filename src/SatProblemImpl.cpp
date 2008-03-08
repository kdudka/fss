#include "SatProblemImpl.h"

using std::string;

namespace SatSolver {

  /**
   * @return SatProblem*
   */
  SatProblem* SatProblem::create ( ) {

  }


  /**
   * @param  fileName
   */
  void SatProblemImpl::loadFromFile (string fileName ) {

  }


  /**
  */
  void SatProblemImpl::loadFromInput ( ) {

  }


  /**
   * @return int
   */
  int SatProblemImpl::getVarsCount ( ) {

  }


  /**
   * @return string
   * @param  index
   */
  string SatProblemImpl::getVarName (int index ) {

  }


  /**
   * @return int
   * @param  data
   */
  int SatProblemImpl::getSatsCount (ISatItem *data ) {

  }


  /**
   * @return bool
   */
  bool SatProblemImpl::hasError ( ) {

  }


  /**
   * @return int
   */
  int VariableContainer::getLength ( ) {

  }


  /**
   * @return string
   * @param  index
   */
  string VariableContainer::getVarName (int index ) {

  }


  /**
   * @return int
   * @param  name
   */
  int VariableContainer::addVariable (string name ) {

  }


  /**
   * @param  token
   */
  void Formula::parse (Token token ) {

  }

  /**
   * @return bool
   */
  bool Formula::isValid ( ) {

  }

  /**
   * @param  data
   */
  void Formula::eval (ISatItem *data ) {

  }

  /**
   * @return int
   */
  int FormulaContainer::getlength ( ) {

  }


  /**
   * @param  index
   */
  void FormulaContainer::getItem (int index ) {

  }


  /**
   * @return int
   * @param  data
   */
  int FormulaContainer::evalAll (ISatItem *data ) {

  }


  /**
   * @param  formula
   */
  void FormulaContainer::addFormula (IFormulaEvaluator *formula ) {

  }

  /**
   * @param  fd
   */
  RawScanner::RawScanner (FILE* fd ) {

  }

  /**
   * @return int
   * @param  token
   */
  int RawScanner::readNext (Token* token ) {

  }


  /**
   * @param  scanner
   * @param  vc
   */
  ScannerStringHandler::ScannerStringHandler (IScanner *scanner, VariableContainer *vc ) {

  }

  /**
   * @return int
   * @param  token
   */
  int ScannerStringHandler::readNext (Token* token ) {

  }

  /**
   * @param  scanner
   * @param  fc
   */
  ScannerFormulaHandler::ScannerFormulaHandler (IScanner *scanner, FormulaContainer *fc ) {

  }

  /**
   * @return int
   * @param  token
   */
  int ScannerFormulaHandler::readNext (Token* token ) {

  }


} // SatSolver

