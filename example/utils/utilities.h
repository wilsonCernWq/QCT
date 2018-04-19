#ifndef _UTILITIES_H_
#define _UTILITIES_H_


#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <float.h>
#include <math.h>


#include "color.h"
#include "timer.h"

inline void CreatePPM(const float * image, int dimX, int dimY, 
		      const std::string& filename)
{
    std::ofstream outputFile(filename.c_str(), std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n";    
    for (int y=0; y<dimY; ++y){
        for (int x=0; x<dimX; ++x){
            int index = (y * dimX + x) * 4;            
            char color[3];
            color[0] = std::min(image[index + 0] , 1.0f) * 255;  // red
            color[1] = std::min(image[index + 1] , 1.0f) * 255;  // green 
            color[2] = std::min(image[index + 2] , 1.0f) * 255;  // blue
            outputFile.write(color,3);
        }
    }    
    outputFile.close();
}

inline void CreateColorPPM(const Color *data, int width, int height, 
			   const std::string& filename)
{
    std::ofstream outputFile(filename.c_str(), std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << width << "\n" << height << "\n" << 255 << "\n";   
    for (int y=0; y<height; ++y){
        for (int x=0; x<width; ++x){
            int index = (y * width) + x;            
            char color[3];
            color[0] = std::min(data[index].r * data[index].a, 1.0f) * 255;  // red
            color[1] = std::min(data[index].g * data[index].a, 1.0f) * 255;  // green 
            color[2] = std::min(data[index].b * data[index].a, 1.0f) * 255;  // blue
            outputFile.write(color,3);
        }
    }    
    outputFile.close();
}

inline bool CompareColor(float currentColor[4], float r, float g, float b, float a)
{
    float epsilon = FLT_MIN;
    if (((fabs(currentColor[0] - a) < epsilon)  && 
	 (fabs(currentColor[1] - b) < epsilon)) && 
	((fabs(currentColor[2] - g) < epsilon)  && 
	 (fabs(currentColor[3] - r) < epsilon)))
        return true;
    else
        return false;
}

#endif//_UTILITIES_H_
