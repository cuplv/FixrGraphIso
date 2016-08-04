// -*- C++ -*-
//
// Code used to serialize (read/write) objects
//
// Author: Sergio Mover
//


#include <iostream>
#include <fstream>
#include <map>
#include <typeinfo>
#include "fixrgraphiso/serialization.h"

namespace fixrgraphiso {

typedef std::map<long, Node*> idMapType;
typedef idMapType::iterator idMapIter;

Node* lookup_node(idMapType& idToNodeMap, long id);

template <typename T> void addEdge(Acdfg* acdfg,
                                   idMapType& idToNodeMap,
                                   T proto_edge);

Acdfg* AcdfgSerializer::create_acdfg(acdfg_protobuf::Acdfg* proto_acdfg)
{
  Acdfg* acdfg = new Acdfg();
  idMapType idToNodeMap;


  /* Read all the nodes
     TODO: to be improved (e.g. with templates), now we are replicating a
     lot of code.
   */

  /* data ndoes */
  for (int j = 0; j < proto_acdfg->data_node_size(); j++) {
    const acdfg_protobuf::Acdfg_DataNode& proto_node =
      proto_acdfg->data_node(j);

    DataNode * node = new DataNode(proto_node.id(),
                         proto_node.name(),
                         proto_node.type());
    Node * app_node = acdfg->add_node(node);
    idToNodeMap[app_node->get_id()] = app_node;
  }

  /* Misc Nodes */
  for (int j = 0; j < proto_acdfg->misc_node_size(); j++) {
    const acdfg_protobuf::Acdfg_MiscNode& proto_node =
      proto_acdfg->misc_node(j);
    
    Node * node = new Node(proto_node.id(), REGULAR_NODE);
    Node *app_node = acdfg->add_node(node);
    idToNodeMap[app_node->get_id()] = app_node;
  }

  /* Method Nodes */
  for (int j = 0; j < proto_acdfg->method_node_size(); j++) {
    const acdfg_protobuf::Acdfg_MethodNode& proto_node =
      proto_acdfg->method_node(j);

    /* read to the receiver */
    DataNode* receiver = (DataNode*) lookup_node(idToNodeMap,
                                                 proto_node.invokee());
    // assert(NULL != receiver);

    /* read the arguments */
    std::vector<DataNode*> arguments;
    for (int k = 0; k < proto_node.argument_size(); k++) {
      const int argument_id = proto_node.argument(k);
      DataNode* n = (DataNode*) lookup_node(idToNodeMap,
                                            argument_id);
      arguments.push_back(n);
    }
    
    MethodNode * node = new MethodNode(proto_node.id(),
				       proto_node.name(),
				       receiver,
				       arguments);
    Node *app_node = acdfg->add_node(node);
    idToNodeMap[app_node->get_id()] = app_node;
  }


  /* Read the edges */

  /* Def edges */
  for (int j = 0; j < proto_acdfg->def_edge_size(); j++) {
    const acdfg_protobuf::Acdfg_DefEdge& proto_edge =
      proto_acdfg->def_edge(j);

    addEdge(acdfg, idToNodeMap, proto_edge);
  }

  for (int j = 0; j < proto_acdfg->use_edge_size(); j++) {
    const acdfg_protobuf::Acdfg_UseEdge& proto_edge =
      proto_acdfg->use_edge(j);
    addEdge(acdfg, idToNodeMap,proto_edge);
  }

  for (int j = 0; j < proto_acdfg->control_edge_size(); j++) {
    const acdfg_protobuf::Acdfg_ControlEdge& proto_edge =
      proto_acdfg->control_edge(j);
    addEdge(acdfg, idToNodeMap,proto_edge);
  }

  for (int j = 0; j < proto_acdfg->trans_edge_size(); j++) {
    const acdfg_protobuf::Acdfg_TransEdge& proto_edge =
      proto_acdfg->trans_edge(j);
    addEdge(acdfg, idToNodeMap,proto_edge);
  }
  
  
  return acdfg;
}

acdfg_protobuf::Acdfg* AcdfgSerializer::read_protobuf_acdfg(const char* file_name)
{
  /* Read all the nodes */
  acdfg_protobuf::Acdfg* acdfg = new acdfg_protobuf::Acdfg();

  /* data nodes */
  std::fstream input_stream (file_name, std::ios::in | std::ios::binary);
  if (input_stream.is_open()) {
    if (acdfg->ParseFromIstream(&input_stream)) {
      return acdfg;
    }
    else {
      return NULL;
    }
  }
  else {
    return NULL;
  }
}

Node* lookup_node(idMapType& idToNodeMap, long id)
{
  typedef std::map<long, Node*> idMapType;
  typedef idMapType::iterator idMapIter;

  idMapIter nodeIter = idToNodeMap.find(id);
  if (idToNodeMap.end() == nodeIter) {
    return NULL;
  }
  else {
    return nodeIter->second;
  }
}


  
  template<typename T> void addEdge(Acdfg* acdfg,
				    idMapType& idToNodeMap,
				    T proto_edge)
  {
    Node* from = lookup_node(idToNodeMap, proto_edge.from());
    Node* to = lookup_node(idToNodeMap, proto_edge.to());
    
    assert(NULL != from);
    assert(NULL != to);
    
    if (typeid(T) == typeid(acdfg_protobuf::Acdfg_ControlEdge)){
      ControlEdge * e = new ControlEdge(proto_edge.id(), from, to);
      acdfg->add_edge(e);
    } else if (typeid(T) == typeid(acdfg_protobuf::Acdfg_UseEdge)){
      UseEdge * e = new UseEdge(proto_edge.id(), from, to);
      acdfg->add_edge(e);
    } else if (typeid(T) == typeid(acdfg_protobuf::Acdfg_DefEdge)){
      DefEdge * e = new DefEdge(proto_edge.id(), from, to);
      acdfg->add_edge(e);
    } else if (typeid(T) == typeid(acdfg_protobuf::Acdfg_TransEdge)) {
      TransitiveEdge * e = new TransitiveEdge(proto_edge.id(), from, to);
      acdfg->add_edge(e);
    } else {
      assert(false); // This should not happen
    }

  

  

  }

  

} // namespace fixrgraphiso
