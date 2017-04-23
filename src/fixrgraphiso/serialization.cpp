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

   /*--
    Function: addEdge

    The specialization of the addEdge Function for edges of type ExceptionalControlEdge

    --*/
  template<class T> void addEdge (Acdfg* acdfg,
                  idMapType& idToNodeMap,
                  T  & proto_edge);

  template <> void addEdge (Acdfg* acdfg,
                idMapType& idToNodeMap,
                acdfg_protobuf::Acdfg_ExceptionalControlEdge const &  proto_edge ){
    Node* from = lookup_node(idToNodeMap, proto_edge.from());
    Node* to = lookup_node(idToNodeMap, proto_edge.to());
    assert(NULL != from);
    assert(NULL != to);
    ExceptionalEdge * e = new ExceptionalEdge(proto_edge.id(), from, to);
    int eSize = proto_edge.exceptions_size();
    for (int eID = 0; eID < eSize; ++eID){
      std::string const & except_str = proto_edge.exceptions(eID);
      e -> addException(except_str);
    }
    acdfg -> add_edge(e);
  }

  /*--
    Function: addEdge

    adds the edge to an ACDFG from the protobuf edge.
    This handles control edge, use edge, def edge but not
    exceptional control edge. A specialization is defined
    below.
    --*/

  template<class T> void addEdge (Acdfg* acdfg,
                  idMapType& idToNodeMap,
                  T  & proto_edge)
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
    } else if (typeid(T) == typeid(acdfg_protobuf::Acdfg_TransEdge)){
      TransitiveEdge * e = new TransitiveEdge(proto_edge.id(), from, to);
      acdfg -> add_edge(e);
    } else if (typeid(T) == typeid(acdfg_protobuf::Iso_Edge)){
      // Determine edge type and work accordingly
      if (from -> get_type() == DATA_NODE){
    assert(to -> get_type() == METHOD_NODE); // This is the only possibility allowed
    UseEdge * e = new UseEdge(proto_edge.id(), from, to);
    acdfg -> add_edge(e);
      } else {
    assert(from -> get_type() == METHOD_NODE);
    if (to -> get_type() == METHOD_NODE){
      TransitiveEdge * e = new TransitiveEdge(proto_edge.id(), from, to);
      acdfg -> add_edge(e);
    } else {
      assert( to -> get_type() == DATA_NODE);
      DefEdge * e = new DefEdge(proto_edge.id(), from, to);
      acdfg -> add_edge(e);
    }
      }
    }  else {
      std::cerr << typeid(T).name() << std::endl;
      assert(false); // This should not happen
    }
  }


  template <class T>
  DataNode * createAndAddDataNode(Acdfg * toWhat, idMapType & idToNodeMap, T & proto_node, data_node_type_t dtype){
    DataNode * node = new DataNode(proto_node.id(), proto_node.name(), proto_node.type(), dtype);
    Node * app_node= toWhat -> add_node(node);
    assert( idToNodeMap.find (app_node -> get_id()) == idToNodeMap.end());
    idToNodeMap[app_node -> get_id()] = app_node;
    assert( node -> get_type() == DATA_NODE);
    return node;
  }

  /*--
    Function create_acdfg

    This function creates the actual ACDFG by reading through the objects
    in the protobuf and translating them into corresponding objects in the
    Acdfg data structure we maintain
    --*/

  Acdfg* AcdfgSerializer::create_acdfg(acdfg_protobuf::Acdfg* proto_acdfg)
  {
    Acdfg* acdfg = new Acdfg();
    idMapType idToNodeMap;
    /* data ndoes */
     for (int j = 0; j < proto_acdfg->data_node_size(); j++) {
      const acdfg_protobuf::Acdfg_DataNode& proto_node = \
        proto_acdfg->data_node(j);
      /* Extract data node type */
      data_node_type_t dtype = DATA_NODE_VAR;
      if (proto_node.has_data_type()){
    dtype = (proto_node.data_type() == acdfg_protobuf::Acdfg_DataNode_DataType_DATA_VAR)? \
      DATA_NODE_VAR : DATA_NODE_CONST;
      }
      createAndAddDataNode(acdfg, idToNodeMap, proto_node, dtype);
      // DataNode * node = new DataNode(proto_node.id(),
      //                     proto_node.name(),
      //                     proto_node.type(),
      //                     dtype);
      // Node * app_node = acdfg->add_node(node);
      // idToNodeMap[app_node->get_id()] = app_node;
    }

    /* Misc Nodes */
    for (int j = 0; j < proto_acdfg->misc_node_size(); j++) {
      const acdfg_protobuf::Acdfg_MiscNode& proto_node =    \
        proto_acdfg->misc_node(j);
      Node * node = new Node(proto_node.id(), REGULAR_NODE);
      Node *app_node = acdfg->add_node(node);
      idToNodeMap[app_node->get_id()] = app_node;
    }

    /* Method Nodes */
    for (int j = 0; j < proto_acdfg->method_node_size(); j++) {
      const acdfg_protobuf::Acdfg_MethodNode& proto_node =  \
        proto_acdfg->method_node(j);

      /* read to the receiver */
      Node* r = lookup_node(idToNodeMap,        \
                proto_node.invokee());

      DataNode * receiver = NULL;
      if (r -> get_type() == DATA_NODE)
    receiver = toDataNode(r);


      // assert(NULL != receiver);
      /* read the assignee */
      DataNode * assignee = NULL;
      if (proto_node.has_assignee()) {
    Node * a =  lookup_node(idToNodeMap,            \
                proto_node.assignee());
    if (a -> get_type() == DATA_NODE)
      assignee = toDataNode(a);
      }
      /* read the arguments */
      std::vector<DataNode*> arguments;
      for (int k = 0; k < proto_node.argument_size(); k++) {
    const int argument_id = proto_node.argument(k);
    Node* n =  lookup_node(idToNodeMap,         \
                   argument_id);
    assert(n -> get_type() == DATA_NODE);
    DataNode * dArg = toDataNode(n);
    arguments.push_back(dArg);
      }

      /*- create the method node -*/
      MethodNode * node = new MethodNode(proto_node.id(),
                     proto_node.name(),
                     receiver,
                     arguments,
                     assignee);
      /*- add it to the ACDFG -*/
      Node *app_node = acdfg->add_node(node);
      idToNodeMap[app_node->get_id()] = app_node;
    }



    /* Read the edges */


    /* Def edges */
    for (int j = 0; j < proto_acdfg->def_edge_size(); j++) {
      const acdfg_protobuf::Acdfg_DefEdge& proto_edge = \
    proto_acdfg->def_edge(j);
      addEdge(acdfg, idToNodeMap, proto_edge);
    }

    for (int j = 0; j < proto_acdfg->use_edge_size(); j++) {
      const acdfg_protobuf::Acdfg_UseEdge& proto_edge = \
    proto_acdfg->use_edge(j);
      addEdge(acdfg, idToNodeMap,proto_edge);
    }

    for (int j = 0; j < proto_acdfg->control_edge_size(); j++) {
      const acdfg_protobuf::Acdfg_ControlEdge& proto_edge = \
    proto_acdfg->control_edge(j);
      addEdge(acdfg, idToNodeMap,proto_edge);
    }

    for (int j = 0; j < proto_acdfg->trans_edge_size(); j++) {
      const acdfg_protobuf::Acdfg_TransEdge& proto_edge = \
    proto_acdfg->trans_edge(j);
      addEdge(acdfg, idToNodeMap,proto_edge);
    }


    for (int j = 0; j < proto_acdfg->exceptional_edge_size(); j++) {
      const acdfg_protobuf::Acdfg_ExceptionalControlEdge& proto_edge = \
    proto_acdfg->exceptional_edge(j);
      addEdge(acdfg, idToNodeMap,proto_edge);
    }


    int nLabeledEdges = proto_acdfg -> edge_labels_size();
    for (int j =0; j < nLabeledEdges; ++j){
      acdfg_protobuf::Acdfg_LabelMap const& label_map = proto_acdfg -> edge_labels(j);
      long e_id = label_map.edge_id();
      Edge * edg = acdfg -> getEdgeFromID(e_id);
      if (edg){
    int l_size = label_map.labels_size();
    for (int l=0; l < l_size; ++l){
      if (label_map.labels(l) == acdfg_protobuf::Acdfg_EdgeLabel_DOMINATE){
        edg -> set_label(DOMINATE);
      }
      if (label_map.labels(l) == acdfg_protobuf::Acdfg_EdgeLabel_POSTDOMINATED){
        edg -> set_label(POST_DOMINATED);
      }
    }
      } else {
    std::cerr << "Warning: no edge with id --> " << e_id << "in the label map" << std::endl;
      }
    }

    acdfg -> fixMissingUseDefEdges();
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
      } else {
        return NULL;
      }
    } else {
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


  Acdfg * AcdfgSerializer::create_acdfg(acdfg_protobuf::Iso * proto_iso){
    Acdfg * acdfg = new Acdfg ();
    idMapType idToNodeMap;
    int j;

    /* -- Data nodes --*/
    for (j = 0; j < proto_iso -> data_nodes_size(); ++j){
      const acdfg_protobuf::Iso_DataNode & proto_node = proto_iso -> data_nodes(j);
      data_node_type_t dtype = DATA_NODE_UNKNOWN;
      if (proto_node .has_data_type()){
    dtype = (proto_node.data_type() == acdfg_protobuf::Iso_DataNode_DataType_DATA_VAR)? \
      DATA_NODE_VAR: DATA_NODE_CONST;
      }
      createAndAddDataNode(acdfg, idToNodeMap, proto_node, dtype);
    }

    /* -- Method nodes --*/
    for (j = 0; j < proto_iso -> method_nodes_size(); ++j){
      const acdfg_protobuf::Iso_MethodNode & proto_node =   \
    proto_iso -> method_nodes(j);
      DataNode * receiver = NULL;
      if (proto_node.has_invokeeid())
    receiver = (DataNode *) lookup_node(idToNodeMap,        \
                        proto_node.invokeeid());


      DataNode * assignee = NULL;

      if (proto_node.has_assigneeid()){
    Node * tmp = lookup_node(idToNodeMap,           \
                 proto_node.assigneeid());
    assert(tmp -> get_type() == DATA_NODE);
    assignee = toDataNode(tmp);
      }

      std::vector<DataNode*> arguments;
      for (int k = 0; k < proto_node.argumentids_size(); ++k){
    int argument_id = proto_node.argumentids(k);
    DataNode * n = (DataNode*) lookup_node(idToNodeMap, \
                           argument_id);
    arguments.push_back(n);
      }
      MethodNode * node = new MethodNode(proto_node.id(),
                     proto_node.name(),
                     receiver,
                     arguments,
                     assignee);
      Node * app_node = acdfg -> add_node(node);
      idToNodeMap[app_node-> get_id()] = app_node;
    }

    /*-- Edges: infer type from the types of the from/to edges --*/

    int nEdges = proto_iso-> edges_size();
    for (j = 0; j < nEdges; ++j){
      const acdfg_protobuf::Iso_Edge & proto_edge = \
    proto_iso -> edges(j);
      addEdge(acdfg, idToNodeMap, proto_edge);
    }

    return acdfg;

  }

} // namespace fixrgraphiso
