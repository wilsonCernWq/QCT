#include "CompositorTree.h"
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#define CLAMP(x, l, h) (x > l ? x < h ? x : h : l)
#include <fstream>
#include <mpi.h>
#ifdef __linux__ 
# include <unistd.h>
# include <limits.h>
#endif

namespace QCT {
namespace algorithms {
namespace tree {

    //inline void GetLineInfo(std::string file, int index, int &send, int &receive){

     //// Find send, receive from tree file
       //file = file + ".csv";
       //std::ifstream f(file);
       //std::string line, csvItem;
       //int lineNum = 0;
       //int Rank;
       //std::vector<int> temp_content;
       //if(f.is_open()){
           //while(f >> Rank >> send >> receive){
               //if(Rank == mpi_rank){
               //}
           //}
           ////while(getline(f, line)){
               ////lineNum++;
               ////if(lineNum == InfoIndex){
                   ////std:: istringstream myline(line);
                   ////while(getline(myline, csvItem, ',')){
                       //////std::cout << "csvItem = " << csvItem << std::endl;        
                       ////temp_content.push_back(std::stoi(csvItem));
                   ////} 
               ////}
           ////}
       //}
    //}   

  struct MetaInfo {
    uint32_t rack : 16, chassis : 8, node : 8;
    float depth;
  };
  static MetaInfo info;
  
  void ExchangeInfo(const int mpiRank, const int mpiSize, std::string tree_output) {
    std::vector<MetaInfo> buffer(mpiSize);
    MPI_Alltoall(&info, sizeof(info), MPI_BYTE,
                 buffer.data(), sizeof(info), MPI_BYTE, 
                 MPI_COMM_WORLD);
    std::string command = "python3 ../algorithms/tree/graph/optimize/create_tree.py -o ";
    command = command + tree_output + " -i ";

    /* pass stuffs to python */
    if (mpiRank == 0) {
      for (int i = 0; i < mpiSize; ++i) {
          command = command + std::to_string(buffer[i].rack) + " "
                            + std::to_string(buffer[i].chassis) + " "
                            + std::to_string(buffer[i].depth) + " " ;
                        
          
        // std::cout << "rack " << buffer[i].rack << " "
        //           << "chassis " << buffer[i].chassis << " "
        //           << "node " << buffer[i].node << " "
        //           << "depth " << buffer[i].depth << "\n"; 
        std::cout << "XXX " << buffer[i].rack << " "
                  << "YYY " << buffer[i].chassis * 10 + buffer[i].node << " "
                  << "depth " << buffer[i].depth << "\n"; 
      }
    }
    std::cout << "debug command = " << std::endl;
    std::cout << command << std::endl;
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
        : W(width), H(height), TreeFile(tree_file){
       // set MPIRank
       MPI_Comm_rank(MPI_COMM_WORLD, &MPIRank);
       MPI_Comm_size(MPI_COMM_WORLD, &MPISize);
       //TreeFile = TreeFile + ".csv"; 
       //InfoIndex = MPIRank + 1;
       //// Find send, receive from tree file
       //TreeFile = TreeFile + ".csv";
       //std::ifstream f(TreeFile);
       //std::string line, csvItem;
       //int lineNum = 0;
       //std::vector<int> temp_content;
       //if(f.is_open()){
           //while(getline(f, line)){
               //lineNum++;
               //if(lineNum == InfoIndex){
                   //std:: istringstream myline(line);
                   //while(getline(myline, csvItem, ',')){
                       ////std::cout << "csvItem = " << csvItem << std::endl;        
                       //temp_content.push_back(std::stoi(csvItem));
                   //} 
               //}
           //}
       //}
        
       //RECEIVE = temp_content[0];
       //SEND = temp_content[1];
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
        return true;
    };

    //! begin frame
    void Compositor_Tree::BeginFrame() 
    {
    };
  
    //! end frame
    void Compositor_Tree::EndFrame() 
    {
        TreeFile = TreeFile + ".csv";
        std::ifstream f(TreeFile);
        std::cout << f.good() << std::endl;
        int Rank;
        if(f.is_open()){
            while(f >> Rank >> SEND >> RECEIVE){
                if(Rank == MPIRank){
                    //check if the tile is RECEIVE
                    if(RECEIVE != -1){
                        int tile_size[2];
                        float* received_tile;
                        float tile_depth;
                        int tile_region[4];
                        // receive tile size
                        MPI_Recv(tile_size, 2, MPI_INT, RECEIVE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);        
                        //receive tiles from other nodes
                        received_tile = new float(4 * tile_size[0] * tile_size[1]);
                        MPI_Recv(&received_tile, 4*tile_size[0]*tile_size[1], MPI_FLOAT, RECEIVE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        MPI_Recv(&tile_depth, 1, MPI_FLOAT, RECEIVE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        MPI_Recv(tile_region, 4, MPI_INT, RECEIVE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        //!Compose
                        // compute new bounding box
                        int bbox[4];
                        bbox[0] = std::min(tile_region[0], region[0]);
                        bbox[1] = std::max(tile_region[1], region[1]);
                        bbox[2] = std::min(tile_region[2], region[2]);
                        bbox[3] = std::max(tile_region[3], region[3]);
                        // new tile size
                        int out_tilesize[2];
                        out_tilesize[0] = bbox[1] - bbox[0];
                        out_tilesize[1] = bbox[3] - bbox[2];
                        // new composed image buffer
                        output = new float(4 * out_tilesize[0] * out_tilesize[1]);
                        // blend area
                        int blend_area[4];
                        blend_area[0] = std::max(tile_region[0], region[0]);
                        blend_area[1] = std::max(tile_region[1], region[1]);
                        blend_area[2] = std::max(tile_region[2], region[2]);
                        blend_area[3] = std::max(tile_region[3], region[3]);

                        for(auto i = bbox[0]; i <bbox[1]; ++i){
                            for(auto j = bbox[2]; j < bbox[3]; ++j){
                                if(i >= blend_area[0] && i <= blend_area[1] && j >= blend_area[2] && j < blend_area[3]){
                                    // this pixel should be compose back to front
                                    int srcIndex, dstIndex;
                                    if(tile_depth > *depth){
                                        // new tile is on the back
                                        srcIndex = ((i - tile_region[0]) + (j - tile_region[2]) * tile_size[0]) * 4;
                                        dstIndex = ((i - region[0]) + (j - region[2]) * tileW) * 4;
                                        float trans = 1.0f - received_tile[srcIndex + 3];
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 0] = CLAMP(rgba[dstIndex + 0] * trans + received_tile[srcIndex + 0], 0.0f, 1.0f);
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 1] = CLAMP(rgba[dstIndex + 1] * trans + received_tile[srcIndex + 1], 0.0f, 1.0f);
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 2] = CLAMP(rgba[dstIndex + 2] * trans + received_tile[srcIndex + 2], 0.0f, 1.0f);
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 3] = CLAMP(rgba[dstIndex + 3] * trans + received_tile[srcIndex + 3], 0.0f, 1.0f);
                                    }else{
                                        // new tile is on the front  
                                        srcIndex = ((i - region[0]) + (j - region[2]) * tileW) * 4;
                                        dstIndex = ((i - tile_region[0]) + (j - tile_region[2]) * tile_region[0]) * 4;
                                        float trans = 1.0f - rgba[srcIndex + 3];
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 0] = CLAMP(received_tile[dstIndex + 0] * trans + rgba[srcIndex + 0], 0.0f, 1.0f);
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 1] = CLAMP(received_tile[dstIndex + 1] * trans + rgba[srcIndex + 1], 0.0f, 1.0f);
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 2] = CLAMP(received_tile[dstIndex + 2] * trans + rgba[srcIndex + 2], 0.0f, 1.0f);
                                        output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 3] = CLAMP(received_tile[dstIndex + 3] * trans + rgba[srcIndex + 3], 0.0f, 1.0f);
                                    }
                                
                                }else if(i >= tile_region[0] && i <= tile_region[1] && j >= tile_region[2] && j <= tile_region[3]){
                                    int srcIndex = ((i - tile_region[0]) + (j - tile_region[2]) * tile_size[0]) * 4;
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 0] = received_tile[srcIndex + 0]; 
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 1] = received_tile[srcIndex + 1]; 
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 2] = received_tile[srcIndex + 2]; 
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 3] = received_tile[srcIndex + 3]; 
                                }else if(i >= region[0] && i <= region[1] && j >= region[2] && j <= region[3]){
                                    int srcIndex = ((i - region[0]) + (j - region[2]) * tileW) * 4;
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 0] = rgba[srcIndex + 0]; 
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 1] = rgba[srcIndex + 1]; 
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 2] = rgba[srcIndex + 2]; 
                                    output[4 * ((i - bbox[0]) + (j - bbox[2]) * out_tilesize[0]) + 3] = rgba[srcIndex + 3]; 
                                }
                            }
                        }
                        // delete pointes and set new composed tile to rgba
                        rgba = nullptr;
                        delete received_tile;
                        rgba = output;
                        output = nullptr; 

            }else{//send tile to other node
                //send region
                MPI_Send(&region, 4, MPI_INT, SEND, MPI_ANY_TAG, MPI_COMM_WORLD);
                //send depth
                MPI_Send(&depth, 1, MPI_FLOAT, SEND, MPI_ANY_TAG, MPI_COMM_WORLD);
                //send rgba 
                MPI_Send(&rgba, 4*tileW*tileH, MPI_FLOAT, SEND, MPI_ANY_TAG, MPI_COMM_WORLD);
            }
                }
            }
        }

    };

    //! upload tile
    void Compositor_Tree::SetTile(Tile &tile) 
    {
      GetMetaInfo(*(tile.z), MPIRank, MPISize);
      ExchangeInfo(MPIRank, MPISize, TreeFile);

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
