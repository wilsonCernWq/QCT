#pragma once

#include "../Compositor.h"
#include "common/TreeDiagram.h"

namespace WarmT {
namespace algorithms {
namespace tree {

  class Compositor_Tree : public Compositor {
  public:
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
