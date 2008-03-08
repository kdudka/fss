#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <string>

namespace SatSovler {

  class ISatItem
  {
    public:
      virtual ~ISatItem() { }

      /**
       * @return int
       */
      virtual int getLength ( ) = 0;

      /**
       * @return bool
       * @param  index
       */
      virtual bool getBit (int index ) = 0;
  };


  class SatProblem
  {
    public:
      virtual ~SatProblem() { }

      /**
       * @return SatProblem*
       */
      static SatProblem* create ( );

      /**
       * @param  fileName
       */
      virtual void loadFromFile (std::string fileName ) = 0;

      /**
      */
      virtual void loadFromInput ( ) = 0;

      /**
       * @return int
       */
      virtual int getVarsCount ( ) = 0;

      /**
       * @return std::string
       * @param  index
       */
      virtual std::string getVarName (int index ) = 0;

      /**
       * @return int
       * @param  data
       */
      virtual int getSatsCount (ISatItem *data) = 0;

      /**
       * @return bool
       */
      virtual bool hasError ( ) = 0;
  };


  class ISatProblemSolver
  {
    public:
      virtual ~ISatProblemSolver() { }

      /**
      */
      virtual void start ( ) = 0;

      /**
      */
      virtual void stop ( ) = 0;

      /**
      */
      virtual void reset ( ) = 0;
  };

  class IEventListener
  {
    public:
      virtual ~IEventListener() { }

      /**
      */
      virtual void notify ( ) = 0;
  };

  class SatProblemSolver : public ISatProblemSolver
  {
    public:
      /**
       * @return SatProblemSolver*
       * @param  problem
       */
      static SatProblemSolver* create (SatProblem* problem );

      /**
      */
      void start ( );

      /**
      */
      void stop ( );

      /**
      */
      void reset ( );

      /**
       * @param  listener
       */
      void addEventListener (IEventListener *listener);

    protected:
      /**
       * @param  problem
       */
      SatProblemSolver (SatProblem* problem );
  };

  class StepsCountStopHandler : public IEventListener
  {
  };

  class FitnessStopHandler : public IEventListener
  {
  };

  class FitnessWatchHandler : public IEventListener
  {
  };

  class RealTimeStopHandler : public IEventListener
  {
  };

} // namespace SatSovler


#endif // SATSOLVER_H
