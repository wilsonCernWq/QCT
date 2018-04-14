// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include "Tile.h"

#include <cmath>
#include <array>

#if PARALLEL
# include <mpi.h>
#endif

namespace WarmT {  
  namespace algorithms {
    /*! Abstract API for image compositing */  
    class Compositor {
    protected:
      int mpiRank = 0, mpiSize = 1;
      Compositor() {
#if PARALLEL
        MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
        MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
#endif
      }
    public:
      //! status
      virtual bool IsValid() = 0;

      //! function to get final results
      virtual const void *MapDepthBuffer() = 0;
      virtual const void *MapColorBuffer() = 0;
      virtual void Unmap(const void *mappedMem) = 0;

      //! upload tile
      virtual void SetTile(Tile &tile) = 0;

      //! clear (the specified channels of) this frame buffer
      virtual void Clear(const uint32_t channelFlags) = 0;

      //! begin frame
      virtual void BeginFrame() = 0;

      //! end frame
      virtual void EndFrame() = 0;
    };
  };
};
