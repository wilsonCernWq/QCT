/*! This is a real example for rendering dummy volume using
 * how we should use our library */
#include "utils/commandline.h"
#include "utils/utilities.h"
#include "utils/color.h"
#include "utils/image.h"
#include "utils/timer.h"

#include "qct/api.h"

#if defined(_OPENMP)
# include <omp.h>
#endif

#include <mpi.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ospray/ospray.h"

#include <iostream>
#include <string>
#include <cmath>
#include <ratio>
#include <limits>
#include <chrono> 
#include <vector>     // c++11
#include <algorithm>  // c++11
#include <functional> // c++11

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

int main(int argc, const char **argv)
{
  //////////////////////////////////////////////////////////////////////////
  // Parameters
  //////////////////////////////////////////////////////////////////////////
  glm::ivec2 screen(1024, 1024), local;
  float cam_pos[] = {0.f, 0.f, 0.f};
  float cam_up [] = {0.f, 1.f, 0.f};
  float cam_view [] = {0.1f, 0.f, 1.f};

  //////////////////////////////////////////////////////////////////////////
  // Command Line Options
  //////////////////////////////////////////////////////////////////////////
  std::vector<std::string> inputFiles;
  for (int i = 1; i < ac; ++i) {
    std::string str(av[i]);
    if (str == "--osp:mpi") {
      std::runtime_error("we should not use --osp:mpi because we are not "
                         "going to use the ospray MPI");
    }
    else if (str == "-w") {
      Parse<1>(ac, av, i, width);
    }
    else if (str == "-h") {
      Parse<1>(ac, av, i, height);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // MPI stuff
  //////////////////////////////////////////////////////////////////////////
  int mpiRank, mpiSize;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  int  hostnamelen;
  char hostname[512];
  MPI_Get_processor_name(hostname, &hostnamelen);

  //////////////////////////////////////////////////////////////////////////
  // ospray setup
  //////////////////////////////////////////////////////////////////////////
  ospInit(&argc, argv);
  ospLoadModule("ispc");
  
  //! create world and renderer
  OSPModel world = ospNewModel();
  OSPRenderer renderer = ospNewRenderer("scivis"); 

  //! create lighting
  OSPLight ambient_light = ospNewLight(renderer, "AmbientLight");
  ospSet1f(ambient_light, "intensity", 0.0f);
  ospCommit(ambient_light);
  OSPLight directional_light = ospNewLight(renderer, "DirectionalLight");
  ospSet1f(directional_light, "intensity", 2.0f);
  ospSetVec3f(directional_light, "direction", osp::vec3f{20.0f, 20.0f, 20.0f});
  ospCommit(directional_light);
  std::vector<OSPLight> light_list { ambient_light, directional_light };
  OSPData lights = 
    ospNewData(light_list.size(), OSP_OBJECT, light_list.data());
  ospCommit(lights);

  //! init camera
  OSPCamera camera = ospNewCamera("perspective");
  ospSetf(camera, "aspect", imgSize.x/(float)imgSize.y);
  ospSet3fv(camera, "pos", cam_pos);
  ospSet3fv(camera, "dir", cam_view);
  ospSet3fv(camera, "up",  cam_up);
  ospCommit(camera); // commit each object to indicate modifications are done

  //! setup volume/geometry
  auto volume = (argc, argv);
  ospCommit(world);

  //! renderer
  ospSetVec3f(renderer, "bgColor", osp::vec3f{0.5f, 0.5f, 0.5f});
  ospSetData(renderer, "lights", lights);
  ospSetObject(renderer, "model", world);
  ospSetObject(renderer, "camera", camera.OSPRayPtr());
  ospSet1i(renderer, "shadowEnabled", 0);
  ospSet1i(renderer, "oneSidedLighting", 0);
  ospCommit(renderer);

  // exit
  Clean();
  return EXIT_SUCCESS;
}

