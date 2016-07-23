// -*- C++ -*-
//
// Data structure used to store an Abstract Control Data Flow Graph ACDFG)
//
// Author: Sergio Mover
//

#include <iostream> // DEBUG
#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso {

//------------------------------------------------------------------------------
// Implementation of the nodes
//------------------------------------------------------------------------------

  Node::Node(long id, node_type_t typ): id_(id), nType_(typ){}

  Node::Node(const Node& node):id_(node.id_), nType_(node.nType_){}
  
  

  void Node::prettyPrint(std::ostream & stream) const {
    stream << "Node id: " << id_ << "\n";
  }

  Node * Node::clone() const {
    return new Node(*this);
  }

  
  std::ostream& operator<<(std::ostream& stream, const Node& node)
  {
    node.prettyPrint(stream);
    return stream;
  }
  

  DataNode::DataNode(long id, const string& name, const string& data_type) : Node(id, DATA_NODE), name_(name), data_type_(data_type){
  }

  
  DataNode::DataNode(const DataNode& node): Node(node.id_, DATA_NODE), name_(node.name_), data_type_(node.data_type_){}

  Node * DataNode::clone() const{
    DataNode * n_node = new DataNode(*this);
    return (Node*) n_node;
  }

  
  const string& DataNode::get_name() const
  {
    return name_;
  }
  
  const string& DataNode::get_data_type() const
  {
    return data_type_;
  }
  
  void DataNode::prettyPrint(std::ostream & stream) const {
    
    stream << "Node id: " << (get_id()) <<
      "\nname: " << get_name() <<
      "\ntype: " << get_data_type() << std::endl;
    
  }
  
  std::ostream& operator<<(std::ostream& stream, const DataNode& node)
  {
    
    node.prettyPrint(stream);
    return stream;
  }


  
  MethodNode::MethodNode(long id, const string& name,
			 DataNode* receiver,
			 std::vector<DataNode*> arguments) : CommandNode(id,METHOD_NODE),
							     name_(name),
							     receiver_(receiver),
							     arguments_(arguments){}

  MethodNode::MethodNode(const MethodNode& node) : CommandNode(node.id_, METHOD_NODE),
						   name_(node.name_)
  {
    if (node.receiver_ != NULL){
      receiver_ = new DataNode(*(node.receiver_));
    } else {
      receiver_ = NULL;
    }
    for (std::vector<DataNode*>::const_iterator it = node.arguments_.begin();
	 it != node.arguments_.end();
	 ++it){
      DataNode * n_node = new DataNode (*(*it));
      arguments_.push_back(n_node);
    }
  }

  Node * MethodNode::clone() const{
    MethodNode * n_node = new MethodNode(*this);
    return (Node*) n_node;
  }

  const string& MethodNode::get_name() const
  {
    return name_;
  }
  
  const DataNode* MethodNode::get_receiver() const
  {
    return receiver_;
  }
  
  const std::vector<DataNode*> MethodNode::get_arguments() const
  {
    return arguments_;
  }

  
  void MethodNode::prettyPrint(std::ostream & stream) const {
    
    stream << "Method Node id: " << get_id() << "\n";
    if (NULL != get_receiver()) {
      stream << get_receiver()->get_id() << ".";
    }
    
    stream << get_name() << "(";
    for (std::vector<DataNode*>::const_iterator it =  arguments_.begin();
	 it != arguments_.end();
	 ++it) {
      if (it != arguments_.begin()) stream << ",";
      stream << (*it)->get_name() << ":" << (*it)->get_id();
    }
    stream << ")" << std::endl;
    
  }
  
  std::ostream& operator<<(std::ostream& stream, const MethodNode& node)
  {
    node.prettyPrint(stream);
    return stream;
  }
  
  //------------------------------------------------------------------------------
  // Implementation of the edges
  //------------------------------------------------------------------------------
  
  Edge::Edge(const Edge& edge)
  {
    id_ = edge.id_;
    eType_ = edge.eType_;
    src_ = edge.src_;
    dst_ = edge.dst_;
  }
  
  const long Edge::get_id() const {return id_;}
  const Node* Edge::get_src() const {return src_;}
  const Node* Edge::get_dst() const {return dst_;}
  
  std::ostream& operator<<(std::ostream& stream, const Edge& edge)
  {
    stream << "Id: Src -> Dst := : " << edge.get_id() << ":" <<
      edge.get_src()->get_id() << " -> " <<
      edge.get_dst()->get_id() << std::endl;
    return stream;
  }

 
  
//------------------------------------------------------------------------------
// Implementation of the graph
//------------------------------------------------------------------------------

Acdfg::~Acdfg()
{
  for (nodes_t::const_iterator it =  nodes_.begin();
       it != nodes_.end(); ++it) delete *(it);

  for (edges_t::const_iterator it =  edges_.begin();
       it != edges_.end(); ++it) delete *(it);
}

  
Node* Acdfg::add_node(Node * node)
{
  
  nodes_.push_back(node);
  nMap_[node->get_id()] = node;
  return node;
}

Edge* Acdfg::add_edge(Edge * edge)
{
  Edge* new_edge = edge;
  edges_.push_back(new_edge);
  eMap_[new_edge -> get_id()] = new_edge;
  return new_edge;
}

nodes_t::const_iterator Acdfg::begin_nodes()
{
  return nodes_.begin();
}

nodes_t::const_iterator Acdfg::end_nodes()
{
  return nodes_.end();
}

edges_t::const_iterator Acdfg::begin_edges()
{
  return edges_.begin();
}

edges_t::const_iterator Acdfg::end_edges()
{
  return edges_.end();
}

Node* Acdfg::getNodeFromID( long id){
  node_id_to_ptr_map_t::iterator it = nMap_.find(id);
  if (nMap_.end() == it){
    assert(false); // No such id exists
    return NULL; // DEAD CODE
  } 

  return it -> second;
  
}


Edge* Acdfg::getEdgeFromID( long id){
  edge_id_to_ptr_map_t::iterator it = eMap_.find(id);
  if (eMap_.end() == it){
    assert(false); // No such id exists
    return NULL; // DEAD CODE
  } 

  return it -> second;
  
}


void printNode(Node * node, std::ostream & stream){
  DataNode * dNode = dynamic_cast<DataNode*> (node);
  if (dNode != NULL){
    dNode -> prettyPrint(stream);
  } else {
    
    MethodNode * mNode= dynamic_cast<MethodNode*> (node);
    if (mNode != NULL){
      mNode -> prettyPrint(stream);
    }
  }

  node -> prettyPrint(stream);
  return;
}

  
std::ostream& operator<<(std::ostream& stream, const Acdfg& acdfg)
{
  stream << "Acdfg\n" << "List of nodes: ";
  for (nodes_t::const_iterator it =  acdfg.nodes_.begin();
       it != acdfg.nodes_.end(); ++it) {
    if (it != acdfg.nodes_.begin())
      printNode((*it),stream);
    stream << std::endl;
  }
  stream << "\nEdges:\n";
  for (edges_t::const_iterator it =  acdfg.edges_.begin();
       it != acdfg.edges_.end(); ++it) {
    stream << (*(*it));
  }
  stream << std::endl;
  return stream;
}


} // namespace fixrgraphiso
