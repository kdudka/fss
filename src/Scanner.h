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

#ifndef SCANNER_H
#define SCANNER_H

/**
 * @file Scanner.h
 * @brief Extensible lexical scanner used for reading SAT Problem specification.
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
 * @ingroup SatProblem
 */

# include <iostream>

namespace FastSatSolver {
  class VariableContainer;
  class FormulaContainer;

  /**
   * @brief Scanner tokens enumeration
   * @attention Do not change enumeration order - it would break parser
   * functionality
   * @ingroup SatProblem
   * @note Parser error notifications can be dramatically extended in future.
   */
  enum EToken {
    T_VARIABLE,           ///< managed variable
    T_FALSE,              ///< @c 0 @c = @c FALSE
    T_TRUE,               ///< @c 1 @c = @c TRUE
    T_NOT,                ///< @c ~ @c = @c NOT
    T_AND,                ///< @c & @c = @c AND
    T_OR,                 ///< @c | @c = @c OR
    T_XOR,                ///< @c ^ @c = @c XOR
    T_LPAR,               ///< @c (
    T_RPAR,               ///< @c )
    T_DELIM,              ///< @c ;
    T_EOF,                ///< end of input
    T_STRING,             ///< string red from input (shoud be catched by
                          ///  ScannerStringHandler)
    T_STACK_BOTTOM,       ///< bottom of parser stack (behaves as terminal)
    T_PARSER_EXPR,        ///< expression (recursively)
    T_PARSER_EQ,          ///< control @c =
    T_PARSER_LT,          ///< control @c <
    T_PARSER_GT,          ///< control @c >
    T_PARSER_INV,         ///< invalid sequence
    T_ERR_LEX = -1,       ///< lexical error
    T_ERR_EXPR = -2,
    T_ERR_PARSE = -3
  };


  /**
   * @brief Syntax unit representation - also called @b token.
   * @ingroup SatProblem
   */
  struct Token {
    int           m_line;         ///< Line number in input file (starting with
                                  ///  number 1)
    EToken        m_token;        ///< token enumeration
    int           m_ext_number;   ///< Extra number carried with token
    std::string   m_ext_text;     ///< Extra text carried with token

    /**
     * @brief Empty default initializer - @b optimized
     * @attention Data are not initialized by default constructor.
     */
    Token() { }

    /**
     * @brief Fast initializer of abstract token
     * @param e token enumeration
     */
    Token(EToken e): m_line(0), m_token(e), m_ext_number(0) { }
  };

  // For debugging and/or error reporting purposes
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

  // For debugging and/or error reporting purposes
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

  /**
   * @interface IScanner
   * @brief Extensible lexical scanner's interface.
   * @ingroup SatProblem
   * @note Design pattern @b decorator.
   */
  class IScanner
  {
    public:
      virtual ~IScanner() { }

      /**
       * @brief Abstract scanner's parsing method.
       * @param  token token to parse
       * @return Returns error code. Zero means success.
       */
      virtual int readNext (Token* token ) = 0;
  };

  /**
   * @brief Low-level scanner parses lexical units from opened file.
   * @ingroup SatProblem
   * @note Design pattern @b decorator.
   */
  class RawScanner : public IScanner
  {
    public:
      /**
       * @param fd opened file's descriptor
       */
      RawScanner (FILE* fd );
      virtual ~RawScanner();
      
      // see IScanner dox
      virtual int readNext (Token* token );

    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief Part of parser handling keywords and variable names.
   * @ingroup SatProblem
   * @note Design pattern @b decorator.
   */
  class ScannerStringHandler : public IScanner
  {
    public:
      /**
       * @param scanner Pointer to superior IScanner object (token source).
       * @param vc Pointer to VaribleContainer to store variable names to.
       */
      ScannerStringHandler (IScanner *scanner, VariableContainer *vc );
      virtual ~ScannerStringHandler();

      // see IScanner dox
      virtual int readNext (Token* token );

    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief High-level part of parser handling almost all tokens and building
   * InterpretedFormula objects.
   * @ingroup SatProblem
   * @note Design pattern @b decorator.
   */
  class ScannerFormulaHandler : public IScanner
  {
    public:
      /**
       * @param scanner Pointer to superior IScanner object (token source).
       * @param fc Container to store valid formulas to.
       */
      ScannerFormulaHandler (IScanner *scanner, FormulaContainer *fc );
      virtual ~ScannerFormulaHandler();

      // see IScanner dox
      virtual int readNext (Token* token );

    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver


#endif // SCANNER_H
