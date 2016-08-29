#ifndef D__ISORESULTS__HH__
#define D__ISORESULTS__HH__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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
    double totalWeight;
    double objValue;

  IsomorphismResults(std::string aName, std::string bName):graphA(aName), graphB(bName),totalWeight(0.0)
    {};

    void addIsoNodePair(long node_a_id, long node_b_id, double weight){
      iso istr;
      istr.a_id = node_a_id;
      istr.b_id = node_b_id;
      istr.wt = weight;
      isoNodes.push_back(istr);
      totalWeight += weight;
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
      for (it = isoNodes.begin(); it != isoNodes.end(); ++it) {
        iso_protobuf::Iso_MapNode * map_node = proto->add_map_node();
        map_node->set_id_1(it->a_id);
        map_node->set_id_2(it->b_id);
        map_node->set_weight(it->wt);
      }

      // add edge maps
      for (it = isoEdges.begin(); it != isoEdges.end(); ++it) {
        iso_protobuf::Iso_MapEdge * map_edge = proto->add_map_edge();
        map_edge->set_id_1(it->a_id);
        map_edge->set_id_2(it->b_id);
        map_edge->set_weight(it->wt);
      }

      proto->set_weight(totalWeight);
      proto->set_obj_value(objValue);
      proto->set_graph_1_id(graphA);
      proto->set_graph_2_id(graphB);

      return proto;
    }

    int dumpProtobuf(std::string output_dir) {
      // ensure we're using a compatible version of Protobuf
      GOOGLE_PROTOBUF_VERIFY_VERSION;

      int condition = 0;
      iso_protobuf::Iso * proto = toProtobuf();

      std::fstream output(output_dir.c_str(),
        std::ios::out | std::ios::trunc | std::ios::binary);

      if (! proto->SerializeToOstream(&output)) {
        std::cerr << "Failed to write isomorphism protobuf." << std::endl;
        condition = -1;
      }

      // clean up our garbage
      delete proto;
      google::protobuf::ShutdownProtobufLibrary();
      return condition;
    }


  };
}

#endif
