#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <iostream>

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
      int getLength() const;
      ISatItem* getItem(int index) const;
      void addItem(ISatItem *);
      void clear();
      void writeOut(SatProblem *, std::ostream &streamTo) const;
    private:
      struct Private;
      Private *d;
  };

  class SatItemSet {
    public:
      SatItemSet();
      ~SatItemSet();
      int getLength();
      void addItem(ISatItem *item);
      SatItemVector* createVector();
      void clear();
    private:
      struct Private;
      Private *d;
  };


  class AbstractSatSolver: public AbstractProcessWatched
  {
    public:
      virtual ~AbstractSatSolver();
      virtual SatProblem* getProblem() = 0;
      virtual int getSolutionsCount() = 0;
      virtual SatItemVector* getSolutionVector() = 0;
      virtual float minFitness() = 0;
      virtual float avgFitness() = 0;
      virtual float maxFitness() = 0;

    protected:
      AbstractSatSolver();

    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver

#endif // SATSOLVER_H
