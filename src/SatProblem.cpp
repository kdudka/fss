/*
 * Copyright (C) 2008 Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
 *
 * This file is part of fss (Fast SAT Solver).
 *
 * fss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * fss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fss.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "fssIO.h"
#include "SatSolver.h"
#include "Scanner.h"
#include "Formula.h"
#include "SatProblem.h"

using std::string;

namespace FastSatSolver {

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatProblem implementation
  struct SatProblem::Private {
    bool                hasError;
    VariableContainer   vc;
    FormulaContainer    fc;
    std::string         fileName;

    void parseFile(FILE *);
    void parserLoop(IScanner *);
    void printError(Token);
  };
  SatProblem::SatProblem():
    d(new Private)
  {
    d->hasError = false;
  }
  SatProblem::~SatProblem() {
    delete d;
  }
  void SatProblem::loadFromFile (std::string fileName ) {
    d->fileName = fileName;

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
    d->parseFile(openedFile.getFd());
  }


  /**
  */
  void SatProblem::loadFromInput ( ) {
    d->fileName = "-";
    d->parseFile(stdin);
  }


  // @private
  void SatProblem::Private::parseFile(FILE *fd) {
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
    } stringScan(rawScan.instance(), &vc);

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
    } formulaScan(stringScan.instance(), &fc);

    this->parserLoop(formulaScan.instance());
    if (0==fc.getLength() || 0==vc.getLength())
      hasError = true;
}


  // @private
  void SatProblem::Private::parserLoop(IScanner *scanner) {
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
  void SatProblem::Private::printError(Token token) {
    this->hasError = true;
    std::cerr << fileName << ":" << token.m_line << ": error: ";
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
  int SatProblem::getVarsCount ( ) {
    return d->vc.getLength();
  }


  /**
   * @return string
   * @param  index
   */
  string SatProblem::getVarName (int index ) {
    return d->vc.getVarName(index);
  }


  /**
   * @return
   */
  int SatProblem::getFormulasCount() {
    return d->fc.getLength();
  }


  /**
   * @return int
   * @param  data
   */
  int SatProblem::getSatsCount (ISatItem *data ) {
    return d->fc.evalAll(data);
  }


  /**
   * @return bool
   */
  bool SatProblem::hasError ( ) {
    return d->hasError;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // VariableContainer implementation
  struct VariableContainer::Private {
    typedef std::string               TVarName;
    typedef std::vector<TVarName>     TIndexToName;
    typedef std::map<TVarName, int>   TNameToIndex;
    TIndexToName    indexToName;
    TNameToIndex    nameToIndex;
    int             currentIndex;
  };
  VariableContainer::VariableContainer():
    d(new Private)
  {
    d->currentIndex = 0;
  }
  VariableContainer::~VariableContainer() {
    delete d;
  }
  int VariableContainer::getLength ( ) {
    return d->currentIndex;
  }
  string VariableContainer::getVarName (int index ) {
    assert(index < d->currentIndex);
    return d->indexToName[index];
  }
  int VariableContainer::addVariable (std::string name ) {
    if (d->nameToIndex.end() != d->nameToIndex.find(name))
      // Variable already exists
      return d->nameToIndex[name];

    // Add new variable
    d->nameToIndex[name] = d->currentIndex;
    d->indexToName.push_back(name);
    return (d->currentIndex)++;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // FormulaContainer implementation
  struct FormulaContainer::Private {
      typedef std::list<IFormulaEvaluator *> TContainer;
      TContainer container;
  };
  FormulaContainer::FormulaContainer():
    d(new Private)
  {
  }
  FormulaContainer::~FormulaContainer() {
    Private::TContainer::iterator iter;
    for(iter=d->container.begin(); iter!=d->container.end(); iter++)
      delete *iter;
    delete d;
  }


  /**
   * @return int
   */
  int FormulaContainer::getLength ( ) {
    return d->container.size();
  }


  /**
   * @return int
   * @param  data
   */
  int FormulaContainer::evalAll (ISatItem *data ) {
    int counter = 0;

    Private::TContainer::iterator iter;
    for(iter=d->container.begin(); iter!=d->container.end(); iter++) {
      IFormulaEvaluator *fe = *iter;
      counter += fe->eval(data);
    }

    return counter;
  }

  /**
   * @param  formula
   */
  void FormulaContainer::addFormula (IFormulaEvaluator *formula ) {
    d->container.push_back(formula);
  }

} // namespace FastSatSolver
