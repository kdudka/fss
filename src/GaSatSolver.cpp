#include <ga/GA1DBinStrGenome.h>
#include <ga/GASimpleGA.h>
#include <ga/GAStatistics.h>

#include "fssIO.h"
#include "SatProblem.h"
#include "GaSatSolver.h"

namespace FastSatSolver {

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
  // GASatSolver implementation
  struct GASatSolver::Private {
    SatProblem                *problem;
    GASatSolver               *solver;
    float                     maxFitness;
    GA1DBinaryStringGenome    *genome;
    GASimpleGA                *ga;
    SatItemSet              *resultSet;

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
    d->resultSet = new SatItemSet;
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
    SatItemSet *resultSet= dynamic_cast<SatItemSet *>(d->resultSet);

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
      resultSet->addItem(new GASatItem(bs));
      solver->notify();
    };

    // TODO: scale fitness?
    return fitness;
  }


} // namespace FastSatSolver

