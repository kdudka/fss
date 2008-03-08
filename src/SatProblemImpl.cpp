#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "SatProblemImpl.h"

using std::string;

namespace FastSatSolver {

#ifndef NDEBUG
  std::ostream& operator<< (std::ostream &stream, Token token) {
    stream << "Token: ";
    switch (token.m_token) {
      case T_EOF:       stream << "T_EOF";        break;
      case T_FALSE:     stream << "T_FALSE";      break;
      case T_TRUE:      stream << "T_TRUE";       break;
      case T_NOT:       stream << "T_NOT";        break;
      case T_AND:       stream << "T_AND";        break;
      case T_OR:        stream << "T_OR";         break;
      case T_XOR:       stream << "T_XOR";        break;
      case T_LPAR:      stream << "T_LPAR";       break;
      case T_RPAR:      stream << "T_RPAR";       break;
      case T_DELIM:     stream << "T_DELIM";      break;
      case T_STRING:    stream << "T_STRING";     break;
      case T_VARIABLE:  stream << "T_VARIABLE";   break;
      case T_ERR_LEX:   stream << "T_ERR_LEX";    break;
      case T_ERR_EXPR:  stream << "T_ERR_EXPR";   break;
      case T_ERR_PARSE: stream << "T_ERR_PARSE";  break;
      default:          stream << token.m_token;  break;
    }
    stream << std::endl;
    stream << "       At line: " << token.m_line  << std::endl;
    if (T_VARIABLE == token.m_token) {
      stream << " Variable name: " << token.m_ext_text << std::endl;
      stream << "   Vairable id: " << token.m_ext_number << std::endl;
    }
    return stream;
  }
#endif // NDEBUG

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


  /**
   * @param  token
   */
  int Formula::parse (Token token ) {
#ifndef NDEBUG
          std::cerr << token << std::endl;
#endif // NDEBUG
  }

  /**
   * @return bool
   */
  bool Formula::isValid ( ) {
    // TODO
  }

  /**
   * @param  data
   */
  bool Formula::eval (ISatItem *data ) {
    // TODO
  }

  /**
   * @return int
   */
  int FormulaContainer::getlength ( ) {
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

  /**
   * @param  fd
   */
  RawScanner::RawScanner (FILE* fd ):
    fd_(fd),
    state_(STATE_INIT),
    line_(1)
  {
  }

  /**
   * @return int
   * @param  token
   */
  int RawScanner::readNext (Token* token ) {
    while (1) {
      token->m_line = line_;
      int c = fgetc(fd_);
      switch(state_) {
        case STATE_INIT:
          switch (c) {
            case '0': token->m_token = T_FALSE;       return 0;
            case '1': token->m_token = T_TRUE;        return 0;
            case '~': token->m_token = T_NOT;         return 0;
            case '&': token->m_token = T_AND;         return 0;
            case '|': token->m_token = T_OR;          return 0;
            case '^': token->m_token = T_XOR;         return 0;
            case '(': token->m_token = T_LPAR;        return 0;
            case ')': token->m_token = T_RPAR;        return 0;
            case ';': token->m_token = T_DELIM;       return 0;
            case EOF: token->m_token = T_EOF;         return 0;
            case '\n':
              line_ ++;
            default:
              if (isspace(c))
                // Ignore white characters
                break;

              if (isalpha(c) || '_'==c) {
                // Initiate string building
                token->m_ext_text = c;
                state_ = STATE_BUILDING_STRING;
                break;
              }
              // Lexical error
              token->m_token = T_ERR_LEX;
              return 0;
          }
          break;

        case STATE_BUILDING_STRING:
          if (isalpha(c) || isdigit(c) || '_'==c) {
            // Building string
            token->m_ext_text += c;
            break;
          }
          // End of string
          ungetc(c, fd_);
          state_ = STATE_INIT;
          token->m_token = T_STRING;
          return 0;

        default:
          // Unknown state
          return 1;
      }
    }
  }


  /**
   * @param  scanner
   * @param  vc
   */
  ScannerStringHandler::ScannerStringHandler (IScanner *scanner, VariableContainer *vc ):
    scanner_(scanner),
    vc_(vc)
  {
    // Keywords definition
    keyWordMap_["NOT"]   = T_NOT;
    keyWordMap_["AND"]   = T_AND;
    keyWordMap_["OR"]    = T_OR;
    keyWordMap_["XOR"]   = T_XOR;
    keyWordMap_["FALSE"] = T_FALSE;
    keyWordMap_["TRUE"]  = T_TRUE;
  }

  /**
   * @return int
   * @param  token
   */
  int ScannerStringHandler::readNext (Token* token ) {
    // Use RawScanner to read token
    scanner_->readNext(token);

    // Transformations
    switch (token->m_token) {
      case T_STRING:
        // Handle only T_STRING
        {
          string &text = token->m_ext_text;
          if (keyWordMap_.end() != keyWordMap_.find(text)) {
            // Map T_STRING to keywords
            token->m_token = keyWordMap_[text];
            return 0;
          }

          // Use string as variable name
          token->m_token = T_VARIABLE;
          token->m_ext_number = vc_->addVariable(text);
        }
      default:
        // Default behavior
        return 0;
    }
  }


  /**
   * @param  scanner
   * @param  fc
   */
  ScannerFormulaHandler::ScannerFormulaHandler (IScanner *scanner, FormulaContainer *fc ):
    scanner_(scanner),
    fc_(fc),
    ignoreToDelim_(false)
  {
    current_ = new Formula;
  }


  ScannerFormulaHandler::~ScannerFormulaHandler() {
    delete current_;
  }

  /**
   * @return int
   * @param  token
   */
  int ScannerFormulaHandler::readNext (Token* token ) {
    // Use ScannerStringHandler to read token
    while (0== scanner_->readNext(token)) {
      switch (token->m_token) {
        case T_FALSE:
        case T_TRUE:
        case T_NOT:
        case T_AND:
        case T_OR:
        case T_XOR:
        case T_LPAR:
        case T_RPAR:
        case T_VARIABLE:
          if (ignoreToDelim_)
            // Ignore token
            break;

          // Parse token by IFormulaBuilder
          if (0!= current_->parse(*token)) {
            // Expression error
            ignoreToDelim_ = true;
            token->m_token = T_ERR_EXPR;
            return 0;
          }
          break;

        case T_ERR_LEX:
          // Lexical error
          if (ignoreToDelim_)
            // Ignore error
            break;
          ignoreToDelim_ = true;
          return 0;

        case T_EOF:
        case T_DELIM:
          if (!ignoreToDelim_ && current_->isValid()) {
            // Formula successfully readed
            fc_->addFormula(current_);
            current_ = new Formula;
            if (T_DELIM==token->m_token)
              break;
            else
              return 0;
          }
          if (!ignoreToDelim_) {
            // Parse error
            token->m_token = T_ERR_PARSE;
            return 0;
          }
          // Revitalization
          ignoreToDelim_ = false;
          break;

        default:
          return 0;
      }
    }
  }


} // namespace FastSatSolver
