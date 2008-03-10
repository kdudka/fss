#include <assert.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <list>
#include <ga/GA1DBinStrGenome.h>
#include <ga/GASimpleGA.h>
#include <ga/GAStatistics.h>
#include "SatProblem.h"
#include "SatSolver.h"

namespace {
  struct FixFloatManip {
    int w,p;
    FixFloatManip(int integral, int decimal): w(integral+decimal+1), p(decimal) { }
  };
  std::ostream& operator<< (std::ostream &stream, const FixFloatManip &manip) {
    return stream << std::fixed << std::setw(manip.w) << std::setprecision(manip.p);
  }
}

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
    for(d->running=true; d->running; d->steps++) {
      this->doStep();
      this->notify();
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
      return diff/RATIO;
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
    std::cout << "<<< Formulas count: " << problem->getFormulasCount() << std::endl;
    std::cout << "<<< Variables count: " << problem->getVarsCount() << std::endl;
    std::cout << "<<< Variables: ";
    for(int i=0; i< problem->getVarsCount(); i++)
      std::cout << problem->getVarName(i) << ", ";
    std::cout << std::endl;
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
  SatProblem* SatSolver::getProblem() {
    return d->problem;
  }
  SatSolverParameters* SatSolver::getParameters() {
    return d->params;
  }
  // protected
  void SatSolver::initialize() {
    delete d->engine;
    d->engine = new SatSolverEngine(this);
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
    SatSolver                 *solver;
    GA1DBinaryStringGenome    *genome;
    GASimpleGA                *ga;
    static float fitness(GAGenome &);
  };
  SatSolverEngine::SatSolverEngine(SatSolver *solver):
    d(new Private)
  {
    d->problem = solver->getProblem();
    d->solver = solver;
    int genomeLength = d->problem->getVarsCount();
    d->genome = new GA1DBinaryStringGenome(genomeLength, Private::fitness, d);
    d->ga = new GASimpleGA(*(d->genome));
    GARandomSeed();
  }
  SatSolverEngine::~SatSolverEngine() {
    delete d->ga;
    delete d->genome;
    delete d;
  }
  SatSolverStatsProxy* SatSolverEngine::getStatsProxy() {
    class ProxyImpl: public SatSolverStatsProxy {
      public:
        ProxyImpl(SatSolverEngine::Private *d):
          SatSolverStatsProxy(d->solver, d->ga->statistics())
        {
        }
    };
    return new ProxyImpl(d);
  }
  void SatSolverEngine::doStep() {
    d->ga->step();
  }
  float SatSolverEngine::Private::fitness(GAGenome &genome) {
    // Static to non-static connections
    Private *d = reinterpret_cast<Private *>(genome.userData());
    SatProblem *problem = dynamic_cast<SatProblem *>(d->problem);
    SatSolver *solver = dynamic_cast<SatSolver *>(d->solver);
    const GABinaryString &bs= dynamic_cast<GABinaryString &>(genome);

    // Compute fitness
    SatItemGalibAdatper data(bs);
    const int formulasCount = problem->getFormulasCount();
    const int satsCount = problem->getSatsCount(&data);
    float fitness = static_cast<float>(satsCount)/formulasCount;

    static float maxFitness = 0.0;
    if (fitness > maxFitness) {
      maxFitness = fitness;
      solver->notify();
    }

    // TODO: scale fitness?
    return fitness;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatItemGalibAdatper implementation
  SatItemGalibAdatper::SatItemGalibAdatper(const GABinaryString &bs): bs_(bs) { }
  int SatItemGalibAdatper::getLength()        { return bs_.size(); }
  bool SatItemGalibAdatper::getBit(int index) { return bs_.bit(index); }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatSolverStatsProxy implementation
  struct SatSolverStatsProxy::Private {
    SatSolver     *solver;
    GAStatistics  stats;
  };
  // protected
  SatSolverStatsProxy::SatSolverStatsProxy(SatSolver *solver, const GAStatistics &stats):
    d(new Private)
  {
    d->solver = solver;
    d->stats.copy(stats);
    // FIXME: Is this necessary?
    d->stats.flushScores();
  }
  SatSolverStatsProxy::~SatSolverStatsProxy() {
    delete d;
  }
  const GAStatistics& SatSolverStatsProxy::statistics() const {
    return d->stats;
  }
  float SatSolverStatsProxy::getMaxFitness() const {
    return statistics().maxEver();
  }
  float SatSolverStatsProxy::getAvgFitness() const {
    return statistics().offlineMax();
  }
  float SatSolverStatsProxy::getMinFitness() const {
    return statistics().offlineMin();
  }
  int SatSolverStatsProxy::getGeneration() const {
    return statistics().generation();
  }
  int SatSolverStatsProxy::getTimeElapsed() const {
    return d->solver->getTimeElapsed();
  }
  std::ostream& operator<< (std::ostream &stream, const SatSolverStatsProxy &proxy) {
    stream << proxy.statistics();
    return stream;
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

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // FitnessWatch implementation
  struct FitnessWatch::Private {
    ISatSolverStats *solver;
    std::ostream    &stream;
    float           maxFitness;

    Private(std::ostream &streamTo): stream(streamTo) { }
  };
  FitnessWatch::FitnessWatch(ISatSolverStats *statsResource, std::ostream &streamTo):
    d(new Private(streamTo))
  {
    d->solver = statsResource;
    d->maxFitness = 0.0;
  }
  FitnessWatch::~FitnessWatch() {
    delete d;
  }
  void FitnessWatch::notify() {
    SatSolverStatsProxy *statsProxy= d->solver->getStatsProxy();
    float maxFitness = statsProxy->getMaxFitness();
    if (maxFitness <= d->maxFitness) {
      // Fitness not changed
      delete statsProxy;
      return;
    }
    d->maxFitness = maxFitness;

    const float avgFitness= statsProxy->getAvgFitness();
    const float minFitness= statsProxy->getMinFitness();
    const int generation= statsProxy->getGeneration();
    const float timeElapsed= (statsProxy->getTimeElapsed())/1000.0;

    d->stream
      << "--- satisfaction:" << FixFloatManip(3,1) << maxFitness*100.0 << "%"
      << "(avg:" << FixFloatManip(3,1) << avgFitness*100.0
      << ", min:" << FixFloatManip(3,1) << minFitness*100.0 << ")"
      << ", generation " << std::setw(5) << generation
      << ", time elapsed: " << FixFloatManip(5,2) << timeElapsed << " s"
      << std::endl;
      /*const int varsCount = problem->getVarsCount();
      for (int i=0; i<varsCount; i++) {
        std::cout << problem->getVarName(i) << "=" << data.getBit(i);
        if (i != varsCount-1)
          std::cout << ", ";
      }
      std::cout << ")" << std::endl;*/
    delete statsProxy;
  }
    /*if (formulasCount == satsCount) {
      std::cout << ">>> Satisfaction reached, stopping GA..." << std::endl;
      solver->stop();*/


} // namespace FastSatSolver

