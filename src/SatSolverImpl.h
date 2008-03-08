#ifndef SATSOLVERIMPL_H
#define SATSOLVERIMPL_H

#include "SatSolver.h"

namespace FastSatSolver {

  class SatItem : public ISatItem
  {
      /**
       * @return int
       */
      virtual int getLength ( );

      /**
       * @return bool
       * @param  index
       */
      virtual bool getBit (int index );
  };

} // namespace FastSatSolver


#endif // SATSOLVERIMPL_H
