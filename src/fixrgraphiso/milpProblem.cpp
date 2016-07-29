#include "fixrgraphiso/milpProblem.h"
#include <sstream>
namespace fixrgraphiso{
  
  MILPVariable MILProblem::createVariable(ilp_variable_t typ, node_id_t i, node_id_t j){
    std::ostringstream ss;
    ss << "X_"<<i<<"__"<<j;
    std::string st = ss.str();
    MILPVariable ret;
    ret.typ = ISO_NODE;
    ret.id = numVariables;
    ret.name = st;
    id2Variable[numVariables] = ret;
    numVariables++;
    return ret;
  }

  MILPVariable MILProblem::createIsoNodeVariable(node_id_t i, node_id_t j){
    MILPVariable var = createVariable(ISO_NODE,i,j);
    node_pair_t ij(i, j);
    isoNodes[ij] = var;
    return var;
  }

  MILPVariable MILProblem::createIsoWtVariable(node_id_t i, node_id_t j){
    MILPVariable var = createVariable(ISO_WT,i,j);
    node_pair_t ij(i, j);
    isoWts[ij] = var;
    return var;
  }

  MILPVariable MILProblem::createIsoEdgeVariable(edge_id_t i, edge_id_t j){
    MILPVariable var = createVariable(ISO_EDGE,i,j);
    edge_pair_t ij(i, j);
    isoEdges[ij] = var;
    return var;
  }

  int MILProblem::lookupIsoVariableInMap( std::map<node_pair_t, MILPVariable> const & mp, node_id_t i, node_id_t j){
    node_pair_t ij(i,j);
    
    std::map<node_pair_t, MILPVariable>::const_iterator it = mp.find(ij);
    assert(it != isoNodes.end());
    MILPVariable v = it -> second;
    return v.id;
  }

  
  
    int lookupIsoWtVariable(node_id_t i, node_id_t j);
    int lookupIsoEdgeVariable(edge_id_t i, edge_id_t j);


  

}
