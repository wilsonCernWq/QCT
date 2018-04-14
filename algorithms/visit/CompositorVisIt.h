#pragma once

#include "../Compositor.h"

namespace WarmT {

  class Compositor_VisIt : public Compositor {
  private:
    uint32_t W, H;
    float* rgba  = nullptr;
    float* depth = nullptr;
  public:
    enum Mode { ONENODE, ICET, SERIAL } mode;
    Compositor_VisIt(const Mode& m, 
		     const uint32_t& width,
		     const uint32_t& height);
    virtual ~Compositor_VisIt();

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
