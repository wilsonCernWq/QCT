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

// #define CLAMP(x, l, h) (x > l ? x < h ? x : h : l)
// void CreatePPM(const float * image, int dimX, int dimY, 
//                const std::string& filename)
// {
//   std::ofstream outputFile(filename.c_str(), std::ios::out | std::ios::binary);
//   outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n";    
//   for (int y=0; y<dimY; ++y) {
//     for (int x=0; x<dimX; ++x) {
//       int index = (y * dimX + x) * 4;            
//       char color[3];
//       color[0] = std::min(image[index + 0] , 1.0f) * 255;  // red
//       color[1] = std::min(image[index + 1] , 1.0f) * 255;  // green 
//       color[2] = std::min(image[index + 2] , 1.0f) * 255;  // blue
//       outputFile.write(color,3);
//     }
//   }    
//   outputFile.close();
// }

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
        const int fIdx = ((i - fRegion[0]) + (j - fRegion[2]) * fSize[0]) * 4;
        const int bIdx = ((i - bRegion[0]) + (j - bRegion[2]) * bSize[0]) * 4;
        const int oIdx = ((i - oRegion[0]) + (j - oRegion[2]) * oSize[0]) * 4;
        const float t = 1.f - fRGBA[fIdx + 3];
        oRGBA[oIdx + 0] = std::clamp(bRGBA[bIdx + 0] * t + fRGBA[fIdx + 0], 0.0f, 1.0f);
        oRGBA[oIdx + 1] = std::clamp(bRGBA[bIdx + 1] * t + fRGBA[fIdx + 1], 0.0f, 1.0f);
        oRGBA[oIdx + 2] = std::clamp(bRGBA[bIdx + 2] * t + fRGBA[fIdx + 2], 0.0f, 1.0f);
        oRGBA[oIdx + 3] = std::clamp(bRGBA[bIdx + 3] * t + fRGBA[fIdx + 3], 0.0f, 1.0f);
      } else if (InRegion(fRegion, i, j)) {    
        const int fIdx = ((i - fRegion[0]) + (j - fRegion[2]) * fSize[0]) * 4;
        const int oIdx = ((i - oRegion[0]) + (j - oRegion[2]) * oSize[0]) * 4;
        oRGBA[oIdx + 0] = fRGBA[fIdx + 0];
        oRGBA[oIdx + 1] = fRGBA[fIdx + 1];
        oRGBA[oIdx + 2] = fRGBA[fIdx + 2];
        oRGBA[oIdx + 3] = fRGBA[fIdx + 3];
      } else if (InRegion(bRegion, i, j)) {
        const int bIdx = ((i - bRegion[0]) + (j - bRegion[2]) * bSize[0]) * 4;
        const int oIdx = ((i - oRegion[0]) + (j - oRegion[2]) * oSize[0]) * 4;
        oRGBA[oIdx + 0] = bRGBA[bIdx + 0];
        oRGBA[oIdx + 1] = bRGBA[bIdx + 1];
        oRGBA[oIdx + 2] = bRGBA[bIdx + 2];
        oRGBA[oIdx + 3] = bRGBA[bIdx + 3];
      } else {
        const int oIdx = ((i - oRegion[0]) + (j - oRegion[2]) * oSize[0]) * 4;
        oRGBA[oIdx + 0] = 0.f;
        oRGBA[oIdx + 1] = 0.f;
        oRGBA[oIdx + 2] = 0.f;
        oRGBA[oIdx + 3] = 0.f;
      }
    }
  }
}

#define TREE_FILE "./tree"

namespace QCT {
namespace algorithms {
namespace tree {

  struct MetaInfo {
    uint32_t rack : 16, chassis : 8, node : 8;
    float depth;
  };
  static MetaInfo info;
  
  void ExchangeInfo(const int mpiRank, const int mpiSize) {
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
  }

  void GetMetaInfo(const float &z, const int mpiRank, const int mpiSize)
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
  };

  //! function to get final results
  const void *Compositor_Tree::MapDepthBuffer() { return finalDepth; };
  const void *Compositor_Tree::MapColorBuffer() { return finalRGBA; };
  void Compositor_Tree::Unmap(const void *mappedMem) {};

  int Compositor_Tree::GetParRank () { return MPIRank; };

  //! clear (the specified channels of) this frame buffer
  void Compositor_Tree::Clear(const uint32_t channelFlags) {};

  //! status
  bool Compositor_Tree::IsValid() { return true; };

  //! begin frame
  void Compositor_Tree::BeginFrame() { /*do nothing by now*/ };
  
  //! end frame
  void Compositor_Tree::EndFrame() 
  {
    std::ifstream f(TREE_FILE + std::to_string(mpiRank) + ".txt");
    if (f.is_open() && f.good()) {
      // std::cout << "current rank = " << MPIRank << std::endl;

      int idx;
      while (f >> idx >> target >> action) {
      if (idx == MPIRank) {

        // std::cout << "rank " << MPIRank 
        //           << " action " << action 
        //           << " target " << target << std::endl;

        if (action == -1) { // send tile to other node
          // std::cout << "SEND == from " << MPIRank << " to " << target << std::endl;

          // // DEBUG
          // std::cout << "SEND tile size   = " 
          //           << tileSize[0] << " " << tileSize[1] << std::endl;
          // std::cout << "SEND tile region = " 
          //           << tileRegion[0] << " " 
          //           << tileRegion[1] << " " 
          //           << tileRegion[2] << " " 
          //           << tileRegion[3] << std::endl;
          // std::cout << "SEND tile depth = "  << tileDepth << " " << std::endl;

          // send data
          MPI_Send( tileSize,   2, MPI_INT, target, 10000, MPI_COMM_WORLD);
          MPI_Send( tileRegion, 4, MPI_INT, target, 10001, MPI_COMM_WORLD);
          MPI_Send(&tileDepth,  1, MPI_FLOAT, target, 10002, MPI_COMM_WORLD);
          MPI_Send( tileRGBA,   4 * tileSize[0] * tileSize[1], MPI_FLOAT, 
                    target, 10003, MPI_COMM_WORLD);

        } else if (action != -1) { // receive a tile from target
          // std::cout << "RECV == at " << MPIRank 
          //           << " receive from " << target << std::endl;
          // std::string filename;

          // receive data
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

          // // DEBUG
          // std::cout << "RECV tile size   = " 
          //           << recvSize[0] << " " << recvSize[1] << std::endl;
          // std::cout << "RECV tile region = " 
          //           << recvRegion[0] << " " 
          //           << recvRegion[1] << " "
          //           << recvRegion[2] << " "
          //           << recvRegion[3] << std::endl;
          // std::cout << "RECV tile depth = " << recvDepth <<  std::endl;
          // std::cout << "RECV OWNS tile size = "
          //           << tileSize[0] << " " << tileSize[1] << std::endl;
          // std::cout << "RECV OWNS tile region = " 
          //           << tileRegion[0] << " "
          //           << tileRegion[1] << " "
          //           << tileRegion[2] << " " 
          //           << tileRegion[3] << std::endl;
          // std::cout << "RECV OWNS tile depth = " << tileDepth <<  std::endl;
          // filename = "./recv_from_" + 
          //   std::to_string(target) + "_to_" + 
          //   std::to_string(MPIRank) + ".ppm";
          // CreatePPM(recvRGBA, recvSize[0], recvSize[1], filename); 

          /* now we compose two tiles together */
          const int newTileRegion[4] = {
            std::min(tileRegion[0], recvRegion[0]),
            std::max(tileRegion[1], recvRegion[1]),
            std::min(tileRegion[2], recvRegion[2]),
            std::max(tileRegion[3], recvRegion[3])
          };
          // std::cout << "RECV new tile region = "
          //           << newTileRegion[0] << " " 
          //           << newTileRegion[1] << " " 
          //           << newTileRegion[2] << " " 
          //           << newTileRegion[3] << std::endl;
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

          // std::cout << "RECV done compose" << std::endl;
          // filename = "./compose_from_" + 
          //   std::to_string(target) + "_to_" + 
          //   std::to_string(MPIRank) + ".ppm";
          // CreatePPM(newTileRGBA, newTileSize[0], newTileSize[1], filename); 
          
          // move data to new 
          // TODO optimization
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
        }
      } // if (idx == MPIRank)
            
      if (action == -2) { // this is the root node
        /* we need to move the final image is on rank 0 */
        if ((idx == MPIRank) && (MPIRank != 0)) {
          MPI_Send( tileSize,   2, MPI_INT, 0, 20000, MPI_COMM_WORLD);
          MPI_Send( tileRegion, 4, MPI_INT, 0, 20001, MPI_COMM_WORLD);
          MPI_Send(&tileDepth,  1, MPI_FLOAT, 0, 20002, MPI_COMM_WORLD);
          MPI_Send( tileRGBA,   4 * tileSize[0] * tileSize[1], MPI_FLOAT, 
                    0, 20003, MPI_COMM_WORLD);
        }        
        if ((idx != MPIRank) && (MPIRank == 0)) {
          MPI_Recv( tileSize,   2, MPI_INT, idx, 20000, 
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
          delete[] tileRGBA;
          tileRGBA = new float[4 * tileSize[0] * tileSize[1]];
          MPI_Recv( tileRegion, 4, MPI_INT, idx, 20001, 
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv(&tileDepth,  1, MPI_FLOAT, idx, 20002, 
                   MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Recv( tileRGBA,   4 * tileSize[0] * tileSize[1], MPI_FLOAT,
                    idx, 20003, MPI_COMM_WORLD, MPI_STATUS_IGNORE);          
        }
        /* final composition */
        if (MPIRank == 0) {
          finalRGBA = new float[4 * finalSize[0] * finalSize[1]];
          finalDepth = new float(tileDepth);
          Place(tileSize, tileRegion, tileRGBA, finalSize, finalRGBA);
        }
      }

      } // end of while
    }
  };

  //! upload tile
  void Compositor_Tree::SetTile(Tile &tile) 
  {
    // compute tree using python  
    GetMetaInfo(*(tile.z), MPIRank, MPISize);
    ExchangeInfo(MPIRank, MPISize);

    // retrieve data
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
    
    // // DEBUG:: save tile to images
    // std::string output = "./setTile" + std::to_string(MPIRank) + ".ppm";
    // CreatePPM(tileRGBA, tileSize[0], tileSize[1], output);
  };

};
};
};
