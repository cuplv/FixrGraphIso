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
#include "fixrgraphiso/proto_acdfg.pb.h"

namespace fixrgraphiso {
  using std::ostringstream;
  using std::string;
  using std::cout;
  using std::endl;
  using std::vector;
  using std::ostream;
  using std::pair;
  using std::set;

  extern bool debug;
  // Match the types of data nodes to be compatible
  bool typeMatchDataNode = false;
  bool varConstMatchDataNode = true;
  bool treatControlEdgesSeparately = false;
  //------------------------------------------------------------------------------
  // Implementation of the nodes
  //------------------------------------------------------------------------------


  Node::Node(long id, node_type_t typ): id_(id), nType_(typ), match_frequency(0){}

  Node::Node(const Node& node):id_(node.id_), nType_(node.nType_), match_frequency(node.match_frequency){}



  void Node::prettyPrint(ostream & stream) const {
    stream << "Node id: " << id_ << "\n";
  }


  string Node::getDotLabel() const {
    assert(get_type() == REGULAR_NODE);
    ostringstream ss;
    ss << "label=\"#"<<get_id()<<"\"";
    return ss.str();
  }



  ostream& operator<<(ostream& stream, const Node& node)
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


  DataNode::DataNode(const DataNode& node): Node(node.get_id(), DATA_NODE),
                                            name_(node.name_),
                                            data_type_(node.data_type_),
                                            data_node_type_(node.data_node_type_){
  }

  DataNode * DataNode::clone() const{
    DataNode * n_node = new DataNode(*this);
    return n_node;
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
  void DataNode::prettyPrint(ostream & stream) const {

    stream << "Data node id: " << (get_id()) <<
      "\n\t\tname: " << get_name() <<
      "\n\t\ttype: " << get_data_type() << endl;

  }


  string my_escape(string  arg){
    string a = "\"";
    string b = "\\\'";
    while (arg.find(a) < arg.length())
      arg= (arg.replace(arg.find(a), a.length(), b));
    return arg;
  }

  string DataNode::getDotLabel() const {
    ostringstream ss;
    assert(get_type() == DATA_NODE);
    ss << "shape=ellipse,color=red,style=dashed,label=\"DataNode #"<<get_id()<<": " << get_data_type() << "  " << my_escape(get_name())<<"\"" ;
    return ss.str();
  }


  ostream& operator<<(ostream& stream, const DataNode& node)
  {

    node.prettyPrint(stream);
    return stream;
  }



  MethodNode::MethodNode(long id, const string& name,
                         DataNode* receiver,
                         vector<DataNode*> arguments,
                         DataNode* assignee): CommandNode(id,METHOD_NODE),
                                              name_(name),
                                              receiver_(receiver),
                                              arguments_(arguments),
                                              assignee_(assignee)
  {

  }

  MethodNode::MethodNode(const MethodNode& node) : CommandNode(node.id_, METHOD_NODE),
                                                   receiver_(NULL),
                                                   name_(node.name_),
                                                   assignee_(NULL)
  {
    if (node.receiver_ != NULL){
      receiver_ = new DataNode(*(node.receiver_));
    } else {
      receiver_ = NULL;
    }



    for (vector<DataNode*>::const_iterator it = node.arguments_.begin();
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
      vector<DataNode*> const & myArgs = this -> get_arguments();
      vector<DataNode*> const & nodeArgs = node -> get_arguments();
      if (myArgs.size() == nodeArgs.size()){
        w += 1.0; /*-- for the two nodes agreeing in the number of arguments --*/
        vector<DataNode*>::const_iterator it_a = myArgs.begin(), it_b = nodeArgs.begin();
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


  const string& MethodNode::get_name() const
  {
    return name_;
  }

  DataNode * MethodNode::get_receiver() {
    return receiver_;
  }

  const DataNode* MethodNode::get_receiver() const
  {
    return receiver_;
  }

  int MethodNode::get_num_arguments() const {
    return (int) arguments_.size();
  }

  const vector<DataNode*> & MethodNode::get_arguments() const
  {
    return arguments_;
  }

  const DataNode * MethodNode::get_assignee() const {
    return assignee_;
  }

  DataNode * MethodNode::get_assignee()  {
    return assignee_;
  }


  bool MethodNode::isSpecialMethod() const {
    set<string> special_methods {"EQ", "NEQ", "GT", "LT", "LE", "GE"};
    return (special_methods.find(get_name() ) != special_methods.end());
  }

  void MethodNode::prettyPrint(ostream & stream) const {

    stream << "Method Node id: " << get_id() << "\n";
    if (NULL != get_receiver()) {
      stream << "\t\t" << get_receiver()-> get_name() << ":" << get_receiver()->get_id() << ".";
    }

    stream << "\t\t " << get_name() << "(";
    for (vector<DataNode*>::const_iterator it =  arguments_.begin();
         it != arguments_.end();
         ++it) {
      if (it != arguments_.begin()) stream << ",";
      if ((*it) != NULL)
        stream << (*it)->get_name() << ":" << (*it)->get_id();
      else
        stream << "NULL:NULL";
    }
    stream << ")" << endl;

  }

  string MethodNode::getDotLabel() const {
    ostringstream ss;
    assert(get_type() == METHOD_NODE);
    ss << " shape=box, style=filled, color=lightgray, label=\" " << get_name() << "[";
    const DataNode* r = get_receiver();
    if (r != NULL){
      ss << "#"<< r-> get_id();
    }
    ss << "](";
    string sep="";
    for (vector<DataNode*>::const_iterator it =  arguments_.begin();
         it != arguments_.end();
         ++it) {
      ss << sep << "#"<< (*it) -> get_id() ;
      sep = ", ";
    }
    ss << ")\"";
    return ss.str();
  }

  ostream& operator<<(ostream& stream, const MethodNode& node)
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

  const MethodNode* toMethodNode(const Node * n){
    assert (n -> get_type() == METHOD_NODE);
    const MethodNode* m = dynamic_cast<const MethodNode*> (n);
    assert( m != NULL);
    return m;
  }

  DataNode * toDataNode(Node * n){
    assert (n -> get_type() == DATA_NODE);
    DataNode* d = dynamic_cast<DataNode*> (n);
    assert( d!= NULL);
    return d;
  }

  const DataNode * toDataNode( const Node * n){
    assert (n -> get_type() == DATA_NODE);
    const DataNode* d = dynamic_cast<const DataNode*> (n);
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
    match_frequency = edge.match_frequency;
  }

  const long Edge::get_id() const {return id_;}
  const Node* Edge::get_src() const {return src_;}
  const Node* Edge::get_dst() const {return dst_;}

  ostream& operator<<(ostream& stream, const Edge& edge)
  {
    stream << "Id: (" << edge.get_id() << ") Src -> Dst := " <<
      edge.get_src()->get_id() << " -> " <<
      edge.get_dst()->get_id() << endl;
    return stream;
  }


  string Edge::get_edge_dot_style() const{
    switch (this -> get_type()){
    case USE_EDGE:
      return string("[color=green, penwidth=2]");
    case DEF_EDGE:
      return string("[color=blue, penwidth=2]");
    case EXCEPTIONAL_EDGE:
      return string("[color=red, penwidth=3]");
    case CONTROL_EDGE:
      return string("[color=black, penwidth=3]");
    case TRANSITIVE_EDGE:
      return string("[color=black, penwidth=3]");
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
        vector<string> :: const_iterator it_a;
        vector<string> :: const_iterator it_b;
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
        vector<edge_label_t> :: const_iterator jt_a, jt_b;
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
    assert(nMap_.find(node -> get_id()) == nMap_.end());
    nMap_[node->get_id()] = node;
    return node;
  }

  Edge* Acdfg::add_edge(Edge * edge)
  {
    Edge* new_edge = edge;
    edges_.push_back(new_edge);
    long eID = new_edge -> get_id();
    assert (eMap_.find(eID) == eMap_.end());
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


  void Acdfg::ensureEdge(edge_type_t eType, Node * src, Node * dest){
    node_id_t srcID = src -> get_id();
    node_id_t destID = dest -> get_id();
    edges_t :: const_iterator jt;
    long new_edge_id = -1;
    for (jt = begin_edges(); jt != end_edges(); ++jt){
      const Edge * e = *jt;

      if (e -> get_id() > new_edge_id)
        new_edge_id = e -> get_id();

      if (e -> get_src_id() == srcID && e -> get_dst_id() == destID && e -> get_type() == eType)
        return;
    }

    new_edge_id++;
    switch (eType){
    case USE_EDGE: {
      UseEdge * e = new UseEdge(new_edge_id, src, dest);
      add_edge(e);
    }
      break;
    case DEF_EDGE:{
      DefEdge * d = new DefEdge(new_edge_id, src, dest);
      add_edge(d);
    }
      break;
    default:
      assert(false);
      break;
    }
  }


  void Acdfg::fixMissingUseDefEdges(){
    /*-- For every method node,
      ensure there is a use edge from receiver to the node,
      ensure there is a def edge from assignee to the node.
      --*/
    nodes_t::const_iterator it;
    for (it = begin_nodes(); it != end_nodes(); ++it){
      Node * n = *it;
      if (n -> get_type() == METHOD_NODE){
        MethodNode * mNode = toMethodNode(n);
        DataNode * rcv = mNode -> get_receiver();
        DataNode * assg = mNode -> get_assignee();
        if (rcv)
          ensureEdge(USE_EDGE, rcv, mNode);
        if (assg)
          ensureEdge( DEF_EDGE, mNode, assg);
      }
    }
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

  bool Acdfg::hasNode( node_id_t id) const {
    return nMap_.find(id) != nMap_.end();
  }

  bool Acdfg::hasEdge( edge_id_t id) const {
    return eMap_.find(id) != eMap_.end();
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


  void printNode(Node * node, ostream & stream){

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
      stream << "Fatal: unhandled node type in acdfg.cpp printNode function" << endl;
      assert(false);
      break;
    }


    return;
  }

  DataNode * fetchOrClone(Acdfg * a, const DataNode * d, set<node_id_t> & addedNodes){
    /*-- Does the node already exist? --*/
    DataNode * new_rcv = NULL;
    if (d != NULL){
      node_id_t id = d -> get_id();
      if (addedNodes.find(id) != addedNodes.end()){
        /*-- If yes, fetch it from the old acdfg --*/
        Node * tmp = a-> getNodeFromID(id);
        assert(tmp && tmp -> get_type() == DATA_NODE) ;
        new_rcv = toDataNode(tmp);
      } else {
        /*-- does not already exist, clone --*/
        new_rcv = d -> clone();
        addedNodes.insert( id );
        a -> add_node(new_rcv);
      }
    }
    return new_rcv;
  }


  void Acdfg::getMethodsFromName(const vector<string> & methodnames,
                                 vector<MethodNode*> & targets){
    /* -- Iterate through all method nodes and check if it is found inside the given vector --*/
    set<string> methodNameSet;
    for (const string str: methodnames) {
      methodNameSet.insert(str);
    }
    for (auto it = this -> begin_nodes(); it != this -> end_nodes(); ++it){
      Node * n = *it;
      if (n -> get_type() == METHOD_NODE){
        MethodNode * mNode = toMethodNode(n);
        string node_name = mNode -> get_name();
        for (string n: methodnames){
          std::size_t found = node_name.find(n);
          if (found != string::npos){
            targets.push_back(mNode);
            if (debug){
              cout << "Added method node: " << node_name << endl;
            }
            break;
          }
        }
      }
    }
    return;
  }

  void Acdfg::getMethodNodes(vector<MethodNode*> & targets) {
    for (auto it = this -> begin_nodes(); it != this -> end_nodes(); ++it){
      Node * n = *it;
      if (n -> get_type() == METHOD_NODE) {
        MethodNode * mNode = toMethodNode(n);
        targets.push_back(mNode);
      }
    }
  }

  Acdfg* Acdfg::sliceACDFG(const vector<MethodNode*>  & targets,
                           const set<int> & ignoreMethodIds) {
    /*-
      1. Extract all related data nodes.
      2. Extract all relevant edges.
      3. Create a new ACDFG
      -*/
    Acdfg * retG = new Acdfg();

    set<node_id_t> nodesToAdd;
    for (const MethodNode* m: targets){

      if (ignoreMethodIds.find(m -> get_id()) != ignoreMethodIds.end())
        continue;

      nodesToAdd.insert(m -> get_id());
      /* -- Now add the arguments, receiver and invokee --*/
      const DataNode * rcv = m -> get_receiver();
      DataNode * new_rcv = fetchOrClone(retG, rcv, nodesToAdd);
      const DataNode * assg = m -> get_assignee();
      DataNode * new_assg = fetchOrClone(retG, assg, nodesToAdd);
      const vector<DataNode*> & args = m -> get_arguments();
      vector<DataNode * > new_args;
      for (DataNode * a: args) {
        DataNode * nArg = fetchOrClone(retG, a, nodesToAdd);
        new_args.push_back(nArg);
      }
      MethodNode * mNew = new MethodNode(m -> get_id(), m -> get_name(),
                                         new_rcv, new_args, new_assg);

      retG -> add_node(mNew);
    }

    /* Now iterate through the edges */
    set<edge_id_t> edgesToAdd;
    for (auto jt = this -> begin_edges(); jt != this -> end_edges(); ++jt){
      const Edge * e = *jt;
      node_id_t srcID = e -> get_src_id();
      node_id_t destID = e -> get_dst_id();
      if (nodesToAdd.find(srcID) != nodesToAdd.end() &&
          nodesToAdd.find(destID) != nodesToAdd.end()){

        /* Both source and destination edges exist */
        Node * srcNode = retG -> getNodeFromID(srcID);
        Node * destNode = retG -> getNodeFromID(destID);
        switch(e -> get_type()){
        case CONTROL_EDGE:
          if (treatControlEdgesSeparately) {
            ControlEdge * nEdge = new ControlEdge(e-> get_id(), srcNode, destNode);
            retG -> add_edge(nEdge);
          } else {
            TransitiveEdge * nEdge = new TransitiveEdge(e -> get_id(), srcNode, destNode);
            retG -> add_edge(nEdge);
          }

          break;
        case TRANSITIVE_EDGE: {
          TransitiveEdge * nEdge = new TransitiveEdge(e -> get_id(), srcNode, destNode);
          retG -> add_edge(nEdge);
        }
          break;
        case USE_EDGE: {
          UseEdge * uEdge = new  UseEdge(e -> get_id(), srcNode, destNode);
          retG -> add_edge(uEdge);
        }
          break;

        case DEF_EDGE: {
          DefEdge * dEdge = new DefEdge(e -> get_id(), srcNode, destNode);
          retG -> add_edge(dEdge);
        }
          break;

        case EXCEPTIONAL_EDGE:
          break;
        default:
          assert(false);
          break;
        }
      }
    }

    retG->setName(getName());

    return retG;
  }

  Acdfg* Acdfg::sliceACDFG(const vector<string>  & methodNames,
                           const set<int> & ignoreMethodIds) {
    vector<MethodNode*> targets;
    getMethodsFromName(methodNames, targets);

    Acdfg * new_acdfg = sliceACDFG(targets, ignoreMethodIds);

    return new_acdfg;
  }

  Acdfg * Acdfg::extractSubgraphWithFrequencyCutoff(int cutoff) const {
    /*--
      1. Extract all nodes that have been matched with some
      minimum frequency.

      2. Complete the graph by adding data nodes and edges that correspond to arguments,
      receivers and invokees of method nodes.
      --*/
    Acdfg * retG = new Acdfg();
    set<node_id_t> nodesAboveFreqCutoff;
    set<node_id_t> addedNodes;

    for (auto it = begin_nodes(); it != end_nodes(); ++it) {
      const Node * n = *it;
      if (n -> get_type() != REGULAR_NODE){
        if (n -> getMatchFrequency() >= cutoff){
          switch (n -> get_type() ){
          case DATA_NODE:{
            const DataNode * d = toDataNode(n);
            nodesAboveFreqCutoff.insert(d -> get_id() );
            /*- Does the node already exist? -*/
            if (addedNodes.find(d -> get_id()) == addedNodes.end()){
              /*-- If not, clone it --*/
              addedNodes.insert(d-> get_id());
              DataNode * dNew = d-> clone();
              retG -> add_node(dNew);
            }
          }
            break;
          case METHOD_NODE:{
            const MethodNode * m = toMethodNode(n);
            assert( addedNodes.find(m -> get_id()) == addedNodes.end() );
            addedNodes.insert( m -> get_id());
            nodesAboveFreqCutoff.insert( m -> get_id());
            /* Clone the receiver */
            const DataNode * rcv = m -> get_receiver();
            DataNode * new_rcv = fetchOrClone(retG, rcv, addedNodes);

            /* Clone the assignee */

            const DataNode * assg = m -> get_assignee();

            DataNode * new_assg = fetchOrClone(retG, assg, addedNodes);
            /* Clone the arguments */
            vector<DataNode *> const & args = m -> get_arguments();
            vector<DataNode *> new_args;
            for (const DataNode * dArg: args){
              assert( dArg != NULL);
              DataNode * nArg = fetchOrClone(retG, dArg, addedNodes);
              new_args.push_back(nArg);
            }


            /* Make a new method node */
            MethodNode * mNew = new MethodNode(m -> get_id(), m -> get_name(), new_rcv, new_args, new_assg);
            retG -> add_node(mNew);

          }
            break;
          default:
            assert(false);
            break;
          }
        }
      }
    }

    /*-- Criteria for adding edges:
      1. Control Edges: if above cutoff frequency.
      2. Def/Use edges: if both source/destination are present in the graph.
      ---*/
    for (auto jt = begin_edges(); jt != end_edges(); ++jt){
      const Edge * e = *jt;
      node_id_t srcID = e -> get_src_id();
      node_id_t destID = e -> get_dst_id();
      switch (e -> get_type()){
      case CONTROL_EDGE:
      case TRANSITIVE_EDGE:
        if (e -> getMatchFrequency() >= cutoff){
          Node * srcNode = retG -> getNodeFromID( srcID);
          Node * destNode = retG -> getNodeFromID( destID);

          TransitiveEdge * nEdge = new TransitiveEdge( e -> get_id(), srcNode, destNode);
          retG -> add_edge(nEdge);
        }
        break;

      case DEF_EDGE:
        if (retG -> hasNode(srcID) && retG -> hasNode(destID)){
          Node * srcNode = retG -> getNodeFromID( srcID);
          Node * destNode = retG -> getNodeFromID( destID);
          DefEdge * nEdge = new DefEdge( e -> get_id(), srcNode, destNode);
          retG -> add_edge(nEdge);
        }
        break;
      case USE_EDGE:
        if (retG -> hasNode(srcID) && retG -> hasNode(destID)){
          Node * srcNode = retG -> getNodeFromID( srcID);
          Node * destNode = retG -> getNodeFromID( destID);
          UseEdge * nEdge = new UseEdge( e -> get_id(), srcNode, destNode);
          retG -> add_edge(nEdge);
        }
        break;
      case EXCEPTIONAL_EDGE:
        std::cerr << "Warning: Exceptional edges are not handled in the frequent subgraph mining yet. " << std::endl;
        break;
      }

    }

    return retG;
  }

  ostream& operator<<(ostream& stream, const Acdfg& acdfg)
  {
    stream << "Acdfg\n" << "List of nodes: " << endl;
    for (nodes_t::const_iterator it =  acdfg.nodes_.begin();
         it != acdfg.nodes_.end(); ++it) {
      printNode((*it),stream);
      stream << endl;
      vector<long> oEdges = acdfg.getOutgoingEdgeIDs((*it) -> get_id());
      // Print outgoing edges
      stream << "\t Successor nodes: \t";
      string sep="";
      for (vector<long>::const_iterator jt = oEdges.begin();
           jt != oEdges.end();
           jt ++){
        const Edge * e = acdfg.getEdgeFromID(*jt);
        assert(e != NULL);
        assert(e -> get_src() -> get_id() == (*it) -> get_id());
        stream << sep << e -> get_dst() -> get_id() ;
        sep=", ";
      }
      stream << endl;
    }
    stream << "\nEdges:\n";
    for (edges_t::const_iterator it =  acdfg.edges_.begin();
         it != acdfg.edges_.end(); ++it) {
      stream << (*(*it));
    }
    stream << endl;
    return stream;
  }


  void Acdfg::dumpToDot(ostream & out, bool transitiveReduce) const  {
    TransitiveReduceAcdfg trRed(this);

    if (transitiveReduce)
      trRed.performTransitiveReduction();
    out << "digraph isoX {" << endl;
    out << " node[shape=box,style=\"filled,rounded\",penwidth=2.0,fontsize=13,]; \n \
 edge[ arrowhead=onormal,penwidth=2.0,]; \n" <<endl;

    for (auto pt = begin_nodes(); pt != end_nodes(); ++pt){
      const Node * na = *pt;
      string strA = na -> getDotLabel();
      out << "\"n_"<< na-> get_id() << "\" [" << strA << "];"<<endl;
    }

    for (auto rt = begin_edges(); rt != end_edges(); ++rt){
      const Edge * e = *rt;
      if (e -> get_type() != TRANSITIVE_EDGE || !transitiveReduce || trRed.hasEdge(e -> get_src_id(), e -> get_dst_id())){
        out << "\"n_"<< e -> get_src_id() << "\" -> \"n_"<< e -> get_dst_id() << "\""<< e-> get_edge_dot_style() << ";" << endl;
      }
    }

    out << " } " << endl;
  }


  void Acdfg::dumpToAcdfgProto(ostream & out, bool transitiveReduce) const {
    acdfg_protobuf::Acdfg* acdfg = new acdfg_protobuf::Acdfg();

    TransitiveReduceAcdfg trRed(this);
    if (transitiveReduce)
      trRed.performTransitiveReduction();

    for (auto pt = begin_nodes(); pt != end_nodes(); ++pt){
      const Node * na = *pt;
      na->addProtoNode(acdfg);
    }

    for (auto rt = begin_edges(); rt != end_edges(); ++rt){
      const Edge * e = *rt;
      e->addProtoEdge(acdfg);
    }

    {
      if (! acdfg->SerializeToOstream(&out)) {
        std::cerr << "Failed to write acdfg." << endl;
      }
    }
  }

  void Node::addProtoNode(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == REGULAR_NODE);

    acdfg_protobuf::Acdfg_MiscNode* misc_node =
      acdfg->add_misc_node();
    misc_node->set_id(get_id());
  }

  void DataNode::addProtoNode(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == DATA_NODE);
    acdfg_protobuf::Acdfg_DataNode* data_node = acdfg->add_data_node();

    data_node->set_id(get_id());
    data_node->set_name(get_name());
    data_node->set_type(get_data_type());

    if (get_data_node_type() == DATA_NODE_VAR) {
      data_node->set_data_type(acdfg_protobuf::Acdfg_DataNode_DataType_DATA_VAR);
    } else if (get_data_node_type() == DATA_NODE_CONST) {
      data_node->set_data_type(acdfg_protobuf::Acdfg_DataNode_DataType_DATA_CONST);
    }
  }

  void  MethodNode::addProtoNode(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == METHOD_NODE);
    acdfg_protobuf::Acdfg_MethodNode* method_node =
      acdfg->add_method_node();

    method_node->set_id(get_id());
    method_node->set_name(get_name());

    const DataNode* r = get_receiver();
    if (r != NULL){
      method_node->set_invokee(r->get_id());
    }

    for (vector<DataNode*>::const_iterator it =  arguments_.begin();
         it != arguments_.end();
         ++it) {
      method_node->add_argument((*it)->get_id());
    }
    // Assignee?
    // optional uint64 assignee = 5;
  }

  void Edge::addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const {
    assert(false);
  }

  void DefEdge::addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == DEF_EDGE);
    acdfg_protobuf::Acdfg_DefEdge* edge =
      acdfg->add_def_edge();

    edge->set_id(get_id());
    edge->set_from(get_src_id());
    edge->set_to(get_dst_id());
  }

  void UseEdge::addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == USE_EDGE);
    acdfg_protobuf::Acdfg_UseEdge* edge =
      acdfg->add_use_edge();

    edge->set_id(get_id());
    edge->set_from(get_src_id());
    edge->set_to(get_dst_id());
  }

  void ControlEdge::addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == CONTROL_EDGE);
    acdfg_protobuf::Acdfg_ControlEdge* edge =
      acdfg->add_control_edge();

    edge->set_id(get_id());
    edge->set_from(get_src_id());
    edge->set_to(get_dst_id());
  }

  void TransitiveEdge::addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == TRANSITIVE_EDGE);
    acdfg_protobuf::Acdfg_TransEdge* edge =
      acdfg->add_trans_edge();

    edge->set_id(get_id());
    edge->set_from(get_src_id());
    edge->set_to(get_dst_id());
  }

  void ExceptionalEdge::addProtoEdge(acdfg_protobuf::Acdfg* acdfg) const {
    assert(get_type() == EXCEPTIONAL_EDGE);
    acdfg_protobuf::Acdfg_ExceptionalControlEdge* edge =
      acdfg->add_exceptional_edge();

    edge->set_id(get_id());
    edge->set_from(get_src_id());
    edge->set_to(get_dst_id());
  }


  void TransitiveReduceAcdfg::extractDataFromACDFG(){
    // Extract all methods nodes
    for (auto it = a -> begin_nodes(); it != a -> end_nodes(); ++it){
      Node * n = *it;
      if (n -> get_type() == METHOD_NODE){
        vertexIDs.insert(n -> get_id());
      }
    }

    for (auto jt = a -> begin_edges(); jt != a -> end_edges(); ++jt){
      Edge * e = *jt;
      if (vertexIDs.find(e -> get_src_id()) != vertexIDs.end()){
        if (vertexIDs.find(e -> get_dst_id()) != vertexIDs.end()){
          // Insert it two ways
          pair<long,long> q(e-> get_src_id(), e -> get_dst_id());
          edges_src_dest[q] = e->get_id();
        }
      }
    }


  }

  bool TransitiveReduceAcdfg::hasEdge(long srcID, long destID){
    pair<long,long> q(srcID, destID);
    auto it = edges_src_dest.find(q);
    if (it != edges_src_dest.end()){
      return true;
    }

    return false;
  }

  void TransitiveReduceAcdfg::deleteEdge(long srcID, long destID){
    pair<long,long> q(srcID, destID);
    auto it = edges_src_dest.find(q);
    if (it != edges_src_dest.end()){
      edges_src_dest.erase(it);
    }
  }

  void TransitiveReduceAcdfg::performTransitiveReduction() {
    // We implement the n^3 algorithm because anything
    // else is going to be a pain
    // to debug in a short period of time.
    for (long id1: vertexIDs){
      for (long id2: vertexIDs){
        if (id1 != id2){
          for (long id3: vertexIDs){
            if (id2 != id3 && id1 != id3){
              if (hasEdge(id1, id2) && hasEdge(id2, id3)){
                deleteEdge(id1, id3);
              }
            }
          }
        }
      }
    }
  }

  bool Acdfg::operator==(const Acdfg& other) const {
    if (! (name_ == other.name_)) return false;

    /* Compare nodes */
    if (nodes_.size() != other.nodes_.size()) return false;

    {
      node_id_to_ptr_map_t idToNode;
      for (nodes_t::const_iterator it1 = nodes_.begin();
           it1 != nodes_.end(); ++it1) {
        Node* a = *it1;
        idToNode[a->get_id()] = a;
      }

      for (nodes_t::const_iterator it2 = other.nodes_.begin();
           it2 != other.nodes_.end(); ++it2) {
        Node* b = *it2;

        if (idToNode.find( b->get_id() ) == idToNode.end()) {
          std::cerr << "Cannot find id for " << (*b) << std::endl;
          return false;
        }

        Node* a = idToNode[b->get_id()];

        if (! ((*a) == (*b))) {
          std::cerr << "Node " << (*a) << " is different from "
                    << (*b) << std::endl;
          return false;
        }
      }
    }


    /* Compare edges */
    if (edges_.size() != other.edges_.size()) return false;
    {
      edge_id_to_ptr_map_t idToEdge;
      for (edges_t::const_iterator it1 = edges_.begin();
           it1 != edges_.end(); ++it1) {
        Edge* a = *it1;
        idToEdge[a->get_id()] = a;
      }

      for(edges_t::const_iterator it2 = other.edges_.begin();
          it2 != other.edges_.end(); ++it2) {
        Edge* b = *it2;

        if (idToEdge.find( b->get_id() ) == idToEdge.end()) {
          std::cerr << "Cannot find id for " << (*b) << std::endl;
          return false;
        }

        Edge* a = idToEdge[b->get_id()];

        if (! (*a == *b)) {
          std::cerr << "Edge " << (*a) <<
            " is different from " << (*b) << std::endl;
          return false;
        }
      }
    }

    return true;
  }

  bool Node::operator==(const Node& other) const {
    return id_ == other.id_ &&
      nType_ == other.nType_ &&
      match_frequency == other.match_frequency;
  }

  bool DataNode::operator==(const DataNode& other) const {
    return id_ == other.id_ &&
      nType_ == other.nType_ &&
      match_frequency == other.match_frequency &&
      name_ == other.name_ &&
      data_type_ == other.data_type_ &&
      data_node_type_ == other.data_node_type_;
  }


  bool MethodNode::operator==(const MethodNode& other) const {
    if (! (id_ == other.id_ &&
           nType_ == other.nType_ &&
           match_frequency == other.match_frequency &&
           name_ == other.name_ &&
           receiver_ == other.receiver_ &&
           assignee_ == other.assignee_)) {
      return false;
    } else if (arguments_.size() != other.arguments_.size()) {
      return false;
    } else {
      auto it1 = arguments_.begin();
      auto it2 = other.arguments_.begin();

      for (; it1 != arguments_.end(); it1++, it2++) {
        if (! (**it1 == **it2)) return false;
      }
    }

    return true;
  }

  bool Edge::operator==(const Edge& other) const {
    if (! (id_ == other.id_ &&
           eType_ == other.eType_ &&
           match_frequency == other.match_frequency &&
           (*src_) == (*other.src_) &&
           (*dst_) == (*other.dst_))) {
      std::cerr << "Different fields" << endl;
      return false;
    } if (eLabels_.size() != other.eLabels_.size()) {
      std::cerr << "Different label size" << endl;
      return false;
    } if (exceptList_.size() != other.exceptList_.size()) {
      std::cerr << "Different exception size" << endl;
      return false;
    } else {
      {
        auto it1 = exceptList_.begin();
        auto it2 = other.exceptList_.begin();

        for (; it1 != exceptList_.end(); it1++, it2++) {
          if (! (*it1 == *it2)) {
            std::cerr << (*it1) << "different from " << (*it2) << endl;
            return false;
          }
        }
      }

      {
        auto it1 = eLabels_.begin();
        auto it2 = other.eLabels_.begin();

        for (; it1 != eLabels_.end(); it1++, it2++) {
          if (!(*it1 == *it2)) {
            std::cerr << (*it1) << "different from " << (*it2) << endl;
            return false;
          }
        }
      }
    }

    return true;
  }


} // namespace fixrgraphiso


