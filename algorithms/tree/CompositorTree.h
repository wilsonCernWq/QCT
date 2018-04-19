#pragma once

#include "../Compositor.h"
#include "../Tile.h"
#include "common/TreeDiagram.h"

#include <array>

#define QCT_ALGO_TREE_USE_SOA 0

namespace QCT {
namespace algorithms {
namespace tree {

  class Compositor_Tree : public Compositor {
  private:
    int MPIRank;
    int MPISize;
#if QCT_ALGO_TREE_USE_SOA
    Tile   tile;
#else
    int    tileSize[2];
    int    tileRegion[4]; /* x0 x1 y0 y1 */
    float  tileDepth;
    float* tileRGBA;
#endif
    int target = -1;
    int action = -1; // (-1 = send) (-2 = root)
    float* finalRGBA  = nullptr;
    float* finalDepth = nullptr;
    uint32_t finalSize[2];
  public:

    Compositor_Tree(const uint32_t& width,
                    const uint32_t& height);
    virtual ~Compositor_Tree() {};

    //! status
    bool IsValid() override;

    //! function to get final results
    const void *MapDepthBuffer() override;
    const void *MapColorBuffer() override;   
    void Unmap(const void *mappedMem) override;

    //! upload tile
    void SetTile(Tile &tile) override;

    //! clear (the specified channels of) this frame buffer
    void Clear(const uint32_t channelFlags) override;

    //! begin frame
    void BeginFrame() override;

    //! end frame
    void EndFrame() override;
  };

};
};
};
