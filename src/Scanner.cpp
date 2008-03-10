#include <iostream>
#include <string>
#include <map>
#include "SatProblemImpl.h"
#include "Scanner.h"
#include "Formula.h"

using std::string;

namespace FastSatSolver {

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // RawScanner implementation
  struct RawScanner::Private {
      enum EState {
        STATE_INIT,
        STATE_BUILDING_STRING
      };
      FILE      *fd;
      EState    state;
      int       line;
  };
  RawScanner::RawScanner (FILE* fd ):
    d(new Private)
  {
    d->fd = fd;
    d->state = Private::STATE_INIT;
    d->line = 1;
  }
  RawScanner::~RawScanner() {
    delete d;
  }
  int RawScanner::readNext (Token *token) {
    while (1) {
      token->m_line = d->line;
      int c = fgetc(d->fd);
      switch(d->state) {
        case Private::STATE_INIT:
          token->m_ext_text.clear();
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
              d->line ++;
            default:
              if (isspace(c))
                // Ignore white characters
                break;

              if (isalpha(c) || '_'==c) {
                // Initiate string building
                token->m_ext_text = c;
                d->state = Private::STATE_BUILDING_STRING;
                break;
              }
              // Lexical error
              token->m_token = T_ERR_LEX;
              return 0;
          }
          break;

        case Private::STATE_BUILDING_STRING:
          if (isalpha(c) || isdigit(c) || '_'==c) {
            // Building string
            token->m_ext_text += c;
            break;
          }
          // End of string
          ungetc(c, d->fd);
          d->state = Private::STATE_INIT;
          token->m_token = T_STRING;
          return 0;

        default:
          // Unknown state
          return 1;
      }
    }
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // ScannerStringHandler implementation
  struct ScannerStringHandler::Private {
    typedef std::map<std::string,EToken> TKeyWordMap;
    IScanner            *scanner;
    VariableContainer   *vc;
    TKeyWordMap         keyWordMap;
  };
  ScannerStringHandler::ScannerStringHandler (IScanner *scanner, VariableContainer *vc ):
    d(new Private)
  {
    d->scanner = scanner;
    d->vc = vc;

    // Keywords definition
    d->keyWordMap["NOT"]   = T_NOT;
    d->keyWordMap["AND"]   = T_AND;
    d->keyWordMap["OR"]    = T_OR;
    d->keyWordMap["XOR"]   = T_XOR;
    d->keyWordMap["FALSE"] = T_FALSE;
    d->keyWordMap["TRUE"]  = T_TRUE;
  }
  ScannerStringHandler::~ScannerStringHandler() {
    delete d;
  }
  int ScannerStringHandler::readNext (Token* token ) {
    // Use RawScanner to read token
    d->scanner->readNext(token);

    // Transformations
    switch (token->m_token) {
      case T_STRING:
        // Handle only T_STRING
        {
          string &text = token->m_ext_text;
          if (d->keyWordMap.end() != d->keyWordMap.find(text)) {
            // Map T_STRING to keywords
            token->m_token = d->keyWordMap[text];
            return 0;
          }

          // Use string as variable name
          token->m_token = T_VARIABLE;
          token->m_ext_number = d->vc->addVariable(text);
        }
      default:
        // Default behavior
        return 0;
    }
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // ScannerFormulaHandler implementation
  struct ScannerFormulaHandler::Private {
    IScanner            *scanner;
    FormulaContainer    *fc;
    Formula             *current;
    bool                ignoreToDelim;
  };
  ScannerFormulaHandler::ScannerFormulaHandler (IScanner *scanner, FormulaContainer *fc ):
    d(new Private)
  {
    d->scanner = scanner;
    d->fc = fc;
    d->ignoreToDelim = false;
    d->current = new Formula;
  }
  ScannerFormulaHandler::~ScannerFormulaHandler() {
    delete d->current;
    delete d;
  }
  int ScannerFormulaHandler::readNext (Token* token ) {
    // Use ScannerStringHandler to read token
    while (0== d->scanner->readNext(token)) {
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
          if (d->ignoreToDelim)
            // Ignore token
            break;

          // Parse token by IFormulaBuilder
          if (0!= d->current->parse(*token)) {
            // Expression error
            d->ignoreToDelim = true;
            token->m_token = T_ERR_EXPR;
            return 0;
          }
          break;

        case T_ERR_LEX:
          // Lexical error
          if (d->ignoreToDelim)
            // Ignore error
            break;
          d->ignoreToDelim = true;
          return 0;

        case T_EOF:
        case T_DELIM:
          d->current -> parse(T_EOF);
          if (!d->ignoreToDelim && d->current->isValid()) {
            // Formula successfully readed
            d->fc->addFormula(d->current);
            d->current = new Formula;
#ifndef NDEBUG
            std::cerr << ">>> Formula red successfully" << std::endl;
#endif // NDEBUG
            if (T_DELIM==token->m_token)
              break;
            else
              return 0;
          }
          if (T_EOF==token->m_token)
            return 0;

          // Error recover
#ifndef NDEBUG
          std::cerr << "--- Error recover" << std::endl;
#endif // NDEBUG
          delete d->current;
          d->current = new Formula;
          if (!d->ignoreToDelim) {
            // Parse error
            token->m_token = T_ERR_PARSE;
            return 0;
          }
          d->ignoreToDelim = false;
          break;

        default:
          return 0;
      }
    }
    // This sohould never happen
    return -1;
  }

} // namespace FastSatSolver
