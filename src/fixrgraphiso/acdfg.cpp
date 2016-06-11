// -*- C++ -*-
//
// Data structure used to store an Abstract Control Data Flow Graph ACDFG)
//
// Author: Sergio Mover
//

#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso {

//------------------------------------------------------------------------------
// Implementation of the nodes
//------------------------------------------------------------------------------

Node::Node(long id)
{
  this->id_ = id;
}

long Node::get_id()
{
  return id_;
}

DataNode::DataNode(long id, const string& name,
                   const string& data_type) : Node(id)
{
  this->name_ = name;
  this->data_type_ = data_type;
}

const string& DataNode::get_name()
{
  return name_;
}

const string& DataNode::get_data_type()
{
  return data_type_;
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

const string& MethodNode::get_name()
{
  return name_;
}

const DataNode* MethodNode::get_receiver()
{
  return receiver_;
}

const std::vector<DataNode*> MethodNode::get_arguments()
{
  return arguments_;
}

//------------------------------------------------------------------------------
// Implementation of the edges
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Implementation of the graph
//------------------------------------------------------------------------------

Node* Acdfg::add_node(Node node)
{
  nodes_.push_back(node);
  return &nodes_.back();
}

Edge* Acdfg::add_edge(Edge edge)
{
  edges_.push_back(edge);
  return &edges_.back();
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

} // namespace fixrgraphiso
