#include <iostream>
#include <vector>
#include <string>

#ifndef D__ISORESULTS__HH__
#define D__ISORESULTS__HH__

namespace fixrgraphiso {

  struct iso {
    long a_id; // ID for graph a
    long b_id; // ID for graph b
    double wt; // weight is only relevant for node pairs.
  };
  
  class IsomorphismResults{
  public:
    std::vector<iso> isoNodes; // Information about nodes
    std::vector<iso> isoEdges; // Information about edges
    std::string graphA;
    std::string graphB;

  IsomorphismResults(std::string aName, std::string bName):graphA(aName), graphB(bName)
    {};

    void addIsoNodePair(long node_a_id, long node_b_id, double weight){
      iso istr;
      istr.a_id = node_a_id;
      istr.b_id = node_b_id;
      istr.wt = weight;
      isoNodes.push_back(istr);
    }

    void addIsoEdgePair(long edge_a_id, long edge_b_id){
      iso istr;
      istr.a_id = edge_a_id;
      istr.b_id = edge_b_id;
      istr.wt = 0.0; // Weight is not important for edges
      isoEdges.push_back(istr);
    }

    void dumpProtobuf();
    
  };

}

#endif
