/*
 * Copyright (C) 2008 Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
 *
 * This file is part of fss (Fast SAT Solver).
 *
 * fss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * fss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fss.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <limits.h>
#include <math.h>
#include "fssIO.h"
#include "SatProblem.h"
#include "BlindSatSolver.h"

namespace FastSatSolver {

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


} // namespace FastSatSolver

