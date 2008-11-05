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

#ifndef SATSOLVER_H
#define SATSOLVER_H

/**
 * @file SatSolver.h
 * @brief ISatItem, IObserver and AbstractSatSolver with its base classes
 * @author Kamil Dudka <xdudka00@gmail.com>
 * @date 2008-11-05
 * @ingroup SatSolver
 */

#include <iostream>

namespace FastSatSolver {
  class SatProblem;

  /**
   * @interface ISatItem
   * Object of this type assigns bool value to each propositional variable.
   * @brief Abstraction of solution candidate.
   * @ingroup SatSolver
   */
  class ISatItem
  {
    public:
      virtual ~ISatItem() { }
      
      /**
       * @brief @return Returns clone of object's instance.
       * @note Design pattern @b prototype.
       */
      virtual ISatItem* clone() const = 0;

      /**
       * @brief @return Returns count of bits (resp. variables) managed by
       * object.
       */
      virtual int getLength ( ) const = 0;

      /**
       * @brief @return Returns value of desired bit (resp. variable).
       * @param index Index value has to be in range <0, getLength()-1>.
       */
      virtual bool getBit (int index ) const = 0;
  };

  /**
   * @interface IObserver
   * @brief Simple observer's base class.
   * @ingroup SatSolver
   * @note Design pattern @b observer.
   */
  class IObserver {
    public:
      virtual ~IObserver() { }
      
      /**
       * @brief Event notification, usually called by object implementing
       * ISubject interface.
       * @note Design pattern @b observer
       */
      virtual void notify() = 0;
  };

  /**
   * @interface ISubject
   * @brief Simple observer's subject base class.
   * @ingroup SatSolver
   * @note Design pattern @b observer.
   */
  class ISubject {
    public:
      virtual ~ISubject() { }
      
      /**
       * @brief Add observer to list of listeners.
       * @param observer Observer object to add to list of listeners.
       * @note Observers are notified in the same order, as there are added to
       * lsit of listeners.
       * @note Design pattern @b observer
       */
      virtual void addObserver(IObserver *observer) = 0;
  };


  /**
   * @brief Simple subject's base class.
   * @ingroup SatSolver
   * @note Design pattern @b observer.
   */
  class AbstractSubject: public ISubject {
    public:
      virtual ~AbstractSubject();
      
      // see ISubject dox
      virtual void addObserver(IObserver *);
    protected:
      AbstractSubject();
      
      /**
       * @brief Send notification to all observers (listeners).
       * @note Observers are notified in the same order, as there are added to
       * lsit of listeners.
       */
      void notify();
    private:
      struct Private;
      Private *d;
  };

  /**
   * @interface IProcess
   * @brief Continous process interface.
   * @ingroup SatSolver
   */
  class IProcess {
    public:
      virtual ~IProcess() { }
      
      /**
       * @brief Start process execution. This method returns control
       * after process ends (or after it is stopped by stop() or reset() method)
       */
      virtual void start() = 0;
      
      /**
       * @brief Stop currently executed process as soon as possible.
       */
      virtual void stop() = 0;
      
      /**
       * @brief Reset process to its initial state.
       * @note This implies stop() if process is running.
       */
      virtual void reset() = 0;
  };

  /**
   * @brief Base class of simple multi-step process.
   * @ingroup SatSolver
   * @note Design pattern @b template @b method
   */
  class AbstractProcess:
    public AbstractSubject,
    public IProcess
  {
    public:
      virtual ~AbstractProcess();
      virtual void start();
      virtual void stop();
      virtual void reset();
      
      /**
       * @brief Returns current step number.
       * @return Returns current step number.
       */
      virtual int getStepsCount();
    protected:
      AbstractProcess();
      
      /**
       * @brief Initialize process.
       * @note Design pattern @b template @b method
       */
      virtual void initialize() = 0;
      
      /**
       * @brief Do one step of process.
       * @note Design pattern @b template @b method
       */
      virtual void doStep() = 0;
    private:
      struct Private;
      Private *d;
  };

  /**
   * @interface IStopWatch
   * @brief Interface of time-watchable activity
   * @ingroup SatSolver
   */
  class IStopWatch {
    public:
      virtual ~IStopWatch() { }
      
      /**
       * @brief Returns time elapsed by activity.
       * @return Returns time elapsed by activity.
       */
      virtual long getTimeElapsed() = 0;
  };

  /**
   * @brief Multi-step process with time-watch extension.
   * @ingroup SatSolver
   * @note Design pattern @b template @b method
   */
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

  /**
   * @brief Linear storage container for ISatItem objects.
   * @attention On heap allocated objects are expected. They will be deleted
   * by container's destructor.
   * @ingroup SatSolver
   */
  class SatItemVector {
    public:
      SatItemVector();
      SatItemVector(const SatItemVector &); ///< @brief Deep copy.
      ~SatItemVector();
      
      /**
       * @brief @return Returns count of item managed by container.
       */
      int getLength() const;
      
      /**
       * @brief Direct access container item.
       * @param index Index should be in range <0, getLength()-1>.
       * @return Returns pointer to desired item.
       */
      ISatItem* getItem(int index) const;
      
      /**
       * @brief Add item to container.
       * @param item Item to add to container.
       * @attention On heap allocated objects is expected. It will be deleted
       * by container's destructor.
       */
      void addItem(ISatItem *item);
      
      /**
       * @brief Remove all item from container and free from memory.
       */
      void clear();
      
      /**
       * @brief Human readable container dump.
       * @param problem Pointer to SatProblem instance, which knows variable
       * names.
       * @param streamTo Standard output stream, which is used for output.
       */
      void writeOut(SatProblem *, std::ostream &streamTo) const;
    private:
      struct Private;
      Private *d;
  };

  /**
   * @brief Associative array for ISatItem objects.
   * @attention On heap allocated objects are expected. They will be deleted
   * by container's destructor.
   * @ingroup SatSolver
   */
  class SatItemSet {
    public:
      SatItemSet();
      ~SatItemSet();
      
      /**
       * @brief @return Returns count of item managed by container.
       */
      int getLength();
      
      /**
       * @brief Add item to container, if it hasn't been there before.
       * @param item Item to add to container.
       * @attention On heap allocated objects is expected. It will be deleted
       * by container's destructor.
       */
      void addItem(ISatItem *item);
      
      /**
       * @brief Export container's content to SatItemVector.
       * @note New SatItemVector will be allocated on the heap.
       */
      SatItemVector* createVector();
      
      /**
       * @brief Remove all item from container and free from memory.
       */
      void clear();
    private:
      struct Private;
      Private *d;
  };

  /**
   * It defines common interface (and partially behavior) for both solver
   * implementations - BlindSolver and GaSolver.
   * @brief SAT Solver base class.
   * @ingroup SatSolver
   */
  class AbstractSatSolver: public AbstractProcessWatched
  {
    public:
      virtual ~AbstractSatSolver();
      
      /**
       * @brief Returns pointer to instance of SatProblem used by solver.
       * @return Returns pointer to instance of SatProblem used by solver.
       */
      virtual SatProblem* getProblem() = 0;
      
      /**
       * @brief Returns current count solutions founded by solver.
       * @return Returns current count solutions founded by solver.
       */
      virtual int getSolutionsCount() = 0;
      
      /**
       * @brief Returns a set of solutions founded by solver.
       * @return Returns on heap allocated instance of SatItemVector containing
       * solutions founded by solver.
       */
      virtual SatItemVector* getSolutionVector() = 0;
      
      /**
       * @brief Returns fitness of the Worst solution managed by solver.
       */
      virtual float minFitness() = 0;
      
      /**
       * @brief Returns average fitness computed by solver.
       */
      virtual float avgFitness() = 0;
      
      /**
       * @brief Returns the Best fitness founded by solver.
       * @note This should be always 1.0 if solution was found.
       */
      virtual float maxFitness() = 0;

    protected:
      AbstractSatSolver();

    private:
      struct Private;
      Private *d;
  };

} // namespace FastSatSolver

#endif // SATSOLVER_H
