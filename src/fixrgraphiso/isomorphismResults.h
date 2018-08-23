#ifndef D__ISORESULTS__HH__
#define D__ISORESULTS__HH__

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <set>
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

    void dataNodeToProtobuf( DataNode * dNode, iso_protobuf::Iso * proto){
      iso_protobuf::Iso_DataNode * data_node = proto-> add_data_nodes();
      data_node -> set_id(dNode -> get_id() );
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

    void methodNodeToProtobuf( MethodNode * mNode, iso_protobuf::Iso* proto, std::set<long> & extraDataNodes){
      iso_protobuf::Iso_MethodNode * method_node = proto -> add_method_nodes();
      method_node -> set_id(mNode -> get_id() );
      method_node -> set_name(mNode -> get_name() );
      proto ->add_methodcallnames(mNode -> get_name() );
      // Iterate through the arguments and add types/id
      const std::vector<DataNode*> & args = mNode -> get_arguments();
      for (const DataNode * dd : args){
        method_node -> add_argumenttypes( dd -> get_data_type()  );
        method_node -> add_argumentids( dd -> get_id());
        //cout << dd -> get_id() << endl;
        extraDataNodes.insert( dd-> get_id());
        assert(dd -> get_type() == DATA_NODE);
      }
      // Add invokee/receiver types
      const DataNode* rcv = mNode -> get_receiver();
      if (rcv != NULL){
        std::cerr << "Debug: invokee type = "<< rcv -> get_name() << endl;
        method_node -> set_invokeetype(rcv -> get_data_type());
        method_node -> set_invokeeid (rcv -> get_id());
        extraDataNodes.insert( rcv -> get_id());
        //cout << rcv -> get_id() << endl;
        assert(rcv -> get_type() == DATA_NODE);
      } else {
        // std::cerr << "Debug: invokee type cleared"<< endl;
        method_node -> clear_invokeetype();
        method_node -> clear_invokeeid();
      }
      // Add assignee types
      const DataNode * assg = mNode -> get_assignee();
      if (assg != NULL){
        method_node -> set_assigneetype(assg -> get_data_type());
        method_node -> set_assigneeid(assg -> get_id());
        extraDataNodes.insert(assg-> get_id());
        //cout << assg -> get_id() <<endl;
        assert(assg -> get_type() == DATA_NODE);
      } else {
        method_node -> clear_assigneetype();
        method_node -> clear_assigneeid();
      }
      // Done!
    }

    iso_protobuf::Iso * toProtobuf(Acdfg * acdfg, bool a_or_b) {
      iso_protobuf::Iso * proto = new iso_protobuf::Iso();

      std::vector<iso>::iterator it;
      std::set<long> extraDataNodeIDs;
      std::set<long> insertedDataNodeIDs;
      std::set<long> insertedMethodNodeIDs;
      // add node maps
      for (it = isoNodes.begin(); it != isoNodes.end(); ++it) {
        iso_protobuf::Iso_MapNode * map_node = proto->add_map_node();
        map_node->set_id_1(it->a_id);
        map_node->set_id_2(it->b_id);
        map_node->set_weight(it->wt);

        Node * nd_a = a_or_b? (acdfg -> getNodeFromID(it -> a_id)) :  \
          (acdfg -> getNodeFromID(it -> b_id));
        switch (nd_a -> get_type() ){
        case DATA_NODE: {
          DataNode * dNode = toDataNode(nd_a);
          insertedDataNodeIDs.insert(dNode -> get_id());
          dataNodeToProtobuf(dNode, proto);
        }
          break;
        case METHOD_NODE:
          {
            MethodNode * mNode= toMethodNode(nd_a);
            methodNodeToProtobuf(mNode, proto, extraDataNodeIDs);
            insertedMethodNodeIDs.insert(mNode -> get_id());
          }
          break;

        default:
          std::cerr << "Warning: dump to protobuf -- node type of regular node not expected in an isomorphism " << std::endl;
          break;
        }

        //

      }
      // Now insert the data nodes that are referred to in the method nodes themselves.
      for (auto jt = extraDataNodeIDs.begin(); jt != extraDataNodeIDs.end() ; ++jt){
        node_id_t id = *jt;
        if ( insertedDataNodeIDs.find(id) == insertedDataNodeIDs.end()){
          Node * n = acdfg-> getNodeFromID(id);
          assert( n != NULL);
          //assert( n -> get_type() == DATA_NODE);
          DataNode * dNode = toDataNode(n);
          dataNodeToProtobuf(dNode, proto);
          // add edges from the extra node ids to everything else
          vector<long> outgoingEdges = acdfg -> getOutgoingEdgeIDs(id);
          for (long id_out: outgoingEdges){
            const Edge * e = acdfg -> getEdgeFromID(id_out);
            long id_tgt = e -> get_dst_id();
            if (insertedMethodNodeIDs.find(id_tgt) != insertedMethodNodeIDs.end()){
              iso_protobuf::Iso_Edge * edge = proto -> add_edges();
              edge -> set_id(e -> get_id() );
              edge -> set_from(e -> get_src_id() );
              edge -> set_to(e-> get_dst_id() );
            }
          }
        }
      }

      // add edge maps
      for (it = isoEdges.begin(); it != isoEdges.end(); ++it) {
        iso_protobuf::Iso_MapEdge * map_edge = proto->add_map_edge();
        map_edge->set_id_1(it->a_id);
        map_edge->set_id_2(it->b_id);
        map_edge->set_weight(it->wt);
        // Add edge from graph_a back to the iso
        const Edge * e = a_or_b? acdfg -> getEdgeFromID(it -> a_id) : \
          acdfg -> getEdgeFromID(it -> b_id);
        iso_protobuf::Iso_Edge * edge = proto -> add_edges();
        edge -> set_id(e -> get_id());
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

    int dumpProtobuf(std::string output_dir, Acdfg * acdfg, bool a_or_b) {
      // ensure we're using a compatible version of Protobuf
      GOOGLE_PROTOBUF_VERIFY_VERSION;

      int condition = 0;
      iso_protobuf::Iso * proto = toProtobuf(acdfg, a_or_b);

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
