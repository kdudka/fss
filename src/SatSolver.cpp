#include <assert.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <list>
#include <vector>
#include <set>
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
  // GASatSolver implementation
  struct GASatSolver::Private {
    SatProblem                *problem;
    GASatSolver               *solver;
    GA1DBinaryStringGenome    *genome;
    GASimpleGA                *ga;

    static float fitness(GAGenome &);
  };
  // protected
  GASatSolver::GASatSolver (SatProblem *problem, const GAParameterList &params):
    d(new Private)
  {
    d->problem = problem;
    d->solver = this;
    const int varsCount = problem->getVarsCount();
#ifndef NDEBUG
    std::cout << "<<< Formulas count: " << problem->getFormulasCount() << std::endl;
    std::cout << "<<< Variables count: " << problem->getVarsCount() << std::endl;
    std::cout << "<<< Variables: ";
    for(int i=0; i< varsCount; i++) {
      std::cout << problem->getVarName(i);
      if (i==varsCount-1)
        std::cout << std::endl;
      else
        std::cout << ", ";
    }
#endif // NDEBUG
    d->genome = new GA1DBinaryStringGenome(varsCount, Private::fitness, d);
    d->ga = new GASimpleGA(*(d->genome));
    d->ga->parameters(params);
    //d->ga->terminator(GAGeneticAlgorithm::TerminateUponPopConvergence);
  }
  GASatSolver::~GASatSolver() {
    delete d->ga;
    delete d->genome;
    delete d;
  }
  GASatSolver* GASatSolver::create (SatProblem *problem, const GAParameterList &params) {
    GASatSolver *obj = new GASatSolver(problem, params);
    obj->initialize();
    return obj;
  }
  GAParameterList& GASatSolver::registerDefaultParameters(GAParameterList &params) {
    return GASimpleGA::registerDefaultParameters(params);
  }
  SatProblem* GASatSolver::getProblem() {
    return d->problem;
  }
  const GAStatistics& GASatSolver::getStatistics() const {
    return d->ga->statistics();
  }
  // protected
  void GASatSolver::initialize() {
    GARandomSeed();
    // TODO
  }
  // protected
  void GASatSolver::doStep() {
    GAGeneticAlgorithm &ga= *(d->ga);
    ga.step();
    if (ga.done()) {
      this->stop();
#ifndef NDEBUG
      std::cerr << ">>> Stopped by GAlib terminator" << std::endl;
#endif // NDEBUG
    }
  }
  float GASatSolver::Private::fitness(GAGenome &genome) {
    // Static to non-static connections
    Private *d = reinterpret_cast<Private *>(genome.userData());
    SatProblem *problem = dynamic_cast<SatProblem *>(d->problem);
    GASatSolver *solver = dynamic_cast<GASatSolver *>(d->solver);
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
  // GASatItem implementation
  struct GASatItem::Private {
    GABinaryString bs;

    Private(const GABinaryString &b): bs(b) { }
  };
  GASatItem::GASatItem(const GABinaryString &bs):
    d(new Private(bs))
  {
  }
  GASatItem::~GASatItem() {
    delete d;
  }
  int GASatItem::getLength() const {
    return d->bs.size();
  }
  bool GASatItem::getBit(int index) const {
    return d->bs.bit(index);
  }
  GASatItem* GASatItem::clone() const {
    return new GASatItem(d->bs);
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatItemGalibAdatper implementation
  SatItemGalibAdatper::SatItemGalibAdatper(const GABinaryString &bs):
    bs_(bs)
  {
  }
  int SatItemGalibAdatper::getLength() const {
    return bs_.size();
  }
  bool SatItemGalibAdatper::getBit(int index) const {
    return bs_.bit(index);
  }
  // FIXME: There is no deep copy (optimalization)
  SatItemGalibAdatper* SatItemGalibAdatper::clone() const {
    return new SatItemGalibAdatper(bs_);
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatItemVector implementation
  struct SatItemVector::Private {
    typedef std::vector<ISatItem *> TVector;
    TVector vect;
  };
  SatItemVector::SatItemVector():
    d(new Private)
  {
  }
  SatItemVector::~SatItemVector() {
    Private::TVector::iterator iter;
    for(iter=d->vect.begin(); iter!=d->vect.end(); iter++)
      delete *iter;

    delete d;
  }
  int SatItemVector::getLength() {
    return d->vect.size();
  }
  ISatItem* SatItemVector::getItem(int index) {
    return d->vect[index];
  }
  void SatItemVector::addItem(ISatItem *item) {
    d->vect.push_back(item);
  }
  void SatItemVector::writeOut(SatProblem *problem, std::ostream &stream) {
    const int nForms= this->getLength();
    const int nVars= problem->getVarsCount();
    for(int f=0; f<nForms; f++) {
      stream << std::setw(5) << f << ". ";
      ISatItem *item= getItem(f);
      for(int v=0; v<nVars; v++) {
        stream << problem->getVarName(v) << "=" << item->getBit(v);
        if (v==nVars-1)
          stream << std::endl;
        else
          stream << ", ";
      }
    }
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatItemSet implementation
  struct GASatItemSet::Private {
    class SatItemHashDecorator: public ISatItem {
      public:
        SatItemHashDecorator(ISatItem *item):
          item_(item)
        {
        }
        virtual ~SatItemHashDecorator() {
          delete item_;
        }
        virtual int getLength() const {
          return item_->getLength();
        }
        virtual bool getBit(int index) const {
          return item_->getBit(index);
        }
        virtual ISatItem* clone() const {
          return item_->clone();
        }
        bool operator< (const SatItemHashDecorator &other) const {
          for(int i=0; i<getLength(); i++) {
            if (getBit(i) < other.getBit(i))
              return true;
          }
          return false;
        }
      private:
        ISatItem *item_;
    };
    typedef std::set<SatItemHashDecorator> TSet;
    TSet set;

    void addItem(ISatItem *item) {
      set.insert(SatItemHashDecorator(item));
    }
  };
  GASatItemSet::GASatItemSet():
    d(new Private)
  {
  }
  GASatItemSet::~GASatItemSet() {
    delete d;
  }
  int GASatItemSet::getLength() {
    return d->set.size();
  }
  void GASatItemSet::addItem(const GABinaryString &bs) {
    d->addItem(new GASatItem(bs));
  }
  SatItemVector* GASatItemSet::createVector() {
    SatItemVector *vect= new SatItemVector;
    Private::TSet::iterator iter;
    for(iter=d->set.begin(); iter!=d->set.end(); iter++) {
      const Private::SatItemHashDecorator &satItemHasable= *iter;
      ISatItem *itemClone= satItemHasable.clone();
      vect->addItem(itemClone);
    }
    return vect;
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
    GASatSolver     *solver;
    std::ostream    &stream;
    float           maxFitness;

    Private(std::ostream &streamTo): stream(streamTo) { }
  };
  FitnessWatch::FitnessWatch(GASatSolver *solver, std::ostream &streamTo):
    d(new Private(streamTo))
  {
    d->solver = solver;
    d->maxFitness = 0.0;
  }
  FitnessWatch::~FitnessWatch() {
    delete d;
  }
  void FitnessWatch::notify() {
    GAStatistics stats = d->solver->getStatistics();
    float maxFitness = stats.maxEver();
    if (maxFitness <= d->maxFitness)
      // Fitness not changed
      return;
    d->maxFitness = maxFitness;

    const float avgFitness= stats.offlineMax();
    const float minFitness= stats.offlineMin();
    const int generation= stats.generation();
    const float timeElapsed= d->solver->getTimeElapsed()/1000.0;

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
  }

} // namespace FastSatSolver

