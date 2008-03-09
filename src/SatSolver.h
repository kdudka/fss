#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <string>

namespace FastSatSolver {

  class GenericException {
    public:
      GenericException(std::string text) {
        this->text_ = text;
      }

      std::string getText() {
        return this->text_;
      }

    private:
      std::string text_;
  };

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


  class SatProblem
  {
    public:
      virtual ~SatProblem() { }

      /**
       * @return SatProblem*
       */
      static SatProblem* create ( );

      /**
       * @param  fileName
       */
      virtual void loadFromFile (std::string fileName ) = 0;

      /**
      */
      virtual void loadFromInput ( ) = 0;

      /**
       * @return int
       */
      virtual int getVarsCount ( ) = 0;

      /**
       * @return std::string
       * @param  index
       */
      virtual std::string getVarName (int index ) = 0;

      /**
       * @return
       */
      virtual int getFormulasCount() = 0;

      /**
       * @return int
       * @param  data
       */
      virtual int getSatsCount (ISatItem *data) = 0;

      /**
       * @return bool
       */
      virtual bool hasError ( ) = 0;
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


  // TODO: Define interfaces
  class SatSolverParameters;
  class SatSolverStatsProxy;

  class ISatSolverStats {
    public:
      virtual ~ISatSolverStats() { }
      virtual SatSolverStatsProxy* getStatsProxy() = 0;
  };


  class SatSolver:
    public AbstractProcessWatched,
    public ISatSolverStats
  {
    public:
      virtual ~SatSolver();

      /**
       * @return SatProblemSolver*
       * @param  problem
       */
      static SatSolver* create (SatProblem *problem, SatSolverParameters *params);

      virtual SatSolverStatsProxy* getStatsProxy();

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


  class TimedStop: public IObserver {
    public:
      TimedStop(AbstractProcessWatched *process, long msec);
      virtual ~TimedStop();
      virtual void notify();
    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver


#endif // SATSOLVER_H
