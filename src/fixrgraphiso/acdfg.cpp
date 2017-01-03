// -*- C++ -*-
//
// Data structure used to store an Abstract Control Data Flow Graph ACDFG)
//
// Author: Sergio Mover,
//         Sriram Sankaranarayanan
//

#include <iostream> // DEBUG
#include "fixrgraphiso/acdfg.h"
#include <sstream>
#include <set>
namespace fixrgraphiso {
  using std::ostringstream;
  // Match the types of data nodes to be compatible
  bool typeMatchDataNode = false;
  bool varConstMatchDataNode = true;
  //------------------------------------------------------------------------------
  // Implementation of the nodes
  //------------------------------------------------------------------------------

  
  Node::Node(long id, node_type_t typ): id_(id), nType_(typ){}

  Node::Node(const Node& node):id_(node.id_), nType_(node.nType_){}
  
  

  void Node::prettyPrint(std::ostream & stream) const {
    stream << "Node id: " << id_ << "\n";
  }

  
  string Node::getDotLabel() const {
    assert(get_type() == REGULAR_NODE);
    ostringstream ss;
    ss << "label=\"#"<<get_id()<<"\"";
    return ss.str();
  }
  
  Node * Node::clone() const {
    return new Node(*this);
  }

  
  std::ostream& operator<<(std::ostream& stream, const Node& node)
  {
    node.prettyPrint(stream);
    return stream;
  }
  

  DataNode::DataNode(long id, const string& name, const string& data_type, data_node_type_t dtype):
    Node(id, DATA_NODE),
    name_(name),
    data_type_(data_type),
    data_node_type_(dtype) {
  }

  
  DataNode::DataNode(const DataNode& node): Node(node.id_, DATA_NODE),
					    name_(node.name_),
					    data_type_(node.data_type_),
					    data_node_type_(node.data_node_type_){
  }

  Node * DataNode::clone() const{
    DataNode * n_node = new DataNode(*this);
    return (Node*) n_node;
  }

  double DataNode::compatibilityWeight(DataNode const * n) const {
    double w = 0.0;
    // Type matches = 0.5
    // varConstMatches = 0.5
    if (n -> data_type_ == this -> data_type_){
      w += 0.5;
    }
    if (n -> data_node_type_ == this -> data_node_type_){
      w += 0.5;
    }
    return w;
  }

  
  
  bool DataNode::isCompatible(DataNode const * n) const {
    bool typeMatches = true;
    if (typeMatchDataNode){
      typeMatches = (n -> data_type_ == this-> data_type_);
    }
    bool varConstMatches = true;
    if (varConstMatchDataNode){
      varConstMatches = (n -> data_node_type_ == this -> data_node_type_);
    }

    return (typeMatches && varConstMatches);
  }
  
  const string& DataNode::get_name() const
  {
    return name_;
  }

  
  const string& DataNode::get_data_type() const
  {
    return data_type_;
  }

  const data_node_type_t DataNode::get_data_node_type() const{
    return data_node_type_;
  }

  bool DataNode::isConstNode() const {
    return this -> data_node_type_ == DATA_NODE_CONST;
  }

  bool DataNode::isVarNode() const{
    return this -> data_node_type_ == DATA_NODE_VAR;
  }
  void DataNode::prettyPrint(std::ostream & stream) const {
    
    stream << "Data node id: " << (get_id()) <<
      "\n\t\tname: " << get_name() <<
      "\n\t\ttype: " << get_data_type() << std::endl;
    
  }

  
  string my_escape(string  arg){
    string a = "\"";
    string b = "\\\"";
    while (arg.find(a) < arg.length())
      arg= (arg.replace(arg.find(a), a.length(), b));
    return arg;
  }

  string DataNode::getDotLabel() const {
    ostringstream ss;
    assert(get_type() == DATA_NODE);
    ss << "shape=ellipse,label=\"DataNode #"<<get_id()<<": " << get_data_type() << "  " << my_escape(get_name())<<"\"" ;
    return ss.str();
  }

  
  std::ostream& operator<<(std::ostream& stream, const DataNode& node)
  {
    
    node.prettyPrint(stream);
    return stream;
  }


  
  MethodNode::MethodNode(long id, const string& name,
			 DataNode* receiver,
			 std::vector<DataNode*> arguments,
			 DataNode* assignee): CommandNode(id,METHOD_NODE),
					      name_(name),
					      receiver_(receiver),
					      arguments_(arguments),
					      assignee_(assignee)
  {}

  MethodNode::MethodNode(const MethodNode& node) : CommandNode(node.id_, METHOD_NODE),
						   name_(node.name_), assignee_(NULL)
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
    if (node.assignee_ != NULL){
      this -> assignee_ = new DataNode(*(node.assignee_));
    }
  }

  double MethodNode::compatibilityWeight(MethodNode const * node) const{
    double w = 0.0;
    // The weight is going to be the sum of the compatibility weights
    // of the receivers, the assignees and corresponding arguments.
    const DataNode * a;
    const DataNode * b;
    if (this -> isCompatible(node)){
      a = this -> get_receiver();
      b = node -> get_receiver();
      if (a != NULL && b != NULL){
	w += a -> compatibilityWeight(b);
      }
      a = this -> get_assignee();
      b = node -> get_assignee();
      if (a != NULL && b != NULL){
	w += a -> compatibilityWeight(b);
      }
      std::vector<DataNode*> const & myArgs = this -> get_arguments();
      std::vector<DataNode*> const & nodeArgs = node -> get_arguments();
      if (myArgs.size() == nodeArgs.size()){
	w += 1.0; /*-- for the two nodes agreeing in the number of arguments --*/
	std::vector<DataNode*>::const_iterator it_a = myArgs.begin(), it_b = nodeArgs.begin();
	for (; it_a != myArgs.end(); ++it_a, ++it_b){
	  w += (*it_a) -> compatibilityWeight((*it_b));
	}
      }
    }
    return w;
  }
  
  bool MethodNode::isCompatible(const MethodNode * node) const {
    return (this -> name_ == node -> name_);
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

  int MethodNode::get_num_arguments() const {
    return (int) arguments_.size();
  }
  
  const std::vector<DataNode*> & MethodNode::get_arguments() const
  {
    return arguments_;
  }

  const DataNode * MethodNode::get_assignee() const {
    return assignee_;
  }

  bool MethodNode::isSpecialMethod() const {
    std::set<std::string> special_methods {"EQ", "NEQ", "GT", "LT", "LE", "GE"};
    return (special_methods.find(get_name() ) != special_methods.end());
  }
  
  void MethodNode::prettyPrint(std::ostream & stream) const {
    
    stream << "Method Node id: " << get_id() << "\n";
    if (NULL != get_receiver()) {
      stream << "\t\t" << get_receiver()-> get_name() << ":" << get_receiver()->get_id() << ".";
    }
    
    stream << "\t\t " << get_name() << "(";
    for (std::vector<DataNode*>::const_iterator it =  arguments_.begin();
	 it != arguments_.end();
	 ++it) {
      if (it != arguments_.begin()) stream << ",";
      if ((*it) != NULL)
	stream << (*it)->get_name() << ":" << (*it)->get_id();
      else
	stream << "NULL:NULL";
    }
    stream << ")" << std::endl;
    
  }

  string MethodNode::getDotLabel() const {
    ostringstream ss;
    assert(get_type() == METHOD_NODE);
    ss << " label=\" MethodNode #" <<get_id()<< ": " << get_name() << "[";
    const DataNode* r = get_receiver();
    if (r != NULL){
      ss << "#"<< r-> get_id();
    }
    ss << "](";
    string sep="";
    for (std::vector<DataNode*>::const_iterator it =  arguments_.begin();
	 it != arguments_.end();
	 ++it) {
      ss << sep << "#"<< (*it) -> get_id() ;
      sep = ", ";
    }
    ss << ")\"";
    return ss.str();
  }
  
  std::ostream& operator<<(std::ostream& stream, const MethodNode& node)
  {
    node.prettyPrint(stream);
    return stream;
  }

  
  MethodNode * toMethodNode(Node * n){
    assert (n -> get_type() == METHOD_NODE);
    MethodNode* m = dynamic_cast<MethodNode*> (n);
    assert( m != NULL);
    return m;
  }

  DataNode * toDataNode(Node * n){
    assert (n -> get_type() == DATA_NODE);
    DataNode* d = dynamic_cast<DataNode*> (n);
    assert( d!= NULL);
    return d;
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
    stream << "Id: (" << edge.get_id() << ") Src -> Dst := " << 
      edge.get_src()->get_id() << " -> " <<
      edge.get_dst()->get_id() << std::endl;
    return stream;
  }

  
  std::string Edge::get_edge_dot_style(){
    switch (this -> get_type()){
    case USE_EDGE:
    case DEF_EDGE:
      return string("[color=blue, penwidth=3]");
    case EXCEPTIONAL_EDGE:
      return string("[color=red, penwidth=3]");
    case CONTROL_EDGE:
      return string("[color=black, penwidth=3]");
    case TRANSITIVE_EDGE:
      return string("[color=black, penwidth=0.5]");
    default:
      return string("");

    }
    
  }
  
  double Edge::compatibilityWeight(Edge * eB) const {
    double w =0.0;
    switch (this -> get_type()){
    case USE_EDGE:
    case DEF_EDGE:
      return (eB -> get_type() == this->get_type())? 1.0 : 0.0;
    case EXCEPTIONAL_EDGE:
      if (eB -> get_type() == this -> get_type()){
	// Calculate how many exceptions are in common
	w = 1.0;
	std::vector<std::string> :: const_iterator it_a;
	std::vector<std::string> :: const_iterator it_b;
	for (it_a = this -> exceptList_.begin(); it_a != this -> exceptList_.end(); ++it_a){
	  for (it_b = eB -> exceptList_.begin(); it_b != eB -> exceptList_.end(); ++it_b){
	    if ((*it_a) == (*it_b)) {
	      w = w + 1.0;
	      break;
	    }
	  }
	}	
      }
      break;
    case CONTROL_EDGE:
    case TRANSITIVE_EDGE:
      if (eB -> get_type() == CONTROL_EDGE || eB -> get_type() == TRANSITIVE_EDGE){
	// Calculate how many edge labels are in common
	w = 1.0;
	std::vector<edge_label_t> :: const_iterator jt_a, jt_b;
	for (jt_a = this -> eLabels_.begin(); jt_a != this -> eLabels_.end(); ++jt_a){
	  for (jt_b = eB -> eLabels_.begin(); jt_b != eB -> eLabels_.end(); ++jt_b){
	    if ((*jt_a) == (*jt_b)) {
	      w = w + 1.0;
	      break;
	    }
	  }
	}	
      
      }
      break;
      
    }
    
    return w;
  }
  
  //------------------------------------------------------------------------------
  // Implementation of the graph
  //------------------------------------------------------------------------------

  Acdfg::~Acdfg()
  {
    for (nodes_t::const_iterator it =  nodes_.begin();
	 it != nodes_.end(); ++it)  delete *(it);
    

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
    long eID = new_edge -> get_id();
    eMap_[eID] = new_edge;

  
    // Add the edge ID to the source node's list of outgoing edges
    const Node * n = new_edge -> get_src();
    long src_id = n -> get_id();
    node_id_to_outgoing_edges_map_t::iterator it = outgoingMap_.find(src_id);
    if (it == outgoingMap_.end()){
      vector<long> tmp;
      tmp.push_back(eID);
      outgoingMap_[src_id] = tmp;
    } else {
      vector<long> & v = it -> second;
      v.push_back(eID);
    }

    // Return the edge pointer
    return new_edge;
  }

  nodes_t::const_iterator Acdfg::begin_nodes() const
  {
    return nodes_.cbegin();
  }

  nodes_t::const_iterator Acdfg::end_nodes() const
  {
    return nodes_.cend();
  }

  edges_t::const_iterator Acdfg::begin_edges() const
  {
    return edges_.cbegin();
  }

  edges_t::const_iterator Acdfg::end_edges() const
  {
    return edges_.cend();
  }

  Node * Acdfg::getNodeFromID(long id){
    node_id_to_ptr_map_t::iterator it = nMap_.find(id);
    if (nMap_.end() == it){
      assert(false); // No such id exists
      return NULL; // DEAD CODE
    } 

    return it -> second;
  }
  
  
  const Node* Acdfg::getNodeFromID( long id) const {
    node_id_to_ptr_map_t::const_iterator it = nMap_.find(id);
    if (nMap_.end() == it){
      assert(false); // No such id exists
      return NULL; // DEAD CODE
    } 

    return it -> second;
  
  }


  const Edge* Acdfg::getEdgeFromID( long id) const{
    edge_id_to_ptr_map_t::const_iterator it = eMap_.find(id);
    if (eMap_.end() == it){
      assert(false); // No such id exists
      return NULL; // DEAD CODE
    } 

    return it -> second;
  
  }

  Edge * Acdfg::getEdgeFromID( long id) {
    edge_id_to_ptr_map_t::iterator it = eMap_.find(id);
    if (eMap_.end() == it){
      assert(false); // No such id exists
      return NULL; // DEAD CODE
    } 

    return it -> second;
  }


  void printNode(Node * node, std::ostream & stream){

    switch (node -> get_type()){
    case REGULAR_NODE:
      node -> prettyPrint(stream);
      break;

    case DATA_NODE:
      {
	DataNode * dNode = dynamic_cast<DataNode*> (node);
	assert(dNode != NULL);
	dNode -> prettyPrint(stream);
      }
      break;

    case METHOD_NODE:
      {
	MethodNode * mNode= dynamic_cast<MethodNode*> (node);
	assert(mNode != NULL);
	mNode -> prettyPrint(stream);
      }
      break;
    default:
      stream << "Fatal: unhandled node type in acdfg.cpp printNode function" << std::endl;
      assert(false);
      break;
    }


    return;
  }

  std::ostream& operator<<(std::ostream& stream, const Acdfg& acdfg)
  {
    stream << "Acdfg\n" << "List of nodes: " << std::endl;
    for (nodes_t::const_iterator it =  acdfg.nodes_.begin();
	 it != acdfg.nodes_.end(); ++it) {
      printNode((*it),stream);
      stream << std::endl;
      std::vector<long> oEdges = acdfg.getOutgoingEdgeIDs((*it) -> get_id());
      // Print outgoing edges
      stream << "\t Successor nodes: \t";
      std::string sep="";
      for (std::vector<long>::const_iterator jt = oEdges.begin();
	   jt != oEdges.end();
	   jt ++){
	const Edge * e = acdfg.getEdgeFromID(*jt);
	assert(e != NULL);
	assert(e -> get_src() -> get_id() == (*it) -> get_id());
	stream << sep << e -> get_dst() -> get_id() ;
	sep=", ";
      }
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


