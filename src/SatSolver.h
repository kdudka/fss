#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <iostream>
#include <string>

class GAGenome;
class GABinaryString;
class GAStatistics;

namespace FastSatSolver {

  class ISatItem
  {
    public:
      virtual ~ISatItem() { }

      /**
       * @return int
       */
      virtual int getLength ( ) = 0;

      /**
       * @return bool
       * @param  index
       */
      virtual bool getBit (int index ) = 0;
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


  // TODO: Define interface
  class SatSolverParameters;

  class SatSolver;
  class SatSolverStatsProxy {
    public:
      virtual ~SatSolverStatsProxy();
      const GAStatistics& statistics() const;
      float getMaxFitness() const;
      float getAvgFitness() const;
      float getMinFitness() const;
      int getGeneration() const;
      int getTimeElapsed() const;
    protected:
      SatSolverStatsProxy(SatSolver *, const GAStatistics &);
    private:
      struct Private;
      Private *d;
  };
  std::ostream& operator<< (std::ostream&, const SatSolverStatsProxy &);

  class ISatSolverStats {
    public:
      virtual ~ISatSolverStats() { }
      virtual SatSolverStatsProxy* getStatsProxy() = 0;
  };


  class SatProblem;
  class SatSolver:
    public AbstractProcessWatched,
    public ISatSolverStats
  {
    friend class SatSolverEngine;

    public:
      virtual ~SatSolver();

      /**
       * @return SatProblemSolver*
       * @param  problem
       */
      static SatSolver* create (SatProblem *problem, SatSolverParameters *params);
      virtual SatSolverStatsProxy* getStatsProxy();
      SatProblem* getProblem();
      SatSolverParameters* getParameters();

    protected:
      /**
       * @param  problem
       */
      SatSolver (SatProblem *problem, SatSolverParameters *params);

      virtual void initialize();
      virtual void doStep();

    private:
      struct Private;
      Private *d;
  };


  class SatSolverEngine: public ISatSolverStats {
    public:
      SatSolverEngine(SatSolver *solver);
      virtual ~SatSolverEngine();
      virtual SatSolverStatsProxy* getStatsProxy();
      void doStep();
    private:
      struct Private;
      Private *d;
  };


  class SatItemGalibAdatper: public ISatItem {
    public:
      SatItemGalibAdatper(const GABinaryString &);
      virtual int getLength();
      virtual bool getBit(int);
    private:
      const GABinaryString &bs_;
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


  class FitnessWatch: public IObserver {
    public:
      FitnessWatch(ISatSolverStats *statsResource, std::ostream &streamTo);
      virtual ~FitnessWatch();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver


#endif // SATSOLVER_H
