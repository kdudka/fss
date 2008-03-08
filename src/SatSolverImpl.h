#ifndef SATSOLVERIMPL_H
#define SATSOLVERIMPL_H

#include "SatSolver.h"

namespace SatSolver {

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

} // namespace SatSolver


#endif // SATSOLVERIMPL_H
