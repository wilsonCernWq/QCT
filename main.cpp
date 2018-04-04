#include "utils/utilities.h"
#include "utils/color.h"
#include "utils/image.h"
#include "utils/timer.h"

#include "composition/avtSLIVRImgMetaData.h"
#include "composition/avtSLIVRImgCommunicator.h"

// build a binary tree compositing program step by step

#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <mpi.h>

std::string outputdir = "./";

bool sortImgMetaDataByDepth(slivr::ImgMetaData const& before, 
			    slivr::ImgMetaData const& after)
{ return before.avg_z > after.avg_z; }
bool sortImgMetaDataByEyeSpaceDepth(slivr::ImgMetaData const& before,
				    slivr::ImgMetaData const& after)
{ return before.eye_z > after.eye_z; }

inline void InitImage
(int minX, int maxX, int minY, int maxY, 
 int myRank, Image &img, int idx, bool randSizeImg)
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
    // std::cout << "spent " << clock.GetDuration() 
    // 	      << " seconds to create subimage " 
    // 	      << "position " 
    // 	      << rminX << " " << rmaxX << " " 
    // 	      << rminY << " " << rmaxY << " "
    // 	      << "color " 
    // 	      << color.r << " " << color.g << " " << color.b << " " << color.a
    // 	      << std::endl;
    // img.OutputPPM(outputdir + 
    // 		  "rank-" + 
    //               std::to_string(myRank) + 
    // 		  "-idx-" + std::to_string(idx) + ".ppm");
}

// Arguments:
//   1 - width of image
//   2 - height of image
//   3 - option: 1-random sized image or 0-full size with bounding box
int main(int argc, char* argv[])
{   
    // read in arguments
    int width  = (argc > 1) ? atoi(argv[1]) : 716 - 34;
    int height = (argc > 2) ? atoi(argv[2]) : 453 - 237;
    bool randSizeImg = (argc > 3) ? atoi(argv[4]) == 0 : true;
    int fullImageExtents[4] = {0, width, 0, height};
    std::cout << "width " << width << " height " << height << std::endl;

    // MPI stuff
    int myRank, numProcs;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    int  hostnamelen;
    char hostname[512];
    MPI_Get_processor_name(hostname, &hostnamelen);
    avtSLIVRImgCommunicator imgComm;

    // random seed
    srand((myRank+1)*25);
    
    // image composer
    Timer clock;

    // init image   
    const int minX = 0, minY = 0;
    const int maxX = width; 
    const int maxY = height;

    // random image
    if (numProcs == 1) {

	int numPatches = 800;
	std::vector<Image> imgList(numPatches);
	// randomly create images
	for (int i = 0; i < numPatches; ++i) {
	    InitImage(minX, maxX, minY, maxY, 
		      myRank, imgList[i], i, randSizeImg);
	}
	// composition
	clock.Start();

        ////////////////////////////////////////////////////////////////////////
	// SERIAL : Single Processor
	std::cout << "Serial Compositing!" << std::endl;
	
	// Get the metadata for all patches
	// contains the metadata to composite the image
	std::vector<slivr::ImgMetaData> allPatchMeta;
	std::vector<slivr::ImgData>     allPatchData;
	
	for (int i=0; i<numPatches; i++)
	{
	    slivr::ImgMetaData currMeta;
	    currMeta.procId = myRank;
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
		  &sortImgMetaDataByEyeSpaceDepth);
	
	// Blend images
	int renderedWidth  = fullImageExtents[1] - fullImageExtents[0];
	int renderedHeight = fullImageExtents[3] - fullImageExtents[2];
	float *composedData = NULL;
	composedData = new float[renderedWidth * renderedHeight * 4]();
	
	for (int i=0; i<numPatches; i++)
	{
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

	clock.Stop();

	slivr::WriteArrayToPPM(outputdir + "composed", 
			       composedData, renderedWidth, renderedHeight);

        ////////////////////////////////////////////////////////////////////////
	
	std::cout << "[Single Node] " 
		  << clock.GetDuration() 
		  << " seconds to finish" << std::endl;
    } 
    else {
	std::cout << "[Multiple Node] Not Implemented" << std::endl;
      
    }

    MPI_Finalize();

    return 0;
}
