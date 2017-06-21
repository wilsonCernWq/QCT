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

#ifndef IMG_METADATA_H
#define IMG_METADATA_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <limits>

// ****************************************************************************
//  Struct:  ImgMetaData
//
//  Purpose:
//    Holds information about patches but not the image 
//
//  Programmer:  
//  Creation:   
//
// ****************************************************************************
namespace slivr
{
    struct ImgMetaData
    {
	int procId;       // processor that produced the patch
	int patchNumber;  // id of the patch on that processor
	int destProcId;   // destination proc where this patch gets composited
	int inUse;        // whether the patch is composed locally or not
	int dims[2];      // height, width
	int screen_ll[2]; // (lower left)  position in the final image
	int screen_ur[2]; // (upper right)
	float avg_z;      // camera space depth of the patch (average)
	float eye_z;      // camera space z
	float clip_z;     // clip space z
    };
};

// ****************************************************************************
//  Struct:  ImgData
//
//  Purpose:
//    Holds the image data generated
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
namespace slivr 
{
    struct ImgData
    {
	// acts as a key
	int procId;      // processor that produced the patch
	int patchNumber; // id of the patch on that processor
	float *imagePatch = NULL; // the image data - RGBA
	bool operator==(const ImgData &a){
	    return (patchNumber == a.patchNumber);
	}
    };
}

// ****************************************************************************
//  Struct:  convexHull
//
//  Purpose:
//    Holds the image data generated
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
namespace slivr 
{
    struct ConvexHull
    {
	int numPatches;
	// [0] rows along x axis, [1] rows along y axis, [2] rows along z axis
	int arrangement[3];
	float extents[6];       // minX, maxX   minY, maxY   minZ, maxZ
	float cellDims[3];      // x, y, z
	float tolerance;  
	// amount of overlap that is considered ok
	// -- typically 2 cells for cell centered data
	// 0: no overlap  
	// 1: overlpa in Z 
	// 2: overlap in Y
	// 3: overlap in Z
	int Overlap(ConvexHull);
    };
};

// ****************************************************************************
//  Template:  
//
//  Purpose:
//    
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
namespace slivr
{

    template<typename T>
    T Clamp(T value, T vmin, T vmax)
    { return std::max(std::min(value, vmax), vmin); }

    template<typename T>
    T Clamp(T x)
    { return std::min(std::max(x, (T)0.0), (T)1.0); }

    inline void WriteArrayToPPM(std::string filename , float *image, int dimX, int dimY)
    {
	std::ofstream outputFile((filename + ".ppm").c_str(), 
				 std::ios::out | std::ios::binary);
	outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
	for (int y=0; y<dimY; ++y)
	{
	    for (int x=0; x<dimX; ++x)
	    {
		int index = (y * dimX + x)*4;
		char color[3];
		float alpha = image[index + 3];
		color[0] = std::max(std::min(image[index + 0]*alpha, 1.0f), 0.0f) * 255;  // red
		color[1] = std::max(std::min(image[index + 1]*alpha, 1.0f), 0.0f) * 255;  // green
		color[2] = std::max(std::min(image[index + 2]*alpha, 1.0f), 0.0f) * 255;  // blue
		outputFile.write(color,3);
	    }
	} 
	outputFile.close();
    }

};

#endif//IMG_METADATA_H