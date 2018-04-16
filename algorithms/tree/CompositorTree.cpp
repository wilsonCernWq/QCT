#include "CompositorTree.h"
#include <sstream>

namespace QCT {
namespace algorithms {
namespace tree {

    Compositor_Tree::Compositor_Tree( const uint32_t& width,
                                      const uint32_t& height,
                                      std::string tree_file)
        : W(width), H(height){
       // set MPIRank
       MPI_Comm_rank(MPI_COMM_WORLD, MPIRank);
       InfoIndex = *MPIRank;
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

    int Compositor_Tree::GetParRank () { return *MPIRank; };

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
