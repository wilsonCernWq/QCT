#include "Tile.h"
#include <iostream>

namespace QCT {

  void Tile::Allocate()
  {
    this->rgba = new float[tileSize * 4];
    this->r = &(this->rgba[0]);
    this->g = &(this->rgba[tileSize]);
    this->b = &(this->rgba[tileSize+tileSize]);
    this->a = &(this->rgba[tileSize+tileSize+tileSize]);
    if (flag & QCT_TILE_REDUCED_DEPTH) {
      this->z = new float();
    } else {
      this->z = new float[tileSize];
    }
  }

  void Tile::Clean() 
  {
    if (!(flag & QCT_TILE_SHARED)) {
      if (this->rgba) delete[] this->rgba;
    }
    if (flag & QCT_TILE_REDUCED_DEPTH) {
      if (this->z) delete this->z;
    } else {
      if (this->z) delete[] this->z;
    }
  }

  Tile::~Tile()
  {}

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
      Error::WarnOnce("The tile cannot be shared because an RGBA "
                      "image in an SOA is required");
    }
    this->rgba = new float[tileSize * 4];
    this->r = &(this->rgba[0]);
    this->g = &(this->rgba[tileSize]);
    this->b = &(this->rgba[tileSize+tileSize]);
    this->a = &(this->rgba[tileSize+tileSize+tileSize]);
    // TODO we hope the compiler is smart enough here
    std::copy(rptr, rptr + tileSize, this->rgba + 0);
    std::copy(gptr, gptr + tileSize, this->rgba + tileSize);
    std::copy(bptr, bptr + tileSize, this->rgba + tileSize * 2);
    std::copy(aptr, aptr + tileSize, this->rgba + tileSize * 3);
    
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
    if (flag & QCT_TILE_FORMAT_SOA) {
      if (flag & QCT_TILE_SHARED) {
        this->rgba = rgba;
      } else {
        this->rgba = new float[tileSize * 4];
        this->r = &(this->rgba[0]);
        this->g = &(this->rgba[tileSize]);
        this->b = &(this->rgba[tileSize+tileSize]);
        this->a = &(this->rgba[tileSize+tileSize+tileSize]);
        std::copy(rgba, rgba + tileSize * 4, this->rgba);
      }
    } else {
      this->rgba = new float[tileSize * 4];
      this->r = &(this->rgba[0]);
      this->g = &(this->rgba[tileSize]);
      this->b = &(this->rgba[tileSize+tileSize]);
      this->a = &(this->rgba[tileSize+tileSize+tileSize]);
      // TODO parallel this loop
      for (auto i = 0; i < tileSize; ++i) {
        this->rgba[               i] = rgba[4 * i + 0];
        this->rgba[tileSize     + i] = rgba[4 * i + 1];
        this->rgba[tileSize * 2 + i] = rgba[4 * i + 2];
        this->rgba[tileSize * 3 + i] = rgba[4 * i + 3];
      }
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
