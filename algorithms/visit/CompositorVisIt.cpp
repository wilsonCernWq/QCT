#include "CompositorVisIt.h"

#include "avtSLIVRImgMetaData.h"
#include "avtSLIVRImgCommunicator.h"

namespace WarmT {
  enum Mode { SEQUENTIAL, ICET } mode;
  Compositor_VisIt::Compositor_VisIt(const Mode& m)
    : mode(m)
  {
    
  };
  Compositor_VisIt::~Compositor_VisIt() {};
};
