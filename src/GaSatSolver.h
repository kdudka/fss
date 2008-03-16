#ifndef GASATSOLVER_H
#define GASATSOLVER_H

/**
 * @file GaSatSolver.h
 * @brief GaSatSolver class using GAlib library to solve SAT problem.
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-03-16
 * @ingroup SatSolver
 */

#include "SatSolver.h"

class GAGenome;
class GABinaryString;
class GAParameterList;
class GAGeneticAlgorithm;
class GAStatistics;

namespace FastSatSolver {

  /**
   * @brief ISatItem implementation used by GaSatSolver
   * @ingroup SatSolver
   */
  class GaSatItem: public ISatItem {
    public:
      /**
       * @param bs Reference to GABinaryString to read data from.
       */
      GaSatItem(const GABinaryString &bs);
      virtual ~GaSatItem();
      virtual int getLength() const;
      virtual bool getBit(int) const;
      virtual GaSatItem* clone() const;
    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief Leightweight, optimized ISatItem implementation used by GaSatSolver
   * time-critical parts of code.
   * @attention This implementation hold only reference of represented object.
   * @attention There is no deep copy, no dynamic memory management!!!
   * @ingroup SatSolver
   */
  class SatItemGalibAdatper: public ISatItem {
    public:
      /**
       * @param bs Reference to GABinaryString to represent.
       */
      SatItemGalibAdatper(const GABinaryString &bs);
      virtual ~SatItemGalibAdatper();
      virtual int getLength() const;
      virtual bool getBit(int) const;
      virtual SatItemGalibAdatper* clone() const;
    private:
      const GABinaryString &bs_;
  };

  /**
   * @brief Solver using GAlib library to solve SAT problem.
   * @ingroup SatSolver
   * @note Design pattern @b simple @b factory
   */
  class GaSatSolver: public AbstractSatSolver
  {
    public:
      virtual ~GaSatSolver();

      /**
       * @brief Simple factory method.
       * @param problem SatProblem instance containing SAT problem to solve.
       * @param params GAParameterList containing GA-specific parameters.
       * @return Returns initialized object of GaSatSolver.
       */
      static GaSatSolver* create(
                                 SatProblem             *problem,
                                 const GAParameterList  &params);
      
      /**
       * @brief Extends GAParameterList with GA-specific parameters.
       * @param params Reference to GAParameterList object managed by caller.
       */
      static void registerDefaultParameters(GAParameterList &params);
      
      /**
       * @brief Returns useful statistic data managed by GAStatistics class.
       */
      const GAStatistics& getStatistics() const;
      virtual SatProblem* getProblem();
      virtual int getSolutionsCount();
      virtual SatItemVector* getSolutionVector();
      virtual float minFitness();
      virtual float avgFitness();
      virtual float maxFitness();

    protected:
      /**
       * @param  problem
       */
      GaSatSolver (SatProblem *problem, const GAParameterList &params);

      virtual void initialize();
      virtual void doStep();

    private:
      struct Private;
      Private *d;
  };


} // namespace FastSatSolver

#endif // GASATSOLVER_H

