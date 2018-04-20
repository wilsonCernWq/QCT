// ======================================================================== //
// Copyright SCI Institute, University of Utah, 2018
// ======================================================================== //

#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace QCT {
namespace algorithms {
namespace tree {

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
      enum { SEND, RECV } type;
      bool IsRecv() const { return !IsSend();   }    
      bool IsSend() const { return type == SEND; }
      void SetSend() { type = SEND; }
      void SetRecv() { type = RECV; }
    };
    typedef uint32_t RankRef;
    typedef std::vector<Node>::const_iterator RankIter;
  private:
    void CheckIndex(const RankRef i) const {
#ifndef NDEBUG
      if (i < RankRef(0) || i >= list.size())
	throw std::runtime_error("TreeDiagram index " + std::to_string(i) + " out of bounds"); 
#endif
    }
  private:
    std::vector<std::vector<Node>> list;
    RankRef rootRank;
  public:
    TreeDiagram_Array() = default;
    virtual ~TreeDiagram_Array() = default;

    void Allocate(const RankRef n) { list.resize(n); };
    RankRef GetRoot() const { return rootRank; }
    bool IsRoot(const RankRef i) const { return rootRank == i; }

    const RankIter GetBegin(const RankRef i) const { CheckIndex(i); return list[i].begin(); }
    const RankIter GetEnd  (const RankRef i) const { CheckIndex(i); return list[i].end(); }
    bool IsSend(const RankIter it) const { return (*it).IsSend(); }
    bool IsRecv(const RankIter it) const { return (*it).IsRecv(); }

    void Shrink() {
      list.shrink_to_fit();
      for (auto& l : list) { l.shrink_to_fit(); }
    }

    /*! building the graph */
    void AddNode(const RankRef rank, const int32_t target, const int32_t action) {
      CheckIndex(rank); list[rank].emplace_back();
      list[rank].back().target = target;
      if (action != -1) { list[rank].back().SetRecv(); }
      else { list[rank].back().SetSend(); }
      if (action == -2) { rootRank = rank; }
    }

  };

  /*! which data structure to use */
  using TreeDiagram = TreeDiagram_Array;

};
};
};
