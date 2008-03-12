#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <iostream>
#include <string>

class GAGenome;
class GABinaryString;
class GAParameterList;
class GAGeneticAlgorithm;
class GAStatistics;

namespace FastSatSolver {

  class ISatItem
  {
    public:
      virtual ~ISatItem() { }
      virtual ISatItem* clone() const = 0;

      /**
       * @return int
       */
      virtual int getLength ( ) const = 0;

      /**
       * @return bool
       * @param  index
       */
      virtual bool getBit (int index ) const = 0;
  };


  class IObserver {
    public:
      virtual ~IObserver() { }
      virtual void notify() = 0;
  };


  class ISubject {
    public:
      virtual ~ISubject() { }
      virtual void addObserver(IObserver *) = 0;
  };


  class AbstractSubject: public ISubject {
    public:
      virtual ~AbstractSubject();
      virtual void addObserver(IObserver *);
    protected:
      AbstractSubject();
      void notify();
    private:
      struct Private;
      Private *d;
  };


  class IProcess {
    public:
      virtual ~IProcess() { }
      virtual void start() = 0;
      virtual void stop() = 0;
      virtual void reset() = 0;
      virtual int getStepsCount() = 0;
  };


  class AbstractProcess:
    public AbstractSubject,
    public IProcess
  {
    public:
      virtual ~AbstractProcess();
      virtual void start();
      virtual void stop();
      virtual void reset();
      virtual int getStepsCount();
    protected:
      AbstractProcess();
      virtual void initialize() = 0;
      virtual void doStep() = 0;
    private:
      struct Private;
      Private *d;
  };


  class IStopWatch {
    public:
      virtual ~IStopWatch() { }
      virtual long getTimeElapsed() = 0;
  };


  class AbstractProcessWatched:
    public AbstractProcess,
    public IStopWatch
  {
    public:
      virtual ~AbstractProcessWatched();
      virtual void start();
      virtual void stop();
      virtual void reset();
      virtual long getTimeElapsed();
    protected:
      AbstractProcessWatched();
    private:
      struct Private;
      Private *d;
  };

  class SatProblem;

  class SatItemVector {
    public:
      SatItemVector();
      SatItemVector(const SatItemVector &);
      ~SatItemVector();
      int getLength();
      ISatItem* getItem(int index);
      void addItem(ISatItem *);
      void writeOut(SatProblem *, std::ostream &streamTo);
    private:
      struct Private;
      Private *d;
  };

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

  class GASatItemSet {
    public:
      GASatItemSet();
      ~GASatItemSet();
      int getLength();
      void addItem(const GABinaryString &);
      SatItemVector* createVector();
    private:
      struct Private;
      Private *d;
  };

  class SatItemGalibAdatper: public ISatItem {
    public:
      ~SatItemGalibAdatper();
      SatItemGalibAdatper(const GABinaryString &);
      virtual int getLength() const;
      virtual bool getBit(int) const;
      virtual SatItemGalibAdatper* clone() const;
    private:
      const GABinaryString &bs_;
  };


  class AbstractSatSolver: public AbstractProcessWatched
  {
    public:
      virtual ~AbstractSatSolver();
      virtual SatProblem* getProblem() = 0;
      virtual SatItemVector* getSolutionVector() = 0;

    protected:
      AbstractSatSolver();

    private:
      struct Private;
      Private *d;
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
      virtual SatItemVector* getSolutionVector();

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


  class BlindSatSolver: public AbstractSatSolver
  {
    public:
      BlindSatSolver(SatProblem *problem, int stepWidth);
      virtual ~BlindSatSolver();

      /**
       * @return SatProblemSolver*
       * @param  problem
       */
      virtual SatProblem* getProblem();
      virtual SatItemVector* getSolutionVector();

    protected:
      virtual void initialize();
      virtual void doStep();

    private:
      struct Private;
      Private *d;
  };


  class TimedStop: public IObserver {
    public:
      TimedStop(AbstractProcessWatched *process, long msec);
      virtual ~TimedStop();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };


  class SolutionsCountStop: public IObserver {
    public:
      SolutionsCountStop(AbstractSatSolver *solver, int minCountOfSolutions);
      virtual ~SolutionsCountStop();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };


  class FitnessWatch: public IObserver {
    public:
      FitnessWatch(GASatSolver *solver, std::ostream &streamTo);
      virtual ~FitnessWatch();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver


#endif // SATSOLVER_H
