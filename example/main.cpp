/*! This is more of an example file demostrating how we should use our library 
 */
#include "utils/commandline.h"
#include "utils/utilities.h"
#include "utils/color.h"
#include "utils/image.h"
#include "utils/timer.h"

// all the algorithms
#include "api.h"
//#include "algorithms/Compositor.h"
//#include "algorithms/visit/CompositorVisIt.h"
//#include "algorithms/tree/CompositorTree.h"

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
using CommandLine::Parse;

int main(const int ac, const char* av[])
{ 
  //////////////////////////////////////////////////////////////////////////
  // Command Line Options
  std::vector<std::string> inputFiles;
  for (int i = 1; i < ac; ++i) {
    std::string str(av[i]);
    if (str == "-w") {
      Parse<1>(ac, av, i, width);
    }
    else if (str == "-h") {
      Parse<1>(ac, av, i, height);
    }
    else if (str == "-random") {
      Parse<1>(ac, av, i, random_size);
    }
    else if (str == "-numPatches") {
      Parse<1>(ac, av, i, numPatches);
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
  const size_t minX = 0, minY = 0, maxX = width, maxY = height;
  std::vector<Image> imgList(numPatches);
  for (int i = 0; i < numPatches; ++i) {
    InitImage(minX, maxX, minY, maxY, mpiRank, imgList[i], random_size);
  }
  
  //////////////////////////////////////////////////////////////////////////
  // Composition
  QCT::Init(ac, av);

  if (mpiSize == 1) {

    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    auto visit = QCT::Create(QCT::ALGO_VISIT_ONE_NODE, width, height);
    clock.Start(); 
    QCT::BeginFrame(visit);
    for (int i=0; i<numPatches; i++) {
      /* porting the code into our API */
      float depth = imgList[i].GetDepth();
      QCT::Tile tile(imgList[i].GetExtents(0),
                     imgList[i].GetExtents(2),
                     imgList[i].GetExtents(1),
                     imgList[i].GetExtents(3),
                     width,
                     height,
                     imgList[i].GetData(),
                     &depth,
                     QCT_TILE_REDUCED_DEPTH);
      QCT::SetTile(visit, tile);
    }    
    QCT::EndFrame(visit);
    clock.Stop();
    for (int i=0; i<numPatches; i++) {
      imgList[i].DeleteImage();
    }    
    ////////////////////////////////////////////////////////////////////////
    // Timing
    CreatePPM((float*)QCT::MapColorBuffer(visit),
              width, height, outputdir);
    std::cout << "[Single Node (VisIt method)] " << clock.GetDuration() 
	      << " seconds to finish" << std::endl;
  } 
  else {

    ////////////////////////////////////////////////////////////////////////
    // Test Tree Method
    auto tree = QCT::Create(QCT::ALGO_TREE, width, height);
    float depth = imgList[0].GetDepth();
    QCT::Tile tile(imgList[0].GetExtents(0),
                   imgList[0].GetExtents(2),
                   imgList[0].GetExtents(1),
                   imgList[0].GetExtents(3),
                   width,
                   height,
                   imgList[0].GetData(),
                   &depth,
                   QCT_TILE_REDUCED_DEPTH);
    QCT::BeginFrame(tree);
    QCT::SetTile(tree, tile);
    QCT::EndFrame(tree);
    
#if 0
    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    auto visit = QCT::Create(QCT::ALGO_VISIT_ICET, width, height);
    if (QCT::IsValid(visit) && numPatches == 1) {
      clock.Start();
      /* porting the code into our API */
      float depth = imgList[0].GetDepth();
      QCT::Tile tile(imgList[0].GetExtents(0),
                     imgList[0].GetExtents(2),
                     imgList[0].GetExtents(1),
                     imgList[0].GetExtents(3),
                     width,
                     height,
                     imgList[0].GetData(),
                     &depth,
                     QCT_TILE_REDUCED_DEPTH);
      QCT::BeginFrame(visit);
      QCT::SetTile(visit, tile);
      QCT::EndFrame(visit);
      clock.Stop();
      ////////////////////////////////////////////////////////////////////////
      // Timing
      CreatePPM((float*)QCT::MapColorBuffer(visit), 
		width, height, outputdir);
      std::cout << "[Multiple Node (VisIt method)] " << clock.GetDuration() 
		<< " seconds to finish" << std::endl;
    }
    else {
      std::cout << "[Multiple Node] Not Implemented" << std::endl;
    }
#endif

  }
  
  MPI_Finalize();

  return 0;
}
