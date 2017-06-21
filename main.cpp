#include "utils/utilities.h"
#include "utils/color.h"
#include "utils/image.h"
#include "utils/timer.h"

#include "visit/avtSLIVRImgMetaData.h"
#include "visit/avtSLIVRImgCommunicator.h"

#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <mpi.h>

std::string outputdir = 
    "/home/sci/qwu/Projects/multiThread/compositing/output/";

bool sortImgMetaDataByDepth
(slivr::ImgMetaData const& before, slivr::ImgMetaData const& after)
{ return before.avg_z > after.avg_z; }
bool sortImgMetaDataByEyeSpaceDepth
(slivr::ImgMetaData const& before, slivr::ImgMetaData const& after)
{ return before.eye_z > after.eye_z; }

inline void InitImage
(int minX, int maxX, int minY, int maxY, 
 int myRank, Image &img, int idx, bool randSizeImg)
{
    int dimsX = maxX - minX;
    int dimsY = maxY - minY;
    // random image dimension
    int rminX = rand() % (dimsX-1);
    int rmaxX = rminX + 1 + rand() % (maxX - (rminX+1));
    rminX = rand() % (dimsX-1-80);
    rmaxX = rminX + 1 + 80;
    int rminY = rand() % (dimsY-1);
    int rmaxY = rminY + 1 + rand() % (maxY - (rminY+1));
    rminY = rand() % (dimsY-1-20);
    rmaxY = rminY + 1 + 20;
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
    if (randSizeImg)  // random sized images
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
	
	//
	// Sort with the largest z first
	//
	std::sort(allPatchMeta.begin(), 
		  allPatchMeta.end(), 
		  &sortImgMetaDataByEyeSpaceDepth);
	
	//
	// Blend images
	//
	int renderedWidth  = fullImageExtents[1] - fullImageExtents[0];
	int renderedHeight = fullImageExtents[3] - fullImageExtents[2];
	float *composedData = NULL;
	composedData = new float[renderedWidth * renderedHeight * 4]();
	
	for (int i=0; i<numPatches; i++)
	{
	    slivr::ImgMetaData currMeta = allPatchMeta[i];
	    slivr::ImgData     currData;

	    currData.imagePatch = imgList[i].GetData();
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
		imgList[i].DeleteImage();
	    }
	    currData.imagePatch = NULL;
	}
	
	allPatchMeta.clear();
	allPatchData.clear();

	clock.Stop();

	slivr::WriteArrayToPPM(outputdir + "composed", 
			       composedData, renderedWidth, renderedHeight);

	//     //
	//     // Create image for visit to display
	//     //
	//     avtImage_p whole_image;
	//     whole_image = new avtImage(this);

	//     vtkImageData *img = 
	// 	avtImageRepresentation::NewImage(screen[0], screen[1]);
	//     whole_image->GetImage() = img;

	//     unsigned char *imgFinal = NULL;
	//     imgFinal = new unsigned char[screen[0] * screen[1] * 3];
	//     imgFinal = whole_image->GetImage().GetRGBBuffer();

	//     //
	//     // Blend in with bounding box and other visit plots
	//     //
	//     vtkMatrix4x4 *Inversepvm = vtkMatrix4x4::New();
	//     vtkMatrix4x4::Invert(pvm, Inversepvm);

	//     int compositedImageWidth  = 
	// 	fullImageExtents[1] - fullImageExtents[0];
	//     int compositedImageHeight = 
	// 	fullImageExtents[3] - fullImageExtents[2];

	//     // Having to adjust the dataset bounds by a arbitrary magic 
	//     // number here. 
	//     // Needs to be sorted out at some point!
	//     // dbounds[5] = dbounds[5]-0.025;

	//     debug5 << "Place in image ~ screen "  
	// 	   <<  screen[0] << ", " << screen[1] 
	// 	   << "  compositedImageWidth:  " << compositedImageWidth 
	// 	   << "  compositedImageHeight: " << compositedImageHeight
	// 	   << "  fullImageExtents: " 
	// 	   << fullImageExtents[0] << ", " 
	// 	   << fullImageExtents[1] << ", " 
	// 	   << fullImageExtents[2] << ", " 
	// 	   << fullImageExtents[3] << std::endl;

	//     for (int _y=0; _y<screen[1]; _y++) 
	//     {
	// 	for (int _x=0; _x<screen[0]; _x++)
	// 	{

	// 	    int index = _y*screen[0] + _x;
	// 	    int indexComposited = 
	// 		(_y-fullImageExtents[2]) * compositedImageWidth + 
	// 		(_x-fullImageExtents[0]);

	// 	    bool insideComposited = false;
	// 	    if (_x >= fullImageExtents[0] && 
	// 		_x < fullImageExtents[1])
	// 	     	if (_y >= fullImageExtents[2] && 
	// 		    _y < fullImageExtents[3])
	// 	     	    insideComposited = true;

	// 	    if (insideComposited)
	// 	    {
	// 		if (composedData[indexComposited*4 + 3] == 0)
	// 		{
	// 		    // No data from rendering here!
	// 		    imgFinal[index*3 + 0] = 
	// 			opaqueImageData[index*3 + 0];
	// 		    imgFinal[index*3 + 1] = 
	// 			opaqueImageData[index*3 + 1];
	// 		    imgFinal[index*3 + 2] = 
	// 			opaqueImageData[index*3 + 2];
	// 		}
	// 		else
	// 		{
	// 		    if (opaqueImageZB[index] != 1)
	// 		    {
	// 			// Might need to do some blending
	// 			double worldCoordinates[3];
	// 			float _tempZ = opaqueImageZB[index] * 2 - 1;
	// 			unProject(_x, _y, _tempZ, worldCoordinates,
	// 				  screen[0], screen[1], Inversepvm);

	// 			if (checkInBounds(dbounds, worldCoordinates))
	// 			{
	// 			    // Completely inside bounding box
	// 			    float alpha =
	// 				composedData[indexComposited*4+3];
	// 			    float oneMinusAlpha = 
	// 				(1.0 - 
	// 				 composedData[indexComposited*4+3]);
	// 			    imgFinal[index*3 + 0] = 
	// 				std::min((((float)opaqueImageData[index*3 + 0]/255.0) 
	// 					  * oneMinusAlpha  
	// 					  + composedData[indexComposited*4 + 0]), 1.0) * 255;
	// 			    imgFinal[index*3 + 1] = 
	// 				std::min((((float)opaqueImageData[index*3 + 1]/255.0) 
	// 					  * oneMinusAlpha
	// 					  + composedData[indexComposited*4 + 1]), 1.0) * 255;
	// 			    imgFinal[index*3 + 2] = 
	// 				std::min((((float)opaqueImageData[index*3 + 2]/255.0) 
	// 					  * oneMinusAlpha 
	// 					  + composedData[indexComposited*4 + 2]), 1.0) * 255;
	// 			}
	// 			else
	// 			{
	// 			    // Intersect inside with bounding box
	// 			    double ray[3], tMin, tMax;
	// 			    computeRay( view.camera, worldCoordinates, ray);
	// 			    if ( intersect(dbounds, ray, view.camera, tMin, tMax) )
	// 			    {
	// 				double tIntersect = 
	// 				    std::min((worldCoordinates[0]-view.camera[0])/ray[0],
	// 					     std::min((worldCoordinates[1]-view.camera[1])/ray[1], 
	// 						      (worldCoordinates[2]-view.camera[2])/ray[2]));
	// 				if (tMin <= tIntersect)
	// 				{
	// 				    // volume infront
	// 				    float alpha = composedData[indexComposited*4+3];
	// 				    float oneMinusAlpha = (1.0 - composedData[indexComposited*4+3]);
	// 				    imgFinal[index*3 + 0] = 
	// 					std::min((((float)opaqueImageData[index*3 + 0]/255.0) * oneMinusAlpha + composedData[indexComposited*4 + 0]), 1.0) * 255;
	// 				    imgFinal[index*3 + 1] = 
	// 					std::min((((float)opaqueImageData[index*3 + 1]/255.0) * oneMinusAlpha + composedData[indexComposited*4 + 1]), 1.0) * 255;
	// 				    imgFinal[index*3 + 2] = 
	// 					std::min((((float)opaqueImageData[index*3 + 2]/255.0) * oneMinusAlpha + composedData[indexComposited*4 + 2]), 1.0) * 255;
	// 				}
	// 				else
	// 				{
	// 				    // box infront
	// 				    imgFinal[index*3 + 0] = opaqueImageData[index*3 + 0];
	// 				    imgFinal[index*3 + 1] = opaqueImageData[index*3 + 1];
	// 				    imgFinal[index*3 + 2] = opaqueImageData[index*3 + 2];
	// 				}
	// 			    }
	// 			    else
	// 			    {
	// 				imgFinal[index*3 + 0] = (composedData[indexComposited*4 + 0]) * 255;
	// 				imgFinal[index*3 + 1] = (composedData[indexComposited*4 + 1]) * 255;
	// 				imgFinal[index*3 + 2] = (composedData[indexComposited*4 + 2]) * 255;
	// 			    }
	// 			}
	// 		    }
	// 		    else
	// 		    {
	// 			// Inside bounding box but only background - Good
	// 			float alpha = composedData[indexComposited*4+3];
	// 			float oneMinusAlpha = (1.0 - composedData[indexComposited*4+3]);
	// 			imgFinal[index*3 + 0] = 
	// 			    std::min((((float)opaqueImageData[index*3 + 0]/255.0) * oneMinusAlpha + composedData[indexComposited*4 + 0]), 1.0) * 255;
	// 			imgFinal[index*3 + 1] = 
	// 			    std::min((((float)opaqueImageData[index*3 + 1]/255.0) * oneMinusAlpha + composedData[indexComposited*4 + 1]), 1.0) * 255;
	// 			imgFinal[index*3 + 2] =
	// 			    std::min((((float)opaqueImageData[index*3 + 2]/255.0) * oneMinusAlpha + composedData[indexComposited*4 + 2]), 1.0) * 255;
	// 		    }
	// 		}
	// 	    }
	// 	    else
	// 	    {
	// 		// Outside bounding box: Use the background - Good
	// 		imgFinal[index*3 + 0] = opaqueImageData[index*3 + 0];
	// 		imgFinal[index*3 + 1] = opaqueImageData[index*3 + 1];
	// 		imgFinal[index*3 + 2] = opaqueImageData[index*3 + 2];
	// 	    }
	// 	}
	//     }
	//     img->Delete();
	//     SetOutput(whole_image);

	//     if (composedData != NULL) {
	// 	delete [] composedData;
	//     }

	//     // clean up
	//     Inversepvm->Delete();
	//     pvm->Delete();

	//     // check time
	//     debug5 << "Final compositing done!" << std::endl;
	//     slivr::CheckMemoryHere
	// 	("avtRayTracer::Execute final compositing done");

        ////////////////////////////////////////////////////////////////////////
	
	std::cout << "[Single Thread] " 
		  << clock.GetDuration() 
		  << " seconds to finish" << std::endl;
    } 
    else
    {

    }

    MPI_Finalize();

    return 0;
}

