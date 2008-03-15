#include <assert.h>
#include <time.h>
#include <math.h>
#include <limits.h>
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
#include "fssIO.h"

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
    bool running;
    long currentElapsed() {
      clock_t diff = clock() - start;
      return diff/RATIO;
    }
  };
  AbstractProcessWatched::AbstractProcessWatched():
    d(new Private)
  {
    d->total = 0;
    d->running = false;
  }
  AbstractProcessWatched::~AbstractProcessWatched() {
    delete d;
  }
  void AbstractProcessWatched::start() {
    d->start = clock();
    d->running = true;
    // Delegate to base
    AbstractProcess::start();
  }
  void AbstractProcessWatched::stop() {
    if (d->running) {
      d->running = false;
      d->total += d->currentElapsed();
    }
    // Delegate to base
    AbstractProcess::stop();
  }
  void AbstractProcessWatched::reset() {
    d->running = false;
    d->total = 0;
    // Delegate to base
    AbstractProcess::reset();
  }
  long AbstractProcessWatched::getTimeElapsed() {
    /*if (!d->running) {
      std::cerr << "AbstractProcessWatched::getTimeElapsed()" << std::endl;
      std::cerr << "  running: false" << std::endl;
      std::cerr << "    total: " << d->total << std::endl;
      std::cerr << "  current: " << d->currentElapsed() << std::endl;
    }*/
    long total = d->total;
    if (d->running)
      total+= d->currentElapsed();
    return total;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // AbstractSatSolver implementation
  AbstractSatSolver::AbstractSatSolver() { }
  AbstractSatSolver::~AbstractSatSolver() { }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // BlindSatSolver implementation
  struct BlindSatSolver::Private {
    SatProblem        *problem;
    int               stepWidth;
    long              end;
    long              current;
    float             minFitness;
    float             maxFitness;
    double            sumFitness;
    SatItemVector     resultSet;

    void init() {
      current = 0L;
      minFitness = INFINITY;
      maxFitness = 0.0;
      sumFitness = 0.0;
    }
  };
  BlindSatSolver::BlindSatSolver(SatProblem *problem, int stepWidth):
    d(new Private)
  {
    const unsigned varsCount= problem->getVarsCount();
    if (varsCount+2 >= LONG_BIT) {
      delete d;
      throw GenericException("Too much variables - can't use blind solver on this machine!");
    }
    d->problem = problem;
    d->stepWidth = stepWidth;
    d->end = 1L<<varsCount;
    d->init();
  }
  BlindSatSolver::~BlindSatSolver() {
    delete d;
  }
  SatProblem* BlindSatSolver::getProblem() {
    return d->problem;
  }
  int BlindSatSolver::getSolutionsCount() {
    return d->resultSet.getLength();
  }
  SatItemVector* BlindSatSolver::getSolutionVector() {
    return new SatItemVector(d->resultSet);
  }
  float BlindSatSolver::minFitness() {
    return d->minFitness;
  }
  float BlindSatSolver::avgFitness() {
    return d->sumFitness / d->current;
  }
  float BlindSatSolver::maxFitness() {
    return d->maxFitness;
  }
  // protected
  void BlindSatSolver::initialize() {
    d->init();
    d->resultSet.clear();
  }
  // protected
  void BlindSatSolver::doStep() {
    const int nVars= d->problem->getVarsCount();
    const int nForms= d->problem->getFormulasCount();
    const int countPerStep = 1 << d->stepWidth;
    for(int i=0; i< countPerStep; i++) {
      if (d->current >= d->end) {
        // all space explored
        this->stop();
        break;
      }

      // Evalueate fitness
      LongSatItem data(nVars, d->current++);
      const int nSats= d->problem->getSatsCount(&data);
      const float fitness= static_cast<float>(nSats)/nForms;

      // Update statistics
      d->sumFitness += fitness;
      if (fitness < d->minFitness)
        d->minFitness = fitness;

      if (fitness > d->maxFitness) {
        // maxFitness increased
        d->maxFitness = fitness;
        this->notify();
      }

      if (nSats == nForms) {
        // Solution found
        d->resultSet.addItem(data.clone());
        this->notify();
      }
    }
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // GASatSolver implementation
  struct GASatSolver::Private {
    SatProblem                *problem;
    GASatSolver               *solver;
    float                     maxFitness;
    GA1DBinaryStringGenome    *genome;
    GASimpleGA                *ga;
    GASatItemSet              *resultSet;

    static float fitness(GAGenome &);
  };
  // protected
  GASatSolver::GASatSolver (SatProblem *problem, const GAParameterList &params):
    d(new Private)
  {
    d->problem = problem;
    d->solver = this;
    d->maxFitness = 0.0;
    const int varsCount = problem->getVarsCount();
    d->genome = new GA1DBinaryStringGenome(varsCount, Private::fitness, d);
    d->ga = new GASimpleGA(*(d->genome));
    d->ga->parameters(params);
    bool termUponConvergence = false;
    params.get("term_upon_convergence", &termUponConvergence);
    if (termUponConvergence)
      d->ga->terminator(GAGeneticAlgorithm::TerminateUponPopConvergence);
    d->resultSet = new GASatItemSet;
  }
  GASatSolver::~GASatSolver() {
    delete d->resultSet;
    delete d->ga;
    delete d->genome;
    delete d;
  }
  GASatSolver* GASatSolver::create (SatProblem *problem, const GAParameterList &params) {
    GASatSolver *obj = new GASatSolver(problem, params);
    obj->initialize();
    return obj;
  }
  void GASatSolver::registerDefaultParameters(GAParameterList &params) {
    GASimpleGA::registerDefaultParameters(params);
    const bool FALSE = false;
    params.add("term_upon_convergence", "convterm", GAParameter::BOOLEAN, &FALSE);
  }
  SatProblem* GASatSolver::getProblem() {
    return d->problem;
  }
  const GAStatistics& GASatSolver::getStatistics() const {
    return d->ga->statistics();
  }
  int GASatSolver::getSolutionsCount() {
    return d->resultSet->getLength();
  }
  SatItemVector* GASatSolver::getSolutionVector() {
    return d->resultSet->createVector();
  }
  float GASatSolver::minFitness() {
    return d->ga->statistics().offlineMin();
  }
  float GASatSolver::avgFitness() {
    return d->ga->statistics().offlineMax();
  }
  float GASatSolver::maxFitness() {
    return d->maxFitness;
  }
  // protected
  void GASatSolver::initialize() {
    GARandomSeed();
    d->maxFitness = 0.0;
    d->ga->initialize();
    // Now using incremental strategy
    // d->resultSet->clear();
  }
  // protected
  void GASatSolver::doStep() {
    GAGeneticAlgorithm &ga= *(d->ga);
    ga.step();
    if (ga.done()) {
      this->stop();
#if 0//ndef NDEBUG
      std::cerr << ">>> Stopped by GAlib terminator" << std::endl;
#endif // NDEBUG
    }
  }
  float GASatSolver::Private::fitness(GAGenome &genome) {
    // Static to non-static binding
    Private *d = reinterpret_cast<Private *>(genome.userData());
    SatProblem *problem = dynamic_cast<SatProblem *>(d->problem);
    GASatSolver *solver = dynamic_cast<GASatSolver *>(d->solver);
    const GABinaryString &bs= dynamic_cast<GABinaryString &>(genome);
    GASatItemSet *resultSet= dynamic_cast<GASatItemSet *>(d->resultSet);

    // Compute fitness
    SatItemGalibAdatper data(bs);
    const int formulasCount = problem->getFormulasCount();
    const int satsCount = problem->getSatsCount(&data);
    float fitness = static_cast<float>(satsCount)/formulasCount;
    if (fitness > d->maxFitness) {
      d->maxFitness = fitness;
      solver->notify();
    }

    if (formulasCount==satsCount) {
      resultSet->addItem(bs);
      solver->notify();
    };

    // TODO: scale fitness?
    return fitness;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // GASatItem implementation
  struct GASatItem::Private {
    GABinaryString bs;

    Private(unsigned size): bs(size) { }
  };
  GASatItem::GASatItem(const GABinaryString &bs):
    d(new Private(bs.size()))
  {
    d->bs.copy(bs);
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
  // LongSatItem implementation
  LongSatItem::LongSatItem(int length, long fromNumber):
    length_(length),
    lNumber_(fromNumber)
  {
  }
  LongSatItem::~LongSatItem() {
  }
  int LongSatItem::getLength() const {
    return length_;
  }
  bool LongSatItem::getBit(int index) const {
    assert(index < length_);
    return (1L<<index) & lNumber_;
  }
  LongSatItem* LongSatItem::clone() const {
    return new LongSatItem(length_, lNumber_);
  }



  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SatItemGalibAdatper implementation
  SatItemGalibAdatper::SatItemGalibAdatper(const GABinaryString &bs):
    bs_(bs)
  {
  }
  SatItemGalibAdatper::~SatItemGalibAdatper() { }
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

    Private(unsigned size=0): vect(size) { }
  };
  SatItemVector::SatItemVector():
    d(new Private)
  {
  }
  SatItemVector::SatItemVector(const SatItemVector &other):
    d(new Private(other.getLength()))
  {
    const int length= other.getLength();
    for(int i=0; i<length; i++)
      d->vect[i] = other.getItem(i)->clone();
  }
  SatItemVector::~SatItemVector() {
    this->clear();
    delete d;
  }
  int SatItemVector::getLength() const {
    return d->vect.size();
  }
  ISatItem* SatItemVector::getItem(int index) const {
    return d->vect[index];
  }
  void SatItemVector::addItem(ISatItem *item) {
    d->vect.push_back(item);
  }
  void SatItemVector::writeOut(SatProblem *problem, std::ostream &stream) const {
    const int nForms= this->getLength();
    const int nVars= problem->getVarsCount();
    for(int f=0; f<nForms; f++) {
      stream << std::setw(5) << f+1 << ". ";
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
  void SatItemVector::clear() {
    Private::TVector::iterator iter;
    for(iter=d->vect.begin(); iter!=d->vect.end(); iter++)
      delete *iter;
    d->vect.clear();
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
        void dispose() {
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
            if (!getBit(i) && other.getBit(i))
              return true;
            else if (getBit(i) && !other.getBit(i))
              return false;
          }
          return false;
        }
      private:
        ISatItem *item_;
    };
    typedef std::set<SatItemHashDecorator> TSet;
    TSet set;

    void addItem(ISatItem *item) {
      SatItemHashDecorator hashableItem(item);
      if (set.end()==set.find(hashableItem))
        set.insert(hashableItem);
      else
        hashableItem.dispose();
    }
  };
  GASatItemSet::GASatItemSet():
    d(new Private)
  {
  }
  GASatItemSet::~GASatItemSet() {
    this->clear();
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
  void GASatItemSet::clear() {
    Private::TSet::iterator iter;
    for(iter=d->set.begin(); iter!=d->set.end(); iter++) {
      Private::SatItemHashDecorator &i= 
        const_cast<Private::SatItemHashDecorator &>(*iter);
      i.dispose();
    }
    d->set.clear();
  }


  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // SolutionsCountStop implementation
  struct SolutionsCountStop::Private {
    AbstractSatSolver *solver;
    int               minCountOfSolutions;
  };
  SolutionsCountStop::SolutionsCountStop(AbstractSatSolver *solver, int minCountOfSolutions):
    d(new Private)
  {
    d->solver = solver;
    d->minCountOfSolutions = minCountOfSolutions;
  }
  SolutionsCountStop::~SolutionsCountStop() {
    delete d;
  }
  void SolutionsCountStop::notify() {
    const int nSolutions= d->solver->getSolutionsCount();
    if (nSolutions >= d->minCountOfSolutions)
      d->solver->stop();
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
    AbstractSatSolver   *solver;
    std::ostream        &stream;
    float               maxFitness;

    Private(std::ostream &streamTo): stream(streamTo) { }
  };
  FitnessWatch::FitnessWatch(AbstractSatSolver *solver, std::ostream &streamTo):
    d(new Private(streamTo))
  {
    d->solver = solver;
    d->maxFitness = 0.0;
  }
  FitnessWatch::~FitnessWatch() {
    delete d;
  }
  void FitnessWatch::notify() {
    AbstractSatSolver *solver= d->solver;
    float maxFitness = solver->maxFitness();
    if (maxFitness <= d->maxFitness)
      // Fitness not changed
      return;

    int generation = 0;
    GASatSolver *gaSolver= dynamic_cast<GASatSolver *>(solver);
    if (gaSolver) {
      GAStatistics stats= gaSolver->getStatistics();
      generation = stats.generation();
      if (0 == generation)
        return;
    }

    // Save maxFitness for next call
    d->maxFitness = maxFitness;

    // Read other statistics
    const float minFitness= solver->minFitness();
    const float avgFitness= solver->avgFitness();
    const float timeElapsed= d->solver->getTimeElapsed()/1000.0;

    d->stream
      << "--- satisfaction:" << FixFloatManip(3,1) << maxFitness*100.0 << "%"
      << "(avg:" << FixFloatManip(3,1) << avgFitness*100.0
      << ", min:" << FixFloatManip(3,1) << minFitness*100.0 << ")"
      << ", generation " << std::setw(5) << generation
      << ", time elapsed: " << FixFloatManip(5,2) << timeElapsed << " s"
      << std::endl;
  }
  void FitnessWatch::reset() {
    d->maxFitness = 0.0;
  }

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // ResultsWatch implementation
  struct ResultsWatch::Private {
    AbstractSatSolver   *solver;
    std::ostream        &stream;
    int                 nResults;

    Private(std::ostream &streamTo): stream(streamTo) { }
  };
  ResultsWatch::ResultsWatch(AbstractSatSolver *solver, std::ostream &streamTo):
    d(new Private(streamTo))
  {
    d->solver = solver;
    d->nResults = 0;
  }
  ResultsWatch::~ResultsWatch() {
    delete d;
  }
  void ResultsWatch::notify() {
    using namespace StreamDecorator;
    AbstractSatSolver *solver= d->solver;
    const int nResults= solver->getSolutionsCount();
    if (nResults <= d->nResults)
      return;
    d->nResults = nResults;

    const float timeElapsed= solver->getTimeElapsed()/1000.0;
    d->stream
      << Color(C_LIGHT_BLUE) << "--- solution #" << nResults
      << " found in " << FixFloatManip(5,2) << timeElapsed << " s"
      << Color() << std::endl;
  }
  void ResultsWatch::reset() {
    d->nResults = 0;
  }





} // namespace FastSatSolver

