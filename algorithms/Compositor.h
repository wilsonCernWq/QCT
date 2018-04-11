// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include <iostream>
#include <cmath>
#include <array>

#define TILE_SIZE 32

namespace WarmT {

  struct Tile {
    // 'red' component; in float.
    float r[TILE_SIZE*TILE_SIZE];
    // 'green' component; in float.
    float g[TILE_SIZE*TILE_SIZE];
    // 'blue' component; in float.
    float b[TILE_SIZE*TILE_SIZE];
    // 'alpha' component; in float.
    float a[TILE_SIZE*TILE_SIZE];
    // 'depth' component; in float.
    float z[TILE_SIZE*TILE_SIZE];
    /*!< screen region that this tile corresponds to */
    std::array<int32_t, 4>  region;
    /*!< total frame buffer size, for the camera */ 
    std::array<uint32_t, 2> fbSize;
    std::array<float, 2>    rcp_fbSize;
 
    Tile() = default;
    Tile(const std::array<int32_t, 2>  &tid, 
	 const std::array<uint32_t, 2> &size)
    : fbSize{size[0], size[1]},
      rcp_fbSize{1.f / static_cast<float>(size[0]),
	         1.f / static_cast<float>(size[1])}
    {
      region[0] = tid[0] * TILE_SIZE;
      region[1] = tid[1] * TILE_SIZE;
      region[2] = std::min(region[0] + TILE_SIZE, 
			   static_cast<int32_t>(fbSize[0]));
      region[3] = std::min(region[1] + TILE_SIZE, 
			   static_cast<int32_t>(fbSize[1]));
    }
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
