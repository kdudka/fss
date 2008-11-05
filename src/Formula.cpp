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

#include <assert.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <list>
#include "fssIO.h"
#include "SatSolver.h"
#include "Formula.h"

using std::string;

namespace FastSatSolver {

  typedef std::stack<bool> TRuntimeStack;

  /**
   * @brief Precedence table size (2 dimensional table)
   */
  static const int TABLE_SIZE = 8;

  /**
   * @brief Precedence table
   */
  static const EToken table[TABLE_SIZE][TABLE_SIZE] = {
    /*            0             1             2             3             4             5             6             7             */
    /*            XOR           OR            AND           NOT           (             )             i             $             */
    /* 0 XOR */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
    /* 1 OR  */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
    /* 2 AND */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
    /* 3 NOT */ { T_PARSER_GT,  T_PARSER_GT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_LT,  T_PARSER_GT,  T_PARSER_LT,  T_PARSER_GT,  },
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

      bool popAndCompare(EToken e) {
        Token t = this->pop();
        return t.m_token == e;
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

  class Cmd {
    public:
      static Cmd* fromToken(Token token);
      virtual ~Cmd() { }
      virtual void execute(TRuntimeStack *, ISatItem *) = 0;
    protected:
      Cmd() { }
  };
  class CmdConstant: public Cmd {
    public:
      CmdConstant(bool value): b(value) { }
      virtual void execute(TRuntimeStack *stack, ISatItem *) {
        stack->push(b);
      }
    private:
      bool b;
  };
  class CmdVariable: public Cmd {
    public:
      CmdVariable(int varId): id(varId) { }
      virtual void execute(TRuntimeStack *stack, ISatItem *data) {
        assert(id >= 0);
        assert(id < data->getLength());
        bool b = data->getBit(id);
        stack->push(b);
      }
    private:
      int id;
  };
  class CmdUnaryNot: public Cmd {
    public:
      virtual void execute(TRuntimeStack *stack, ISatItem *) {
        assert(!stack->empty());
        bool b = stack->top();
        stack->pop();
        stack->push(!b);
      }
  };
  class CmdBinary: public Cmd {
    public:
      CmdBinary(EToken e): et(e) { }
      virtual void execute(TRuntimeStack *stack, ISatItem *) {
        assert(!stack->empty());
        bool a = stack->top();
        stack->pop();

        assert(!stack->empty());
        bool b = stack->top();
        stack->pop();

        bool c;
        switch (et) {
          case T_AND: c = a & b; break;
          case T_OR:  c = a | b; break;
          case T_XOR: c = a ^ b; break;
          default:
            {
              std::ostringstream stream;
              stream << "CmdBinary::execute(): unknown token: " << et;
              throw GenericException(stream.str());
            }
        }
        stack->push(c);
      }
    private:
      EToken et;
  };
  Cmd* Cmd::fromToken(Token token) {
    switch (token.m_token) {
      case T_FALSE:       return new CmdConstant(false);
      case T_TRUE:        return new CmdConstant(true);
      case T_VARIABLE:    return new CmdVariable(token.m_ext_number);
      case T_NOT:         return new CmdUnaryNot;
      case T_AND:
      case T_OR:
      case T_XOR:
                          return new CmdBinary(token.m_token);
      default:
                          {
                            std::ostringstream stream;
                            stream << "Cmd::fromToken(): unknown token: " << token;
                            throw GenericException(stream.str());
                          }
    }
  }
  class CmdList: public Cmd {
    public:
      virtual ~CmdList() {
        TContainer::iterator iter;
        for(iter=container_.begin(); iter!=container_.end(); iter++)
          delete *iter;
      }
      virtual void execute(TRuntimeStack *stack, ISatItem *data) {
        TContainer::iterator iter;
        for(iter=container_.begin(); iter!=container_.end(); iter++) {
          Cmd *cmd = *iter;
          cmd->execute(stack, data);
        }
      }
      void operator<< (Cmd *cmd) {
        container_.push_back(cmd);
      }
      void operator<< (const Token &token) {
        operator<< (Cmd::fromToken(token));
      }
    private:
      typedef std::list<Cmd *> TContainer;
      TContainer container_;
  };

  struct InterpretedFormula::Private {
    ParserStack     parserStack;
    bool            errorDetected;
    CmdList         cmdList;
  };

  InterpretedFormula::InterpretedFormula():
    d(new Private)
  {
    d->parserStack.push(T_STACK_BOTTOM);
    d->errorDetected = false;
  }

  InterpretedFormula::~InterpretedFormula() {
    delete d;
  }

  /**
   * @param  token
   */
  int InterpretedFormula::parse (Token token ) {
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

          switch (topTerm.m_token) {
            case T_FALSE:
            case T_TRUE:
            case T_VARIABLE:
              {
                // Operand reduction
                Token opToken = stack.pop();
                if (!stack.popAndCompare(T_PARSER_LT)) {
                  // Invalid expression
                  d->errorDetected = true;
                  return T_ERR_EXPR;
                }

                // Handle operand
                d->cmdList << opToken;
                //std::cerr << "<<< Execute command: " << opToken << std::endl;
              }
              break;

            case T_RPAR:
                // Parthesis reduction
                stack.pop();
                if (
                    !stack.popAndCompare(T_PARSER_EXPR) ||
                    !stack.popAndCompare(T_LPAR) ||
                    !stack.popAndCompare(T_PARSER_LT)
                   )
                {
                  // Error in parenthesis
                  d->errorDetected = true;
                  return T_ERR_EXPR;
                }
#if 0//ndef NDEBUG
                std::cerr << "Droping brackets..." << std::endl;
#endif // NDEBUG
              break;

            case T_NOT:
              {
                // Unary operator NOT
                if (!stack.popAndCompare(T_PARSER_EXPR)) {
                  // operand expected
                  d->errorDetected = true;
                  return T_ERR_EXPR;
                }

                Token t = stack.pop();
                // Handle token
                d->cmdList << t;
                //std::cerr << "<<< Execute command: " << t << std::endl;

                if (!stack.popAndCompare(T_PARSER_LT)) {
                  // invalid expression
                  d->errorDetected = true;
                  return T_ERR_EXPR;
                }
              }
              break;

            default:
              {
                // Binary operator
                if (!stack.popAndCompare(T_PARSER_EXPR)) {
                  // operand expected
                  d->errorDetected = true;
                  return T_ERR_EXPR;
                }

                Token t = stack.pop();
                // Handle token
                d->cmdList << t;
                //std::cerr << "<<< Execute command: " << t << std::endl;

                if (!stack.popAndCompare(T_PARSER_EXPR)) {
                  // operand expected
                  d->errorDetected = true;
                  return T_ERR_EXPR;
                }

                if (!stack.popAndCompare(T_PARSER_LT)) {
                  // invalid expression
                  d->errorDetected = true;
                  return T_ERR_EXPR;
                }
              }
          }
          stack.push(T_PARSER_EXPR);
          break;

        case T_EOF:
          // End of expression
          d->errorDetected = !(this->isValid());
          if (d->errorDetected)
            return T_ERR_EXPR;
          else
            return 0;

        case T_PARSER_INV:
          // Invalid token sequence
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
  bool InterpretedFormula::isValid ( ) {
    ParserStack &stack = d->parserStack;
    Token topTerm = stack.topTerm();
    Token top = stack.top();
#if 0//ndef NDEBUG
    std::cerr << "InterpretedFormula::isValid(): topTerm = " << topTerm;
    std::cerr << "InterpretedFormula::isValid(): top = " << top;
    std::cerr << "InterpretedFormula::isValid(): errorDetected = " << d->errorDetected << std::endl;
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
  bool InterpretedFormula::eval (ISatItem *data) {
    if (!this->isValid())
      throw GenericException("InterpretedFormula::eval(): called for invalid formula");

    TRuntimeStack stack;
    d->cmdList.execute(&stack, data);

    // Check stack size (should be 1)
    const int stackSize = stack.size();
    if (1!=stackSize) {
      std::ostringstream stream;
      stream << "InterpretedFormula::eval(): incorrect stack size after cmdList.execute(): " << stackSize;
      throw GenericException(stream.str());
    }

    return stack.top();
  }


} // namespace FastSatSolver

