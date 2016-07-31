/*
 * Structure for encoding a mixed ilp problem
 */

#include <vector>
#include <map>
#include <ostream>
#include <iostream>
#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso {

  typedef enum { ISO_NODE, ISO_WT, ISO_EDGE } ilp_variable_t;
  extern int numVariables;

  typedef std::map<int, float> expr_t;
  
  struct MILPVariable {
    ilp_variable_t typ;
    int id;
    string name;
    int binVal;
    double floatVal;
    
  };
 
  typedef std::pair<node_id_t, node_id_t> node_pair_t;
  typedef std::pair<edge_id_t, edge_id_t> edge_pair_t;
  typedef std::pair<expr_t, float> constr_t;
  
  class MILProblem {
  public:
   
  MILProblem():numVariables(0) {}; // Constructor
    MILPVariable createIsoNodeVariable(node_id_t i, node_id_t j);
    MILPVariable createIsoWtVariable(node_id_t i, node_id_t j);
    MILPVariable createIsoEdgeVariable(edge_id_t i, edge_id_t j);
   
    int lookupIsoNodeVariable(node_id_t i, node_id_t j){
      return lookupIsoVariableInMap(isoNodes,i,j);
    }
    int lookupIsoWtVariable(node_id_t i, node_id_t j){
      return lookupIsoVariableInMap(isoWts, i, j);
    }
    int lookupIsoEdgeVariable(edge_id_t i, edge_id_t j){
      return lookupIsoVariableInMap(isoEdges, i, j);
    }
   
    void addEq(expr_t eqExpr, float rhs)
    {
      constr_t c(eqExpr, rhs);
      eqs.push_back(c);
    }
    void addLeq(expr_t eqExpr, float rhs){
      constr_t c(eqExpr, rhs);
      ineqs.push_back(c);
    }
    void setObjective(expr_t eqExpr){
      this -> obj = eqExpr;
    }


    void prettyPrintAMPLFormat(std::ostream & out);

    void solveUsingGLPKLibrary();
    
    
    MILPVariable getVariableFromID(int i){
      std::map<int, MILPVariable>::iterator it =  id2Variable.find(i);
      assert(it != id2Variable.end());
      return it -> second;
    }
  private:
    int numVariables;
    std::map<int, MILPVariable> id2Variable;
    std::vector<constr_t > ineqs;// lhs <= rhs
    std::vector<constr_t> eqs;

    std::map<node_pair_t, MILPVariable> isoNodes;
    std::map<node_pair_t, MILPVariable> isoWts;
    std::map<edge_pair_t, MILPVariable> isoEdges;
    expr_t obj;
    
    int lookupIsoVariableInMap( std::map<node_pair_t, MILPVariable> const & mp, node_id_t i, node_id_t j);
    MILPVariable createVariable(ilp_variable_t typ, long i, long j);
    void prettyPrintVariable(std::ostream & out, MILPVariable & var);
    void prettyPrintExpr(std::ostream & out,  expr_t e);
    int createRowFromExpr(expr_t const & e, int * ind, double* val);
    
  };

  
 
}

