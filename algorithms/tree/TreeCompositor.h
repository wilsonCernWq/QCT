#pragma once

#include "../Compositor.h"

namespace WarmT {

  class Compositor_Tree : public Compositor {
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
