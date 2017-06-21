#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <cstddef>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <malloc.h>

#include "color.h"
#include "utilities.h"

class Image{    
private:
    float *data;
    float depth;
    int bbox[4];    // the bounding box in the image
    int extents[4]; // minX, maxX, minY, maxY
public:
    Image() : extents{-1}, bbox{-1}, depth{0.0f}, data{NULL} {}
    ~Image() = default;
    
    void CreateImage();
    void CreateImage(int, int, int, int);
    void CreateImage(int, int, int, int, int, int);

    void CreateOpaqueImage();
    void CreateOpaqueImage(int, int, int, int);

    void DeleteImage();
    
    void InitZero();
    void ColorImage(Color);
    void ColorImage(Color, int, int, int, int);
    void ColorImage(float*, int, int, int, int);

    void PlaceInImage(float*, int extents[4], int, int, int, int);
    void PlaceInOpaqueImage(float*, int originalExtents[4], int, int, int, int);
    void BlendWithBackground(Color);
    
    void SetDepth(float d) { depth = d; }
    void SetDims(int, int);
    void SetExtents(int, int);
    void SetExtents(int, int, int, int);
    void SetBBox(int, int);
    void SetBBox(int, int, int, int);
    void SetColor(int, int, float, float, float, float);
    void UpdateBBox(int imageExtents[4]);
    
    float  GetDepth() { return depth; }
    float* GetData() { return data; }
    int GetWidth();
    int GetHeight();
    int GetExtents(int);
    void GetExtents(int extents[4]);

    int GetBBox(int);
    void GetBBox(int bb[4]);
    void GetColor(int, int, float colorAt[4]);

    void OutputPPM(std::string filename);
    void OutputOpaquePPM(std::string filename);
};

#endif
