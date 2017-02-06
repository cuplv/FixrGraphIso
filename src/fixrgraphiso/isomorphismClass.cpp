#include <fstream>
#include <algorithm>
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serialization.h"


using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::to_string;

namespace fixrgraphiso {
  /*--
    Constructor for IsoEncoder 
    --*/
  extern bool debug;
  
  IsoEncoder::IsoEncoder():  ctx(), s(ctx), alreadySolved(false), satisfiable(false){}

  IsoEncoder::~IsoEncoder(){
    
  }

  /*-
    Create a boolean variable with a given name 
    -*/
  IsoEncoder::var_t IsoEncoder::createBooleanVariable(const string & vName){
    return ctx.bool_const(vName.c_str());
  }
  
  IsoEncoder::var_t  IsoSubsumption::getNodePairVar(node_id_t a, node_id_t b) const {
    id_pair_t ab (a,b);
    auto it = node_pairs_to_var.find(ab);
    assert(it != node_pairs_to_var.end());
    return it -> second;
  }

  IsoEncoder::var_t IsoSubsumption::getEdgePairVar(edge_id_t a, edge_id_t b) const {
    id_pair_t ab(a,b);
    auto it = edge_pairs_to_var.find(ab);
    assert ( it != edge_pairs_to_var.end());
    return it -> second;
  }

  /*-
    Create all variables needed for the encoding
    -*/
  void IsoSubsumption::createEncodingVariables(){
    /*-
      Iterate through all node pairs
      -*/
    string np_pref("np__");
    for (const auto p: nodes_a_to_b){
      node_id_t id_a = p.first;
      vector<node_id_t> const & v = p.second;
      assert(v.size() > 0);
      for (node_id_t id_b: v){
	IsoEncoder::var_t v = e.createBooleanVariable(np_pref+to_string(id_a) + "__" + to_string(id_b));
	node_pairs_to_var.insert( std::pair<id_pair_t, z3::expr>( id_pair_t(id_a, id_b) , v));
      }
    }

    /*-
      Iterate through all edge pairs
      -*/
    string ep_pref("ep__");
    for (const auto q: edges_a_to_b){
      edge_id_t edge_a = q.first;
      vector<edge_id_t> const & v = q.second;
      for (edge_id_t edge_b: v){
	IsoEncoder::var_t v = e.createBooleanVariable(ep_pref+to_string(edge_a)+"__"+to_string(edge_b));
	edge_pairs_to_var.insert( std::pair<id_pair_t, z3::expr>( id_pair_t(edge_a, edge_b) , v));
	//	edge_pairs_to_var[ id_pair_t(edge_a, edge_b)] = v;
      }
    }
    
  }


  void IsoEncoder::atmostOne(const vector<var_t> & vars){
    for (auto it= vars.cbegin(); it != vars.cend(); ++it){
      auto jt = it+1;
      IsoEncoder::var_t x = *it;
      for (; jt != vars.cend(); ++jt){
	IsoEncoder::var_t y  = *jt;
	s.add( (!x) || (!y) );
      }
    }
  }

  void IsoEncoder::atleastOne(const vector<var_t> & vars){
    std::vector<Z3_ast> array;
    for (int i = 0; i < vars.size(); ++i)
      array.push_back(vars[i]);
    s.add( to_expr ( this -> ctx, Z3_mk_or( this -> ctx, vars.size(), &(array[0]) ) ) );
  }

  void IsoEncoder::exactlyOne( const vector<var_t> & vars){
    atleastOne(vars);
    atmostOne(vars);
  }

  void IsoEncoder::addImplication(var_t a, var_t b){
    s.add (implies(a, b));
  }

  void IsoEncoder::solve(){
    if (!alreadySolved){
      switch(s.check()){
      case z3::unsat:
	satisfiable = false;
	break;
      case z3::sat:
	satisfiable = true;
	break;
      default:
	assert(false);
	break;
      }
    }
    alreadySolved = true;
    return;
  }

  bool IsoEncoder::isSat(){
    if (!alreadySolved)
      solve();
    return satisfiable;
  }

  bool IsoEncoder::getTruthValuation(IsoEncoder::var_t x){
    assert(satisfiable);
    z3::model m = s.get_model();
    z3::expr e = m.eval(x);
    if (e == ctx.bool_val(true))
      return true;
    else {
      assert (e == ctx.bool_val(false));
      return false;
    }
  }
  /*-- 
    Check for each type of node and edge, that graph b has no more
    than the number in graph a. If this fails, we directly conclude
    that no subsumption can exist and move on.
    --*/
  bool IsoSubsumption::checkNodeCounts() const {
    vector< std::pair<string, int> > a_counts = acdfg_a -> all_counts();
    vector< std::pair<string, int> > b_counts = acdfg_b -> all_counts();
    auto jt = b_counts.cbegin();
    for (auto it = a_counts.cbegin(); it != a_counts.cend(); ++it, ++jt){
      int count1 = it -> second;
      int count2 = jt -> second;
      if (count1 < count2){
	if (debug) {
	  cout << "\t \t --> " << it -> first << " iso A: " << count1 << " iso B: " << count2 << endl;
	}
	return false;
      }
    }

    return true;
  }

  /*--
    Check if a, b pair are compatible by searching our bookkeeping
    --*/

  bool IsoSubsumption::isCompatibleNodePair(node_id_t a, node_id_t b) const {
    auto it = nodes_a_to_b.find(a);
    if (it == nodes_a_to_b.end()) return false;
    vector<node_id_t> const & v = it -> second;
    auto jt = std::find(v.begin(), v.end(), b);
    return (jt != v.end());
  }

  bool method_node_compatibility_check_assignee = false;

  /*--
    Function to check if two method nodes are compatible 
    --*/
  bool IsoSubsumption::staticCheckMethodNodeCompatible(MethodNode const * ma, MethodNode const * mb) const{
    /*--
      Condition for static compatibility
       1. The function name must be identical.
       2. The number of arguments must be identical.
           No arguments in mb/ma can be null
       3. If ma has a non-null receiver then so must mb and vice-versa
       4. If ma has a non-null assignee then so must mb and vice-versa
             ( SS: Is this needed? For the time being placing this under 
                  a switch method_node_compatibility_check_assignee)
       --*/
    
    if (ma -> get_name() != mb -> get_name()) return false;
    if (ma -> get_num_arguments() != mb-> get_num_arguments()) return false;
    if (ma -> get_receiver() == NULL && mb -> get_receiver() != NULL) return false;
    if (mb -> get_receiver() == NULL && ma -> get_receiver() != NULL) return false;
    const vector<DataNode*> & va = ma -> get_arguments();
    for (const DataNode* d: va){
      if (d == NULL) return false;
    }
    
    /* This check below is not needed since we already have done this
       before calling the current method for the sake of
       efficiency. */
    
    // const vector<DataNode*> & vb = mb -> get_arguments();
    // for (const DataNode * d: vb){
    //   if (d == NULL) return false;
    // }
    
    if (method_node_compatibility_check_assignee){
      if (ma -> get_assignee() == NULL && mb -> get_assignee() != NULL) return false;
      if (ma -> get_assignee() != NULL && mb -> get_assignee() == NULL) return false;
    }
    return true;
  }


  void add_id_pair_to_map_pair(long id_a, long id_b, map< long, vector<long> > & a_to_b, map< long, vector<long> > & b_to_a){

    auto it_a = a_to_b.find(id_a);
    auto it_b = b_to_a.find(id_b);
    if (it_a == a_to_b.end()){
      vector<long> vA {id_b};
      a_to_b[id_a] = vA;
    } else {
      vector<long> & vA = a_to_b[id_a];
      auto it_a = std::find(vA.begin(), vA.end(), id_b);
      if (it_a == vA.end())
	vA.push_back(id_b);
    }
    if (it_b == b_to_a.end()){
      vector<long> vB {id_a};
      b_to_a[id_b] = vB;
    } else {
      vector<long> & vB = b_to_a[id_b];      
      auto it_b = std::find(vB.begin(), vB.end(), id_a);
      if (it_b == vB.end())
	vB.push_back(id_a);
    }
    
  }
  
  /*--
    Nodes na and nb are possible candidates for matching up. Let us 
    mark them as such by adding them to our book keeping.
    --*/
  void IsoSubsumption::addCompatibleNodePair(Node * na, Node * nb){
    node_id_t id_a = na -> get_id();
    node_id_t id_b = nb -> get_id();
    assert(na -> get_type() == nb -> get_type());
    add_id_pair_to_map_pair(id_a, id_b, nodes_a_to_b, nodes_b_to_a);
  }

  void IsoSubsumption::addCompatibleEdgePair(Edge * ea, Edge * eb){
    edge_id_t id_a = ea -> get_id();
    edge_id_t id_b = eb -> get_id();
    assert(ea -> get_type() == eb -> get_type());
    add_id_pair_to_map_pair(id_a, id_b, edges_a_to_b, edges_b_to_a);
  }

  /*--
    Check for each method node in b, that the corresponding compatible
    node in a exists. Also collect nodes that could be compatible. 
    If a node in b is found that is not compatible with any node in a,
    we will have to declare that A does not subsume B and move on. 

    Returns true if every method node in b has at least one compatible node in a.
            false otherwise
    --*/
  bool IsoSubsumption::findCompatibleMethodNodes() {
    /*-
      For each method node in graph b, 
         For each method node in graph a,
              if (is_compatible..)
                 then add node pair
         if no method node is compatible, then 
            we cannot have a subsumption.
      -*/
    nodes_t::const_iterator it, jt;
    for (it = acdfg_b -> begin_nodes(); it != acdfg_b -> end_nodes(); ++it){
      if ((*it) -> get_type() == METHOD_NODE){
	Node * nb = *it;
	MethodNode * mb = toMethodNode(nb);

	/* Make sure that no argument is null */
	const vector<DataNode*> & vb = mb -> get_arguments();
	for (const DataNode * d: vb){
	  if (d == NULL){
	    std::cerr << "Warning: NULL argument found in method node. " ;
	    mb -> prettyPrint(std::cerr);
	    std::cerr << std::endl;
	    return false;
	  }
	  
	}
	
	/* Find all compatible method nodes by iterating through graph a*/
	bool something_compatible = false;
	for (jt = acdfg_a -> begin_nodes(); jt != acdfg_a -> end_nodes(); ++jt){
	  if ((*jt) -> get_type() == METHOD_NODE){
	    Node * na = (*jt);
	    MethodNode * ma = toMethodNode(na);
	    
	    if (this -> staticCheckMethodNodeCompatible(ma, mb)){
	      something_compatible = true;
	      this -> addCompatibleNodePair(ma, mb);
	    }
	  }
	}
	if (!something_compatible){
	  if (debug){
	    std::cout << "Method node does not have a compatible counterpart -- subsumption ruled out ! " << std::endl;
	    mb-> prettyPrint(std::cout);
	  }
	  return false;
	}
      }
    }
    return true;
  }

  /*--
    Add pairs of data nodes that are compatible.
    Since we do not have type checking or comparison on yet, 
    we will simply declare that every data node is compatible to every other.
    --*/
  bool IsoSubsumption::findCompatibleDataNodes() {
    nodes_t::const_iterator it, jt;
    
    for (it = acdfg_b -> begin_nodes(); it != acdfg_b -> end_nodes(); ++it){
      // Iterate through all nodes in b
      if ((*it) -> get_type() == DATA_NODE){
	for (jt = acdfg_a -> begin_nodes(); jt != acdfg_a -> end_nodes(); ++jt)
	  if ((*jt) -> get_type() == DATA_NODE)
	    this -> addCompatibleNodePair(*jt, *it);
      }
    }
    
    return true;
  }

  /*-- 
    Iterate through pairs of edges and make sure to match up pairs of compatible edges.
    This method should only be called after we have computed compatible nodes.
    Return true: all edges in b have at least one compatible edge in a
           false: there is some edge in b for which no compatible a edge could be found.
    --*/
  bool IsoSubsumption::findCompatibleEdgePairs() {
    /*--
      Iterate through all edges in b,
        Iterate through all edges in a of the same type,
	  Check if the source and destination nodes are compatible,
	     if so the edge pair is compatible.
         Otherwise, if no edge in a is compatible, we cannot have a subsumption
      --*/
    edges_t::const_iterator it, jt;
    for (jt = acdfg_b -> begin_edges(); jt != acdfg_b -> end_edges(); ++jt){
      Edge * eb = (*jt);
      const Node * src_b = eb -> get_src();
      const Node * dst_b = eb -> get_dst();
      bool something_compatible = false;
      for (it = acdfg_a -> begin_edges(); it != acdfg_a -> end_edges(); ++it){
	Edge * ea = (*it);
	if (ea -> get_type() == eb -> get_type()){
	  const Node * src_a = ea -> get_src();
	  const Node * dst_a = ea -> get_dst();
	  if (this -> isCompatibleNodePair(src_a, src_b) &&	\
	      this -> isCompatibleNodePair(dst_a, dst_b)){
	    something_compatible = true;
	    addCompatibleEdgePair(ea, eb);
	  } 
	}
      }
      if (!something_compatible){
	if (debug){
	  cout << "\t Incompatible edge found in graph B" << endl;
	  cout << "\t" << eb -> get_src_id() << "--> " << eb -> get_dst_id() << endl;
	}
	return false;
      }
    }
    return true;
  }

  /*- 
    Create the encoding in Z3 but do not solve it yet.
    -*/
  void IsoSubsumption::makeEncoding(){
    createEncodingVariables();
    /*-
      Every node in B must be connected to exactly one node in a.
      -*/
    for (const auto p: nodes_b_to_a){
      node_id_t id_b = p.first;
      vector<node_id_t> const & v = p.second;
      vector< IsoEncoder::var_t > var_pairs;
      for (node_id_t id_a: v){
	IsoEncoder::var_t var = getNodePairVar(id_a, id_b);
	var_pairs.push_back( var);
      }
      e.exactlyOne(var_pairs);
    }
    /*-
      Every node in A can be connected to at most one node in b.
      -*/

    for (const auto p: nodes_a_to_b){
      node_id_t id_a = p.first;
      vector<node_id_t> const & v = p.second;
      vector<IsoEncoder::var_t> var_pairs;
      for (node_id_t id_b: v){
	IsoEncoder::var_t var = getNodePairVar(id_a, id_b);
	var_pairs.push_back( var);
      }
      e.atmostOne(var_pairs);
    }

    /*-
      Every edge in B must be connected to at least one edge in A.
      At most one edge is implied by the rest of the encoding.
      -*/
    for (const auto p: edges_b_to_a){
      edge_id_t edg_b = p.first;
      vector<edge_id_t> const & v = p.second;
      vector< IsoEncoder::var_t> var_pairs;
      for (edge_id_t edg_a: v){
	IsoEncoder::var_t var = getEdgePairVar(edg_a, edg_b);
	var_pairs.push_back( var );
      }
      e.atleastOne(var_pairs);
    }

    

    
    
    
    // Every edge in A must be connected to at most one edge in b.
    // Sriram: this is actually redundant.

    // for (const auto p: edges_a_to_b){
    //   edge_id_t edg_a = p.first;
    //   vector<edge_id_t> const & v = p.second;
    //   vector<IsoEncoder::var_t > var_pairs;
    //   for (edge_id_t edg_b: v){
    // 	IsoEncoder::var_t var = getEdgePairVar(edg_a, edg_b);
    // 	var_pairs.push_back(var);
    //   }
    //   e.atmostOne(var_pairs);
    // }

    

    /*-
      If two method nodes are connected, then 
      their arguments, receiver and assignees should also be connected
      -*/

    for (const auto p: nodes_a_to_b){
      node_id_t id_a = p.first;
      Node * na = acdfg_a -> getNodeFromID(id_a);
      assert(na != NULL);
      if (na -> get_type() == METHOD_NODE){
	MethodNode * ma = toMethodNode(na);
	vector<node_id_t> const & v =  p.second;
	assert( v.size() > 0);
	for (node_id_t id_b: v){
	  Node * nb = acdfg_b -> getNodeFromID(id_b);
	  assert( nb != NULL && nb -> get_type() == METHOD_NODE);
	  MethodNode * mb = toMethodNode(nb);
	  // Now match up the assignee, receivers and arguments of ma, mb
	  IsoEncoder::var_t method_node_var = getNodePairVar(ma -> get_id(), mb -> get_id());
	  const DataNode * ra = ma -> get_receiver();
	  const DataNode * rb = mb -> get_receiver();
	  if ((ra != NULL) || (rb != NULL)){
	    assert(rb != NULL);
	    assert(ra != NULL);
	    IsoEncoder::var_t  receiver_var = getNodePairVar(ra -> get_id(), rb -> get_id());
	    e.addImplication(method_node_var, receiver_var);
	  }
	  
	  const DataNode* asa = ma -> get_assignee();
	  const DataNode* asb = mb -> get_assignee();
	  if ( (asa != NULL) && (asb != NULL)){
	    assert(asa != NULL);
	    assert(asb != NULL);
	    IsoEncoder::var_t assignee_var = getNodePairVar(asa -> get_id(), asb -> get_id());
	    e.addImplication(method_node_var, assignee_var);
	  }

	  const vector<DataNode*> & vA = ma -> get_arguments();
	  const vector<DataNode*> & vB = mb -> get_arguments();
	  assert(vA.size() == vB. size());
	  for (int i  = 0; i < vA.size(); ++i){
	    DataNode const * di_a = (vA)[i];
	    DataNode const * di_b = (vB)[i];
	    assert(di_a != NULL);
	    assert(di_b != NULL);
	    IsoEncoder::var_t argument_var = getNodePairVar(di_a -> get_id(), di_b -> get_id());
	    e.addImplication(method_node_var, argument_var);
	  }
	}
      }
    }

    /*-
      If two edges are connected then, the source and destinations must
      also be connected 
      -*/

    for (auto p: edges_a_to_b){
      edge_id_t edge_a = p.first;
      const Edge * eA = acdfg_a -> getEdgeFromID(edge_a);
      assert( eA != NULL);
      node_id_t srcA = eA -> get_src_id();
      node_id_t dstA = eA -> get_dst_id();
      const vector<edge_id_t> & vB = p.second;
      for (node_id_t edge_b: vB){
	const Edge * eB = acdfg_b -> getEdgeFromID(edge_b);
	assert( eB != NULL);
	node_id_t srcB = eB -> get_src_id();
	node_id_t dstB = eB -> get_dst_id();
	IsoEncoder::var_t edge_pair = getEdgePairVar (edge_a, edge_b);
	IsoEncoder::var_t  srcs = getNodePairVar(srcA, srcB);
	IsoEncoder::var_t dsts = getNodePairVar(dstA, dstB);
	e.addImplication(edge_pair, srcs);
	e.addImplication(edge_pair, dsts);	
      }
    }
    
    /*-
      That's all folks!
      -*/
    
  }

  bool IsoSubsumption::check(){
    
    if (!checkNodeCounts()){
      if (debug) std::cout << "\t Node counts rule out subsumption" << endl;
      return false;
    }
    
    if (! findCompatibleMethodNodes()){
      if (debug) std::cout << "\t Incompatible method node found. Subsumption ruled out" << endl;
      return false;
    }
    if (! findCompatibleDataNodes()){
      if (debug) std::cout << "\t Incompatible data node found. Subsumption ruled out" << endl;
      return false;
    }

    if (!findCompatibleEdgePairs()){
      if (debug) std::cout << "\t Incompatible edges found. Subsumption ruled out" << endl;
      return false;
    }

    makeEncoding();
    
    e.solve();
    return e.isSat();
  }
  
  /*--
    Constructors for IsomorphismClass.
    --*/
  IsomorphismClass::IsomorphismClass(Acdfg * what): iso_filename(what -> getName()), freq(1), acdfg(what){
    subsumingACDFGs.push_back(what -> getName());
  }
  
  IsomorphismClass::IsomorphismClass(string const & fname):iso_filename(fname), freq(1){
    iso_protobuf::Iso iso;
    std::fstream inp_file (fname.c_str(), std::ios::in | std::ios::binary);
    assert(inp_file.is_open());
    iso.ParseFromIstream(& inp_file);
    inp_file.close();
    AcdfgSerializer s;
    acdfg = s.create_acdfg(&iso);
    filename_a = iso.graph_1_id();
    filename_b = iso.graph_2_id();
  }

  IsomorphismClass::~IsomorphismClass(){
  }


  
  
  bool IsomorphismClass::subsumes(IsomorphismClass const * b) const {
    /* -- Encode the precise subsumption of b in a. I.e, b is a subgraph of a.
          This is different from the approximate embeddings that were computed for 
          pairs of ACDFGs. 

           Here we are going to consider actual subgraph isomorphism,
           we will make some tradeoffs
          1. We will not match the types of the data nodes.
          2. However, we will match the method nodes in terms of the type signature, arguments and 
	     control flow edges between them.

          0. Tally the counts of number of nodes and edges of various types. For each node type in b,
             its count must be less than or equal to that in a.

          1. Collect pairs of method nodes that can be potentially isomorphic to each other.
             1.1 If we have unmatchable nodes in b, then there is no way we can have isomorphisms

	  2. Collect pairs of data nodes that can be potentially isomorphic to each other.
          3. Collect pairs of edges that can be isomorphic.

          SAT encoding
          1. Create variables for all isomorphic pairs.
          2. Every node in <b> must be isomorphic to exactly one node in <a>.
             2.1 Every node in <a> must be connected to at most one node in <b>
          3. Every edge in <b> must be isomoprhic to exactly one other edge in <a>.
             3.1 Every edge in <a> must be connected to at most one  edge in <b>.
          4. If two method nodes are isomorphic, then 
             4.1 they must have the same method call, same number of arguments, the arguments, 
                 assignee and receiver must be matched to each other.
          5. If two edges are isomorphic then their source and target nodes must be isomorphic to each other.
	  ---*/
    IsoSubsumption isoSub(this -> acdfg, b -> acdfg);
    if (isoSub.check()) {
      if (debug) cout << "\t SAT solver returned SAT! " << endl;
      return true;
    }
    if (debug) cout << "\t SAT solver returned UNSAT!" << endl;
    return false;
  }
  

  
  
}
