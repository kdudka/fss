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

#ifndef SATPROBLEMIMPL_H
#define SATPROBLEMIMPL_H

/**
 * @file SatProblem.h
 * @brief SAT Problem representation
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
 * @ingroup SatProblem
 */

#include <string>
#include "Scanner.h"

namespace FastSatSolver {
  class ISatItem;
  class IFormulaEvaluator;

  /**
   * It can transform variable name to its integral index and vice versa.
   * @brief Container for variables names.
   * @ingroup SatProblem
   */
  class VariableContainer
  {
    public:
      VariableContainer();
      ~VariableContainer();
      /**
       * @brief @return Returns count of variables managed by container.
       */
      int getLength ( );

      /**
       * @brief Read name of variable on desired index.
       * @param index Index of variable should be in range <0, getLength()-1>
       * @return Returns variable's name as STL string.
       */
      std::string getVarName (int index );

      /**
       * @brief Add variable to container, if it wasn't there before.
       * @param name Name of variable to add.
       * @return Return variable's integral index.
       */
      int addVariable (std::string name );

    private:
      struct Private;
      Private *d;
  };

  /**
   * @note The only one known implementation is now InterpretedFormula, but
   * there is no restriction to this class. It can be any class implementing
   * IFormulaEvaluator interface.
   * @brief Container for evaluable formulas.
   * @ingroup SatProblem
   */
  class FormulaContainer
  {
    public:
      FormulaContainer();
      ~FormulaContainer();

      /**
       * @brief @return Returns count of formulas managed by container.
       */
      int getLength ( );

      /**
       * @brief Evaluate all formulas in container using given data and return
       * satisfaction ratio.
       * @param data Evaluation data to use for evaluation. Consider
       * FastSatSolver::ISatItem interface for detail.
       * @return Returns count of satisfaced formulas.
       */
      int evalAll (ISatItem *data);

      /**
       * @brief Add formula to container.
       * @param formula Formula object to add.
       */
      void addFormula (IFormulaEvaluator *formula );

    private:
      struct Private;
      Private *d;
  };


  /**
   * @brief SAT Problem module's facade.
   * @ingroup SatProblem
   */
  class SatProblem {
    public:
      SatProblem();
      ~SatProblem();

      /**
       * @brief Load SAT Problem specification from file
       * @param fileName File name to read and parse.
       */
      void loadFromFile (std::string fileName );

      /**
       * @brief Load SAT Problem specification from standard input.
       */
      void loadFromInput ( );

      /**
       * @brief @return Returns total count of variables managed by SatProblem.
       */
      int getVarsCount ( );

      /**
       * @brief @return Returns name of variable with desired index.
       * @param index Index of variable should be in range
       * <0, getVarsCount()-1>.
       */
      std::string getVarName (int index );

      /**
       * @brief @return Returns total count of formulas managed by SatProblem.
       */
      int getFormulasCount();

      /**
       * @brief @copydoc FastSatSolver::FormulaContainer::evalAll(ISatItem*)
       */
      int getSatsCount (ISatItem *data);

      /**
       * @brief @return Returns true if SAT Problem is @b not valid.
       */
      bool hasError ( );

    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver


#endif // SATPROBLEMIMPL_H
