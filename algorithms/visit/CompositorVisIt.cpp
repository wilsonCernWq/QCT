#include "CompositorVisIt.h"

#include "avtSLIVRImgMetaData.h"
#include "avtSLIVRImgCommunicator.h"

static avtSLIVRImgCommunicator* imgComm;
static float* IceT_tile_buffer = nullptr;

namespace WarmT {

  Compositor_VisIt::Compositor_VisIt(const Mode& m,
				     const uint32_t& width,
				     const uint32_t& height)    
    : mode(m), W(width), H(height)
  {
    if (mpiRank == 0) { rgba = new float[4 * W * H](); }
    imgComm = new avtSLIVRImgCommunicator();
  };

  Compositor_VisIt::~Compositor_VisIt() {
    if (mpiRank == 0) {
    if (rgba) delete[] rgba;
      }
    if (IceT_tile_buffer) delete[] IceT_tile_buffer;
    delete imgComm;
  };

  //! function to get final results
  const void *Compositor_VisIt::MapDepthBuffer() { return depth; };
  const void *Compositor_VisIt::MapColorBuffer() { return rgba; };
  void Compositor_VisIt::Unmap(const void *mappedMem) { mappedMem = nullptr; };

  //! clear (the specified channels of) this frame buffer
  void Compositor_VisIt::Clear(const uint32_t channelFlags) {};

  //! status
  bool Compositor_VisIt::IsValid() {
    switch (mode) {
    case(ICET):
      return imgComm->IceTValid();
    }
  }

  //! upload tile
  void Compositor_VisIt::SetTile(Tile &tile) {
    switch (mode) {
    case(ICET):
      if (IceT_tile_buffer) {
	delete[] IceT_tile_buffer;
	Error::WarnAlways("IceT_tile_buffer is non empty");
      } 
      IceT_tile_buffer = new float[4 * tile.tileDim[0] * tile.tileDim[1]];
      for (auto i = 0; i < tile.tileSize; ++i) {
	IceT_tile_buffer[4 * i + 0] = tile.r[i];
	IceT_tile_buffer[4 * i + 1] = tile.g[i];
	IceT_tile_buffer[4 * i + 2] = tile.b[i];
	IceT_tile_buffer[4 * i + 3] = tile.a[i];
      }
      int e[4] = {(int)tile.region[0], (int)tile.region[2],
		  (int)tile.region[1], (int)tile.region[3]}; /* x0 x1 y0 y1 */
      imgComm->IceTSetTile(IceT_tile_buffer, e,
			   *(tile.z));
      break;
    }
  };
    
  //! begin frame
  void Compositor_VisIt::BeginFrame() {
    switch (mode) {
    case(ICET):
      imgComm->IceTInit(W, H);
      break;
    }
  };
  
  //! end frame
  void Compositor_VisIt::EndFrame() {
    switch (mode) {
    case(ICET):
      imgComm->IceTComposite(rgba);
    }
  };
};
