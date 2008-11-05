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

#ifndef GASATSOLVER_H
#define GASATSOLVER_H

/**
 * @file GaSatSolver.h
 * @brief GaSatSolver class using GAlib library to solve SAT problem.
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
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
       * @brief Non-public constructor. Use static method create() instead.
       * @param problem SatProblem instance containing SAT problem to solve.
       * @param params GAParameterList containing GA-specific parameters.
       * @return Returns initialized object of GaSatSolver.
       * @note Design pattern @b Simple @b factory
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

