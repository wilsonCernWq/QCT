// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include <cstdint>
#include <stdexcept>

namespace WarmT {

  /*! This is simple a linked list. ``nullptr'' represents a empty node
   */
  struct TreeDiagram_Link {
  private:
    size_t rank; /* MPI rank */
    TreeDiagram_Link* parent = nullptr;
    TreeDiagram_Link* child[2] = { nullptr, nullptr };
  public:
    size_t GetRank() const { return rank; }
    void   SetRank(const size_t r) { rank = r; }

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
  public:
    struct Node {
      int32_t target; /* send/recv target */
      int32_t index;
      bool IsSend() const { return index == -1; }
      bool IsRecv() const { return !IsSend();   }    
      bool IsRoot() const { return target == -1; }
    };
    typedef uint32_t NodeRef;
  private:
    Node    *list = nullptr;
    NodeRef nlist = 0;
    void CheckIndex(const NodeRef i) const {
#ifndef NDEBUG
      if (i < NodeRef(0) || i >= nlist)
	throw std::runtime_error("TreeDiagram index out of bounds"); 
#endif
    }
  public:
    TreeDiagram_Array(const NodeRef n) : nlist(n) {
      list = new Node[nlist];
    }
    virtual ~TreeDiagram_Array() {
      if (list != nullptr) { delete[] list; }
    }

    const Node* Data() const { return list; }
    Node*       Data() { return list; }

    void Init() {
      for (auto i = 0; i < nlist; ++i) {
	list[i].target = -1;
	list[i].index  = -1;
      }
    }

    Node&       GetParent(const NodeRef i)       { CheckIndex(i); return list[i]; }
    const Node& GetParent(const NodeRef i) const { CheckIndex(i); return list[i]; }

    bool IsRoot(const NodeRef i) const { 
      CheckIndex(i);
      return i == (nlist - 1);
    }

    bool MoveRef(NodeRef& i) const { 
      CheckIndex(i);
      if (list[i].IsSend()) { return false; }
      else { i = static_cast<NodeRef>(list[i].index); return true; }
    }

    /*! building the graph */
    void SetNode(const NodeRef i, const int32_t target, const int32_t index) {
      CheckIndex(i);
      list[i].target = target;
      list[i].index = index;
    }

  };

  /*! which data structure to use */
  using TreeDiagram = TreeDiagram_Array;

};
