/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "avtSLIVRImgMetaData.h"
#include <fstream>

// ***************************************************************************
// Threaded Blending
// ***************************************************************************

inline bool CheckThreadedBlend_MetaData()
{
    bool use = true;
    const char* env_use = std::getenv("SLIVR_NOT_USE_THREADED_BLEND");
    if (env_use) { 
	use = atoi(env_use) <= 0; 
    }
    if (!use) {
	std::cout << "[avtSLIVRImgMetaData] "
		  << "Not Using Multi-Threading for Blending"
		  << std::endl;
    } else {
	std::cout << "[avtSLIVRImgMetaData] "
		  << "Using Multi-Threading for Blending"
		  << std::endl;
    }
    return use;
}
#ifdef VISIT_OSPRAY
bool UseThreadedBlend_MetaData = CheckThreadedBlend_MetaData();
#else
bool UseThreadedBlend_MetaData = false;
#endif

// ****************************************************************************
//  Namespace:  slivr
//
//  Purpose:
//    
//
//  Programmer:  
//  Creation:   
//
// ****************************************************************************
void
slivr::CompositeBackground(int screen[2],
			 int compositedImageExtents[4],
			 int compositedImageWidth,
			 int compositedImageHeight,
			 float *compositedImageBuffer,
			 unsigned char *opaqueImageColor,
			 float         *opaqueImageDepth,
			 unsigned char *&imgFinal)
{
#ifdef VISIT_OSPRAY
    if (UseThreadedBlend_MetaData) {
    visit::CompositeBackground(screen,
			       compositedImageExtents,
			       compositedImageWidth,
			       compositedImageHeight,
			       compositedImageBuffer,
			       opaqueImageColor,
			       opaqueImageDepth,
			       imgFinal);
    } else {
#endif
    for (int y = 0; y < screen[1]; y++)
    {
	for (int x = 0; x < screen[0]; x++)
	{
	    int indexScreen     = y * screen[0] + x;
	    int indexComposited =
		(y - compositedImageExtents[2]) * compositedImageWidth +
		(x - compositedImageExtents[0]);

	    bool insideComposited = 
		((x >= compositedImageExtents[0] && 
		  x < compositedImageExtents[1]) &&
		 (y >= compositedImageExtents[2] && 
		  y < compositedImageExtents[3]));

	    if (insideComposited)
	    {
		if (compositedImageBuffer[indexComposited*4 + 3] == 0)
		{
		    // No data from rendering here! - Good
		    imgFinal[indexScreen * 3 + 0] = 
			opaqueImageColor[indexScreen * 3 + 0];
		    imgFinal[indexScreen * 3 + 1] = 
			opaqueImageColor[indexScreen * 3 + 1];
		    imgFinal[indexScreen * 3 + 2] = 
			opaqueImageColor[indexScreen * 3 + 2];
		}
		else
		{
		    // Volume in front
		    float alpha = 
			(1.0 - compositedImageBuffer[indexComposited * 4 + 3]);
		    imgFinal[indexScreen * 3 + 0] = 
			CLAMP(opaqueImageColor[indexScreen * 3 + 0] * alpha +
			      compositedImageBuffer[indexComposited * 4 + 0] *
			      255.f,
			      0.f, 255.f);
		    imgFinal[indexScreen * 3 + 1] = 
			CLAMP(opaqueImageColor[indexScreen * 3 + 1] * alpha +
			      compositedImageBuffer[indexComposited * 4 + 1] *
			      255.f,
			      0.f, 255.f);
		    imgFinal[indexScreen * 3 + 2] =
			CLAMP(opaqueImageColor[indexScreen * 3 + 2] * alpha +
			      compositedImageBuffer[indexComposited * 4 + 2] *
			      255.f,
			      0.f, 255.f);
		}
	    }
	    else
	    {
		// Outside bounding box: Use the background : Good
		imgFinal[indexScreen * 3 + 0] = 
		    opaqueImageColor[indexScreen * 3 + 0];
		imgFinal[indexScreen * 3 + 1] =
		    opaqueImageColor[indexScreen * 3 + 1];
		imgFinal[indexScreen * 3 + 2] =
		    opaqueImageColor[indexScreen * 3 + 2];
	    }
	}
    }
#ifdef VISIT_OSPRAY
    }
#endif
}

// ****************************************************************************
//  Function:  
//
//  Purpose:
//    
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
void WriteArrayToPPM(std::string filename, float * image, int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
			     std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        for (int x=0; x<dimX; ++x)
        {
            int index = (y * dimX + x)*4;
            char color[3];
            float alpha = image[index + 3];
            color[0] = CLAMP(image[index + 0]*alpha, 0.0f, 1.0f) * 255;
            color[1] = CLAMP(image[index + 1]*alpha, 0.0f, 1.0f) * 255;
            color[2] = CLAMP(image[index + 2]*alpha, 0.0f, 1.0f) * 255;
            outputFile.write(color,3);
        }
    } 
    outputFile.close();
}

void WriteArrayToPPM(std::string filename, 
		     unsigned char *image, 
		     int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
			     std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
	outputFile.write(reinterpret_cast<char*>(&image[y * dimX * 3]), 
			 dimX * 3);
    } 
    outputFile.close();
}

void WriteArrayGrayToPPM(std::string filename, 
			 float* image, 
			 int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
			     std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        for (int x=0; x<dimX; ++x)
        {
            int index = (y * dimX + x);
	    char var = CLAMP(image[index], 0.f, 1.f) * 255;
            char color[3];
            color[0] = var;
            color[1] = var;
            color[2] = var;
            outputFile.write(color,3);
        }
    } 
    outputFile.close();
}
