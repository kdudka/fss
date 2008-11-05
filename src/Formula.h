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

#ifndef FOROMULA_H
#define FOROMULA_H

/**
 * @file Formula.h
 * @brief Propositional formula representation
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
 * @ingroup SatProblem
 */

#include "SatProblem.h"


namespace FastSatSolver {

  /**
   * @brief Interpreted formula's interface for parser which can read it.
   * @interface IFormulaBuilder
   * @ingroup SatProblem
   */
  class IFormulaBuilder
  {
    public:
      virtual ~IFormulaBuilder() { }

      /**
       * @brief Parse given token.
       * @param  token Token to parse
       * @return Returns zero, if no fatal error occurs.
       * @note Parase error is not fatal in this context.
       */
      virtual int parse (Token token ) = 0;

      /**
       * @brief @return Return true if formula is valid.
       * @note Invalid formulas should be never evaluated.
       */
      virtual bool isValid ( ) = 0;
  };

  /**
   * @brief Evaluable formula's interface.
   * @interface IFormulaEvaluator
   * @note The only one known implementation is now InterpretedFormula, but
   * there is no restriction to this class. There can be other implementations
   * of this interface.
   * @ingroup SatProblem
   */
  class IFormulaEvaluator
  {
    public:
      virtual ~IFormulaEvaluator() { }

      /**
       * @brief @return Return true if formula is satisfied for given data.
       * @param data Evaluation data to use for evaluation. Consider
       * @link FastSatSolver::ISatItem ISatItem @endlink interface for detail.
       */
      virtual bool eval (ISatItem *data ) = 0;
  };

  /**
   * @brief Interpreted formula implementation.
   * @note Implementation of this class is not the Fastest one, but there
   * hasn't been any request for a better implementation yet.
   * @ingroup SatProblem
   */
  class InterpretedFormula:
      public IFormulaBuilder,
      public IFormulaEvaluator
  {
    public:
      InterpretedFormula();
      ~InterpretedFormula();

      /**
       * @brief @copydoc FastSatSolver::IFormulaBuilder::parse(Token)
       */
      int parse (Token token );

      /**
       * @brief @copydoc FastSatSolver::IFormulaBuilder::isValid()
       */
      bool isValid ( );
      
      /**
       * @brief @copydoc FastSatSolver::IFormulaEvaluator::eval(ISatItem*)
       */
      bool eval (ISatItem *data );

    private:
      struct Private;
      Private *const d;
  };


} // namespace FastSatSolver


#endif // FOROMULA_H
