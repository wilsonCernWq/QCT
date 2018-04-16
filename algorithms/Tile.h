// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include "common/error.h"
#include <cmath>
#include <array>

#define QCT_TILE_SHARED        (1 << 0)
#define QCT_TILE_REDUCED_DEPTH (1 << 1)

namespace QCT {  

  typedef std::array<uint32_t, 2> arr2u;
  typedef std::array<uint32_t, 3> arr3u;
  typedef std::array<uint32_t, 4> arr4u;

  class Tile {
  public:
    /*! we select this tile format because we want to eventually port our 
      library into ospray */
    /*!< register flag */
    uint32_t flag{0};
    /*!< screen region that this tile corresponds to */
    std::array<uint32_t, 4> region; /* x0 y0 x1 y1 */
    /*!< total frame buffer size, for the camera */ 
    std::array<uint32_t, 2> fbSize;
    /*!< tile size */ 
    std::array<uint32_t, 2> tileDim;
    uint32_t tileSize{0};
    // 'red' component; in float.
    float *r = nullptr;
    // 'green' component; in float.
    float *g = nullptr;
    // 'blue' component; in float.
    float *b = nullptr;
    // 'alpha' component; in float.
    float *a = nullptr;
    // 'depth' component; in float.
    float *z = nullptr;

    ~Tile();
    Tile() = default;   

    Tile(const std::array<uint32_t, 4> &region, 
         const std::array<uint32_t, 2> &fbSize,
         float* rptr, float* gptr, float* bptr, float* aptr,
         float* depth, 
         const uint32_t flag = 0);

    Tile(const std::array<uint32_t, 4> &region, 
         const std::array<uint32_t, 2> &fbSize,
         float* rgba, float* depth, 
         const uint32_t flag = 0);

    Tile(const uint32_t* region, 
         const uint32_t* fbSize,
         float* rptr, float* gptr, float* bptr, float* aptr,
         float* depth, const uint32_t flag = 0) 
      : Tile(arr4u{region[0], region[1], region[2], region[3]},
             arr2u{fbSize[0], fbSize[1]},
             rptr, gptr, bptr, aptr, depth, flag)
      {}

    Tile(const uint32_t* region, 
         const uint32_t* fbSize,
         float* rgba, float* depth, 
         const uint32_t flag = 0)
      : Tile(arr4u{region[0], region[1], region[2], region[3]},
             arr2u{fbSize[0], fbSize[1]},
             rgba, depth, flag)
      {}

    Tile(const uint32_t& r0, const uint32_t& r1, 
         const uint32_t& r2, const uint32_t& r3,
         const uint32_t& f0, const uint32_t& f1,
         float* rgba, float* d, 
         const uint32_t flag = 0)
      : Tile(arr4u{r0, r1, r2, r3}, arr2u{f0, f1}, rgba, d, flag)
      {}

  private:
    Tile(const std::array<uint32_t, 4> &region, 
         const std::array<uint32_t, 2> &fbSize,
         const uint32_t flag);
    void SetDepth(float* depth, const uint32_t flag = 0);
  };

};
