#include "CompositorTree.h"
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <mpi.h>
#ifdef __linux__ 
# include <unistd.h>
# include <limits.h>
#endif

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
    MPI_Alltoall(&info, sizeof(info), MPI_BYTE,
                 buffer.data(), sizeof(info), MPI_BYTE, 
                 MPI_COMM_WORLD);
    if (mpiRank == 0) {
      for (int i = 0; i < mpiSize; ++i) {
        std::cout << "rack " << buffer[i].rack << " "
                  << "chassis " << buffer[i].chassis << " "
                  << "node " << buffer[i].node << " "
                  << "depth " << buffer[i].depth << "\n"; 
      }
    }
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
        std::string(dir) + "/hostname." + std::to_string(mpiRank * 40) + ".txt";
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




    Compositor_Tree::Compositor_Tree( const uint32_t& width,
                                      const uint32_t& height,
                                      std::string tree_file)
        : W(width), H(height){
       // set MPIRank
       MPI_Comm_rank(MPI_COMM_WORLD, &MPIRank);
       MPI_Comm_size(MPI_COMM_WORLD, &MPISize);       
       InfoIndex = MPIRank;
       // Find send, receive from tree file
       TreeFile.open(tree_file);
       std::string line, csvItem;
       int lineNum = 0;
       if(TreeFile.is_open()){
           while(getline(TreeFile, line)){
               lineNum++;
               if(lineNum == InfoIndex){
                   std:: istringstream myline(line);
                   while(getline(myline, csvItem, ' ')){
                           std::cout << "csvItem = " << csvItem << std::endl;
                           } 
               }
           }

       }
       
       
        
    };

    //! function to get final results
    const void *Compositor_Tree::MapDepthBuffer() { return depth; };
    const void *Compositor_Tree::MapColorBuffer() { return rgba; };
    void Compositor_Tree::Unmap(const void *mappedMem) {};

    int Compositor_Tree::GetParRank () { return MPIRank; };

    //! clear (the specified channels of) this frame buffer
    void Compositor_Tree::Clear(const uint32_t channelFlags) {};

    //! status
    bool Compositor_Tree::IsValid() 
    {
    };

    //! begin frame
    void Compositor_Tree::BeginFrame() 
    {
    };
  
    //! end frame
    void Compositor_Tree::EndFrame() 
    {
        //check if the tile is RECEIVE
        if(RECEIVE == -1){//wait for another tile



        }else{//send tile to other node
            //send region
            MPI_Send(&region, 4, MPI_INT, SEND, MPI_ANY_TAG, MPI_COMM_WORLD);
            //send depth
            MPI_Send(&depth, 1, MPI_FLOAT, SEND, MPI_ANY_TAG, MPI_COMM_WORLD);
            //send rgba 
            MPI_Send(&rgba, 4*tileW*tileH, MPI_FLOAT, SEND, MPI_ANY_TAG, MPI_COMM_WORLD);

            MPI_Barrier(MPI_COMM_WORLD);
        }
    };

    //! upload tile
    void Compositor_Tree::SetTile(Tile &tile) 
    {
      GetMetaInfo(*(tile.z), MPIRank, MPISize);
      ExchangeInfo(MPIRank, MPISize);

        rgba = new float[4 * tile.tileDim[0] * tile.tileDim[1]];
        for(auto i = 0; i < tile.tileSize; ++i){
            rgba[4 * i + 0] = tile.r[i];
            rgba[4 * i + 1] = tile.g[i];
            rgba[4 * i + 2] = tile.b[i];
            rgba[4 * i + 3] = tile.a[i];
        }
        // set depth
        depth = tile.z;
        tileW = tile.tileDim[0];
        tileH = tile.tileDim[1];
        // get tile region
        /* x0 x1 y0 y1 */
        region[0] = (int)tile.region[0];
        region[1] = (int)tile.region[2];
        region[2] = (int)tile.region[1];
        region[3] = (int)tile.region[3];
    
        // for(auto i = e[0]; i < e[1]; ++i){
        //     for(auto j = e[2]; j < e[3]; ++j){
        //         rgba[4 * (i + j * W) + 0] = ptr[4 * ((i - e[0]) + (j - e[2]) * tileDim[1]) + 0];
        //         rgba[4 * (i + j * W) + 1] = ptr[4 * ((i - e[0]) + (j - e[2]) * tileDim[1]) + 1];
        //         rgba[4 * (i + j * W) + 2] = ptr[4 * ((i - e[0]) + (j - e[2]) * tileDim[1]) + 2];
        //         rgba[4 * (i + j * W) + 3] = ptr[4 * ((i - e[0]) + (j - e[2]) * tileDim[1]) + 3];
        //     }
        // }
    };

    void Compositor_Tree::SetSend(int send)
    {
        SEND = send;

    };

    void Compositor_Tree::SetReceive(int receive)
    {
        RECEIVE = receive;
    };

};
};
};
