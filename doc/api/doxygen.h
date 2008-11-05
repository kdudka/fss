// This file contains no code inside
namespace FastSatSolver {
 /**
  * @file doxygen.h
  * @brief Doxygen documentation (common part)
  * @author Kamil Dudka <xdudka00@gmail.com>
  * @date 2008-11-05
  */
  
 /**
  * @mainpage
  * @brief SAT Problem solver using GAlib library.
  * @author Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
  * @date 2008-11-05
  * @remarks
  * @htmlonly
  * <ul>
  * <li>Module <a class="el" href="group__SatSolver.html">SatSolver</a> -
  * Class AbstractSatSolver with its derived classes BlindSatSolver and
  * GaSatSolver and their observers.</li>
  * <li>Module <a class="el" href="group__SatProblem.html">SatProblem</a> -
  * Internal SAT Problem representation with necessary tools for reading
  * and working with SAT Problems.</li>
  * <li><a class="el" href="group__fssIO.html">I/O module</a> -
  * Set of common classes for I/O used in this project.</li>
  * </ul>
  * @endhtmlonly
  */

 /**
  * @namespace FastSatSolver
  * All public and/or non-public classes and types belongs to this namespace.
  * There can be also some nested namespaces.
  * @brief Namespace encapsulating whole project.
  */
  
 /**
  * @defgroup SatSolver SAT Solver
  * @b SAT @b solvers:
  * - Class GaSatSolver - solver using GAlib library to solve SAT problem
  * - Class BlindSatSolver - solver using brute force method to solve SAT
  * problem
  * - Class AbstractSatSolver - common interface of both solvers
  * 
  * @b Observers:
  * - Class TimedStop - observer which stops process after specified time
  * - Class SolutionsCountStop - Observer which stop solver after specified
  * count of solutions is found.
  * - Class ProgressWatch - Observer which write out progress percentage when
  * it is changed.
  * - Class FitnessWatch - Observer which write out message when maxFitness
  * value is increased.
  * - Class ResultsWatch - Observer which write out message when solution is
  * found.
   * @brief Class AbstractSatSolver with its derived classes BlindSatSolver and
   * GaSatSolver and their observers.
  */
 
 /**
  * @defgroup SatProblem SAT Problem
  * @brief Internal SAT Problem representation with necessary tools for reading
  * and working with SAT Problems.
  * @remarks
  * - Interface IFormulaEvaluator - evaulable proposition formula's interface
  * - Class InterpretedFormula - interpreted proposition formula implementation
  * - Class SatProblem - facade to this module
  */
   
 /**
  * @defgroup fssIO I/O module
  * @brief Set of common classes for I/O used in this project.
  * @remarks
  * - Exception GenericException - FSS specific exception
  * - Namespace StreamDecorator - Colored console output, etc.
  * - Function @link FastSatSolver::printError printError @endlink - Common
  * routine for printing errors
  */
} // namespace FastSatSolver

