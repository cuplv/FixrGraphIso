// -*- C++ -*-
//
// Data structure used to store an Abstract Control Data Flow Graph ACDFG)
//
// Author: Sergio Mover
//

#ifndef ACDFG_H_INCLUDED
#define ACDFG_H_INCLUDED

#include <vector>
#include <string>


namespace fixrgraphiso {
using std::string;

// Represent a node in the graph
class Node {
public:
  Node() {};
  Node(long id);
  long get_id();

private:
  long id_;
};

// Node that represent a data (e.g variable)
class DataNode : public Node {
public:
  DataNode(long id, const string& name, const string& data_type);
  const string& get_name();
  const string& get_data_type();

private:
  string name_;
  string data_type_;
};

// Base class for control nodes
class CommandNode : public Node {
public:
  CommandNode(long id);
};


// Represent a method invocation
class MethodNode : public CommandNode {
public:
  MethodNode(long id, const string& name,
             const DataNode* receiver,
             std::vector<DataNode*> arguments);

  const string& get_name();
  const DataNode* get_receiver();
  const std::vector<DataNode*> get_arguments();

private:
  // Name of the method
  string name_;
  // Id of the receiver of the method
  DataNode* receiver_;
  // Parameters passed to the method invocation
  std::vector<DataNode*> arguments_;
};

// Represent an edge of the Acdfg
class Edge {
public:
  Edge(long id, Node* src, Node* dst) {id_ = id; src_ = src; dst_ = dst;};

private:
  long id_;
  // Src node
  Node* src_;
  // Dst node
  Node* dst_;
};

class DefEdge : public Edge {
public:
  DefEdge(long id, Node* src, Node* dst) : Edge(id, src, dst) {};
};
class UseEdge : public Edge {};
class ControlEdge : public Edge {};

typedef std::vector<Node> nodes_t;
typedef std::vector<Edge> edges_t;

class Acdfg {
public:
  Node* add_node(Node node);
  Edge* add_edge(Edge edge);

  nodes_t::const_iterator begin_nodes();
  nodes_t::const_iterator end_nodes();

  edges_t::const_iterator begin_edges();
  edges_t::const_iterator end_edges();

private:
  nodes_t nodes_;
  edges_t edges_;
};


} // namespace fixrgraphiso

#endif // ACDFG_H_INCLUDED
