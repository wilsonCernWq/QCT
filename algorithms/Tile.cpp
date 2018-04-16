#include "Tile.h"
#include <iostream>

namespace QCT {

  Tile::~Tile()
  {
    if (flag & QCT_TILE_SHARED) {
      if (this->r) delete[] this->r;
      if (this->g) delete[] this->g;
      if (this->b) delete[] this->b;
      if (this->a) delete[] this->a;
    }
    if (flag & QCT_TILE_REDUCED_DEPTH) {
      if (this->z) delete this->z;
    } else {
      if (this->z) delete[] this->z;
    }
  }

  Tile::Tile(const std::array<uint32_t, 4> &region, 
             const std::array<uint32_t, 2> &fbSize,
             const uint32_t flag)
    :  flag(flag), region(region), fbSize(fbSize)
  {
    tileDim[0] = region[2]-region[0];
    tileDim[1] = region[3]-region[1];
    tileSize = tileDim[0] * tileDim[1];	
  }
  
  Tile::Tile(const std::array<uint32_t, 4> &region, 
             const std::array<uint32_t, 2> &fbSize,
             float* rptr, float* gptr, float* bptr, float* aptr,
             float* depth, const uint32_t flag)
    : Tile(region, fbSize, flag)
  {
    /*! check if we need to do deep copy of the image data */
    if (flag & QCT_TILE_SHARED) {
      this->r = rptr;
      this->g = gptr;
      this->b = bptr;
      this->a = aptr;
    } else {
      this->r = new float[tileSize];
      this->g = new float[tileSize];
      this->b = new float[tileSize];
      this->a = new float[tileSize];
      // TODO we hope the compiler is smart enough here
      std::copy(rptr, rptr + tileSize, this->r);
      std::copy(gptr, gptr + tileSize, this->g);
      std::copy(bptr, bptr + tileSize, this->b);
      std::copy(aptr, aptr + tileSize, this->a);
    }

    SetDepth(depth, flag);

  }

  Tile::Tile(const std::array<uint32_t, 4> &region, 
             const std::array<uint32_t, 2> &fbSize,
             float* rgba, float* depth, const uint32_t flag)
    : Tile(region, fbSize, flag)
  {
    /*! because we are having different data structures, we have to do deep
     *  copy here
     */
    if (flag & QCT_TILE_SHARED) {
      Error::WarnOnce("The tile cannot be shared because the RGBA "
                      "data is in an AOS style,\n"
                      "\twhile a SOA style is required");
    }
    this->r = new float[tileSize];
    this->g = new float[tileSize];
    this->b = new float[tileSize];
    this->a = new float[tileSize];
    // TODO parallel this loop
    for (auto i = 0; i < tileSize; ++i) {
      this->r[i] = rgba[4 * i + 0];
      this->g[i] = rgba[4 * i + 1];
      this->b[i] = rgba[4 * i + 2];
      this->a[i] = rgba[4 * i + 3];
    }

    SetDepth(depth, flag);

  }

  void Tile::SetDepth(float* depth, const uint32_t flag)
  {
    /*! we support two modes here, one mode stores only one float 
     *  as tile depth, the other mode stores an depth buffer since
     *  each pixel can have different depths
     */
    if (flag & QCT_TILE_REDUCED_DEPTH) {
      this->z = new float(*depth);
    } else {
      this->z = new float[tileSize];
      std::copy(depth, depth + tileSize, this->z);
    }
  }

};
