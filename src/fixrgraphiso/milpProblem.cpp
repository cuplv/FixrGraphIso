#include "fixrgraphiso/milpProblem.h"
#include <sstream>
namespace fixrgraphiso{
  
  MILPVariable MILProblem::createVariable(ilp_variable_t typ, node_id_t i, node_id_t j){
    std::ostringstream ss;
    std::string pref="NONE";
    switch(typ){
    case ISO_NODE:
      pref = "X_";
      break;
    case ISO_WT:
      pref = "W_";
      break;
    case ISO_EDGE:
      pref = "E_";
      break;
    }
    ss <<pref<<i<<"__"<<j;
    std::string st = ss.str();
    MILPVariable ret;
    ret.typ = typ;
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

  void MILProblem::prettyPrintExpr(std::ostream & out,   expr_t e){
    expr_t::const_iterator it;
    std::string sep = "";
    bool printed = false;
    for (it = e.begin(); it != e.end(); ++it){
      int id = it -> first;
      MILPVariable var = getVariableFromID(id);
      float c= it -> second;

      if (c < 0.0){
	out << c << " * " << var.name;
	printed = true;
      } else if (c > 0.0){
	out << sep << c << " * " << var.name;
	printed = true;
      }
      sep = "+";
    }

    if (!printed){
      out << "0.0";
    }
  }
  void MILProblem::prettyPrintVariable(std::ostream & out, MILPVariable & var){
    out << "var " << var.name;
    switch (var.typ){
    case ISO_NODE:
    case ISO_EDGE:
      out << " binary; " << std::endl;
      break;
    case ISO_WT:
      out << ", >= 0, <= 1; " << std::endl;
      break;
    default:
      assert(false);
      break;
    }
    return;
  }
  
  void MILProblem::prettyPrintAMPLFormat(std::ostream & out){
    // 1. Print variables
    std::map<int, MILPVariable>::iterator it;
    for (it = id2Variable.begin(); it != id2Variable.end(); ++it){
      MILPVariable var = it -> second;
      prettyPrintVariable(out, var);
    }
      
    // 2. Print Objective
    out << " maximize obj: " ;
    prettyPrintExpr(out, this -> obj);
    out << ";" << std::endl;
    int xCount = 0;
    int exprCount = 0;
    // 3. Print Equality Constraints
    std::vector<constr_t>::iterator jt;
    for (jt = eqs.begin(); jt != eqs.end(); ++jt){
      out << " C__"<< exprCount<<":";
      exprCount ++;
      
      expr_t e = jt -> first;
      prettyPrintExpr(out, e);
      out << " = " << jt -> second << ";" << std::endl;
    }
    
    // 4. Print Inequality Constraints
    for (jt = ineqs.begin(); jt != ineqs.end(); ++jt){
      out << " C__"<< exprCount<<":";
      exprCount ++;
      expr_t e = jt -> first;
      prettyPrintExpr(out, e);
      out << " <= " << jt -> second << ";" << std::endl;
    }

    // 5. Solve

    out << "solve; " << std::endl;

    // 6. Display
    for (it = id2Variable.begin(); it != id2Variable.end(); ++it){
      MILPVariable var = it -> second;
      out << "display \'"<< var.name <<" = \', " << var.name <<";" << std::endl; 
    }
    out << "end; " <<std::endl;
  }
  
  

}
