#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "SatProblemImpl.h"
#include "Formula.h"

using std::string;

namespace FastSatSolver {

  /**
   * @return SatProblem*
   */
  SatProblem* SatProblem::create ( ) {
    return new SatProblemImpl;
  }

  SatProblemImpl::SatProblemImpl():
    hasError_(false)
  {
  }

  /**
   * @param  fileName
   */
  void SatProblemImpl::loadFromFile (string fileName ) {
    fileName_ = fileName;

    // OpenedFile RAII
    class OpenedFileRAII {
      public:
        OpenedFileRAII(string fileName) {
          fd_ = fopen(fileName.c_str(), "r");
          if (0== fd_) {
            string error("Could not open file: ");
            throw GenericException(error + fileName);
          }
        }
        ~OpenedFileRAII() { fclose(fd_); }
        FILE* getFd() { return fd_; }
      private:
        FILE* fd_;
    } openedFile(fileName);
    this->parseFile(openedFile.getFd());
  }


  /**
  */
  void SatProblemImpl::loadFromInput ( ) {
    fileName_ = "-";
    this->parseFile(stdin);
  }


  // @private
  void SatProblemImpl::parseFile(FILE *fd) {
    // RawScanner RAII
    class RawScanRAII {
      public:
        RawScanRAII(FILE *fd) { ptr_ = new RawScanner(fd); }
        ~RawScanRAII() { delete ptr_; }
        RawScanner* instance() { return ptr_; }
      private:
        RawScanner *ptr_;
    } rawScan(fd);

    // ScannerStringHandler RAII
    class StringScanRAII {
      public:
        StringScanRAII(IScanner *scan, VariableContainer *vc) {
          ptr_ = new ScannerStringHandler(scan, vc);
        }
        ~StringScanRAII() { delete ptr_; }
        ScannerStringHandler* instance() { return ptr_; }
      private:
        ScannerStringHandler *ptr_;
    } stringScan(rawScan.instance(), &vc_);

    // ScannerFormulaHandler RAII
    class FormulaScanRAII {
      public:
        FormulaScanRAII(IScanner *scan, FormulaContainer *fc) {
          ptr_ = new ScannerFormulaHandler(scan, fc);
        }
        ~FormulaScanRAII() { delete ptr_; }
        ScannerFormulaHandler* instance() { return ptr_; }
      private:
        ScannerFormulaHandler *ptr_;
    } formulaScan(stringScan.instance(), &fc_);

    this->parserLoop(formulaScan.instance());
  }


  // @private
  void SatProblemImpl::parserLoop(IScanner *scanner) {
    Token token;
    while (0== scanner->readNext(&token) && T_EOF!=token.m_token) {
      switch (token.m_token) {
        case T_ERR_LEX:
        case T_ERR_EXPR:
        case T_ERR_PARSE:
          this->printError(token);
          break;

        default:
          throw GenericException("Unhandled token in SatProblemImp::parserLoop");
      }
    }
  }


  // @private
  void SatProblemImpl::printError(Token token) {
    this->hasError_ = true;
    std::cerr << fileName_ << ":" << token.m_line << ": error: ";
    switch (token.m_token) {
      case T_ERR_LEX:   std::cerr << "lexical error";     break;
      case T_ERR_EXPR:  std::cerr << "expression error";  break;
      case T_ERR_PARSE: std::cerr << "syntax error";      break;
      default:
          throw GenericException("Unhandled error in SatProblemImp::printError");
    }
    std::cerr << std::endl;
  }


  /**
   * @return int
   */
  int SatProblemImpl::getVarsCount ( ) {
    return vc_.getLength();
  }


  /**
   * @return string
   * @param  index
   */
  string SatProblemImpl::getVarName (int index ) {
    return vc_.getVarName(index);
  }


  /**
   * @return
   */
  int SatProblemImpl::getFormulasCount() {
    return fc_.getLength();
  }


  /**
   * @return int
   * @param  data
   */
  int SatProblemImpl::getSatsCount (ISatItem *data ) {
    return fc_.evalAll(data);
  }


  /**
   * @return bool
   */
  bool SatProblemImpl::hasError ( ) {
    return hasError_;
  }


  /**
   * @return int
   */
  int VariableContainer::getLength ( ) {
    return currentIndex_;
  }


  /**
   * @return string
   * @param  index
   */
  string VariableContainer::getVarName (int index ) {
    assert(index < currentIndex_);
    return indexToName_[index];
  }


  /**
   * @return int
   * @param  name
   */
  int VariableContainer::addVariable (string name ) {
    if (nameToIndex_.end() != nameToIndex_.find(name))
      // Variable already exists
      return nameToIndex_[name];

    // Add new variable
    nameToIndex_[name] = currentIndex_;
    indexToName_.push_back(name);
    return currentIndex_++;
  }


  FormulaContainer::~FormulaContainer() {
    TContainer::iterator iter;
    for(iter=container_.begin(); iter!=container_.end(); iter++)
      delete *iter;
  }


  /**
   * @return int
   */
  int FormulaContainer::getLength ( ) {
    return container_.size();
  }


  /**
   * @param  index
   *
  void FormulaContainer::getItem (int index ) {

  }*/


  /**
   * @return int
   * @param  data
   */
  int FormulaContainer::evalAll (ISatItem *data ) {
    int counter = 0;

    TContainer::iterator iter;
    for(iter=container_.begin(); iter!=container_.end(); iter++) {
      IFormulaEvaluator *fe = *iter;
      counter += fe->eval(data);
    }

    return counter;
  }


  /**
   * @param  formula
   */
  void FormulaContainer::addFormula (IFormulaEvaluator *formula ) {
    container_.push_back(formula);
  }

} // namespace FastSatSolver
