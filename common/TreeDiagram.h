// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include <cstdint>
#include <stdexcept>

namespace hotT {

  /*! This is simple a linked list. ``nullptr'' represents a empty node
   */
  typedef uint32_t NodeRef;
  struct TreeDiagram {
  private:
    NodeRef rank; /* MPI rank */
    TreeDiagram* parent = nullptr;
    TreeDiagram* child[2] = { nullptr, nullptr };
  public:
    NodeRef GetRank() const { return rank; }
    void    SetRank(const NodeRef r) { rank = r; }

    TreeDiagram*       GetParent() { return parent; }
    const TreeDiagram* GetParent() const { return parent; }

    void SetChild(const int i, TreeDiagram* n) 
    {
#ifndef NDEBUG
      if (i < 0 || i > 1) { 
	throw std::runtime_error("wrong child node index, a tree diagram "
				 "can only have two nodes."); 
      }
#endif
      child[i] = n; 
    }
  };

};
