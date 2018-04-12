#include "utils/commandline.h"
#include "utils/utilities.h"
#include "utils/color.h"
#include "utils/image.h"
#include "utils/timer.h"

#include "algorithms/visit/avtSLIVRImgMetaData.h"
#include "algorithms/visit/avtSLIVRImgCommunicator.h"

// build a binary tree compositing program step by step
#include "algorithms/Compositor.h"
#include "algorithms/tree/common/TreeDiagram.h"

#include <cstdlib>
#include <unistd.h>
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
  srand((mpiRank+1)*25);

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
  if (mpiSize == 1 && false) {

    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    ////////////////////////////////////////////////////////////////////////
    avtSLIVRImgCommunicator imgComm;       
    const int fullImageExtents[4] = {0, (int)width, 0, (int)height};
    // Start SERIAL : Single Processor    
    clock.Start();   
    // Get the metadata for all patches
    // contains the metadata to composite the image
    std::vector<slivr::ImgMetaData> allPatchMeta;
    std::vector<slivr::ImgData>     allPatchData;
    for (int i=0; i<numPatches; i++) {
      slivr::ImgMetaData currMeta;
      currMeta.procId = mpiRank;
      currMeta.patchNumber = i;
      currMeta.destProcId = 0;
      currMeta.inUse = 1;
      currMeta.dims[0] = imgList[i].GetWidth();
      currMeta.dims[1] = imgList[i].GetHeight();
      currMeta.screen_ll[0] = imgList[i].GetExtents(0);
      currMeta.screen_ll[1] = imgList[i].GetExtents(2);
      currMeta.screen_ur[0] = imgList[i].GetExtents(1);
      currMeta.screen_ur[1] = imgList[i].GetExtents(3);
      currMeta.avg_z = imgList[i].GetDepth();
      allPatchMeta.push_back(currMeta);
    }    
    // Sort with the largest z first
    std::sort(allPatchMeta.begin(), 
	      allPatchMeta.end(), 
	      [](slivr::ImgMetaData const& before,
		 slivr::ImgMetaData const& after)
	      { return before.eye_z > after.eye_z; });
    // Blend images
    int renderedWidth  = fullImageExtents[1] - fullImageExtents[0];
    int renderedHeight = fullImageExtents[3] - fullImageExtents[2];
    float *composedData = NULL;
    composedData = new float[renderedWidth * renderedHeight * 4]();    
    for (int i=0; i<numPatches; i++) {
      slivr::ImgMetaData currMeta = allPatchMeta[i];
      slivr::ImgData     currData;      
      currData.imagePatch = imgList[currMeta.patchNumber].GetData();
      currData.procId = currMeta.procId;
      currData.patchNumber = currMeta.patchNumber;
      int currExtents[4] = 
	{currMeta.screen_ll[0], currMeta.screen_ur[0], 
	 currMeta.screen_ll[1], currMeta.screen_ur[1]};
      imgComm.BlendBackToFront
	(currData.imagePatch, currExtents,
	 composedData, fullImageExtents);      
      // clean up data
      if (currData.imagePatch != NULL) {
	imgList[currMeta.patchNumber].DeleteImage();
      }
      currData.imagePatch = NULL;
    }    
    allPatchMeta.clear();
    allPatchData.clear();
    // Finalize
    clock.Stop();
    ////////////////////////////////////////////////////////////////////////
    CreatePPM(composedData, renderedWidth, renderedHeight, outputdir + "composed.ppm");   
    std::cout << "[Single Node (VisIt method)] " << clock.GetDuration() 
	      << " seconds to finish" << std::endl;
    ////////////////////////////////////////////////////////////////////////    

  } 
  else {

    ////////////////////////////////////////////////////////////////////////
    // Using VisIt Method
    ////////////////////////////////////////////////////////////////////////    
    avtSLIVRImgCommunicator imgComm;
    const int fullImageExtents[4] = {0, (int)width, 0, (int)height};
    clock.Start();
    if (imgComm.IceTValid() && numPatches == 1) {
      slivr::ImgMetaData currMeta;
      slivr::ImgData     currData;
      currMeta.procId = mpiRank;
      currMeta.patchNumber = 0;
      currMeta.destProcId = 0;
      currMeta.inUse = 1;
      currMeta.dims[0] = imgList[0].GetWidth();
      currMeta.dims[1] = imgList[0].GetHeight();
      currMeta.screen_ll[0] = imgList[0].GetExtents(0);
      currMeta.screen_ll[1] = imgList[0].GetExtents(2);
      currMeta.screen_ur[0] = imgList[0].GetExtents(1);
      currMeta.screen_ur[1] = imgList[0].GetExtents(3);
      currMeta.avg_z = imgList[0].GetDepth();
      currMeta.eye_z = imgList[0].GetDepth();
      // std::cout << imgList[0].GetDepth() << std::endl;
      // First Composition
      int compositedW = fullImageExtents[1] - fullImageExtents[0];
      int compositedH = fullImageExtents[3] - fullImageExtents[2];
      float *compositedData = NULL;
      if (mpiRank == 0) {
	compositedData = new float[4 * compositedW * compositedH]();
      }
      int currExtents[4] = 
	{std::max(currMeta.screen_ll[0]-fullImageExtents[0], 0), 
	 std::min(currMeta.screen_ur[0]-fullImageExtents[0], 
		  compositedW), 
	 std::max(currMeta.screen_ll[1]-fullImageExtents[2], 0),
	 std::min(currMeta.screen_ur[1]-fullImageExtents[2],
		  compositedH)};
      imgComm.IceTInit(compositedW, compositedH);
      imgComm.IceTSetTile(imgList[0].GetData(), 
			  currExtents,
			  currMeta.eye_z);
      imgComm.IceTComposite(compositedData);
      if (currData.imagePatch != NULL) {
	delete[] currData.imagePatch;
	currData.imagePatch = NULL;
      }     
      // Finalize
      clock.Stop();
      ////////////////////////////////////////////////////////////////////////
      CreatePPM(compositedData, 
		compositedW, compositedH,
		outputdir + "composed.ppm");
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
