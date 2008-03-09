#include <assert.h>
#include <time.h>
#include <iostream>
#include <list>
#include "SatSolverImpl.h"

namespace FastSatSolver {

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // AbstractSubject implementation
  struct AbstractSubject::Private {
    typedef std::list<IObserver *> TContainer;
    TContainer container;
  };
  AbstractSubject::AbstractSubject():
    d(new Private)
  {
  }
  AbstractSubject::~AbstractSubject() {
    // ATTENTION: Observers are not deleted on object destruction
    delete d;
  }
  void AbstractSubject::addObserver(IObserver *observer) {
    d->container.push_back(observer);
  }
  void AbstractSubject::notify() {
    Private::TContainer::iterator iter;
    for(iter=d->container.begin(); iter!=d->container.end(); iter++) {
      IObserver *observer = *iter;
      observer->notify();
    }
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // AbstractProcess implementation
  struct AbstractProcess::Private {
    bool running;
    int steps;
  };
  AbstractProcess::AbstractProcess():
    d(new Private)
  {
    d->running = false;
    d->steps = 0;
  }
  AbstractProcess::~AbstractProcess() {
    delete d;
  }
  void AbstractProcess::start() {
    d->running = true;
    while (d->running) {
      this->doStep();
      this->notify();
      d->steps ++;
    }
  }
  void AbstractProcess::stop() {
    d->running = false;
  }
  void AbstractProcess::reset() {
    d->running = false;
    d->steps = 0;
    this->initialize();
  }
  int AbstractProcess::getStepsCount() {
    return d->steps;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // AbstractProcessWatched implementation
  struct AbstractProcessWatched::Private {
    static const long RATIO = CLOCKS_PER_SEC/1000L;
    clock_t start;
    long total;
    long currentElapsed() {
      clock_t diff = clock() - start;
      return diff / Private::RATIO;
    }
  };
  AbstractProcessWatched::AbstractProcessWatched():
    d(new Private)
  {
    d->total = 0;
  }
  AbstractProcessWatched::~AbstractProcessWatched() {
    delete d;
  }
  void AbstractProcessWatched::start() {
    d->start = clock();
    // Delegate to base
    AbstractProcess::start();
  }
  void AbstractProcessWatched::stop() {
    d->total += d->currentElapsed();
    // Delegate to base
    AbstractProcess::stop();
  }
  void AbstractProcessWatched::reset() {
    d->total = 0;
    // Delegate to base
    AbstractProcess::reset();
  }
  long AbstractProcessWatched::getTimeElapsed() {
    return d->total + d->currentElapsed();
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatSolver implementation
  struct SatSolver::Private {
    SatProblem            *problem;
    SatSolverParameters   *params;
    SatSolverEngine       *engine;
  };
  SatSolver::SatSolver (SatProblem *problem, SatSolverParameters *params):
    d(new Private)
  {
    d->problem = problem;
    d->params = params;
    d->engine = 0;
#ifndef NDEBUG
    std::cerr << "  Formulas count: " << problem->getFormulasCount() << std::endl;
    std::cerr << "  Variables count: " << problem->getVarsCount() << std::endl;
    std::cerr << "  Variables: ";
    for(int i=0; i< problem->getVarsCount(); i++)
      std::cerr << problem->getVarName(i) << ", ";
    std::cerr << std::endl;
#endif // NDEBUG
  }
  SatSolver::~SatSolver() {
    delete d->engine;
    delete d;
  }
  SatSolver* SatSolver::create (SatProblem *problem, SatSolverParameters *params) {
    SatSolver *obj = new SatSolver(problem, params);
    obj->initialize();
    return obj;
  }
  SatSolverStatsProxy* SatSolver::getStatsProxy() {
    ISatSolverStats *stats = d->engine;
    assert(0!=stats);
    return stats->getStatsProxy();
  }
  void SatSolver::initialize() {
    delete d->engine;
    d->engine = new SatSolverEngine(d->problem, d->params);
  }
  void SatSolver::doStep() {
    SatSolverEngine *engine = d->engine;
    assert(0!=engine);
    engine->doStep();
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatSolverEngine implementation
  struct SatSolverEngine::Private {
  };
  SatSolverEngine::SatSolverEngine(SatProblem *problem, SatSolverParameters *params):
    d(new Private)
  {
  }
  SatSolverEngine::~SatSolverEngine() {
    delete d;
  }
  SatSolverStatsProxy* SatSolverEngine::getStatsProxy() {
  }
  void SatSolverEngine::doStep() {
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // TimedStop implementation
  struct TimedStop::Private {
    AbstractProcessWatched *process;
    long msec;
  };
  TimedStop::TimedStop(AbstractProcessWatched *process, long msec):
    d(new Private)
  {
    d->process = process;
    d->msec = msec;
  }
  TimedStop::~TimedStop() {
    delete d;
  }
  void TimedStop::notify() {
    long elapsed = d->process->getTimeElapsed();
    if (elapsed > d->msec)
      d->process->stop();
  }




} // namespace FastSatSolver

