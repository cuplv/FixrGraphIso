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

Node::Node(long id)
{
  this->id_ = id;
}

Node::Node(const Node& node)
{
  // DEBUG
  std::cout << "Copy constructor for: " << node.get_id() << std::endl;
  id_ = node.id_;
}

long Node::get_id() const
{
  return id_;
}

std::ostream& operator<<(std::ostream& stream, const Node& node)
{
  stream << "Node id: " << node.id_ << "\n";
  return stream;
}

DataNode::DataNode(long id, const string& name,
                   const string& data_type) : Node(id)
{
  this->name_ = name;
  this->data_type_ = data_type;
}

DataNode::DataNode(const DataNode& node)
{
  id_ = node.id_;
  name_ = node.name_;
  data_type_ =  node.data_type_;
}

const string& DataNode::get_name() const
{
  return name_;
}

const string& DataNode::get_data_type() const
{
  return data_type_;
}

std::ostream& operator<<(std::ostream& stream, const DataNode& node)
{
  stream << "Node id: " << (node.get_id()) <<
    "\nname: " << node.get_name() <<
    "\ntype: " << node.get_data_type() << std::endl;

  return stream;
}

CommandNode::CommandNode(long id) : Node(id) {};

MethodNode::MethodNode(long id, const string& name,
                       const DataNode* receiver,
                       std::vector<DataNode*> arguments) : CommandNode(id)
{
  name_ = name;
  receiver_ = receiver_;
  arguments_ = arguments;
}

MethodNode::MethodNode(const MethodNode& node) : CommandNode(node.get_id())
{
  id_ = node.id_;
  name_ = node.name_;
  receiver_ = node.receiver_;
  arguments_ = node.arguments_;
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

std::ostream& operator<<(std::ostream& stream, const MethodNode& node)
{
  stream << "Node id: " << node.get_id() << "\n";
  if (NULL != node.get_receiver()) {
    stream << node.get_receiver()->get_id() << ".";
  }

  stream << node.get_name() << "(";
  for (std::vector<DataNode*>::const_iterator it =  node.arguments_.begin();
       it != node.arguments_.end();
       ++it) {
    if (it != node.arguments_.begin()) stream << ",";
    stream << (*it)->get_name() << ":" << (*it)->get_id();
  }
  stream << ")" << std::endl;
  return stream;
}

//------------------------------------------------------------------------------
// Implementation of the edges
//------------------------------------------------------------------------------

Edge::Edge(const Edge& edge)
{
  id_ = edge.id_;
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

DefEdge::DefEdge(const DefEdge& edge) : Edge(edge.id_, edge.src_, edge.dst_)
{
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

Node* Acdfg::add_node(const Node& node)
{
  Node* new_node = new Node(node);
  nodes_.push_back(new_node);
  return new_node;
}

Edge* Acdfg::add_edge(const Edge& edge)
{
  Edge* new_edge = new Edge(edge);
  edges_.push_back(new_edge);
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

std::ostream& operator<<(std::ostream& stream, const Acdfg& acdfg)
{
  stream << "Acdfg\n" << "List of nodes: ";
  for (nodes_t::const_iterator it =  acdfg.nodes_.begin();
       it != acdfg.nodes_.end(); ++it) {
    if (it != acdfg.nodes_.begin()) stream << ",";
    stream << (*it)->get_id();
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
