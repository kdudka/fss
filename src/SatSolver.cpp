#include <assert.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <list>
#include <ga/GA1DBinStrGenome.h>
#include <ga/GASimpleGA.h>
#include "SatProblem.h"
#include "SatSolver.h"

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
  // protected
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
  // protected
  void SatSolver::initialize() {
    delete d->engine;
    d->engine = new SatSolverEngine(d->problem, d->params);
  }
  // protected
  void SatSolver::doStep() {
    SatSolverEngine *engine = d->engine;
    assert(0!=engine);
    engine->doStep();
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatSolverEngine implementation
  struct SatSolverEngine::Private {
    SatProblem                *problem;
    GA1DBinaryStringGenome    *genome;
    GASimpleGA                *ga;
    static float fitness(GAGenome &);
  };
  SatSolverEngine::SatSolverEngine(SatProblem *problem, SatSolverParameters *params):
    d(new Private)
  {
    d->problem = problem;
    int genomeLength = problem->getVarsCount();
    d->genome = new GA1DBinaryStringGenome(genomeLength, Private::fitness, d);
    d->ga = new GASimpleGA(*(d->genome));
  }
  SatSolverEngine::~SatSolverEngine() {
    delete d->ga;
    delete d->genome;
    delete d;
  }
  SatSolverStatsProxy* SatSolverEngine::getStatsProxy() {
  }
  void SatSolverEngine::doStep() {
    d->ga->step();
  }
  float SatSolverEngine::Private::fitness(GAGenome &genome) {
    // Static to non-static connections
    Private *d = reinterpret_cast<Private *>(genome.userData());
    SatProblem *problem = dynamic_cast<SatProblem *>(d->problem);
    const GABinaryString &bs= dynamic_cast<GABinaryString &>(genome);

    // Adapter
    class Data: public ISatItem {
      public:
        Data(const GABinaryString &bs): bs_(bs) { }
        virtual int getLength() { return bs_.size(); }
        virtual bool getBit(int index) { return bs_.bit(index); }
      private:
        const GABinaryString &bs_;
    } data(bs);

    // Compute fitness
    const int formulasCount = problem->getFormulasCount();
    const int satsCount = problem->getSatsCount(&data);
    float fitness = static_cast<float>(satsCount)/formulasCount;

#ifndef NDEBUG
    static float maxFitness = 0.0;
    if (fitness > maxFitness) {
      maxFitness = fitness;
      using namespace std;
      std::cerr << "--- satisfaction: " << fixed << setw(5) << setprecision(1) << maxFitness*100.0 << "% (";
      const int varsCount = problem->getVarsCount();
      for (int i=0; i<varsCount; i++) {
        std::cerr << problem->getVarName(i) << "=" << data.getBit(i);
        if (i != varsCount-1)
          std::cerr << ", ";
      }
      std::cerr << ")" << std::endl;
    }
#endif // NDEBUG

    // TODO: scale fitness?
    return fitness;
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

