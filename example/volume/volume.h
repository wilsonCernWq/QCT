#pragma once

#include <ospray/ospray.h>
#include <glm/glm.hpp>

struct VolumeSource {
  virtual void Init(const int argc, const char** argv,
                    int mpiRank, int mpiSize) = 0;
  virtual OSPVolume Create() = 0;
};


