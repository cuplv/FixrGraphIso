#ifndef D__ISORESULTS__HH__
#define D__ISORESULTS__HH__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "fixrgraphiso/acdfg.h"
#include "fixrgraphiso/proto_iso.pb.h"

namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;
using std::cout;
using std::endl;
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
    int dataNodeMatchCount;
    int methodNodeMatchCount;
    int dataEdgeMatchCount;
    int controlEdgeMatchCount;
    double avgMatchWeight;
    double avgDataNodeInDegree;
    double avgDataNodeOutDegree;
    double avgMethodNodeInDegree;
    double avgMethodNodeOutDegree;
    
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

    iso_protobuf::Iso * toProtobuf(Acdfg * acdfg_a) {
      iso_protobuf::Iso * proto = new iso_protobuf::Iso();

      std::vector<iso>::iterator it;

      // add node maps
      for (it = isoNodes.begin(); it != isoNodes.end(); ++it) {
        iso_protobuf::Iso_MapNode * map_node = proto->add_map_node();
        map_node->set_id_1(it->a_id);
        map_node->set_id_2(it->b_id);
        map_node->set_weight(it->wt);

	Node * nd_a = acdfg_a -> getNodeFromID(it -> a_id);
	switch (nd_a -> get_type() ){
	case DATA_NODE: {
	  DataNode * dNode = toDataNode(nd_a);
	  iso_protobuf::Iso_DataNode * data_node = proto-> add_data_nodes();
	  data_node -> set_id(it -> a_id);
	  data_node -> set_name(dNode -> get_name());
	  data_node -> set_type(dNode -> get_data_type() );
	  proto -> add_alldatatypes(dNode -> get_data_type());
	  if (dNode -> isConstNode()){
	    data_node -> set_data_type(iso_protobuf::Iso_DataNode_DataType_DATA_CONST);
	  } else {
	    assert(dNode -> isVarNode());
	    data_node -> set_data_type(iso_protobuf::Iso_DataNode_DataType_DATA_VAR);
	  }
	}
	  break;
	case METHOD_NODE:
	  {
	    MethodNode * mNode= toMethodNode(nd_a);
	    iso_protobuf::Iso_MethodNode * method_node = proto -> add_method_nodes();
	    method_node -> set_id(it -> a_id);
	    method_node -> set_name(mNode -> get_name() );
	    proto ->add_methodcallnames(mNode -> get_name() );
	    // Iterate through the arguments and add types/id
	    const std::vector<DataNode*> & args = mNode -> get_arguments();
	    std::vector<DataNode*>::const_iterator iT;
	    for (iT = args.begin(); iT != args.end(); ++iT){
	      method_node -> add_argumenttypes((*iT) -> get_data_type()  );
	      method_node -> add_argumentids((*iT) -> get_id());
	    }
	    // Add invokee/receiver types
	    const DataNode* rcv = mNode -> get_receiver();
	    if (rcv){
	      //	      std::cerr << "Debug: invokee type = "<< rcv -> get_data_type() << endl;
	      method_node -> set_invokeetype(rcv -> get_data_type());
	      method_node -> set_invokeeid (rcv -> get_id());
	    } else {
	      // std::cerr << "Debug: invokee type cleared"<< endl;
	      method_node -> clear_invokeetype();
	      method_node -> clear_invokeeid();
	    }
	    
	    const DataNode * assg = mNode -> get_assignee();
	    if (assg){
	      method_node -> set_assigneetype(assg -> get_data_type());
	      method_node -> set_assigneeid(assg -> get_id());
	    } else {
	      method_node -> clear_assigneetype();
	      method_node -> clear_assigneeid();
	    }
	    // Done!
	  }
	  break;
	  
	default:
	  std::cerr << "Warning: dump to protobuf -- node type of regular node not expected in an isomorphism " << std::endl;
	  break;
	}
	
	//	
	
      }

      // add edge maps
      for (it = isoEdges.begin(); it != isoEdges.end(); ++it) {
        iso_protobuf::Iso_MapEdge * map_edge = proto->add_map_edge();
        map_edge->set_id_1(it->a_id);
        map_edge->set_id_2(it->b_id);
        map_edge->set_weight(it->wt);
	// Add edge from graph_a back to the iso
	const Edge * e = acdfg_a -> getEdgeFromID(it -> a_id);
	iso_protobuf::Iso_Edge * edge = proto -> add_edges();
	edge -> set_id(it -> a_id);
	edge -> set_from(e -> get_src_id() );
	edge -> set_to(e-> get_dst_id() );
      }

      proto->set_weight(totalWeight);
      proto->set_obj_value(objValue);
      proto->set_graph_1_id(graphA);
      proto->set_graph_2_id(graphB);
      proto->set_datanodematchcount(dataNodeMatchCount);
      proto->set_methodnodematchcount(methodNodeMatchCount);
      proto->set_dataedgematchcount(dataEdgeMatchCount);
      proto->set_controledgematchcount(controlEdgeMatchCount);
      proto -> set_averagematchweight(avgMatchWeight);
      proto -> set_averagedatanodeindegree(avgDataNodeInDegree);
      proto -> set_averagemethodnodeindegree(avgMethodNodeInDegree);
      proto -> set_averagedatanodeoutdegree(avgDataNodeOutDegree);
      proto -> set_averagemethodnodeoutdegree(avgMethodNodeOutDegree);

      // extract the isomorphism node and edge data from graph A
      
      
      
      return proto;
    }

    int dumpProtobuf(std::string output_dir, Acdfg * acdfg_a) {
      // ensure we're using a compatible version of Protobuf
      GOOGLE_PROTOBUF_VERIFY_VERSION;

      int condition = 0;
      iso_protobuf::Iso * proto = toProtobuf(acdfg_a);

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
