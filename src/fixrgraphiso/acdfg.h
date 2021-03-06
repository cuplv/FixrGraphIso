// -*- C++ -*-
//
// Data structure used to store an Abstract Control Data Flow Graph ACDFG)
//
// Author: Sergio Mover
//

#ifndef ACDFG_H_INCLUDED
#define ACDFG_H_INCLUDED 1

#include <vector>
#include <map>
#include <string>
#include <ostream>
#include <cassert>
#include <iostream>
#include <set>
#include "fixrgraphiso/proto_acdfg.pb.h"

namespace fixrgraphiso {
  namespace acdfg_protobuf = edu::colorado::plv::fixr::protobuf;

  using std::vector;
  using std::string;
  using std::set;
  using std::map;

  typedef enum { REGULAR_NODE, DATA_NODE, METHOD_NODE } node_type_t;
  typedef enum { DATA_NODE_CONST, DATA_NODE_VAR, DATA_NODE_UNKNOWN } data_node_type_t;

  typedef enum { CONTROL_EDGE, DEF_EDGE, USE_EDGE, TRANSITIVE_EDGE, EXCEPTIONAL_EDGE} edge_type_t;
  typedef enum {  DOMINATE, POST_DOMINATED, UNKNOWN_EDGE_LABEL } edge_label_t;
  typedef long node_id_t;
  typedef long edge_id_t;

  // Represent a node in the graph
  class Node {

  public:

    //Node():nType_(REGULAR_NODE){};

    Node(const Node& node);
    Node(long id, node_type_t typ);
    virtual ~Node() {}

    long get_id() const{
      return id_;
    }

    virtual node_type_t get_type() const{
      return nType_;
    }

    virtual string getDotLabel() const;
    virtual void addProtoNode(acdfg_protobuf::Acdfg* acdfg) const;


    virtual void prettyPrint(std::ostream & out) const ;

    friend std::ostream& operator<<(std::ostream&, const Node&);

    int getMatchFrequency() const { return match_frequency; }
    void setMatchFrequency(int m) { match_frequency = m; }
    void incrMatchFrequency() { match_frequency++; }

    bool operator==(const Node& other) const;

  protected:
    long id_;
    node_type_t nType_;
    int match_frequency;

  };

  // Node that represent a data (e.g variable)
  class DataNode : public Node {
  public:
    DataNode(long id, const string& name, const string& data_type, data_node_type_t dtype);
    DataNode(const DataNode& node);
    virtual ~DataNode() {}
    const string& get_name() const;
    const string& get_data_type() const;
    const data_node_type_t get_data_node_type() const;
    bool isConstNode() const;
    bool isVarNode() const;
    bool isCompatible(DataNode const * n) const;
    double compatibilityWeight(DataNode const * n) const;
    virtual string getDotLabel() const;
    virtual void addProtoNode(acdfg_protobuf::Acdfg* acdfg) const;
    void prettyPrint(std::ostream & out) const;
    DataNode * clone() const;
    friend std::ostream& operator<<(std::ostream&, const DataNode&);
    virtual node_type_t get_type() const { return DATA_NODE; }

    bool operator==(const DataNode& other) const;

  protected:
    string name_;
    string data_type_;
    data_node_type_t data_node_type_;
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
               std::vector<DataNode*> arguments,
               DataNode * assignee);
    MethodNode(const MethodNode& node);
    virtual ~MethodNode() {}

    const string& get_name() const;
    const DataNode* get_receiver() const;
    const DataNode * get_assignee() const;
    DataNode * get_receiver();
    DataNode * get_assignee();
    const std::vector<DataNode*> &  get_arguments() const;
    int get_num_arguments() const;
    virtual string getDotLabel() const;
    virtual void addProtoNode(acdfg_protobuf::Acdfg* acdfg) const;
    void prettyPrint(std::ostream & out) const;
    bool isCompatible(MethodNode const * n) const;
    double compatibilityWeight(MethodNode const * n) const;
    bool isSpecialMethod() const;
    friend std::ostream& operator<<(std::ostream&, const MethodNode&);
    virtual node_type_t get_type() const { return METHOD_NODE; }

    bool operator==(const MethodNode& other) const;

  protected:
    // Name of the method
    string name_;
    // Id of the receiver of the method
    DataNode* receiver_;
    // Parameters passed to the method invocation
    std::vector<DataNode*> arguments_;
    // Assignee information
    DataNode* assignee_; // Could be a null pointer if no assignee -- be careful
  };

  // General Conversion functions that will be useful for us.
  MethodNode * toMethodNode(Node* n);
  DataNode * toDataNode(Node* n);
  const DataNode * toDataNode( const Node * n);
  const MethodNode * toMethodNode(const Node* n);
  // Represent an edge of the Acdfg
  class Edge {
  public:

    Edge(long id, edge_type_t typ, Node* src, Node* dst): id_(id),
                                                          eType_(typ),
                                                          src_(src),
                                                          dst_(dst),
                                                          match_frequency(0)
    {};


    Edge(const Edge& edge);
    const long get_id() const;
    const edge_type_t get_type() const
    { return eType_; };

    const std::vector<edge_label_t> & get_labels() const
    { return eLabels_; };

    void set_label( edge_label_t eNew)
    { eLabels_.push_back(eNew); };

    const std::vector<std::string> & get_exceptList() const
    { return exceptList_; };

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

    double compatibilityWeight(Edge * eB) const;
    std::string get_edge_dot_style() const;

    int getMatchFrequency() const { return match_frequency; }
    void incrMatchFrequency() { match_frequency++; }
    virtual void addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const;

    bool operator==(const Edge& other) const;

  protected:
    long id_;
    edge_type_t eType_;
    // Src node
    Node* src_;
    // Dst node
    Node* dst_;
    // edge label
    std::vector<edge_label_t> eLabels_;
    // Only used in exception edges.
    std::vector<std::string> exceptList_;
    // Match frequency
    int match_frequency;

  };

  class DefEdge : public Edge {
  public:
    DefEdge(long id, Node* src, Node* dst) : Edge(id, DEF_EDGE, src, dst) {};
    DefEdge(const DefEdge& edge): Edge(edge.id_, DEF_EDGE, edge.src_, edge.dst_){};
    virtual void addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const;
  };


  class UseEdge : public Edge {
  public:
    UseEdge(long id, Node* src, Node* dst): Edge(id, USE_EDGE, src, dst){};
    UseEdge(const UseEdge & edge): Edge(edge.id_, USE_EDGE, edge.src_, edge.dst_){};
    virtual void addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const;
  };

  class ControlEdge : public Edge {
  public:
    ControlEdge(long id, Node* src, Node* dst): Edge(id, CONTROL_EDGE, src, dst){};
    ControlEdge(const ControlEdge & edge): Edge(edge.id_, CONTROL_EDGE, edge.src_, edge.dst_){};
    virtual void addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const;

  };

  class TransitiveEdge: public Edge {
  public:
    TransitiveEdge(long id, Node * src, Node * dst): Edge(id, TRANSITIVE_EDGE, src, dst){};
    TransitiveEdge(const TransitiveEdge & edge): Edge(edge.id_, TRANSITIVE_EDGE, edge.src_, edge.dst_){};
    virtual void addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const;
  };

  class ExceptionalEdge: public Edge{

  public:
    ExceptionalEdge(long id, Node* src, Node * dst): Edge(id, EXCEPTIONAL_EDGE, src, dst){};
    ExceptionalEdge(const ExceptionalEdge & edge): Edge(edge.id_, EXCEPTIONAL_EDGE, edge.src_, edge.dst_){};
    void addException(std::string const & what){
      exceptList_.push_back(what);
    }
    virtual void addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const;
  };

  typedef std::vector<Node*> nodes_t;
  typedef std::vector<Edge*> edges_t;
  typedef std::map<long, Node*> node_id_to_ptr_map_t;
  typedef std::map<long, int> node_id_to_int_t;
  typedef std::map<long, Edge*> edge_id_to_ptr_map_t;
  typedef std::map<long, vector<long> > node_id_to_outgoing_edges_map_t;

  class SourceInfo {
    public:

    SourceInfo() {
      package_name = "";
      class_name = "";
      method_name = "";
      class_line_number = 0;
      method_line_number = 0;
      source_class_name = "";
      abs_source_class_name = "";
    }

    string package_name;
    string class_name;
    string method_name;
    int class_line_number;
    int method_line_number;
    string source_class_name;
    string abs_source_class_name;
  };

  class RepoTag {
    public:

    RepoTag() {
      repo_name = "";
      user_name = "";
      url = "";
      commit_hash = "";
      commit_date = "";
    }

    string repo_name;
    string user_name;
    string url;
    string commit_hash;
    string commit_date;
  };

  class Acdfg {

  public:
    ~Acdfg();
    Node* add_node(Node *  node);
    Edge* add_edge(Edge *  edge);

    void fixMissingUseDefEdges();

    nodes_t::const_iterator begin_nodes() const ;
    nodes_t::const_iterator end_nodes() const;
    int node_count() const {return nodes_.size();};
    int typed_node_count(node_type_t t) const {
      int rVal =0;
      for (nodes_t::const_iterator it = begin_nodes(); it != end_nodes(); ++it){
        if ( (*it) -> get_type() == t)
          rVal ++;
      }
      return rVal;
    }

    int data_node_count() const {
      return typed_node_count(DATA_NODE);
    }

    int method_node_count() const {
      return typed_node_count(METHOD_NODE);
    }

    edges_t::const_iterator begin_edges() const;
    edges_t::const_iterator end_edges() const;
    int edge_count() const {return edges_.size();};
    int typed_edge_count(edge_type_t t) const{
      int rVal = 0;
      for (edges_t::const_iterator jt = begin_edges(); jt != end_edges(); ++jt){
        if ( (*jt) -> get_type() == t)
          ++rVal;
      }
      return rVal;
    }

    int control_edge_count() const { return typed_edge_count(CONTROL_EDGE); }
    int def_edge_count() const { return typed_edge_count(DEF_EDGE); }
    int use_edge_count() const { return typed_edge_count(USE_EDGE); }
    int transitive_edge_count() const { return typed_edge_count(TRANSITIVE_EDGE); }
    int exceptional_edge_count() const { return typed_edge_count(EXCEPTIONAL_EDGE); }

    std::vector< std::pair<string,int> > all_counts() const {
      std::vector< std::pair<string, int> > rVal {
        {"nodes" , node_count()} ,
          { "edges", edge_count()},
            {"data nodes", data_node_count()},
              {"method nodes", method_node_count()},
                {"control edges", (control_edge_count() + transitive_edge_count())},
                  {"use edges", use_edge_count()},
                    {"def edges", def_edge_count()},
                      {"exceptional edges", exceptional_edge_count() }
      };
      return rVal;
    }

    bool canSubsumeB(Acdfg& b);

    bool hasNode (node_id_t id) const;
    bool hasEdge (edge_id_t id) const;

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

    void setName(std::string const & name){
      this -> name_ = name;
    }

    std::string getName() const {
      return this -> name_;
    }

    void dumpToDot(std::ostream & os, bool transitiveReduce=true) const;

    void dumpToAcdfgProto(std::ostream & out,
                          bool transitiveReduce=false) const;

    Acdfg * extractSubgraphWithFrequencyCutoff(int freqCutoff) const;

    Acdfg * sliceACDFG(const std::vector<MethodNode*> & targets,
                       const set<int> & ignoreMethodIds);
    Acdfg * sliceACDFG(const std::vector<string>  & methodNames,
                       const set<int> & ignoreMethodIds);

    void getMethodsFromName(const std::vector<string> & methodnames,
                            std::vector<MethodNode*> & targets);
    void getMethodNodes(std::vector<MethodNode*> & targets);

    const node_id_to_int_t getNodeToLine() const {
      return node_to_line;
    }

    void addLine(long id, int line) {
      node_to_line[id] = line;
    }

    bool operator==(const Acdfg& other) const;

    void fill_methods(std::set<std::string> & method_names);

  public:
    SourceInfo source_info;
    RepoTag repo_tag;

  private:
    nodes_t nodes_;
    edges_t edges_;
    node_id_to_ptr_map_t nMap_;
    edge_id_to_ptr_map_t eMap_;
    node_id_to_outgoing_edges_map_t outgoingMap_;
    std::string name_;
    node_id_to_int_t node_to_line;

    void ensureEdge(edge_type_t eType, Node * src, Node * dest);
  };


  class TransitiveReduceAcdfg {
  protected:
    const Acdfg * a;
    std::set<long> vertexIDs;
    std::map< std::pair<long, long>, long > edges_src_dest;

    void deleteEdge(long srcID, long destID);
    void extractDataFromACDFG();

  public:
    TransitiveReduceAcdfg(const Acdfg * acdfg_a):a(acdfg_a){ extractDataFromACDFG(); }
    void performTransitiveReduction();
    bool hasEdge(long srcID, long destID);
  };


} // namespace fixrgraphiso

#endif // ACDFG_H_INCLUDED
