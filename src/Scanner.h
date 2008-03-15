#ifndef SCANNER_H
#define SCANNER_H

# include <iostream>

namespace FastSatSolver {

  // ATTENTION: Do not change enumeration order - it would break parser functionality
  enum EToken {
    T_VARIABLE,           // managed variable
    T_FALSE,              // 0 FALSE
    T_TRUE,               // 1 TRUE
    T_NOT,                // ~ NOT
    T_AND,                // & AND
    T_OR,                 // | OR
    T_XOR,                // ^ XOR
    T_LPAR,               // (
    T_RPAR,               // )
    T_DELIM,              // ;
    T_EOF,                // end of input
    T_STRING,             // string red from input (shoud be catched by ScannerStringHandler)
    T_STACK_BOTTOM,       // bottom of parser stack (behaves as terminal)
    T_PARSER_EXPR,        // expression (recursively)
    T_PARSER_EQ,          // control =
    T_PARSER_LT,          // control <
    T_PARSER_GT,          // control >
    T_PARSER_INV,         // invalid sequence
    T_ERR_LEX = -1,       // lexical error
    T_ERR_EXPR = -2,
    T_ERR_PARSE = -3
  };


  struct Token {
    int           m_line;
    EToken        m_token;
    int           m_ext_number;
    std::string   m_ext_text;

    // Empty default initializer
    Token() { }

    // Fast initializer of abstract token
    Token(EToken e): m_line(0), m_token(e), m_ext_number(0) { }
  };


  inline std::ostream& operator<< (std::ostream &stream, EToken e) {
    switch (e) {
      case T_EOF:         stream << "T_EOF";          break;
      case T_FALSE:       stream << "T_FALSE";        break;
      case T_TRUE:        stream << "T_TRUE";         break;
      case T_NOT:         stream << "T_NOT";          break;
      case T_AND:         stream << "T_AND";          break;
      case T_OR:          stream << "T_OR";           break;
      case T_XOR:         stream << "T_XOR";          break;
      case T_LPAR:        stream << "T_LPAR";         break;
      case T_RPAR:        stream << "T_RPAR";         break;
      case T_DELIM:       stream << "T_DELIM";        break;
      case T_STRING:      stream << "T_STRING";       break;
      case T_VARIABLE:    stream << "T_VARIABLE";     break;
      case T_STACK_BOTTOM:stream << "T_STACK_BOTTOM"; break;
      case T_PARSER_EXPR: stream << "T_PARSER_EXPR";  break;
      case T_PARSER_EQ:   stream << "T_PARSER_EQ";    break;
      case T_PARSER_LT:   stream << "T_PARSER_LT";    break;
      case T_PARSER_GT:   stream << "T_PARSER_GT";    break;
      case T_PARSER_INV:  stream << "T_PARSER_INV";   break;
      case T_ERR_LEX:     stream << "T_ERR_LEX";      break;
      case T_ERR_EXPR:    stream << "T_ERR_EXPR";     break;
      case T_ERR_PARSE:   stream << "T_ERR_PARSE";    break;
      default:            stream << e;                break;
    }
    return stream;
  }

  inline std::ostream& operator<< (std::ostream &stream, Token token) {
    stream << "Token: " << token.m_token << std::endl;
    if (token.m_line)
      stream << "       At line: " << token.m_line  << std::endl;
    if (T_VARIABLE == token.m_token) {
      stream << " Variable name: " << token.m_ext_text << std::endl;
      stream << "   Vairable id: " << token.m_ext_number << std::endl;
    }
    return stream;
  }


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

  class RawScanner : public IScanner
  {
    public:
      /**
       * @param  fd
       */
      RawScanner (FILE* fd );
      virtual ~RawScanner();

      /**
       * @return int
       * @param  token
       */
      virtual int readNext (Token* token );

    private:
      struct Private;
      Private *d;
  };

  class VariableContainer;
  class ScannerStringHandler : public IScanner
  {
    public:
      /**
       * @param  scanner
       * @param  vc
       */
      ScannerStringHandler (IScanner *scanner, VariableContainer *vc );
      virtual ~ScannerStringHandler();

      /**
       * @return int
       * @param  token
       */
      virtual int readNext (Token* token );

    private:
      struct Private;
      Private *d;
  };

  class FormulaContainer;
  class ScannerFormulaHandler : public IScanner
  {
    public:
      /**
       * @param  scanner
       * @param  fc
       */
      ScannerFormulaHandler (IScanner *scanner, FormulaContainer *fc );
      virtual ~ScannerFormulaHandler();

      /**
       * @return int
       * @param  token
       */
      virtual int readNext (Token* token );

    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver


#endif // SCANNER_H
