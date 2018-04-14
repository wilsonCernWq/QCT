#include "utils/commandline.h"
#include "utils/utilities.h"
#include "utils/color.h"
#include "utils/image.h"
#include "utils/timer.h"

// debug
#include "algorithms/tree/common/TreeDiagram.h"

// build a binary tree compositing program step by step
#include "algorithms/Compositor.h"
#include "algorithms/visit/CompositorVisIt.h"

#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include <mpi.h>

std::string outputdir = "./image.ppm";

void InitImage(int minX, int maxX, int minY, int maxY, int mpiRank, 
               Image &img, bool randomSizedImage)
{
  int dimsX = maxX - minX;
  int dimsY = maxY - minY;
  // random image dimension
  int rminX = minX + rand() % (dimsX-1);
  int rminY = minY + rand() % (dimsY-1);
  int rmaxX = rminX + 1 + rand() % (maxX - (rminX+1));
  int rmaxY = rminY + 1 + rand() % (maxY - (rminY+1));
  // random image 
  float depth = (rand() % 100)/100.0;
  // random color
  Color color;
  color.r = (rand() % 256)/256.0;
  color.g = (rand() % 256)/256.0;
  color.b = (rand() % 256)/256.0;
  color.a = (rand() % 256)/256.0;
  // create image
  Timer clock;
  clock.Start();
  if (randomSizedImage) { // random sized images    
    img.CreateImage(rminX, rmaxX, rminY, rmaxY);
    img.ColorImage(color);
  }
  else { // full sized images with bounding box    
    img.CreateImage(maxX, maxY, rminX, rmaxX, rminY, rmaxY);
    img.ColorImage(color, rminX, rmaxX, rminY, rmaxY);
  }
  img.SetDepth(depth);
  clock.Stop();
}

namespace CMD {
  static size_t width  = 716 - 34;
  static size_t height = 453 - 237;
  static bool random_size = true;
  static size_t numPatches = 1;
};
using namespace CMD;

int main(const int ac, const char* av[])
{ 
  //////////////////////////////////////////////////////////////////////////
  // Command Line Options
  std::vector<std::string> inputFiles;
  for (int i = 1; i < ac; ++i) {
    std::string str(av[i]);
    if (str == "-w") {
      CommandLine::Parse<1>(ac, av, i, CMD::width);
    }
    else if (str == "-h") {
      CommandLine::Parse<1>(ac, av, i, CMD::height);
    }
    else if (str == "-random") {
      CommandLine::Parse<1>(ac, av, i, CMD::random_size);
    }
    else if (str == "-numPatches") {
      CommandLine::Parse<1>(ac, av, i, CMD::numPatches);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // MPI stuff
  int mpiRank, mpiSize;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  int  hostnamelen;
  char hostname[512];
  MPI_Get_processor_name(hostname, &hostnamelen);

  //////////////////////////////////////////////////////////////////////////
  // Initialization
  srand((mpiRank+1)*25*time(NULL)); // random seed
  Timer clock; // Timer
  
  //////////////////////////////////////////////////////////////////////////
  // Randomly create images
  const size_t minX = 0, minY = 0, maxX = CMD::width, maxY = CMD::height;
  std::vector<Image> imgList(CMD::numPatches);
  for (int i = 0; i < numPatches; ++i) {
    InitImage(minX, maxX, minY, maxY, mpiRank, imgList[i], 
              CMD::random_size);
  }
  
  //////////////////////////////////////////////////////////////////////////
  // Composition
  if (mpiSize == 1) {

    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    WarmT::algorithms::visit::Compositor_VisIt
      visit(WarmT::algorithms::visit::Compositor_VisIt::ONENODE,
            CMD::width, CMD::height);  
    clock.Start(); 
    visit.BeginFrame();
    for (int i=0; i<numPatches; i++) {
      /* porting the code into our API */
      float depth = imgList[i].GetDepth();
      WarmT::Tile tile(imgList[i].GetExtents(0),
		       imgList[i].GetExtents(2),
		       imgList[i].GetExtents(1),
		       imgList[i].GetExtents(3),
		       width,
		       height,
		       imgList[i].GetData(),
		       &depth,
		       WARMT_TILE_REDUCED_DEPTH);
      visit.SetTile(tile);
    }    
    visit.EndFrame();
    clock.Stop();
    for (int i=0; i<numPatches; i++) {
      imgList[i].DeleteImage();
    }    
    ////////////////////////////////////////////////////////////////////////
    // Timing
    CreatePPM((float*)visit.MapColorBuffer(),
              width, height, outputdir);   
    std::cout << "[Single Node (VisIt method)] " << clock.GetDuration() 
	      << " seconds to finish" << std::endl;
  } 
  else {

    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    WarmT::algorithms::visit::Compositor_VisIt
      visit(WarmT::algorithms::visit::Compositor_VisIt::ICET, 
            CMD::width, CMD::height);  
    if (visit.IsValid() && numPatches == 1) {
      clock.Start();
      /* porting the code into our API */
      float depth = imgList[0].GetDepth();
      WarmT::Tile tile(imgList[0].GetExtents(0),
		       imgList[0].GetExtents(2),
		       imgList[0].GetExtents(1),
		       imgList[0].GetExtents(3),
		       width,
		       height,
		       imgList[0].GetData(),
		       &depth,
		       WARMT_TILE_REDUCED_DEPTH);
      visit.BeginFrame();
      visit.SetTile(tile);
      visit.EndFrame();
      clock.Stop();
      ////////////////////////////////////////////////////////////////////////
      // Timing
      CreatePPM((float*)visit.MapColorBuffer(), 
		width, height, outputdir);
      std::cout << "[Multiple Node (VisIt method)] " << clock.GetDuration() 
		<< " seconds to finish" << std::endl;
    }
    else {
      std::cout << "[Multiple Node] Not Implemented" << std::endl;
    }

  }
  
  MPI_Finalize();

  return 0;
}
