#include "CompositorTree.h"

namespace QCT {
namespace algorithms {
namespace tree {

    //! function to get final results
    const void *Compositor_Tree::MapDepthBuffer() { };
    const void *Compositor_Tree::MapColorBuffer() { };
    void Compositor_Tree::Unmap(const void *mappedMem) 
    {
    };

    //! clear (the specified channels of) this frame buffer
    void Compositor_Tree::Clear(const uint32_t channelFlags) {};

    //! status
    bool Compositor_Tree::IsValid() 
    {
    };

    //! begin frame
    void Compositor_Tree::BeginFrame() 
    {
    };
  
    //! end frame
    void Compositor_Tree::EndFrame() 
    {
    };

    //! upload tile
    void Compositor_Tree::SetTile(Tile &tile) 
    {
    };

};
};
};
