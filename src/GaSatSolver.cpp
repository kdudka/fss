#include <ga/GA1DBinStrGenome.h>
#include <ga/GASimpleGA.h>
#include <ga/GAStatistics.h>

#include "fssIO.h"
#include "SatProblem.h"
#include "GaSatSolver.h"

namespace FastSatSolver {

  // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // GaSatItem implementation
  struct GaSatItem::Private {
    GABinaryString bs;

    Private(unsigned size): bs(size) { }
  };
  GaSatItem::GaSatItem(const GABinaryString &bs):
    d(new Private(bs.size()))
  {
    d->bs.copy(bs);
  }
  GaSatItem::~GaSatItem() {
    delete d;
  }
  int GaSatItem::getLength() const {
    return d->bs.size();
  }
  bool GaSatItem::getBit(int index) const {
    return d->bs.bit(index);
  }
  GaSatItem* GaSatItem::clone() const {
    return new GaSatItem(d->bs);
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
  // GaSatSolver implementation
  struct GaSatSolver::Private {
    SatProblem                *problem;
    GaSatSolver               *solver;
    float                     maxFitness;
    GA1DBinaryStringGenome    *genome;
    GASimpleGA                *ga;
    SatItemSet              *resultSet;

    static float fitness(GAGenome &);
  };
  // protected
  GaSatSolver::GaSatSolver (SatProblem *problem, const GAParameterList &params):
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
  GaSatSolver::~GaSatSolver() {
    delete d->resultSet;
    delete d->ga;
    delete d->genome;
    delete d;
  }
  GaSatSolver* GaSatSolver::create (SatProblem *problem, const GAParameterList &params) {
    GaSatSolver *obj = new GaSatSolver(problem, params);
    obj->initialize();
    return obj;
  }
  void GaSatSolver::registerDefaultParameters(GAParameterList &params) {
    GASimpleGA::registerDefaultParameters(params);
    const bool FALSE = false;
    params.add("term_upon_convergence", "convterm", GAParameter::BOOLEAN, &FALSE);
  }
  SatProblem* GaSatSolver::getProblem() {
    return d->problem;
  }
  const GAStatistics& GaSatSolver::getStatistics() const {
    return d->ga->statistics();
  }
  int GaSatSolver::getSolutionsCount() {
    return d->resultSet->getLength();
  }
  SatItemVector* GaSatSolver::getSolutionVector() {
    return d->resultSet->createVector();
  }
  float GaSatSolver::minFitness() {
    return d->ga->statistics().offlineMin();
  }
  float GaSatSolver::avgFitness() {
    return d->ga->statistics().offlineMax();
  }
  float GaSatSolver::maxFitness() {
    return d->maxFitness;
  }
  // protected
  void GaSatSolver::initialize() {
    GARandomSeed();
    d->maxFitness = 0.0;
    d->ga->initialize();
    // Now using incremental strategy
    // d->resultSet->clear();
  }
  // protected
  void GaSatSolver::doStep() {
    GAGeneticAlgorithm &ga= *(d->ga);
    ga.step();
    if (ga.done()) {
      this->stop();
#if 0//ndef NDEBUG
      std::cerr << ">>> Stopped by GAlib terminator" << std::endl;
#endif // NDEBUG
    }
  }
  float GaSatSolver::Private::fitness(GAGenome &genome) {
    // Static to non-static binding
    Private *d = reinterpret_cast<Private *>(genome.userData());
    SatProblem *problem = dynamic_cast<SatProblem *>(d->problem);
    GaSatSolver *solver = dynamic_cast<GaSatSolver *>(d->solver);
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
      resultSet->addItem(new GaSatItem(bs));
      solver->notify();
    };

    // TODO: scale fitness?
    return fitness;
  }


} // namespace FastSatSolver

