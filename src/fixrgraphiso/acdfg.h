// -*- C++ -*-
//
// Data structure used to store an Abstract Control Data Flow Graph ACDFG)
//
// Author: Sergio Mover
//

#ifndef ACDFG_H_INCLUDED
#define ACDFG_H_INCLUDED

#include <vector>
#include <map>
#include <string>
#include <ostream>
#include <cassert>
#include <iostream>
namespace fixrgraphiso {

  using std::vector;
  using std::string;

  typedef enum { REGULAR_NODE, DATA_NODE, METHOD_NODE } node_type_t;

  typedef enum { CONTROL_EDGE, DEF_EDGE, USE_EDGE} edge_type_t;
  typedef long node_id_t;
  typedef long edge_id_t;
  
  // Represent a node in the graph
  class Node {

  public:
  
    Node():nType_(REGULAR_NODE){};

    Node(const Node& node);
    Node(long id, node_type_t typ);
  
    long get_id() const{
      return id_;
    }
  
    node_type_t get_type() const{
      return nType_;
    }

    virtual Node * clone() const;
  
    virtual void prettyPrint(std::ostream & out) const ;
  
    friend std::ostream& operator<<(std::ostream&, const Node&);

  protected:
    long id_;
    node_type_t nType_;
  
  };

  // Node that represent a data (e.g variable)
  class DataNode : public Node {
  public:
    DataNode(long id, const string& name, const string& data_type);
    DataNode(const DataNode& node);
    const string& get_name() const;
    const string& get_data_type() const;
    bool isCompatible(DataNode const * n) const;
  
    void prettyPrint(std::ostream & out) const;
    Node * clone() const;
    friend std::ostream& operator<<(std::ostream&, const DataNode&);

  protected:
    string name_;
    string data_type_;
  };

  // Base class for control nodes
  class CommandNode : public Node {
  public:
    CommandNode(long id, node_type_t nTyp): Node(id, nTyp){};
  };


  // Represent a method invocation
  class MethodNode : public CommandNode {
  public:
    MethodNode(long id, const string& name,
	       DataNode* receiver,
	       std::vector<DataNode*> arguments);
    MethodNode(const MethodNode& node);

    const string& get_name() const;
    const DataNode* get_receiver() const;
    const std::vector<DataNode*> &  get_arguments() const;
    Node * clone() const;
  
    void prettyPrint(std::ostream & out) const;
    bool isCompatible(MethodNode const * n) const;

    friend std::ostream& operator<<(std::ostream&, const MethodNode&);
    
  protected:
    // Name of the method
    string name_;
    // Id of the receiver of the method
    DataNode* receiver_;
    // Parameters passed to the method invocation
    std::vector<DataNode*> arguments_;
  };

  // General Conversion functions that will be useful for us.
  MethodNode * toMethodNode(Node* n);
  DataNode * toDataNode(Node* n);

  // Represent an edge of the Acdfg
  class Edge {
  public:

    Edge(long id, edge_type_t typ, Node* src, Node* dst): id_(id), eType_(typ), src_(src), dst_(dst){};
  
    Edge(const Edge& edge);
    const long get_id() const;
    const edge_type_t get_type() const { return eType_; }
    const Node* get_src() const;
    const Node* get_dst() const;
    long get_src_id() const{
      const Node * n = get_src();
      return n -> get_id();
    }
    long get_dst_id() const {
      const Node * n = get_dst();
      return n -> get_id();
    }
    friend std::ostream& operator<<(std::ostream&, const Edge&);

  protected:
    long id_;
    edge_type_t eType_;
    // Src node
    Node* src_;
    // Dst node
    Node* dst_;
  };

  class DefEdge : public Edge {
  public:
    DefEdge(long id, Node* src, Node* dst) : Edge(id, DEF_EDGE, src, dst) {};
    DefEdge(const DefEdge& edge): Edge(edge.id_, DEF_EDGE, edge.src_, edge.dst_){};
  };

  
  class UseEdge : public Edge {
  public:
    UseEdge(long id, Node* src, Node* dst): Edge(id, USE_EDGE, src, dst){};
    UseEdge(const UseEdge & edge): Edge(edge.id_, USE_EDGE, edge.src_, edge.dst_){};
  };
  
  class ControlEdge : public Edge {
  public:
    ControlEdge(long id, Node* src, Node* dst): Edge(id, CONTROL_EDGE, src, dst){};
    ControlEdge(const ControlEdge & edge): Edge(edge.id_, CONTROL_EDGE, edge.src_, edge.dst_){};
  };

  typedef std::vector<Node*> nodes_t;
  typedef std::vector<Edge*> edges_t;
  typedef std::map<long, Node*> node_id_to_ptr_map_t;
  typedef std::map<long, Edge*> edge_id_to_ptr_map_t;
  typedef std::map<long, vector<long> > node_id_to_outgoing_edges_map_t;
  class Acdfg {
  public:
    ~Acdfg();
    Node* add_node(Node *  node);
    Edge* add_edge(Edge *  edge);

    nodes_t::const_iterator begin_nodes();
    nodes_t::const_iterator end_nodes();
    int node_count() {return nodes_.size();};

    edges_t::const_iterator begin_edges();
    edges_t::const_iterator end_edges();
    int edge_count() {return edges_.size();};

    const Node* getNodeFromID(long id) const;
    const Edge* getEdgeFromID(long id) const;
    Node * getNodeFromID(long id);
    Edge * getEdgeFromID(long id);
    
    std::vector<long> getOutgoingEdgeIDs(long nodeID) const{
      node_id_to_outgoing_edges_map_t::const_iterator it = outgoingMap_.find(nodeID);
      if (it == outgoingMap_.end()){
	vector<long> tmp;// return a dummy empty vector
	return tmp; 
      } else {
	return (it -> second);
      }
    }

    
    vector<long> getOutgoingEdgeIDs(Node * n) const {
      return getOutgoingEdgeIDs(n -> get_id() );
    }
    
  
    friend std::ostream& operator<<(std::ostream&, const Acdfg&);

  
  private:
    nodes_t nodes_;
    edges_t edges_;
    node_id_to_ptr_map_t nMap_;
    edge_id_to_ptr_map_t eMap_;
    node_id_to_outgoing_edges_map_t outgoingMap_;
  
  };


  
  
} // namespace fixrgraphiso

#endif // ACDFG_H_INCLUDED
