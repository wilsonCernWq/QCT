#pragma once


#include "../Compositor.h"
#include "../Tile.h"
#include "common/TreeDiagram.h"

#include <fstream>
#include <mpi.h>

namespace QCT {
namespace algorithms {
namespace tree {

  class Compositor_Tree : public Compositor {
    private:
    std::ifstream TreeFile;
    uint32_t W, H;
    uint32_t tileW, tileH;
    int region[4];
    float* rgba = nullptr; //save tile before compose
    float* output = nullptr; // output after compose
    float* depth = nullptr;
    int MPIRank;
    int SEND = -1;
    int RECEIVE = -1;
    int InfoIndex;
  public:

    Compositor_Tree(const uint32_t& width,
                    const uint32_t& height,
                    std::string tree_file);
    virtual ~Compositor_Tree() {};
    //! status
    bool IsValid() override;

    //! function to get final results
    const void *MapDepthBuffer() override;
    const void *MapColorBuffer() override;
    
    int GetParRank () ;

    void Unmap(const void *mappedMem) override;

    //! upload tile
    void SetTile(Tile &tile) override;
    //! set send 
    void SetSend(int send);
    //! set receive
    void SetReceive(int receive);

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
