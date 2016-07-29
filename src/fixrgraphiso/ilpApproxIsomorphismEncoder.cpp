#include <iostream>
#include "fixrgraphiso/ilpApproxIsomorphismEncoder.h"

namespace fixrgraphiso {
  bool debug = true;
  bool encodeRegularNodes = true;

  void IlpApproxIsomorphism::addCompatibleNodes(Node * na, Node * nb){
    node_id_t id_a = na -> get_id();
    node_id_t id_b = nb -> get_id();

    std::cout <<" Compatible nodes: " << id_a <<" , " << id_b << std::endl;
    
    assert(na -> get_type() == nb -> get_type());
    if (node_map_a_to_b.find(id_a) == node_map_a_to_b.end()){
      std::vector<long> vA;
      vA.push_back(id_b);
      node_map_a_to_b[id_a] = vA; // Add map from a -> b
    } else {
      std::vector<long> & vA = node_map_a_to_b[id_a];
      vA.push_back(id_b); // Map from a -> b
    }

     if (node_map_b_to_a.find(id_b) == node_map_b_to_a.end()){
      std::vector<long> vB;
      vB.push_back(id_a);
      node_map_b_to_a[id_b] = vB; // Add map from b -> a
    } else {
      std::vector<long> & vB = node_map_b_to_a[id_b];
      vB.push_back(id_a); // Map from b -> a
    }
    

    return;
  }
  
  void IlpApproxIsomorphism::computeCompatibleNodes(){
    
    nodes_t::const_iterator it, jt;
    for (it = acdfg_a -> begin_nodes();
	 it != acdfg_a -> end_nodes();
	 ++it){
      // Iterate through all nodes of graph a
      Node * na = (*it);
     

      for (jt = acdfg_b -> begin_nodes();
	   jt != acdfg_b -> end_nodes();
	   ++jt){
	// Iterate through nodes of graph b
	Node * nb = (*jt);
	// If the types match
	if (na -> get_type() == nb -> get_type()){
	  // Switch based on the type of the nodes
	  switch( na -> get_type()){
	  case REGULAR_NODE:
	    {
	      // We are not going to bother encoding compatibilities
	      // for regular nodes.  This decision should be OK since
	      // we are going to look at transitive closure edges in
	      // the next updated.

	      if (encodeRegularNodes){
		this -> addCompatibleNodes(na, nb);
	      }
	      
	     
	    }
	    break;

	  case METHOD_NODE:
	    {
	      // cast to method nodes
	      MethodNode* ma = toMethodNode(na);
	      MethodNode* mb = toMethodNode(nb);
	      if (ma -> isCompatible(mb)){
		this -> addCompatibleNodes(na,nb);
	      }
	      

	    }
	    break;

	  case DATA_NODE:
	    {
	      // cast to data nodes
	      DataNode* da = toDataNode(na);
	      DataNode* db = toDataNode(nb);
	      if (da -> isCompatible(db)){
		this -> addCompatibleNodes(na,nb);
	      }
	      

	    }
	    break;

	  default:
	    // We have not handled the type?
	    std::cerr << "Fatal: unknown type for a node in function computeCompatibleNodes ilpApproxIsomorphismEncoder.cpp " << std::endl;
	    assert(false);
	    break;
	  }  // switch()
	} // if na -> get_type() == nb -> get_type() 
      } // Iterate through nodes of b
    } // iterate through nodes of a
  } // computeCompatibleNodes ()


  void IlpApproxIsomorphism::insertCompatibleEdges(edge_id_t eA, edge_id_t eB){
    if (debug){
      std::cout << " Compatible edges " << eA << ", " << eB << std::endl;
    }
    compat_edges_a_to_b.push_back(edge_pair_t(eA,eB));
    
  }
  
  void IlpApproxIsomorphism::addAllCompatibleEdges(std::vector<edge_id_t> const & vA, std::vector<edge_id_t> const & vB){
    std::vector<edge_id_t>::const_iterator it, jt;
    for (it = vA. begin(); it != vA.end(); ++it){
      const Edge * eA = acdfg_a -> getEdgeFromID(*it);
      const Node * sA = eA -> get_src();
      const Node * tA = eA -> get_dst();
      assert(eA != NULL);
      for (jt = vB.begin(); jt != vB.end(); ++jt){
	const Edge* eB = acdfg_b -> getEdgeFromID(*jt);
	assert(eB != NULL);
	const Node * sB = eB -> get_src();
	const Node * tB = eB-> get_dst();
	assert( areCompatibleNodes(sA -> get_id(), sB -> get_id()));
	if (areCompatibleNodes(tA -> get_id(), tB -> get_id())){
	  insertCompatibleEdges(*it, *jt); 
	}
      }
    }
  }

  bool IlpApproxIsomorphism::areCompatibleNodes(node_id_t aID, node_id_t bID){
    compatible_node_map_t::const_iterator it = node_map_a_to_b.find(aID);
    if (it == node_map_a_to_b.end()) return false;
    std::vector<node_id_t> const & compats = it -> second;
    std::vector<node_id_t>::const_iterator jt = std::find(compats.begin(), compats.end(), bID);
    return (jt != compats.end());
  }

  std::vector<node_id_t> IlpApproxIsomorphism::getCompatibleNodeIDs(char a_or_b, node_id_t id){
    switch(a_or_b){
    case 'a': {
      compatible_node_map_t:: const_iterator it = node_map_a_to_b.find(id);
      if (it != node_map_a_to_b.end()){
	return it -> second;
      }
    }
      break;
    case 'b': {
      compatible_node_map_t:: const_iterator it = node_map_b_to_a.find(id);
      if (it != node_map_b_to_a.end()){
	return it -> second;
      }
    }
      break;
    default:
      assert(false); // This is fatal
      break;
    }
    return std::vector<node_id_t>();
      
  }
  void IlpApproxIsomorphism::computeCompatibleEdges() {
    // Two edges i -> j and k -> l are compatible iff
    //   i,j are compatible and nodes j,l are.
    // Iterate through all nodes
    nodes_t::const_iterator it;
    for (it = acdfg_a -> begin_nodes();
	 it != acdfg_a -> end_nodes();
	 ++it){
      // Iterate through all nodes of graph a
      Node * na = (*it);
      // Get all nodes compatible with na
      std::vector<node_id_t> compatIDs = getCompatibleNodeIDs('a',na -> get_id());
      // get all outgoing edges of na
      std::vector<edge_id_t> outgoingEdges_na = acdfg_a-> getOutgoingEdgeIDs(na -> get_id());
      // Iterate through all compatible Ids
      std::vector<node_id_t> :: const_iterator jt;
      for (jt = compatIDs.begin(); jt != compatIDs.end(); ++jt){
	const Node * nb = acdfg_b -> getNodeFromID(*jt);
	long id_b = *jt;
	std::vector<long> outgoingEdges_nb = acdfg_b -> getOutgoingEdgeIDs(id_b);
	addAllCompatibleEdges(outgoingEdges_na, outgoingEdges_nb);
      }
    }
  }

  void IlpApproxIsomorphism::createVariablesForCompatibleNodes(){
    compatible_node_map_t::const_iterator it;
    vector<node_id_t>::const_iterator jt;
    
    for (it = node_map_a_to_b.begin();
	 it != node_map_a_to_b.end();
	 ++it){
      node_id_t srcNodeID = it -> first;
      vector<node_id_t> const & compats = it -> second;
      for (jt = compats.begin(); jt!= compats.end(); ++jt){
	milp.createIsoNodeVariable(srcNodeID, *jt);
	milp.createIsoWtVariable(srcNodeID,*jt);
      }
    }
  }

  void IlpApproxIsomorphism::createVariablesForCompatibleEdges(){
    std::vector<edge_pair_t> :: const_iterator it;
    for (it = compat_edges_a_to_b.begin(); it != compat_edges_a_to_b.end(); ++it){
      milp.createIsoEdgeVariable(it -> first, it -> second);
    }
  }

  void IlpApproxIsomorphism::addUniqueMatchingConstraint(char a_or_b){
    compatible_node_map_t::const_iterator it;
    vector<node_id_t>::const_iterator jt;
  
    if (a_or_b == 'a'){
      for (it = node_map_a_to_b.begin();
	   it != node_map_a_to_b.end();
	   ++it){
	expr_t eqExpr;
	node_id_t srcNodeID = it -> first;
	vector<node_id_t> const & compats = it -> second;
	for (jt = compats.begin(); jt!= compats.end(); ++jt){
	  int vid = milp.lookupIsoNodeVariable(srcNodeID,*jt);
	  eqExpr[vid] = 1.0;
	}
	milp.addLeq(eqExpr,1.0);
      }
    } else {
      for (it = node_map_b_to_a.begin();
	   it != node_map_b_to_a.end();
	   ++it){
	expr_t eqExpr;
	node_id_t srcNodeID = it -> first;
	vector<node_id_t> const & compats = it -> second;
	for (jt = compats.begin(); jt!= compats.end(); ++jt){
	  int vid = milp.lookupIsoNodeVariable(*jt,srcNodeID); //Crucial reverse the order of variables a first b second.
	  eqExpr[vid] = 1.0;
	}
	milp.addLeq(eqExpr,1.0);
      }
    }
  }

  void IlpApproxIsomorphism::addWeightOfCompatibleNodesEquation(){
    compatible_node_map_t::const_iterator it;
    vector<node_id_t>::const_iterator jt;
     for (it = node_map_a_to_b.begin();
	  it != node_map_a_to_b.end();
	  ++it){ // Iterate through nodes in graph A
       // Iterate through the list of compatible nodes in graph B
       vector<node_id_t> const & compats = it -> second;
       node_id_t i = it -> first;
       
       for (jt = compats.begin(); jt!= compats.end(); ++jt){
	 // i, j are the two nodes we are now working with

	 node_id_t j = *jt;

	 // Retreive the nodes
	 Node * na = acdfg_a -> getNodeFromID(i);
	 Node * nb = acdfg_b -> getNodeFromID(j);
	 
	 // Lookup variables xij and wij
	 int wij = milp.lookupIsoWtVariable(i,j);
	 int xij = milp.lookupIsoNodeVariable(i,j);
	 assert(na -> get_type() == nb -> get_type());

	 
	 // Add constraints
	 //   (a) w_i_j - x_i_j <= 0, this ensures that w_i_j can be 1 only if x_i_j is.
	 
	 expr_t eqExprA;
	 eqExprA[wij] = 1.0;
	 eqExprA[xij] = -1.0;
	 milp.addLeq(eqExprA, 0.0);
	 
	 
	 
	 if (na -> get_type() == METHOD_NODE){ // they are both method nodes?
	   assert(nb -> get_type() == METHOD_NODE);
	   //  Move on to the next constraint
	   //  Get the method nodes for i and j
	   MethodNode * methA = toMethodNode(na); // Get the method nodes
	   MethodNode * methB = toMethodNode(nb);
	   
	   //  (b) The receiver nodes must be compatible
	   expr_t eqExprB; // A new expression 
	   eqExprB[wij] = 1.0; // set wij coefficient to 1
	   const DataNode* receiverA = methA -> get_receiver(); // receiver for A
	   const DataNode* receiverB = methB -> get_receiver(); // receiver for B
	   
	   if (receiverA != NULL && receiverB != NULL){ // If both receivers exist
	     node_id_t rA = receiverA -> get_id(); 
	     node_id_t rB = receiverB -> get_id();
	     if (areCompatibleNodes(rA, rB)){ // Are they compatible with each other?
	       int rij = milp.lookupIsoNodeVariable(rA, rB); // If yes. add constraint w_i_j <= r_i_j
	       eqExprB[rij] = -1.0;
	       milp.addLeq(eqExprB, 0.0); // Done.
	     } else { // If not compatible
	       if (debug){
		 std::cout << "Warning incompatible receivers for method nodes ("<< i << " , " << j << ")" << std::endl; 
	       }
	       milp.addEq(eqExprB, 0.0);// wij = 0
	     }
	   } else if ( (receiverA == NULL && receiverB != NULL) || (receiverA != NULL && receiverB == NULL)){
	     // Else if only one receiver exists
	     if (debug){
	       std::cout << "Warning: For method nodes ("<< i << " , " << j << "): one has receiver but other receiver is NULL" << std::endl;
	     }
	     milp.addEq(eqExprB,0.0); // Set w_ij == 0
	   } else {
	     // Both receivers are null, so do not do anything about w_i_j here
	   }
	   
	   // (c) n*  w_i_j <= \sum_{i=1}^n x_argi_argj 

	   const std::vector<DataNode*> & argsA = methA -> get_arguments();
	   const std::vector<DataNode*> & argsB = methB -> get_arguments();
	   expr_t eqExprC;
	   std::vector<DataNode*>::const_iterator kt,lt;
	   if (argsA.size() == argsB.size()){
	     eqExprC[wij] = (float) argsA.size();
	     for (kt = argsA.begin(), lt = argsB.begin();
		  kt != argsA.end() && lt != argsB.end();
		  ++kt, ++lt){
	       DataNode * arg_a = *kt;
	       DataNode * arg_b = *lt;
	       node_id_t kid = arg_a -> get_id(), lid = arg_b-> get_id();
	       if (areCompatibleNodes(kid, lid)){
		 int vkl = milp.lookupIsoNodeVariable(kid, lid);
		 eqExprC[vkl]= -1.0;
	       }
	     }
	     milp.addLeq(eqExprC,0.0);
	   } else {
	     // w_i_j == 0
	     eqExprC[wij] = 1.0;
	     milp.addEq(eqExprC,0.0);
	   }
	 }  else if ( na-> get_type() == DATA_NODE){
	   // Iterate through all compatible edges whose sources and destinations match na, nb
	   // n* w_i_j <= \sum compatible incident edges
	   std::vector<edge_pair_t>::const_iterator mt;
	   int compatibleIncidentEdgeCount = 0;
	   expr_t eqExprD;
	   for (mt = compat_edges_a_to_b.begin(); mt != compat_edges_a_to_b.end(); ++mt){
	     edge_id_t eAID = mt -> first;
	     edge_id_t eBID = mt -> second;
	     Edge * eA = acdfg_a -> getEdgeFromID(eAID);
	     Edge * eB = acdfg_b -> getEdgeFromID(eBID);
	     if (eA -> get_src_id() == i && eB -> get_src_id() == j){
	       compatibleIncidentEdgeCount ++;
	       int vAB = milp.lookupIsoEdgeVariable(eAID, eBID);
	       eqExprD[vAB] = -1.0;
	     }

	     if (eA -> get_dst_id() == i && eB -> get_dst_id() == j){
	       compatibleIncidentEdgeCount ++;
	       int vAB = milp.lookupIsoEdgeVariable(eAID, eBID);
	       eqExprD[vAB] = -1.0;
	     }
	    
	   } // for (mt ..)
	   if (compatibleIncidentEdgeCount > 0){
	     eqExprD[wij] = (float) compatibleIncidentEdgeCount;
	     milp.addLeq(eqExprD,0.0);
	   } else {
	     eqExprD[wij] = 1.0;
	     milp.addEq(eqExprD, 0.0);
	   }
	 }
       }
     }
  }

  void IlpApproxIsomorphism::addEdgeSourceTargetMatch(){
    // 1. Iterate through all compatible edge pairs.
    // 2. For each pair, 2* x_i_k_j_l <= x_ij + x_kl
    std::vector<edge_pair_t>::const_iterator mt;	   
    for (mt = compat_edges_a_to_b.begin(); mt != compat_edges_a_to_b.end(); ++mt){
      edge_id_t eAID = mt -> first;
      edge_id_t eBID = mt -> second;
      Edge * eA = acdfg_a -> getEdgeFromID(eAID);
      Edge * eB = acdfg_b -> getEdgeFromID(eBID);
      node_id_t i_a = eA -> get_src_id();
      node_id_t i_b = eB -> get_src_id();
      node_id_t j_a = eA -> get_dst_id();
      node_id_t j_b = eB -> get_dst_id();
      assert(areCompatibleNodes(i_a,i_b));
      assert(areCompatibleNodes(j_a,j_b));
      int vi = milp.lookupIsoNodeVariable(i_a,i_b);
      int vj = milp.lookupIsoNodeVariable(j_a,j_b);
      int eij = milp.lookupIsoEdgeVariable(eAID, eBID);
      expr_t eqExpr;
      eqExpr[eij] = 2.0;
      eqExpr[vi] = -1.0;
      eqExpr[vj] = -1.0;
      milp.addLeq(eqExpr,0.0);
    }
  }

  void IlpApproxIsomorphism::addDataNodeMustNotBeIsolated(){
    // 1. Iterate through all compatible pairs data nodes i,j
    // 2. For each pair of nodes , iterate through all compatible edges that are incident on the pair.
    //  Add constraint that x_i_j \leq \sum_{incidentEdgePairs kl} e_kl
    compatible_node_map_t::const_iterator it;
    vector<node_id_t>::const_iterator jt;
    
    for (it = node_map_a_to_b.begin();
	 it != node_map_a_to_b.end();
	 ++it){ // Iterate through all nodes
      node_id_t i = it -> first;
      Node * na = acdfg_a -> getNodeFromID(i);
      if (na -> get_type() == DATA_NODE){ // Is it a data node?
	vector<node_id_t> const & compats = it -> second; 
	for (jt = compats.begin(); jt!= compats.end(); ++jt){ // Iterate through all compatible nodes
	  node_id_t j = *jt;
	  Node * nb = acdfg_b -> getNodeFromID(j);
	  assert(nb -> get_type() == DATA_NODE);
	  int xij = milp.lookupIsoNodeVariable(i,j); 
	  expr_t eqExpr;
	  eqExpr[xij] = 1.0; 
	  std::vector<edge_pair_t>::const_iterator mt;	   
	  for (mt = compat_edges_a_to_b.begin(); mt != compat_edges_a_to_b.end(); ++mt){
	    // Iterate through all compatible pair of edges.
	    edge_id_t eAID = mt -> first;
	    edge_id_t eBID = mt -> second;
	    Edge * eA = acdfg_a -> getEdgeFromID(eAID);
	    Edge * eB = acdfg_b -> getEdgeFromID(eBID);
	    node_id_t i_a = eA -> get_src_id();
	    node_id_t i_b = eB -> get_src_id();
	    node_id_t j_a = eA -> get_dst_id();
	    node_id_t j_b = eB -> get_dst_id();
	    if ((i_a == i && i_b == j) || (j_a == i && j_b == j)) { // If the edge is incident on i,j
	      int eij = milp.lookupIsoEdgeVariable(eAID, eBID); 
	      eqExpr[eij] = -1.0;
	    }
	  }
	  milp.addLeq(eqExpr,0.0);
	}
      }
    }
  }

  void IlpApproxIsomorphism::createObjectiveFunction(){
    expr_t objExpr;
    // Obj = sum x_i_j + sum w_i_j + sum e_a_b
    compatible_node_map_t::const_iterator it;
    vector<node_id_t>::const_iterator jt;
     for (it = node_map_a_to_b.begin();
	  it != node_map_a_to_b.end();
	  ++it){ // Iterate through nodes in graph A
       // Iterate through the list of compatible nodes in graph B
       vector<node_id_t> const & compats = it -> second;
       node_id_t i = it -> first;
       Node * na = acdfg_a -> getNodeFromID(i);
       if (na -> get_type() != REGULAR_NODE){
	 for (jt = compats.begin(); jt!= compats.end(); ++jt){
	   // i, j are the two nodes we are now working with
	   node_id_t j = *jt;
	   int xij = milp.lookupIsoNodeVariable(i,j);
	   int wij = milp.lookupIsoWtVariable(i,j);
	   objExpr[xij] = 1.0;
	   objExpr[wij] = 1.0;
	 }
       }
     }

     std::vector<edge_pair_t>::const_iterator mt;	   
     for (mt = compat_edges_a_to_b.begin(); mt != compat_edges_a_to_b.end(); ++mt){
       edge_id_t eA = mt -> first;
       edge_id_t eB = mt -> second;
       int eij = milp.lookupIsoEdgeVariable(eA,eB);
       objExpr[eij] = 1.0;
     }

     milp.setObjective(objExpr);
  }
  
  void IlpApproxIsomorphism::initializeMILP(){
    // 1. Create variables that we will need.
    // Each variable will correspond to the compatible nodes and edges.
    createVariablesForCompatibleNodes();
    createVariablesForCompatibleEdges();
    
    // 2. Create the constraints for each category of constraints.
    //  2.1 A node cannot be mapped to more than one other node
    addUniqueMatchingConstraint('a');
    addUniqueMatchingConstraint('b');
    
    
    //  2.2 An edge cannot be mapped to more than one other compatible edge.
    // UPDATE: Sriram thinks this constraint is directly implied by the others.
    
    //  2.3 The weight of a matching between two compatible nodes must equal the sum of compatible arguments + compatible return value.
    addWeightOfCompatibleNodesEquation();
    
    //  2.4 Each edge can only match if the source/target of the edges match.
    addEdgeSourceTargetMatch();
    
    
    //  2.5 Every data node match must mean that at least one incoming or outgoing edge is matched.
    addDataNodeMustNotBeIsolated();
    

    // 3. Create the objective function.
    createObjectiveFunction();
    

    // That is it for the problem.
    
  }
  
  void IlpApproxIsomorphism::computeILPEncoding(){
    // 1. Compute the compatible nodes
    computeCompatibleNodes();
    // 2. Compute the pair of compatible edges
    computeCompatibleEdges();
    // 3. Create variables for the encoding
    initializeMILP();
    // 4. Print AMPL

    milp.prettyPrintAMPLFormat(std::cout);
  }


  
  
} // namespace
