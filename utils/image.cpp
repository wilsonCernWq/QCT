#include "image.h"

///////////////////////////////////////////////////////////////////////////////
// implementation
///////////////////////////////////////////////////////////////////////////////
void Image::CreateImage() {
#ifdef __INTEL_COMPILER
    data = (float *)_mm_malloc((extents[1]-extents[0])*
			       (extents[3]-extents[2])*4*
			       sizeof(float), ALIGN);
#else
    data = new float[(extents[1]-extents[0]) * (extents[3]-extents[2]) * 4];
#endif
}

void Image::CreateOpaqueImage() {
#ifdef __INTEL_COMPILER
    data = (float *)_mm_malloc((extents[1]-extents[0])*
			       (extents[3]-extents[2])*3* 
			       sizeof(float), ALIGN);
#else
    data = new float[(extents[1]-extents[0]) * (extents[3]-extents[2]) * 3];
#endif
}

void Image::CreateImage(int minX, int maxX, int minY, int maxY) {
    extents[0] = minX; extents[1] = maxX;
    extents[2] = minY; extents[3] = maxY;
    CreateImage();
}

void Image::CreateOpaqueImage(int minX, int maxX, int minY, int maxY) {
    extents[0] = minX; extents[1] = maxX;
    extents[2] = minY; extents[3] = maxY;
    CreateOpaqueImage();
}

void Image::CreateImage
(int dimsX, int dimsY, int minX, int maxX, int minY, int maxY)
{
    extents[0] = 0; extents[1] = dimsX;
    extents[2] = 0; extents[3] = dimsY;
    bbox[0] = minX; bbox[1] = maxX;
    bbox[2] = minY; bbox[3] = maxY;   
    CreateImage();
}

void Image::DeleteImage() {
#ifdef __INTEL_COMPILER
    if (data != NULL){
	_mm_free(data);
	data = NULL;
    }
#else
    if (data != NULL){
	delete []data;
	data = NULL;
    }
#endif
}

void Image::InitZero() {
    int dims = (extents[3]-extents[2]) * (extents[1]-extents[0]);
    #pragma omp parallel for simd
    for (int i = 0; i < dims * 4; i++) { // estimated potential speedup: 8.370
        data[i] = 0;
    }
}

    void Image::SetColor
	(int x, int y,  float r, float g, float b, float a) 
    {
    int index = 
	(y-extents[2]) * (extents[1]-extents[0]) * 4 + (x-extents[0]) * 4;
    data[index+0] = r; 
    data[index+1] = g;
    data[index+2] = b;
    data[index+3] = a;
}

    void Image::GetColor(int x, int y, float colorAt[4])
    {
    int index = 
	(y-extents[2]) * (extents[1]-extents[0]) * 4 + (x-extents[0])*4;
    colorAt[0] = data[index+0]; 
    colorAt[1] = data[index+1];
    colorAt[2] = data[index+2];
    colorAt[3] = data[index+3];
}

    void Image::ColorImage(Color col){
    int i;
    int step = 4; // r,g,b,a = 4
    int sizeImg = (extents[1]-extents[0]) * (extents[3]-extents[2]);
    float colorData[4] = {col.r, col.g, col.b, col.a};
    float *tempImg = data;

    bbox[0] = extents[0];
    bbox[1] = extents[1];
    bbox[2] = extents[2];
    bbox[3] = extents[3];

    #pragma omp parallel for simd shared(col) aligned(tempImg:ALIGN) 
    for (i=0; i<sizeImg*step; i+=step){
        tempImg[i+0] = col.r;
        tempImg[i+1] = col.g;
        tempImg[i+2] = col.b;
        tempImg[i+3] = col.a;
    }
}

void Image::ColorImage
(Color col, int minX, int maxX, int minY, int maxY)
{
    int widthImg = extents[1]-extents[0];
    int widthOffset = minX-extents[0];

    int index;
    int step = 4;   // r,g,b,a = 4

    bbox[0] = minX;
    bbox[1] = maxX;
    bbox[2] = minY;
    bbox[3] = maxY;

    float *tempImg = data;
    #pragma omp parallel for simd shared(col) aligned(tempImg:ALIGN) 
    for (int y=minY; y<maxY; y++) {
	for (int x=minX; x<maxX; x++) {
	    index = y*widthImg*step + x*step;
	    tempImg[index+0] = col.r;
	    tempImg[index+1] = col.g;
	    tempImg[index+2] = col.b;
	    tempImg[index+3] = col.a;
	}
    }
}

void Image::ColorImage
    (float *cdata, int minX, int maxX, int minY, int maxY) 
{
    int widthSrc = maxX - minX;
    int widthDst = extents[1]-extents[0]; 

    int indexDst, indexSrc;
    int step = 4;   // r,g,b,a = 4

    bbox[0] = minX;
    bbox[1] = maxX;
    bbox[2] = minY;
    bbox[3] = maxY;

    #pragma omp parallel for simd 
    for (int y=minY; y<maxY; y++) {
        for (int x=minX; x<maxX; x++) {
            indexDst = (y-extents[2])*widthDst*step + (x-extents[0])*step;
            indexSrc = (y-minY)      *widthSrc*step + (x-minX)      *step;
            data[indexDst+0] = cdata[indexSrc+0];
            data[indexDst+1] = cdata[indexSrc+1];
            data[indexDst+2] = cdata[indexSrc+2];
            data[indexDst+3] = cdata[indexSrc+3];
        }
    }
}

void Image::BlendWithBackground(Color backgroundColor)
{
    int dims = (extents[3]-extents[2]) * (extents[1]-extents[0]);
    
    // estimated potential speedup: 0.470
    #pragma omp parallel for simd 
    for (int indexDst=0; indexDst<dims*4; indexDst+=4)
    {
	float alpha = (1.0 - data[indexDst+3]);
	data[indexDst+0] = backgroundColor.r * alpha +  data[indexDst+0];
	data[indexDst+1] = backgroundColor.g * alpha +  data[indexDst+1];
	data[indexDst+2] = backgroundColor.b * alpha +  data[indexDst+2];
	data[indexDst+3] = backgroundColor.a * alpha +  data[indexDst+3];
    }
}

// estimated potential speedup: 0.900 
void Image::PlaceInImage
    (float *cdata, int oriExtents[4], 
	int srcMinX, int srcMaxX, int srcMinY, int srcMaxY)
{
    int widthSrc = srcMaxX - srcMinX;
    int widthDst = extents[1] - extents[0]; 

    int indexDst, indexSrc;
    int step = 4;   // r,g,b,a = 4

    // estimated potential speedup: 0.920
    #pragma omp parallel for simd 
    for (int y=srcMinY; y<srcMaxY; y++) {
        for (int x=srcMinX; x<srcMaxX; x++) {
            indexDst = ((y-extents[2]   ) * widthDst + x-extents[0]   ) * step;
            indexSrc = ((y-oriExtents[2]) * widthSrc + x-oriExtents[0]) * step;
            data[indexDst+0] = cdata[indexSrc+0];
            data[indexDst+1] = cdata[indexSrc+1];
            data[indexDst+2] = cdata[indexSrc+2];
            data[indexDst+3] = cdata[indexSrc+3];
        }
    }
}

void Image::PlaceInOpaqueImage
(float *cdata, int oriExtents[4], 
 int srcMinX, int srcMaxX, int srcMinY, int srcMaxY)
{
    int widthSrc = srcMaxX   - srcMinX;
    int widthDst = extents[1] - extents[0]; 
    int indexDst, indexSrc;
    int step = 3;   // r,g,b,a = 4

    // estimated potential speedup: 0.920
    #pragma omp parallel for simd 
    for (int y=srcMinY; y<srcMaxY; y++) {      
	for (int x=srcMinX; x<srcMaxX; x++) {
	    indexDst = ((y-extents[2]   ) * widthDst + x-extents[0]   ) * step;
	    indexSrc = ((y-oriExtents[2]) * widthSrc + x-oriExtents[0]) * step;
	    data[indexDst+0] = cdata[indexSrc+0];
	    data[indexDst+1] = cdata[indexSrc+1];
	    data[indexDst+2] = cdata[indexSrc+2];
	}
    }
}

void Image::SetDims(int x, int y) {
    extents[0] = 0; extents[1] = x;
    extents[2] = 0; extents[3] = y;
}

void Image::SetExtents(int value, int pos) {
    extents[pos] = value;
}

void Image::SetExtents(int minX, int maxX, int minY, int maxY) {
    extents[0] = minX; extents[1] = maxX;
    extents[2] = minY; extents[3] = maxY;
}

void Image::SetBBox(int value, int pos) {
    bbox[pos] = value;
}

void Image::SetBBox(int minX, int maxX, int minY, int maxY) {
    bbox[0] = minX; bbox[1] = maxX;
    bbox[2] = minY; bbox[3] = maxY;
}

int Image::GetWidth() {
    return (extents[1]-extents[0]);
}

int Image::GetHeight() {
    return (extents[3]-extents[2]);
}

int Image::GetExtents(int pos) {
    return extents[pos];
}

void Image::GetExtents(int extents[4]) {
    extents[0] = extents[0];
    extents[1] = extents[1];
    extents[2] = extents[2];
    extents[3] = extents[3];
}

int Image::GetBBox(int pos) {
    return bbox[pos];
}

void Image::GetBBox(int bb[4]) {
    bb[0] = bbox[0];
    bb[1] = bbox[1];
    bb[2] = bbox[2];
    bb[3] = bbox[3];
}

void Image::UpdateBBox(int imageExtents[4]){
    if (imageExtents[0] < bbox[0])
        bbox[0] = imageExtents[0];

    if (imageExtents[2] < bbox[2])
        bbox[2] = imageExtents[2];

    if (imageExtents[1] > bbox[1])
        bbox[1] = imageExtents[1];

    if (imageExtents[3] > bbox[3])
        bbox[3] = imageExtents[3];
}

void Image::OutputPPM(std::string filename){
    std::cout << filename << std::endl;
    if (GetHeight() == 0 || GetWidth() == 0){
	std::cout << "Image " 
		  << filename
		  << " has dimensions 0" << std::endl;
	return;
    }
    std::ofstream outputFile(filename.c_str(), 
			     std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" 
	       << GetWidth() << "\n" 
	       << GetHeight() << "\n" 
	       << 255 << "\n";
    
    for (int y=0; y<GetHeight(); ++y) {
	for (int x=0; x<GetWidth(); ++x) {
	    int index = (y*GetWidth()*4) + x*4;            
	    char color[3];          
	    color[0] = std::min(data[index+0] , 1.0f) * 255;  // red
	    color[1] = std::min(data[index+1] , 1.0f) * 255;  // green 
	    color[2] = std::min(data[index+2] , 1.0f) * 255;  // blue
	    outputFile.write(color,3); 
	}
    }    
    outputFile.close();
}

void Image::OutputOpaquePPM(std::string filename) {
    if (GetHeight() == 0 || GetWidth() == 0){
	std::cout << "Image " 
		  << filename 
		  << " has dimensions 0" << std::endl;
	return;
    }

    std::ofstream outputFile(filename.c_str(), 
			     std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" 
	       << GetWidth() << "\n" 
	       << GetHeight() << "\n" 
	       << 255 << "\n";
    
    for (int y=0; y<GetHeight(); ++y) {
	for (int x=0; x<GetWidth(); ++x) {
	    int index = (y*GetWidth()*3) + x*3;            
	    char color[3];        
	    color[0] = std::min(data[index+0] , 1.0f) * 255;  // red
	    color[1] = std::min(data[index+1] , 1.0f) * 255;  // green 
	    color[2] = std::min(data[index+2] , 1.0f) * 255;  // blue
	    outputFile.write(color,3);
	}
    }    
    outputFile.close();
}
