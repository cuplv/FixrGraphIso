#include "fixrgraphiso/milpProblem.h"
#include <sstream>
#include <glpk.h>

using namespace std;
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
    ret.binVal =0;
    ret.floatVal = -112989189391.0;
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


  int MILProblem::createRowFromExpr(expr_t const & e, int * ind, double* val){
    int len  = 0;
    expr_t::const_iterator it;
    for (it = e.begin(); it != e.end(); ++it){
      int id = it -> first;
      float c  = it -> second;
      len = len +1;
      ind[len] = 1 + id;
      val[len] = c;
    }
    // Clear the rest of the entries
    for (int i = len+1; i < numVariables; ++i){
      ind[i] = 0;
      val[i] = 0.0;
    }

    return len;
  }
  void MILProblem::solveUsingGLPKLibrary(const bool debug){
    glp_prob * lp;
    lp = glp_create_prob();
    int nRows = (int) (this -> ineqs.size() + this -> eqs.size());
    int nCols = this -> numVariables;
    glp_add_rows(lp, nRows);
    glp_add_cols(lp, nCols);

    // Declare the variable types.
    std::map<int, MILPVariable>::iterator it;
    for (it = id2Variable.begin(); it != id2Variable.end(); ++it){

      // Iterate through all variables
      MILPVariable var = it -> second;

      glp_set_col_name(lp, 1+ var.id, var.name.c_str()); // set the column name

      switch( var.typ ){
      case ISO_NODE:
      case ISO_EDGE:{
        // set it to a binary variable
        glp_set_col_kind(lp,1 + var.id, GLP_BV);
        break;
      }
      case ISO_WT: {
        // set it to a continuous variable with limits between 0 and 1
        glp_set_col_kind(lp,1 + var.id, GLP_CV);
        glp_set_col_bnds(lp, 1+ var.id, GLP_DB, 0.0, 1.0);
        break;
      }
      default: {
        // We have a unhandled type -- this should never happen!
        assert(false);
        break;

      }
      } // switch
    } // for it = ..
    int * ind = new int[nCols+1];
    double * val = new double[nCols + 1];

    int i = 0;
    std::vector<constr_t>::iterator jt;
    // Add equalities
    for (jt = eqs.begin(); jt != eqs.end(); ++jt){
      i = i +1; // increment the row count
      expr_t e = jt -> first;
      float f = jt -> second;
      // e == f
      int len = createRowFromExpr(e, ind, val);
      glp_set_mat_row(lp, i, len, ind, val);
      glp_set_row_bnds(lp, i, GLP_FX, f, f);
    }
    // Add the inequalities
    for (jt = ineqs.begin(); jt != ineqs.end(); ++jt){
      i = i +1; // increment the row count
      expr_t e = jt -> first;
      float f = jt -> second;
      // e <= f
      int len = createRowFromExpr(e, ind, val);
      glp_set_mat_row(lp, i, len, ind, val);
      glp_set_row_bnds(lp, i, GLP_UP, 0.0, f);
    }
    // Set the objective.
    expr_t::const_iterator kt;
    for (kt = obj.begin(); kt != obj.end(); ++kt){
      int id = kt -> first;
      float val = kt -> second;
      glp_set_obj_coef(lp, 1 + id, val);
    }

    glp_set_obj_dir(lp, GLP_MAX); // set it to a maximization problem

    // Now let's solve the problem.
    glp_iocp params;
    glp_init_iocp(&params);
    params.presolve = GLP_ON;
    params.gmi_cuts=GLP_ON;
    params.mir_cuts=GLP_ON;
    params.cov_cuts=GLP_ON;
    int rVal = glp_intopt(lp, &params);
    if (rVal == 0){
      std::cout << "Problem sucessfully solved ! " << std::endl;
    } else {
      std::cout << " GLPK bailed with error code " << std::endl;
      assert(false);
    }

    int stat = glp_mip_status(lp);
    switch (stat){
    case GLP_OPT:
      {
        std::cout << "Optimal solution found " << std::endl;

      }
      break;
    case GLP_FEAS:
      {
        std::cout << "Solver could not find optimal integer solution due to premature termination, perhaps " << std::endl;
        assert(false);
      }
      break;

    case GLP_NOFEAS:
      {
        std::cout << " Problem is primal infeasible " << std::endl;
        assert(false);
      }
      break;

    case GLP_UNDEF:
      {
        std::cout << "Solver bailed out with undefined message " << std::endl;
        assert(false);
      }
      break;

    }


    // Extract the solution from the result, if feasible.
    objValue =  glp_mip_obj_val(lp);
    std:: cout << " \t Objective Value : " << objValue << std::endl;

    for (it = id2Variable.begin(); it != id2Variable.end(); ++it){

      // Iterate through all variables
      MILPVariable & var = it -> second;
      if (debug ) std::cout << " \t " << var.name << " := " ;

      switch( var.typ ){
      case ISO_NODE:
      case ISO_EDGE:{
        // set it to a binary variable
        double v = glp_mip_col_val(lp, 1 + var.id);
        int val;
        assert( (v >= -1e-06 && v <= 1e-06) || (v >= 1.0-1e-06 && v <= 1.0+1e-06));
        if (v <= 1e-06){
          val = 0;
        } else {
          val = 1;
        }
        var.binVal = val;
        if (debug) std::cout << var.binVal << std::endl;
        break;
      }
      case ISO_WT: {
        // set it to a continuous variable with limits between 0 and 1
        double v = glp_mip_col_val(lp, 1 + var.id);
        assert( v >= -1e-06 && v<= 1.0 + 1e-06);
        var.floatVal = v;
        if (debug) std::cout << v << std::endl;
        break;
      }
      default: {
        // We have a unhandled type -- this should never happen!
        assert(false);
        break;
      }
      } // switch
    } // for it = ..
    solvedSuccessfully=true;
    delete[] (ind);
    delete[] (val);
    glp_delete_prob(lp);

  }




#ifdef USE_GUROBI_SOLVER


  GRBLinExpr createGRBLinExprFromExpr(expr_t const & e, std::map<int, GRBVar> id2VarMap){
    expr_t ::const_iterator it;
    GRBLinExpr retExpr = 0.0;
    for (it = e.begin(); it != e.end(); ++ it){
      std::map<int,GRBVar>::const_iterator jt = id2VarMap.find(it -> first);
      assert(jt != id2VarMap.end());
      retExpr += (it -> second) * (jt -> second);
    }
    return retExpr;
  }

  bool MILProblem::solveUsingGurobiLibrary(const bool debug,
                                           const double gurobi_timeout){

    try {
      GRBEnv env = GRBEnv();
      env.set(GRB_DoubleParam_TimeLimit, gurobi_timeout);
      GRBModel m = GRBModel(env);

      std::map<int, GRBVar> id2VarMap;
      // Now declare the variables
      std::map<int, MILPVariable>::iterator it;
      for (it = id2Variable.begin(); it != id2Variable.end(); ++it){
        MILPVariable var = it -> second;
        switch (var.typ){
        case ISO_NODE:
        case ISO_EDGE:{
          // set it to a binary variable
          GRBVar v = m.addVar(0.0, 1.0, 0.0, GRB_BINARY, var.name.c_str());
          id2VarMap[var.id] = v;
          break;
        }

        case ISO_WT: {
          GRBVar v = m.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, var.name.c_str());
          id2VarMap[var.id] = v;
          break;
        }
        default: {
          // We have a unhandled type -- this should never happen!
          assert(false);
          break;
        }
        }// switch
      }// for (it = ..
      m.update();
      // Now let us add the constraints
      std::vector<constr_t>::iterator jt;
      for (jt = eqs.begin(); jt != eqs.end(); ++jt){
        expr_t e = jt -> first;
        float f = jt -> second;
        GRBLinExpr lC = createGRBLinExprFromExpr(e,id2VarMap);
        m.addConstr(lC == f);
      }

      for (jt = ineqs.begin(); jt != ineqs.end(); ++jt){
        expr_t e = jt -> first;
        float f = jt -> second;
        GRBLinExpr lC = createGRBLinExprFromExpr(e,id2VarMap);
        m.addConstr(lC <= f);
      }
      // Set the objective
      GRBLinExpr objExpr = createGRBLinExprFromExpr(obj,id2VarMap);
      m.setObjective(objExpr, GRB_MAXIMIZE);
      //Solve the thing
      m.optimize();
      int optimstatus = m.get(GRB_IntAttr_Status);
      // Retrieve the solution
      if (optimstatus == GRB_OPTIMAL){
        std::cout << "Gurobi successfully optimized " << std::endl;
        float objValue = m.get(GRB_DoubleAttr_ObjVal);
        std:: cout << " \t Objective Value : " << objValue << std::endl;

        for (it = id2Variable.begin(); it != id2Variable.end(); ++it){

          // Iterate through all variables
          MILPVariable & var = it -> second;
          if (debug) std::cout << " \t " << var.name << " := " ;
          map<int, GRBVar>::iterator vt = id2VarMap.find(var.id);
          assert(vt != id2VarMap.end());
          GRBVar gVar = vt -> second;
          double v = gVar.get(GRB_DoubleAttr_X);

          switch( var.typ ){
          case ISO_NODE:
          case ISO_EDGE:{
            // set it to a binary variable
            int val;
            assert( (v >= -1e-06 && v <= 1e-06) || (v >= 1.0-1e-06 && v <= 1.0+1e-06));
            if (v <= 1e-06){
              val = 0;
            } else {
              val = 1;
            }
            var.binVal = val;
            if (debug) std::cout << var.binVal << std::endl;
            break;
          }
          case ISO_WT: {
            // set it to a continuous variable with limits between 0 and 1
            assert( v >= -1e-06 && v<= 1.0 + 1e-06);
            var.floatVal = v;
            if (debug) std::cout << v << std::endl;
            break;
          }
          default: {
            // We have a unhandled type -- this should never happen!
            assert(false);
            break;
          }
          } // switch
        } // for it = ..
        solvedSuccessfully=true;
      } else {
        switch (optimstatus) {
        case GRB_INF_OR_UNBD:
          std::cout << "Model is either infeasible or unbounded but not sure which! " << std::endl;
          break;
        case GRB_INFEASIBLE:
          std::cout << "Model is infeasible " << std::endl;
          break;

        case GRB_UNBOUNDED:
          std::cout << "Model is unbounded " << std::endl;
          break;

        case GRB_TIME_LIMIT:
          std::cout << "Time limit exceeded " << std::endl;
          solvedSuccessfully = false;
          break;
        default:
          std::cout << "Optimization was stopped with status = "
                    << optimstatus << std::endl;
          break;
        }
        solvedSuccessfully=false;

      }

    }  catch (GRBException e){
      std::cerr << "Exit with error code " << e.getErrorCode() << std::endl;
      std::cerr << e.getMessage() << std::endl;
    }
    return solvedSuccessfully;
  }

#endif

}
