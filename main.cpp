#include "utils/commandline.h"
#include "utils/utilities.h"
#include "utils/color.h"
#include "utils/image.h"
#include "utils/timer.h"

#include "algorithms/visit/CompositorVisIt.h"
#include "algorithms/visit/avtSLIVRImgMetaData.h"
#include "algorithms/visit/avtSLIVRImgCommunicator.h"

// build a binary tree compositing program step by step
#include "algorithms/Compositor.h"
#include "algorithms/tree/common/TreeDiagram.h"

#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

#include <mpi.h>

std::string outputdir = "./";

void InitImage
(int minX, int maxX, int minY, int maxY, 
 int mpiRank, Image &img, int idx, bool randSizeImg)
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
    if (randSizeImg) // random sized images
    {
        img.CreateImage(rminX, rmaxX, rminY, rmaxY);
        img.ColorImage(color);
    }
    else // full sized images with bounding box
    {
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

  // MPI stuff
  int mpiRank, mpiSize;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
  int  hostnamelen;
  char hostname[512];
  MPI_Get_processor_name(hostname, &hostnamelen);

  // random seed
  srand((mpiRank+1)*25*time(NULL));

  // Timer
  Timer clock;
  
  // init image   
  const size_t minX = 0, minY = 0, maxX = CMD::width, maxY = CMD::height;
  
  ////////////////////////////////////////////////////////////////////////
  // Randomly create images
  std::vector<Image> imgList(CMD::numPatches);
  for (int i = 0; i < numPatches; ++i) {
    InitImage(minX, maxX, minY, maxY, mpiRank, imgList[i], i, CMD::random_size);
  }
  
  // random image
  if (mpiSize == 1) {

    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    ////////////////////////////////////////////////////////////////////////
    avtSLIVRImgCommunicator imgComm;       
    //const int fullImageExtents[4] = {0, (int)width, 0, (int)height};
    // Start SERIAL : Single Processor    
    imgComm.OneNodeInit(width, height);
    clock.Start();   
    // Get the metadata for all patches
    // contains the metadata to composite the image
    //std::vector<slivr::ImgMetaData> allPatchMeta;
    //std::vector<slivr::ImgData>     allPatchData;
    for (int i=0; i<numPatches; i++) {
      int extents[4] = {imgList[i].GetExtents(0),
                        imgList[i].GetExtents(1),
                        imgList[i].GetExtents(2),
                        imgList[i].GetExtents(3)};
      imgComm.OneNodeSetTile(imgList[i].GetData(),
                             extents, imgList[i].GetDepth());
      // float depth = imgList[i].GetDepth();
      // WarmT::Tile tile(imgList[i].GetExtents(0),
      // 		       imgList[i].GetExtents(2),
      // 		       imgList[i].GetExtents(1),
      // 		       imgList[i].GetExtents(3),
      // 		       width,
      // 		       height,
      // 		       imgList[i].GetData(),
      // 		       &depth,
      // 		       WARMT_TILE_REDUCED_DEPTH);

      // slivr::ImgMetaData currMeta;
      // currMeta.procId = mpiRank;
      // currMeta.patchNumber = i;
      // currMeta.destProcId = 0;
      // currMeta.inUse = 1;
      // currMeta.dims[0] = imgList[i].GetWidth();
      // currMeta.dims[1] = imgList[i].GetHeight();
      //currMeta.screen_ll[0] = imgList[i].GetExtents(0);
      //currMeta.screen_ll[1] = imgList[i].GetExtents(2);
      //currMeta.screen_ur[0] = imgList[i].GetExtents(1);
      //currMeta.screen_ur[1] = imgList[i].GetExtents(3);
      //currMeta.eye_z = imgList[i].GetDepth();
      //allPatchMeta.push_back(currMeta);
    }    
    // // Sort with the largest z first
    // std::sort(allPatchMeta.begin(), 
    //           allPatchMeta.end(), 
    //           [](slivr::ImgMetaData const& before,
    //     	 slivr::ImgMetaData const& after)
    //           { return before.eye_z > after.eye_z; });

    // // Blend images
    // int renderedWidth  = fullImageExtents[1] - fullImageExtents[0];
    // int renderedHeight = fullImageExtents[3] - fullImageExtents[2];

    float *composedData = new float[CMD::width * CMD::height * 4]();    
    imgComm.OneNodeComposite(composedData);
    
    for (int i=0; i<numPatches; i++) {
      // slivr::ImgMetaData currMeta = allPatchMeta[i];
      // slivr::ImgData     currData;      
      // currData.imagePatch = imgList[currMeta.patchNumber].GetData();
      // currData.procId = currMeta.procId;
      // currData.patchNumber = currMeta.patchNumber;
      // int currExtents[4] = 
      //   {currMeta.screen_ll[0], currMeta.screen_ur[0], 
      //    currMeta.screen_ll[1], currMeta.screen_ur[1]};
      // imgComm.BlendBackToFront
      //   (currData.imagePatch, currExtents,
      //    composedData, fullImageExtents);      
      // clean up data
      //if (currData.imagePatch != NULL) {
      imgList[i].DeleteImage();
        //}
      //currData.imagePatch = NULL;
    }    
    //allPatchMeta.clear();
    //allPatchData.clear();
    // Finalize
    clock.Stop();
    ////////////////////////////////////////////////////////////////////////
    CreatePPM(composedData, width, height, outputdir + "image.ppm");   
    std::cout << "[Single Node (VisIt method)] " << clock.GetDuration() 
	      << " seconds to finish" << std::endl;
    ////////////////////////////////////////////////////////////////////////    
  } 
  else {

    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    ////////////////////////////////////////////////////////////////////////    
    clock.Start();
    WarmT::Compositor_VisIt visit(WarmT::Compositor_VisIt::ICET,
				  CMD::width, CMD::height);
    if (visit.IsValid() && numPatches == 1) {
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
      CreatePPM((float*)visit.MapColorBuffer(), 
		width, height, outputdir + "image.ppm");
      std::cout << "[Multiple Node (VisIt method)] " << clock.GetDuration() 
		<< " seconds to finish" << std::endl;
      ////////////////////////////////////////////////////////////////////////    
    }
    else {
      std::cout << "[Multiple Node] Not Implemented" << std::endl;
    }

  }
  
  MPI_Finalize();

  return 0;
}
