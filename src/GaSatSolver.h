#ifndef GASATSOLVER_H
#define GASATSOLVER_H

#include "SatSolver.h"

class GAGenome;
class GABinaryString;
class GAParameterList;
class GAGeneticAlgorithm;
class GAStatistics;

namespace FastSatSolver {

  class GASatItem: public ISatItem {
    public:
      GASatItem(const GABinaryString &);
      virtual ~GASatItem();
      virtual int getLength() const;
      virtual bool getBit(int) const;
      virtual GASatItem* clone() const;
    private:
      struct Private;
      Private *d;
  };

  class SatItemGalibAdatper: public ISatItem {
    public:
      SatItemGalibAdatper(const GABinaryString &);
      virtual ~SatItemGalibAdatper();
      virtual int getLength() const;
      virtual bool getBit(int) const;
      virtual SatItemGalibAdatper* clone() const;
    private:
      const GABinaryString &bs_;
  };


  class GASatSolver: public AbstractSatSolver
  {
    public:
      virtual ~GASatSolver();

      /**
       * @return SatProblemSolver*
       * @param  problem
       */
      static GASatSolver* create (SatProblem *problem, const GAParameterList &params);
      static void registerDefaultParameters(GAParameterList &);
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
      GASatSolver (SatProblem *problem, const GAParameterList &params);

      virtual void initialize();
      virtual void doStep();

    private:
      struct Private;
      Private *d;
  };


} // namespace FastSatSolver

#endif // GASATSOLVER_H

