#include "CompositorTree.h"

#include <mpi.h>

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#ifdef __linux__ 
# include <unistd.h>
# include <limits.h>
#endif

#if QCT_ALGO_TREE_USE_SOA
bool InTile(const QCT::Tile tile, const int x, const int y) 
{
  /* x0 x1 y0 y1 */
  return 
    (tile.region[0] <= x && tile.region[2] > x) && 
    (tile.region[1] <= y && tile.region[3] > y);
}
void Place(const QCT::Tile& tile,
           const uint32_t oSize[2],
           float*         oRGBA)
{
  for (auto i = tile.region[0]; i < tile.region[2]; ++i) {
    for (auto j = tile.region[1]; j < tile.region[3]; ++j) {
      const auto iIdx = 
        ((i - tile.region[0]) + (j - tile.region[1]) * tile.tileDim[0]);
      const auto oIdx = (i + j * oSize[0]) * 4;
      oRGBA[oIdx + 0] = tile.r[iIdx];
      oRGBA[oIdx + 1] = tile.g[iIdx];
      oRGBA[oIdx + 2] = tile.b[iIdx];
      oRGBA[oIdx + 3] = tile.a[iIdx];
    }
  }
}
void Blend(const QCT::Tile& fTile,
           const QCT::Tile& bTile,
           QCT::Tile&       oTile)
{
  for (auto i = oTile.region[0]; i < oTile.region[2]; ++i) {  /* x0 x1 */
    for (auto j = oTile.region[1]; j < oTile.region[3]; ++j) { /* y0 y1 */
      if (InTile(fTile, i, j) && InTile(bTile, i, j)) {
        const auto fIdx = 
          ((i-fTile.region[0]) + (j-fTile.region[1]) * fTile.tileDim[0]);
        const auto bIdx = 
          ((i-bTile.region[0]) + (j-bTile.region[1]) * bTile.tileDim[0]);
        const auto oIdx = 
          ((i-oTile.region[0]) + (j-oTile.region[1]) * oTile.tileDim[0]);
        const auto t = 1.f - fTile.a[fIdx];
        oTile.r[oIdx] =
          std::clamp(bTile.r[bIdx] * t + fTile.r[fIdx], 0.0f, 1.0f);
        oTile.g[oIdx] = 
          std::clamp(bTile.g[bIdx] * t + fTile.g[fIdx], 0.0f, 1.0f);
        oTile.b[oIdx] = 
          std::clamp(bTile.b[bIdx] * t + fTile.b[fIdx], 0.0f, 1.0f);
        oTile.a[oIdx] = 
          std::clamp(bTile.a[bIdx] * t + fTile.a[fIdx], 0.0f, 1.0f);
      } else if (InTile(fTile, i, j)) {
        const auto fIdx = 
          ((i-fTile.region[0]) + (j-fTile.region[1]) * fTile.tileDim[0]);
        const auto oIdx = 
          ((i-oTile.region[0]) + (j-oTile.region[1]) * oTile.tileDim[0]);
        oTile.r[oIdx] = fTile.r[fIdx];
        oTile.g[oIdx] = fTile.g[fIdx];
        oTile.b[oIdx] = fTile.b[fIdx];
        oTile.a[oIdx] = fTile.a[fIdx];
      } else if (InTile(bTile, i, j)) {
        const auto bIdx = 
          ((i-bTile.region[0]) + (j-bTile.region[1]) * bTile.tileDim[0]);
        const auto oIdx = 
          ((i-oTile.region[0]) + (j-oTile.region[1]) * oTile.tileDim[0]);
        oTile.r[oIdx] = bTile.r[bIdx];
        oTile.g[oIdx] = bTile.g[bIdx];
        oTile.b[oIdx] = bTile.b[bIdx];
        oTile.a[oIdx] = bTile.a[bIdx];
      } else {
        const auto oIdx = 
          ((i-oTile.region[0]) + (j-oTile.region[1]) * oTile.tileDim[0]);
        oTile.r[oIdx] = 0.f;
        oTile.g[oIdx] = 0.f;
        oTile.b[oIdx] = 0.f;
        oTile.a[oIdx] = 0.f;
      }
    }
  }
}
void CreatePPM(const float *r, const float *g, const float *b, 
               const int dimX, const int dimY, 
               const std::string& filename)
{
  std::ofstream outputFile(filename.c_str(), 
                           std::ios::out | std::ios::binary);
  outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n";    
  for (int y=0; y<dimY; ++y) {
    for (int x=0; x<dimX; ++x) {
      int index = (y * dimX + x);
      char color[3];
      color[0] = std::min(r[index] , 1.0f) * 255;  // red
      color[1] = std::min(g[index] , 1.0f) * 255;  // green 
      color[2] = std::min(b[index] , 1.0f) * 255;  // blue
      outputFile.write(color,3);
    }
  }    
  outputFile.close();
}
#else
bool InRegion(const int region[4], const int x, const int y) 
{
  /* x0 x1 y0 y1 */
  return 
    (region[0] <= x && region[1] > x) && 
    (region[2] <= y && region[3] > y);
}
void Place(const int    iSize[2],
           const int    iRegion[4],
           const float* iRGBA,
           const uint32_t oSize[2],
           float*         oRGBA)
{
  for (int i = iRegion[0]; i < iRegion[1]; ++i) {
    for (int j = iRegion[2]; j < iRegion[3]; ++j) {
      const int iIdx = ((i - iRegion[0]) + (j - iRegion[2]) * iSize[0]) * 4;
      const int oIdx = (i + j * oSize[0]) * 4;
      oRGBA[oIdx + 0] = iRGBA[iIdx + 0];
      oRGBA[oIdx + 1] = iRGBA[iIdx + 1];
      oRGBA[oIdx + 2] = iRGBA[iIdx + 2];
      oRGBA[oIdx + 3] = iRGBA[iIdx + 3];
    }
  }
}
void Blend(const int    fSize[2],
           const int    fRegion[4],
           const float* fRGBA,
           const int    bSize[2],
           const int    bRegion[4], 
           const float* bRGBA,
           const int    oSize[2],
           const int    oRegion[4],
           float*       oRGBA)
{
  for (int i = oRegion[0]; i < oRegion[1]; ++i) {
    for (int j = oRegion[2]; j < oRegion[3]; ++j) {
      if (InRegion(fRegion, i, j) && InRegion(bRegion, i, j)) {
        const int fIdx = ((i-fRegion[0]) + (j-fRegion[2]) * fSize[0]) * 4;
        const int bIdx = ((i-bRegion[0]) + (j-bRegion[2]) * bSize[0]) * 4;
        const int oIdx = ((i-oRegion[0]) + (j-oRegion[2]) * oSize[0]) * 4;
        const float t = 1.f - fRGBA[fIdx + 3];
        oRGBA[oIdx + 0] = std::clamp(bRGBA[bIdx+0] * t + fRGBA[fIdx+0],
                                     0.0f, 1.0f);
        oRGBA[oIdx + 1] = std::clamp(bRGBA[bIdx+1] * t + fRGBA[fIdx+1],
                                     0.0f, 1.0f);
        oRGBA[oIdx + 2] = std::clamp(bRGBA[bIdx+2] * t + fRGBA[fIdx+2],
                                     0.0f, 1.0f);
        oRGBA[oIdx + 3] = std::clamp(bRGBA[bIdx+3] * t + fRGBA[fIdx+3],
                                     0.0f, 1.0f);
      } else if (InRegion(fRegion, i, j)) {    
        const int fIdx = ((i-fRegion[0]) + (j-fRegion[2]) * fSize[0]) * 4;
        const int oIdx = ((i-oRegion[0]) + (j-oRegion[2]) * oSize[0]) * 4;
        oRGBA[oIdx + 0] = fRGBA[fIdx + 0];
        oRGBA[oIdx + 1] = fRGBA[fIdx + 1];
        oRGBA[oIdx + 2] = fRGBA[fIdx + 2];
        oRGBA[oIdx + 3] = fRGBA[fIdx + 3];
      } else if (InRegion(bRegion, i, j)) {
        const int bIdx = ((i-bRegion[0]) + (j-bRegion[2]) * bSize[0]) * 4;
        const int oIdx = ((i-oRegion[0]) + (j-oRegion[2]) * oSize[0]) * 4;
        oRGBA[oIdx + 0] = bRGBA[bIdx + 0];
        oRGBA[oIdx + 1] = bRGBA[bIdx + 1];
        oRGBA[oIdx + 2] = bRGBA[bIdx + 2];
        oRGBA[oIdx + 3] = bRGBA[bIdx + 3];
      } else {
        const int oIdx = ((i-oRegion[0]) + (j-oRegion[2]) * oSize[0]) * 4;
        oRGBA[oIdx + 0] = 0.f;
        oRGBA[oIdx + 1] = 0.f;
        oRGBA[oIdx + 2] = 0.f;
        oRGBA[oIdx + 3] = 0.f;
      }
    }
  }
}
void CreatePPM(const float * image, int dimX, int dimY, 
               const std::string& filename)
{
  std::ofstream outputFile(filename.c_str(), 
                           std::ios::out | std::ios::binary);
  outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n";    
  for (int y=0; y<dimY; ++y) {
    for (int x=0; x<dimX; ++x) {
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
#endif

// std::string filename;
#define TREE_FILE "./tree"

struct MetaInfo {
  uint32_t rack : 16, chassis : 8, node : 8;
  float depth;
};
static MetaInfo info;

namespace QCT {
namespace algorithms {
namespace tree {

  void Compositor_Tree::ExchangeInfo(const int mpiRank, const int mpiSize) 
  {
    std::vector<MetaInfo> buffer(mpiSize);
    MPI_Allgather(&info, sizeof(info), MPI_BYTE,
                  buffer.data(), sizeof(info), MPI_BYTE, 
                  MPI_COMM_WORLD);

    /* pass stuffs to python */
    std::string command;
    command += "python3 ../algorithms/tree/graph/optimize/create_tree.py ";
    command += " -o " + (TREE_FILE + std::to_string(mpiRank));
    command += " -i ";
    for (int i = 0; i < mpiSize; ++i) {
      command += 
        std::to_string(buffer[i].rack) + " " +
        std::to_string(buffer[i].chassis * 10 + buffer[i].node) + " " +
        std::to_string(buffer[i].depth) + " " ;
    }
    system(command.c_str());
  
    /* construct tree */  
    std::ifstream f(TREE_FILE + std::to_string(mpiRank) + ".txt");
    if (f.is_open() && f.good()) {
      int r, t, a;
      while (f >> r >> t >> a) { tree.AddNode(r, t, a); }
    }
    tree.Shrink();
  }

  void Compositor_Tree::GetMetaInfo(const float &z, const int mpiRank, const int mpiSize)
  {
    /** first we check if we are on stampede2 
     *   --> check if TACC_SYSTEM=stampede2 */
    bool onStampede2 = false;
    if (const char* tacc = std::getenv("TACC_SYSTEM")) {
      if (std::string(tacc) == "stampede2") {  onStampede2 = true; }
    }
    /* retrieve meta info  */
    std::string hname;
    if (onStampede2) {
      /* we need to read from hostname */
      char hostname[35]; gethostname(hostname, 35);
      hname = std::string(hostname);      
    } else {
      /* we need to read simulated hostnames from files */
      /* where are the files */
      const char* dir;
      if (!(dir = std::getenv("HOSTNAME_FILES"))) {
        throw std::runtime_error("please define environmental variable "
                                 "'HOSTNAME_FILES' indicating the directory "
                                 "of all the hostnames files");      
      }
      /* compute filename */
      std::string fname = 
        std::string(dir) + "/hostname." + std::to_string(mpiRank) + ".txt";
      /* okay we need to read all the files */
      std::ifstream infile(fname.c_str(), std::ifstream::in);
      if (!infile) {
        throw std::runtime_error(("unable to open file " + fname).c_str());
      }      
      if (!std::getline(infile, hname)) { // process the first line
        throw std::runtime_error(("empty file " + fname).c_str());
      }
    }
    /* construct meta info */ 
    info.rack = std::stoi(hname.substr(1,3));
    info.chassis = std::stoi(hname.substr(5,2));
    info.node = std::stoi(hname.substr(7,1));
    info.depth = z;    
  }

  Compositor_Tree::Compositor_Tree(const uint32_t& width,
                                   const uint32_t& height)
    : finalSize{width, height}
  {
    // set MPIRank and MPISize
    MPI_Comm_rank(MPI_COMM_WORLD, &MPIRank);
    MPI_Comm_size(MPI_COMM_WORLD, &MPISize);
    tree.Allocate(MPISize);
  };

  //! function to get final results
  const void *Compositor_Tree::MapDepthBuffer() { return finalDepth; };
  const void *Compositor_Tree::MapColorBuffer() { return finalRGBA; };
  void Compositor_Tree::Unmap(const void *mappedMem) {};

  //! clear (the specified channels of) this frame buffer
  void Compositor_Tree::Clear(const uint32_t channelFlags) {};

  //! status
  bool Compositor_Tree::IsValid() { return true; };

  //! begin frame
  void Compositor_Tree::BeginFrame() { /*do nothing by now*/ };
  
  //! end frame
  void Compositor_Tree::EndFrame() 
  {
    for (auto it = tree.GetBegin(MPIRank); it != tree.GetEnd(MPIRank); ++it) {
      int target = (*it).target;
      if (tree.IsSend(it)) { // send a tile to target
        // std::cout << "send from " << MPIRank << " to " << target << std::endl; 
        /* pass meta data */
#if QCT_ALGO_TREE_USE_SOA
        MPI_Send(&tile, sizeof(tile), MPI_BYTE,
                 target, 30000, MPI_COMM_WORLD);
        MPI_Send(tile.rgba, 4 * tile.tileSize, MPI_FLOAT,
                 target, 30001, MPI_COMM_WORLD);
        MPI_Send(tile.z, 1, MPI_FLOAT,
                 target, 30005, MPI_COMM_WORLD);
#else
        MPI_Send( tileSize,   2, MPI_INT, target, 10000, MPI_COMM_WORLD);
        MPI_Send( tileRegion, 4, MPI_INT, target, 10001, MPI_COMM_WORLD);
        MPI_Send(&tileDepth,  1, MPI_FLOAT, target, 10002, MPI_COMM_WORLD);
        MPI_Send( tileRGBA,   4 * tileSize[0] * tileSize[1], MPI_FLOAT, 
                  target, 10003, MPI_COMM_WORLD);
#endif
      } else { // receive a tile from target
        // std::cout << "recv at " << MPIRank << " from " << target << std::endl; 
#if QCT_ALGO_TREE_USE_SOA
        /* recv meda data */
        Tile recv;
        MPI_Recv(&recv, sizeof(recv), MPI_BYTE, 
                 target, 30000, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        recv.Allocate();
        MPI_Recv(recv.rgba, 4 * recv.tileSize, MPI_FLOAT, 
                 target, 30001, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        MPI_Recv(recv.z, 1, MPI_FLOAT, 
                 target, 30005, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        /* now we compose two tiles together */
        const std::array<uint32_t, 4> composedRegion  = {
          std::min(tile.region[0], recv.region[0]),
          std::min(tile.region[1], recv.region[1]),
          std::max(tile.region[2], recv.region[2]),
          std::max(tile.region[3], recv.region[3])
        };
        const std::array<uint32_t, 2> composedDim = {
          finalSize[0], finalSize[1]
        };
        Tile composed(composedRegion, composedDim, QCT_TILE_REDUCED_DEPTH);
        composed.Allocate();
        *(composed.z) = std::min(*(tile.z), *(recv.z));
        if (*(tile.z) > *(recv.z)) {
          Blend(recv, tile, composed);
        } else {
          Blend(tile, recv, composed);
        }
        recv.Clean();
        tile.Clean();
        tile = composed;
#else
        /* recv meda data */
        int    recvSize[2];
        int    recvRegion[4];
        float* recvRGBA;
        float  recvDepth;
        MPI_Recv( recvSize,   2, MPI_INT, target, 10000, 
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
        recvRGBA = new float[4 * recvSize[0] * recvSize[1]];
        MPI_Recv( recvRegion, 4, MPI_INT, target, 10001, 
                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&recvDepth,  1, MPI_FLOAT, target, 10002, 
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv( recvRGBA,   4 * recvSize[0] * recvSize[1], MPI_FLOAT,
                  target, 10003, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        /* now we compose two tiles together */
        const int newTileRegion[4] = {
          std::min(tileRegion[0], recvRegion[0]),
          std::max(tileRegion[1], recvRegion[1]),
          std::min(tileRegion[2], recvRegion[2]),
          std::max(tileRegion[3], recvRegion[3])
        };
        const int newTileSize[2] = { newTileRegion[1] - newTileRegion[0], 
                                     newTileRegion[3] - newTileRegion[2] };
        float *newTileRGBA = new float[4 * newTileSize[0] * newTileSize[1]];
        float  newTileDepth = std::min(tileDepth, recvDepth);
        if (tileDepth > recvDepth) {
          Blend(recvSize, recvRegion, recvRGBA,
                tileSize, tileRegion, tileRGBA,
                newTileSize, newTileRegion, newTileRGBA);
        } else {
          Blend(tileSize, tileRegion, tileRGBA,
                recvSize, recvRegion, recvRGBA,
                newTileSize, newTileRegion, newTileRGBA);
        }
        delete [] recvRGBA;
        delete [] tileRGBA;
        tileRGBA = newTileRGBA;          
        tileRegion[0] = newTileRegion[0];
        tileRegion[1] = newTileRegion[1];
        tileRegion[2] = newTileRegion[2];
        tileRegion[3] = newTileRegion[3];
        tileSize[0] = newTileSize[0];
        tileSize[1] = newTileSize[1];
        tileDepth = newTileDepth;
#endif
      }
    } // if (idx == MPIRank)

    /* we need to move the final image is on rank 0 */
    if (tree.IsRoot(MPIRank) && (MPIRank != 0)) {
      /* pass meta data */
#if QCT_ALGO_TREE_USE_SOA
      MPI_Send(&tile, sizeof(tile), MPI_BYTE,
               0, 20000, MPI_COMM_WORLD);
      MPI_Send(tile.rgba, 4 * tile.tileSize, MPI_FLOAT,
               0, 20001, MPI_COMM_WORLD);
#else
      MPI_Send( tileSize,   2, MPI_INT, 0, 20000, MPI_COMM_WORLD);
      MPI_Send( tileRegion, 4, MPI_INT, 0, 20001, MPI_COMM_WORLD);
      MPI_Send(&tileDepth,  1, MPI_FLOAT, 0, 20002, MPI_COMM_WORLD);
      MPI_Send( tileRGBA,   4 * tileSize[0] * tileSize[1], MPI_FLOAT, 
                0, 20003, MPI_COMM_WORLD);
#endif
    }        
    if (!(tree.IsRoot(MPIRank)) && (MPIRank == 0)) {
      /* recv meda data */
#if QCT_ALGO_TREE_USE_SOA
      tile.Clean();
      MPI_Recv(&tile, sizeof(tile), MPI_BYTE, 
               tree.GetRoot(), 20000, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      tile.Allocate();
      MPI_Recv(tile.rgba, 4 * tile.tileSize, MPI_FLOAT, 
               tree.GetRoot(), 20001, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
#else
      MPI_Recv( tileSize,   2, MPI_INT, 
                MPI_ANY_SOURCE /*tree.GetRoot()*/, 20000, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
      delete[] tileRGBA;
      tileRGBA = new float[4 * tileSize[0] * tileSize[1]];
      MPI_Recv( tileRegion, 4, MPI_INT, 
                MPI_ANY_SOURCE /*tree.GetRoot()*/, 20001, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&tileDepth,  1, MPI_FLOAT, 
                MPI_ANY_SOURCE /*tree.GetRoot()*/, 20002, 
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv( tileRGBA,   4 * tileSize[0] * tileSize[1], MPI_FLOAT,
                MPI_ANY_SOURCE /*tree.GetRoot()*/, 20003,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
#endif
    }
    /* final composition */
    if (MPIRank == 0) {
      finalRGBA = new float[4 * finalSize[0] * finalSize[1]];
#if QCT_ALGO_TREE_USE_SOA
      finalDepth = new float(*(tile.z));
      Place(tile, finalSize, finalRGBA);
#else
      finalDepth = new float(tileDepth);
      Place(tileSize, tileRegion, tileRGBA, finalSize, finalRGBA);
#endif
    }
  };

  //! upload tile
  void Compositor_Tree::SetTile(Tile &tile) 
  {
    // compute tree using python  
    GetMetaInfo(*(tile.z), MPIRank, MPISize);
    ExchangeInfo(MPIRank, MPISize);
    // retrieve data
#if QCT_ALGO_TREE_USE_SOA
    this->tile = tile;
#else
    tileDepth = (*tile.z);
    tileSize[0] = tile.tileDim[0];
    tileSize[1] = tile.tileDim[1];
    tileRGBA = new float[4 * tile.tileDim[0] * tile.tileDim[1]];   
    for(auto i = 0; i < tile.tileSize; ++i){
      tileRGBA[4 * i + 0] = tile.r[i] * tile.a[i];
      tileRGBA[4 * i + 1] = tile.g[i] * tile.a[i];
      tileRGBA[4 * i + 2] = tile.b[i] * tile.a[i];
      tileRGBA[4 * i + 3] = tile.a[i];
    }
    tileRegion[0] = (int)tile.region[0];
    tileRegion[1] = (int)tile.region[2];
    tileRegion[2] = (int)tile.region[1];
    tileRegion[3] = (int)tile.region[3];
#endif
  };

};
};
};
