#ifndef D__ISORESULTS__HH__
#define D__ISORESULTS__HH__

#include <iostream>
#include <vector>
#include <string>
#include "fixrgraphiso/acdfg.h"
#include "fixrgraphiso/proto_iso.pb.h"

namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;

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

    iso_protobuf::Iso * toProtobuf() {
      iso_protobuf::Iso * proto = new iso_protobuf::Iso();

      std::vector<iso>::iterator it;

      // add node maps
      for (it = isoNodes.begin(); it < isoNodes.end(); ++it) {
	iso_protobuf::Iso_MapNode * map_node = proto->add_map_node();
	map_node->set_id_1(it->a_id);
	map_node->set_id_2(it->b_id);
	map_node->set_weight(it->wt);
      }

      // add edge maps
      for (it = isoEdges.begin(); it < isoEdges.end(); ++it) {
	iso_protobuf::Iso_MapEdge * map_edge = proto->add_map_edge();
	map_node->set_id_1(it->a_id);
	map_node->set_id_2(it->b_id);
      }
      
      proto->set_graph_1_id(graphA);
      proto->set_graph_2_id(graphB);

      return proto;
    }

    void dumpProtobuf();
  };
}

#endif
