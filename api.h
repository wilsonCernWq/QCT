#pragma once

#include "algorithms/Tile.h"
#include "algorithms/Compositor.h"
#include <memory>

namespace QCT {

  using Context = std::unique_ptr<QCT::algorithms::Compositor>;

  enum Algorithms { 
    ALGO_VISIT_ICET, 
    ALGO_VISIT_ONE_NODE, 
    ALGO_TREE,
  };

  //! create compositor
  void Init(const int argc, const char** argv);
  Context Create(const Algorithms, 
                 const size_t& w,
                 const size_t& h);
  
  //! status
  bool IsValid(const Context&);

  //! function to get final results
  const void *MapDepthBuffer(const Context&);
  const void *MapColorBuffer(const Context&);
  void Unmap(const Context&, const void *mappedMem);
  
  //! upload tile
  void SetTile(Context&, Tile &tile);
  
  //! clear (the specified channels of) this frame buffer
  void Clear(Context&, const uint32_t channelFlags);
  
  //! begin frame
  void BeginFrame(Context&);
  
  //! end frame
  void EndFrame(Context&);
 
};
