#include "api.h"

#include "algorithms/visit/CompositorVisIt.h"
#include "algorithms/tree/CompositorTree.h"

#include <mpi.h>

namespace QCT {

  //! create compositor
  void Init(const int argc, const char** argv) {};
  Context Create(const Algorithms algo, 
                 const size_t& w,
                 const size_t& h)
  {   
    /* namescope */
    using namespace algorithms;
    /* create a compositor */
    Context compositor;
    switch (algo) {
    case ALGO_VISIT_ICET:
      compositor = 
        std::make_unique<algorithms::visit::Compositor_VisIt>
        (algorithms::visit::Compositor_VisIt::ICET, w, h);
      break;
    case ALGO_VISIT_ONE_NODE:
      compositor =
        std::make_unique<algorithms::visit::Compositor_VisIt>
        (algorithms::visit::Compositor_VisIt::ONENODE, w, h);
      break;
    case ALGO_TREE:
      compositor = 
	std::make_unique<algorithms::tree::Compositor_Tree>
        (w, h);
      break;
    }
    return compositor;
  };
  
  //! status
  bool IsValid(const Context& c) {
    return c->IsValid();
  };

  //! function to get final results
  const void *MapDepthBuffer(const Context& c) { 
    return c->MapDepthBuffer(); 
  };
  const void *MapColorBuffer(const Context& c) {
    return c->MapColorBuffer(); 
  };
  void Unmap(const Context& c, const void *mappedMem) {
    c->Unmap(mappedMem);
  };
  
  //! upload tile
  void SetTile(Context& c, Tile &tile) { c->SetTile(tile); };
  
  //! clear (the specified channels of) this frame buffer
  void Clear(Context& c, const uint32_t channelFlags) {
    c->Clear(channelFlags);
  };
  
  //! begin frame
  void BeginFrame(Context& c) { c->BeginFrame(); };
  
  //! end frame
  void EndFrame(Context& c) { c->EndFrame(); };

};
