// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include <iostream>
#include <cmath>
#include <array>
#include "common/error.h"

#define WARMT_TILE_SHARED        1 << 0
#define WARMT_TILE_REDUCED_DEPTH 1 << 1

namespace WarmT {
  
  class Tile {
  public:
    /*! we select this tile format because we want to eventually port our 
     *  library into ospray
     */
    // 'red' component; in float.
    float *r;
    // 'green' component; in float.
    float *g;
    // 'blue' component; in float.
    float *b;
    // 'alpha' component; in float.
    float *a;
    // 'depth' component; in float.
    float *z;
    /*!< screen region that this tile corresponds to */
    std::array<uint32_t, 4> region;
    /*!< total frame buffer size, for the camera */ 
    std::array<uint32_t, 2> fbSize;
    /*!< tile size */ 
    std::array<uint32_t, 2> tileDim;
    uint32_t tileSize;
 
    Tile() = default;
    Tile(const std::array<uint32_t, 4> &region, 
	 const std::array<uint32_t, 2> &fbSize,
	 float* rptr, float* gptr, float* bptr, float* aptr,
	 float* depth, 
	 const uint32_t flag);
    Tile(const std::array<uint32_t, 4> &region, 
	 const std::array<uint32_t, 2> &fbSize,
	 float* rgba, float* depth, 
	 const uint32_t flag);

  private:
    void SetDepth(float* depth, const uint32_t flag);
  };

  /*! Abstract API for image compositing */  
  class Compositor {
  public:
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
