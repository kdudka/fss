#include <iostream>
#include <sstream>
#include <vector>
#include "Formula.h"

using std::string;

namespace FastSatSolver {

  static const int TABLE_SIZE = 8;
  static const EToken table[TABLE_SIZE][TABLE_SIZE] = {
    /*            0             1             2             3             4             5             6             7             */
    /*            XOR           OR            AND           NOT           (             )             i             $             */
    /* 0 XOR */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_EOF      ,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
    /* 1 OR  */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_EOF      ,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
    /* 2 AND */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_EOF      ,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
    /* 3 NOT */ { T_EOF      ,  T_EOF      ,  T_EOF      ,  T_EOF      ,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
    /* 4 (   */ { T_PARSER_LT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_EQ,  T_PARSER_LT,  T_PARSER_INV, },
    /* 5 )   */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_INV, T_PARSER_GT,  T_PARSER_INV, T_PARSER_GT,  },
    /* 6 i   */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_INV, T_PARSER_GT,  T_PARSER_INV, T_PARSER_GT,  },
    /* 7 $   */ { T_PARSER_LT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_INV, T_PARSER_LT,  T_EOF,        },
  };

  inline int tableGetIndex(EToken e) {
    switch (e) {
      case T_XOR:  /* 0 XOR */ return 0;
      case T_OR:   /* 1 OR  */ return 1;
      case T_AND:  /* 2 AND */ return 2;
      case T_NOT:  /* 3 NOT */ return 3;
      case T_LPAR: /* 4 (   */ return 4;
      case T_RPAR: /* 5 )   */ return 5;

      case T_VARIABLE:
      case T_FALSE:
      case T_TRUE:
                   /* 6 i   */
                               return 6;

      case T_DELIM:
      case T_EOF:
      case T_STACK_BOTTOM:
                   /* 7 $   */
                               return 7;

      default:
                               // Out of range
                               return -1;
    }
  }

  inline EToken tableLookup(EToken topTerm, EToken input) {
    const int i = tableGetIndex(topTerm);
    const int j = tableGetIndex(input);
    if (i<0 || j<0)
      // Out of range
      return T_PARSER_INV;
    
    EToken result = table[i][j];
#if 0//ndef NDEBUG
    std::cerr << "tableLookup(" << i << ", " << j << ") = " << result << std::endl;
#endif // NDEBUG
    return result;
  }

  inline bool isTokenOperand (Token token) {
    switch(token.m_token) {
      case T_FALSE:
      case T_TRUE:
      case T_VARIABLE:
        return true;

      default:
        return false;
    }
  }

  inline bool isTokenTerminal (Token token) {
    // FIXME: not sure while copy-pasting
    return (token.m_token < T_PARSER_EXPR);
  }

  class ParserStack {
    public:
      void push(Token token) {
        container_.push_back(token);
      }

      // TODO: Copy warning from original code
      void insertLt() {
        Token last = container_.back();
        if (isTokenTerminal(last))
          container_.push_back(T_PARSER_LT);
        else {
          Token last = container_.back();
          container_.pop_back();
          container_.push_back(T_PARSER_LT);
          container_.push_back(last);
        }
      }

      Token top() {
        return container_.back();
      }

      Token pop() {
        Token last = container_.back();
        container_.pop_back();
        return last;
      }

      Token topTerm() {
        size_t size = container_.size();
        Token last = container_.back();
        if (isTokenTerminal(last))
          return last;
        else
          // return one before last
          return container_[size-2];
      }

    private:
      typedef std::vector<Token> TContainer;
      TContainer container_;
  };

  struct Formula::Private {
    ParserStack parserStack;
    bool errorDetected;
  };

  Formula::Formula():
    d(new Formula::Private)
  {
    d->parserStack.push(T_STACK_BOTTOM);
    d->errorDetected = false;
  }

  Formula::~Formula() {
    delete d;
  }

  /**
   * @param  token
   */
  int Formula::parse (Token token ) {
    ParserStack &stack = d->parserStack;
    EToken mode;
    do {
      Token topTerm = stack.topTerm();
      mode = tableLookup(topTerm.m_token, token.m_token);
      switch (mode) {
        case T_PARSER_LT:
          // Insert '<' after last terminal
          stack.insertLt();
        case T_PARSER_EQ:
          // Insert token to stack
          stack.push(token);
          break;

        case T_PARSER_GT:
          // Reduction

          if (isTokenOperand(topTerm)) {
            // Operand reduction
            Token opToken = stack.pop();
            Token lt = stack.pop();
            if (T_PARSER_LT != lt.m_token) {
              // Invalid expression
              d->errorDetected = true;
              return T_ERR_EXPR;
            }
            // Replace terminal with non-terminal
            stack.push(T_PARSER_EXPR);

            // TODO: Handle operand

          } else if (T_RPAR == topTerm.m_token) {
            // Parthesis reduction
            EToken rList[4];
            for (int i=0; i<4; i++)
              rList[i] = stack.pop().m_token;
            if (
                rList[1] != T_PARSER_EXPR ||
                rList[2] != T_LPAR ||
                rList[3] != T_PARSER_LT
               )
            {
              // Error in parenthesis
              d->errorDetected = true;
              return T_ERR_EXPR;
            }
            stack.push(T_PARSER_EXPR);
#ifndef NDEBUG
            std::cerr << "Drop brackets..." << std::endl;
#endif // NDEBUG
          } else
            throw GenericException("Formula::parse(): Reduction not implemented yet");
          break;

        case T_EOF:
          d->errorDetected = !(this->isValid());
          if (d->errorDetected)
            return T_ERR_EXPR;
          else
            return 0;

        case T_PARSER_INV:
          d->errorDetected = true;
          return T_ERR_EXPR;

        default:
          {
            d->errorDetected = true;
            std::ostringstream stream;
            stream << "Unexpected symbol in precedence table: " << mode;
            throw GenericException(stream.str());
          }
      } // switch
    } while (T_PARSER_GT == mode);
    // TODO
    return 0;
  }

  /**
   * @return bool
   */
  bool Formula::isValid ( ) {
    ParserStack &stack = d->parserStack;
    Token topTerm = stack.topTerm();
    Token top = stack.top();
#if 0//ndef NDEBUG
    std::cerr << "Formula::isValid(): topTerm = " << topTerm;
    std::cerr << "Formula::isValid(): top = " << top;
    std::cerr << "Formula::isValid(): errorDetected = " << d->errorDetected << std::endl;
    std::cerr << std::endl;
#endif // NDEBUG
    return
      topTerm.m_token == T_STACK_BOTTOM &&
      top.m_token == T_PARSER_EXPR &&
      !(d->errorDetected);
  }

  /**
   * @param  data
   */
  bool Formula::eval (ISatItem *data ) {
    // TODO
  }


} // namespace FastSatSolver

