#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <string>

namespace FastSatSolver {

  class GenericException {
    public:
      GenericException(std::string text) {
        this->text_ = text;
      }

      std::string getText() {
        return this->text_;
      }

    private:
      std::string text_;
  };

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


  class ISatSolver
  {
    public:
      virtual ~ISatSolver() { }

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

  class SatSolver : public ISatSolver
  {
    public:
      /**
       * @return SatProblemSolver*
       * @param  problem
       */
      static SatSolver* create (SatProblem* problem );

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
      SatSolver (SatProblem* problem );
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

} // namespace FastSatSolver


#endif // SATSOLVER_H
