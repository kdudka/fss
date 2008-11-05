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

#include <time.h>
#include <iostream>
#include <iomanip>
#include <list>
#include <vector>
#include <set>
#include "fssIO.h"
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
  struct SatItemSet::Private {
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
  SatItemSet::SatItemSet():
    d(new Private)
  {
  }
  SatItemSet::~SatItemSet() {
    this->clear();
    delete d;
  }
  int SatItemSet::getLength() {
    return d->set.size();
  }
  void SatItemSet::addItem(ISatItem *item) {
    d->addItem(item);
  }
  SatItemVector* SatItemSet::createVector() {
    SatItemVector *vect= new SatItemVector;
    Private::TSet::iterator iter;
    for(iter=d->set.begin(); iter!=d->set.end(); iter++) {
      const Private::SatItemHashDecorator &satItemHasable= *iter;
      ISatItem *itemClone= satItemHasable.clone();
      vect->addItem(itemClone);
    }
    return vect;
  }
  void SatItemSet::clear() {
    Private::TSet::iterator iter;
    for(iter=d->set.begin(); iter!=d->set.end(); iter++) {
      Private::SatItemHashDecorator &i= 
        const_cast<Private::SatItemHashDecorator &>(*iter);
      i.dispose();
    }
    d->set.clear();
  }

} // namespace FastSatSolver

