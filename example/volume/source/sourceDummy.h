#pragma once

#include "../volume.h"
#include <string>

class VolumeSource_Dummy : public VolumeSource {
 private:
  void SetupTF(const void *colors, const void *opacities, 
               int colorL, int opacityL, float lower, float upper)
  {
    OSPTransferFunction transferFcn = ospNewTransferFunction("piecewise_linear");
    OSPData colorsData = ospNewData(colorL, OSP_FLOAT3, colors);
    ospCommit(colorsData);
    OSPData opacitiesData = ospNewData(opacityL, OSP_FLOAT, opacities);
    ospCommit(opacitiesData);
    ospSetData(transferFcn,  "colors",    colorsData);
    ospSetData(transferFcn,  "opacities", opacitiesData);
    ospSetVec2f(transferFcn, "valueRange", osp::vec2f{lower, upper});
    ospCommit(transferFcn);
    ospRelease(colorsData);
    ospRelease(opacitiesData);
  };

 private:
  typedef unsigned char TYPE_CXX;
  const int TYPE_OSP = OSP_UCHAR;
  const std::string TYPE_STR = "uchar";  
  TYPE_CXX *volumeData;
  int mpiRank, mpiSize;
  
 public:
  void Init(const int argc, const char** argv,
            int mpiRank, int mpiSize) override 
  {
    this->mpiRank = mpiRank;
    this->mpiSize = mpiSize;
  };

  OSPVolume Create() override 
  {
    //! generate data
    const glm::ivec3 dims(20, 10, 10);
    auto volumeData = new TYPE_CXX[dims.x * dims.y * dims.z];
    cleanlist.push_back([=](){ delete[] volumeData; });
    for (int x = 0; x < dims.x; ++x) {
      for (int y = 0; y < dims.y; ++y) {
        for (int z = 0; z < dims.z; ++z) {
          int c = sin(x/(float)(dims.x-1) * M_PI / 2.0) * 255.0;
          int i = z * dims.y * dims.x + y * dims.x + x;
          volumeData[i] = (TYPE_CXX)c;
        }
      }
    }

    //! transfer function
    const std::vector<glm::vec3> colors = {
      glm::vec3(0, 0, 0.563),
      glm::vec3(0, 0, 1),
      glm::vec3(0, 1, 1),
      glm::vec3(0.5, 1, 0.5),
      glm::vec3(1, 1, 0),
      glm::vec3(1, 0, 0),
      glm::vec3(0.5, 0, 0),
    };
    const std::vector<float> opacities = { 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f };
    SetupTF(colors.data(), opacities.data(), 
            colors.size(), opacities.size(), 0.f, 255.f);
    
    //! create ospray volume
    auto t1 = std::chrono::system_clock::now();
    {
      OSPVolume volume = ospNewVolume("shared_structured_volume");
      OSPData voxelData = ospNewData(dims.x * dims.y * dims.z, 
                                     TYPE_OSP, volumeData //, 
                                     OSP_DATA_SHARED_BUFFER);
      ospSetString(volume, "voxelType", TYPE_STR);
      ospSetVec3i(volume, "dimensions", (osp::vec3i&)dims);
      ospSetVec3f(volume, "gridOrigin",  
                  osp::vec3f{-dims.x/2.0f,-dims.y/2.0f,-dims.z/2.0f});
      ospSetVec3f(volume, "gridSpacing", osp::vec3f{1.0f, 1.0f, 1.0f});
      ospSet1f(volume, "samplingRate", 1.0f);
      ospSet1i(volume, "gradientShadingEnabled", 0);
      ospSet1i(volume, "useGridAccelerator", 0);
      ospSet1i(volume, "adaptiveSampling", 0);
      ospSet1i(volume, "preIntegration", 0);
      ospSet1i(volume, "singleShade", 0);
      ospSetData(volume, "voxelData", voxelData);
      ospSetObject(volume, "transferFunction", transferFcn);
      ospCommit(volume);
      ospAddVolume(world, volume);
    }
    auto t2 = std::chrono::system_clock::now();
    std::chrono::duration<double> dur = t2 - t1;
    std::cout << "finish commits " << dur.count() << " seconds" << std::endl;
  };
};

