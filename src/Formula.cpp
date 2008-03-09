#include <iostream>
#include <vector>
#include "Formula.h"

using std::string;

namespace FastSatSolver {

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
#ifndef NDEBUG
    std::cerr << token << std::endl;
#endif // NDEBUG
    // TODO
    return 0;
  }

  /**
   * @return bool
   */
  bool Formula::isValid ( ) {
    Token topTerm = d->parserStack.topTerm();
#ifndef NDEBUG
    std::cerr << "Formula::isValid(): topTerm = " << topTerm;
    std::cerr << "Formula::isValid(): errorDetected = " << d->errorDetected << std::endl;
#endif // NDEBUG
    return
      topTerm.m_token == T_STACK_BOTTOM
      && !(d->errorDetected);
  }

  /**
   * @param  data
   */
  bool Formula::eval (ISatItem *data ) {
    // TODO
  }


} // namespace FastSatSolver

