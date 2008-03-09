#include <iostream>
#include "SatSolverImpl.h"

namespace FastSatSolver {

  /**
  */
  void SatSolver::start ( ) {

  }


  /**
  */
  void SatSolver::stop ( ) {

  }


  /**
  */
  void SatSolver::reset ( ) {

  }

  /**
   * @return SatSolver*
   * @param  problem
   */
  SatSolver* SatSolver::create (SatProblem *problem) {
    SatSolver *obj = new SatSolver(problem);
    obj->reset();
    return obj;
  }


  /**
   * @param  problem
   */
  SatSolver::SatSolver (SatProblem* problem ) {
#ifndef NDEBUG
    std::cerr << "  Formulas count: " << problem->getFormulasCount() << std::endl;
    std::cerr << "  Variables count: " << problem->getVarsCount() << std::endl;
    std::cerr << "  Variables: ";
    for(int i=0; i< problem->getVarsCount(); i++)
      std::cerr << problem->getVarName(i) << ", ";
    std::cerr << std::endl;
#endif // NDEBUG
  }


  /**
   * @param  listener
   */
  void SatSolver::addEventListener (IEventListener *listener ) {

  }

  /**
   * @return int
   */
  int SatItem::getLength ( ) {

  }

  /**
   * @return bool
   * @param  index
   */
  bool SatItem::getBit (int index ) {

  }

} // namespace FastSatSolver

