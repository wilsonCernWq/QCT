// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include <cstdint>
#include <stdexcept>

namespace WarmT {

  /*! This is simple a linked list. ``nullptr'' represents a empty node
   */
  typedef uint32_t NodeRef;
  struct TreeDiagram_Link {
  private:
    NodeRef rank; /* MPI rank */
    TreeDiagram_Link* parent = nullptr;
    TreeDiagram_Link* child[2] = { nullptr, nullptr };
  public:
    NodeRef GetRank() const { return rank; }
    void    SetRank(const NodeRef r) { rank = r; }

    TreeDiagram_Link*       GetParent() { return parent; }
    const TreeDiagram_Link* GetParent() const { return parent; }

    void SetChild(const int i, TreeDiagram_Link* n) 
    {
#ifndef NDEBUG
      if (i < 0 || i > 1)
	throw std::runtime_error("wrong child node index, a tree diagram "
				 "can only have two nodes.");
#endif
      child[i] = n; 
    }
  };

  /*! This is an array implementation of TreeDiagram,
   *  where the array has <number-of-nodes> elements,
   *  with each array value indicating the corresponding
   *  node parent. If the array value equals to the node
   *  index, this node should be a root node.
   */
  struct TreeDiagram_Array {
  private:
    NodeRef *list = nullptr;
    NodeRef nlist = 0;
  public:
    TreeDiagram_Array(const NodeRef n) : nlist(n) {
      list = new NodeRef[nlist];
    }
    virtual ~TreeDiagram_Array() {
      if (list != nullptr) { delete[] list; }
    }

    const NodeRef* Data() const { return list; }
    NodeRef*       Data() { return list; }

    void Init() {
      for (auto i = 0; i < nlist; ++i) list[i] = i;
    }

    bool IsRoot(const NodeRef i) {
#ifndef NDEBUG
      if (i < NodeRef(0) || i >= nlist)
	throw std::runtime_error("TreeDiagram index out of bounds"); 
#endif
      return (list[i] == i);
    }

    void SetParent(const NodeRef i, const NodeRef p) {
#ifndef NDEBUG
      if (i < NodeRef(0) || i >= nlist)
	throw std::runtime_error("TreeDiagram index out of bounds"); 
#endif
      list[i] = p;
    }

    NodeRef GetParent(const NodeRef i) {
#ifndef NDEBUG
      if (i < NodeRef(0) || i >= nlist)
	throw std::runtime_error("TreeDiagram index out of bounds"); 
#endif
      return list[i];
    }
  };

  /*! which data structure to use */
  using TreeDiagram = TreeDiagram_Array;

};
