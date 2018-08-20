// -*- C++ -*-
//
// Encodes the approximate graph isomorphism in MaxSAT
//
// Author: Sergio Mover
//


#include <string>
#include <cstring>
#include <sstream>
#include <typeinfo>
#include <iterator>

#include "fixrgraphiso/iso.h"

namespace fixrgraphiso {

  IsoSolver::~IsoSolver()
  {
    if (last_isomorphism != NULL) {
      delete last_isomorphism;
    }

    for (std::vector<string*>::const_iterator it = var_names.begin();
         it != var_names.end(); ++it) {
      delete (*it);
    }
  }

  bool IsoSolver::is_iso()
  {
    // Check if precisely isomorphic using SAT?

    z3::solver solver(z3_context); // Initialize Z3 context

    std::vector<z3::expr> nodes_iso; // Expressions corresponding to isomorphic node pairs
    std::vector<z3::expr> edges_iso; // Corresponding to isomorphic edge pairs
    std::vector<z3::expr> unique; // Express uniqueness of the isomorphism
    std::vector<z3::expr> nodes_iso_vars; // variables for node pairs
    std::vector<z3::expr> edges_iso_vars; // variables for edge pairs

    // Cannot be iso if it does not have the same number of nodes and
    // edges
    if (acdfg_a.node_count() != acdfg_b.node_count()) return false;
    if (acdfg_a.edge_count() != acdfg_b.edge_count()) return false;

    // Get the conjuncts of the encodings
    get_encoding(nodes_iso, edges_iso, unique,
                 nodes_iso_vars, edges_iso_vars);

    // assert all the constraints as hard constraints
    for (std::vector<z3::expr>::const_iterator it = nodes_iso.begin();
         it != nodes_iso.end(); ++it) solver.add(*it);
    for (std::vector<z3::expr>::const_iterator it = edges_iso.begin();
         it != edges_iso.end(); ++it) solver.add(*it);
    for (std::vector<z3::expr>::const_iterator it = unique.begin();
         it != unique.end(); ++it) solver.add(*it);

    // The number of iso variable for nodes must be equal to
    // the total number of nodes
    z3::expr n_iso_nodes = exactly_n(nodes_iso_vars, acdfg_a.node_count());
    solver.add(n_iso_nodes);

    // if the source and dst nodes are iso, we must match also the edges
    for (edges_t::const_iterator it_a =  acdfg_a.begin_edges();
         it_a != acdfg_a.end_edges(); ++it_a) {
      Edge& edge_a = *(*it_a);

      for (edges_t::const_iterator it_b =  acdfg_b.begin_edges();
           it_b != acdfg_b.end_edges(); ++it_b) {
        Edge& edge_b = *(*it_b);

        z3::expr iso_var_edge = get_iso_var(edge_a, edge_b);
        z3::expr iso_var_src = get_iso_var(*edge_a.get_src(), *edge_b.get_src());
        z3::expr iso_var_dst = get_iso_var(*edge_a.get_dst(), *edge_b.get_dst());

        solver.add((!iso_var_src) || iso_var_edge);
        solver.add((!iso_var_dst) || iso_var_edge);
      }
    }


    {
      z3::check_result res;
      res = solver.check();
      assert(res != z3::unknown);

      if (res == z3::sat) {
        Isomorphism* iso = get_isomorphism(solver.get_model());
        set_last_iso(iso);
      }

      return res == z3::sat;
    }
  }

  z3::expr IsoSolver::exactly_n(std::vector<z3::expr>& formulas,
                                const int n) {
    assert (n <= formulas.size());
    z3::expr res = z3_context.bool_val(false);

    for (std::vector<z3::expr>::const_iterator it = formulas.begin();
         it != formulas.end(); ++it) {
      std::vector<z3::expr>::const_iterator it2 = it;

      for (++it2; it2 != formulas.end(); ++it2) {
        int size = 1;
        z3::expr res2 = (*it);

        // build the n-length conjunction
        for (std::vector<z3::expr>::const_iterator it3 = it2;
             it3 != formulas.end() && size < n; ++it3) {
          res2 = res2 && (*it3);
          size = size + 1;
        }

        // enough elements
        if (size == n) res = res || res2;
      }
    }
    return res;
  }

  bool IsoSolver::get_max_embedding()
  {
    z3::optimize opt(z3_context);

    std::vector<z3::expr> nodes_iso;
    std::vector<z3::expr> edges_iso;
    std::vector<z3::expr> unique;
    std::vector<z3::expr> nodes_iso_vars;
    std::vector<z3::expr> edges_iso_vars;

    // Get the conjuncts of the encodings
    get_encoding(nodes_iso, edges_iso, unique,
                 nodes_iso_vars, edges_iso_vars);

    // Hard constraints
    for (std::vector<z3::expr>::const_iterator it = nodes_iso.begin();
         it != nodes_iso.end(); ++it) opt.add(*it);
    for (std::vector<z3::expr>::const_iterator it = edges_iso.begin();
         it != edges_iso.end(); ++it) opt.add(*it);
    for (std::vector<z3::expr>::const_iterator it = unique.begin();
         it != unique.end(); ++it) opt.add(*it);

    // Soft constraints - to be maximized
    for (std::vector<z3::expr>::const_iterator it = nodes_iso_vars.begin();
         it != nodes_iso_vars.end(); ++it) opt.add(*it, 1);
    for (std::vector<z3::expr>::const_iterator it = edges_iso_vars.begin();
         it != edges_iso_vars.end(); ++it) opt.add(*it, 1);

    {
      z3::check_result res = opt.check();
      assert(res != z3::unknown);

      if (res == z3::sat) {
        Isomorphism* iso = get_isomorphism(opt.get_model());
        set_last_iso(iso);
      }

      return res == z3::sat;
    }
  }

  Isomorphism& IsoSolver::get_last_isomorphism()
  {
    assert(NULL != last_isomorphism);
    return *last_isomorphism;
  }

  /*
   * MaxSAT encoding
   *
   * Hard constraints:
   *   1. Node isomorphism conditions
   *     If v1 is isomorphic to v2 (iso_v1_v2), then label(v1) =
   *     label(v2)
   *     If the label contains a node, = is interpreted as
   *     isomorphism. For example, if a node is a method call, then the
   *     predicate = among labels hold if the arguments to the method
   *     call (that are nodes) are isomorphic.
   *
   *   2. Edge isomorphism condition
   *     If e1 is isomorphism to e2 (iso_e1_e2), then:
   *       - iso_src(e1)_src_(e2) and iso_dst(e1)_dst_(e2)
   *         Source and destination nodes of the edges are matched in
   *         the isomorphism.
   *       - label(e1) = label(e2)
   *
   *   3. Force the ismorphism to be a partial function:
   *     - For nodes: iso_vi_vj => ! (/\_{z != 1} iso_vi_vz)
   *     - For edges: iso_ei_ej => ! (/\_{z != 1} iso_ei_ez)
   *
   *   Injectivity of the partial function is obtained by construction
   *     (i.e. there is a variable iso_vi_vj but no variable iso_vj_vi,
   *     and the isomorphim relation is unique)
   *
   *  Soft constraints:
   *    Isomorphism variables: iso_vi_vj and iso_el_vz for nodes and
   *    edges
   *
   *  The distinction of soft and hard constraints is done when
   *  asserting the formulas in the solver.
   *
   */
  void IsoSolver::get_encoding(std::vector<z3::expr>& nodes_iso,
                               std::vector<z3::expr>& edges_iso,
                               std::vector<z3::expr>& uniqueness_constraints,
                               std::vector<z3::expr>& nodes_iso_vars,
                               std::vector<z3::expr>& edges_iso_vars)
  {
    // Uniqueness constraints (3) and soft constraints are encoded with
    // the isomorphism condition
    uniqueness_constraints.clear();
    nodes_iso_vars.clear();
    edges_iso_vars.clear();

    /* 1. Node isomorphism conditions.
     * For all pairs of nodes (that could match) iso_node_v0_v1
     * We have a single variable for (v0,v1) and (v1,v0) under the
     * injectivity assumption
     *
     * iso_node_v0_v1 -> (label(v0) = label(v1))
     */
    nodes_iso.clear();

    // Iterate through all the nodes in acdfg_a

    for (nodes_t::const_iterator it_a =  acdfg_a.begin_nodes();
         it_a != acdfg_a.end_nodes(); ++it_a) {
      Node& node_a = *(*it_a); // For each node

      // Start from the next element
      // Iterate through all nodes in acdfg_b
      for (nodes_t::const_iterator it_b = acdfg_b.begin_nodes();
           it_b != acdfg_b.end_nodes(); ++it_b) {
        Node& node_b = *(*it_b);

        z3::expr iso_var = get_iso_var(node_a, node_b); // Get the iso_var corresponding to the pair node_a, node_b
        nodes_iso_vars.push_back(iso_var);

        if (may_match(node_a, node_b)) {
          nodes_t::const_iterator it_c = it_b;
          z3::expr iso_eq = get_iso_eq(node_a, node_b);

          nodes_iso.push_back((! iso_var) || iso_eq);

          // Uniqueness constraint
          // iso_var_a_b => ! iso_var_a_c, for all c > b
          for (++it_c; it_c != acdfg_b.end_nodes(); ++it_c) {
            Node& node_c = *(*it_c);
            z3::expr iso_var_a_c = get_iso_var(node_a, node_c);
            // iso_var -> ! (iso_var_a_c);
            uniqueness_constraints.push_back( (! iso_var) || (! iso_var_a_c) );
          } // Uniqueness loop
        } // may match
        else {
          // The match is not possible (the labels are already
          // different)
          // This can be simplified (directly in the encoding or by
          // inlining)
          nodes_iso.push_back(! iso_var);
        }
      } // inner loop on nodes
    } // outer loop on nodes

    // 2. Encode the constraints of iso_edge_e0_e1
    /* For all pairs of edges (that could match) iso_edge_e0_e1
     * Also here is not all the pairs, but we exploit injectivity.
     *
     * iso_edge_e1_e2 -> (iso_node_v0_v1 & iso_node_v1_v2 &
     *                    label(e1) = label(e2))
     *
     * WARNING label(e1) = label(e2) may be defined recursively on
     * other isomorphism variables for nodes
     */
    edges_iso.clear();
    for (edges_t::const_iterator it_a =  acdfg_a.begin_edges();
         it_a != acdfg_a.end_edges(); ++it_a) {
      Edge& edge_a = *(*it_a);

      for (edges_t::const_iterator it_b =  acdfg_b.begin_edges();
           it_b != acdfg_b.end_edges(); ++it_b) {
        Edge& edge_b = *(*it_b);

        z3::expr iso_var = get_iso_var(edge_a, edge_b);
        edges_iso_vars.push_back(iso_var);

        if (may_match(edge_a, edge_b)) {
          edges_t::const_iterator it_c = it_b;
          z3::expr iso_eq = get_iso_eq(edge_a, edge_b);

          edges_iso.push_back((! iso_var) || iso_eq);

          // Uniquenes
          for (++it_c; it_c != acdfg_b.end_edges(); ++it_c) {
            Edge& edge_c = *(*it_c);
            z3::expr iso_var_a_c = get_iso_eq(edge_a, edge_c);

            // iso_a_b => ! iso
            uniqueness_constraints.push_back( (! iso_var) || (! iso_var_a_c) );
          } // Loop on remaning nodes of b
        }
        else {
          // cannot match
          edges_iso.push_back(! iso_var);
        } // end of can match
      } // loop on b edges
    } // loop on a edges
  }

  void IsoSolver::set_last_iso(Isomorphism* new_iso)
  {
    if (last_isomorphism != NULL) {
      delete last_isomorphism;
    }
    last_isomorphism = new_iso;
  }

  Isomorphism* IsoSolver::get_isomorphism(const z3::model model)
  {
    /* iterates through the model
       We are only interested in the constant of the model (there are no
       other symbols)
    */
    Isomorphism* iso = new Isomorphism(acdfg_a, acdfg_b);
    assert(model.num_consts() == model.size());

    for (unsigned i = 0; i < model.num_consts(); i++) {
      z3::func_decl v = model.get_const_decl(i);
      z3::expr e = model.get_const_interp(v);

      if (e == z3_context.bool_val(true)) {
        string name = v.name().str();
        // [SM] Now we use string comparison, but we need
        // to find the right API in z3 to go from name to expr
        if (0 == name.compare(0, std::strlen("isonode_"), "isonode_")) {
          idPair ids = get_ids(name);
          iso->add_node_map(ids.first, ids.second);
        }
        else if (0 == name.compare(0, std::strlen("isoedge_"), "isoedge_")) {
          idPair ids = get_ids(name);
          iso->add_edge_map(ids.first, ids.second);
        }
        else {
          assert(false);
        }
      }
    }

    return iso;
  }

  z3::expr IsoSolver::get_iso_var(const Node &n_a, const Node &n_b) {
    string* s = NULL;
    idPair p = std::make_pair(n_a.get_id(), n_b.get_id());

    std::map<idPair, string*>::iterator iter = nodes2varname.find(p);

    if (nodes2varname.end() != iter) {
      s = (*iter).second;
      z3::expr iso_var = z3_context.bool_const(s->c_str());
      return iso_var;
    }
    else {
      string* s = get_var_name("isonode", n_a.get_id(),
                               n_b.get_id());
      nodes2varname[p] = s;

      z3::expr iso_var = z3_context.bool_const(s->c_str());
      z3id2nodes[Z3_get_ast_id(z3_context, iso_var)] = p;
      return iso_var;
    }
  }

  z3::expr IsoSolver::get_iso_var(const Edge &e_a, const Edge &e_b)
  {
    string* s = NULL;
    idPair p = std::make_pair(e_a.get_id(), e_b.get_id());

    std::map<idPair, string*>::iterator iter = edges2varname.find(p);

    if (edges2varname.end() != iter) {
      s = (*iter).second;
      z3::expr iso_var = z3_context.bool_const(s->c_str());
      return iso_var;
    }
    else {
      string* s = get_var_name("isoedge", e_a.get_id(),
                               e_b.get_id());
      edges2varname[p] = s;

      z3::expr iso_var = z3_context.bool_const(s->c_str());
      z3id2edges[Z3_get_ast_id(z3_context, iso_var)] = p;
      return iso_var;
    }
  }

  bool IsoSolver::may_match(const Node& n_a, const Node& n_b)
  {
    const std::type_info& t_a = typeid(n_a);
    const std::type_info& t_b = typeid(n_b);

    return t_a == t_b;
  }

  bool IsoSolver::may_match(const DataNode& n_a, const DataNode& n_b)
  {
    const std::type_info& t_a = typeid(n_a);
    const std::type_info& t_b = typeid(n_b);

    return t_a == t_b &&
      n_a.get_data_type() == n_b.get_data_type();
  }

  bool IsoSolver::may_match(const MethodNode& n_a, const MethodNode& n_b)
  {
    const std::type_info& t_a = typeid(n_a);
    const std::type_info& t_b = typeid(n_b);

    return t_a == t_b &&
      n_a.get_name() == n_b.get_name() &&
      n_a.get_arguments().size() == n_b.get_arguments().size();
  }

  bool IsoSolver::may_match(const Edge& e_a, const Edge& e_b)
  {
    const std::type_info& t_a = typeid(e_a);
    const std::type_info& t_b = typeid(e_b);

    return t_a == t_b;
  }


  z3::expr IsoSolver::get_iso_eq(const Node& n_a, const Node& n_b)
  {
    /* nothing to match for instances of:
       - Node
       - DataNode
       - CommandNode
    */
    return z3_context.bool_val(true);
  }

  z3::expr IsoSolver::get_iso_eq(const MethodNode& n_a, const MethodNode& n_b)
  {
    /* On a method node we have to match the receiver and all the
       arguments.
    */
    std::vector<DataNode*> arguments_a = n_a.get_arguments();
    std::vector<DataNode*> arguments_b = n_b.get_arguments();

    assert(arguments_a.size() == arguments_b.size());

    z3::expr match_args = z3_context.bool_val(true);
    {
      std::vector<DataNode*>::const_iterator it_a = arguments_a.begin();
      std::vector<DataNode*>::const_iterator it_b = arguments_b.begin();
      for (; it_a != arguments_a.end(); it_a++, it_b++) {
        match_args = match_args && get_iso_var(*(*(it_a)), *(*(it_b)));
      }
    }

    return get_iso_var(*n_a.get_receiver(), *n_b.get_receiver()) &&
      match_args;
  }

  z3::expr IsoSolver::get_iso_eq(const Edge& e_a, const Edge& e_b)
  {
    // Condition on source and destination nodes, they must be in
    // the isomorphism.
    z3::expr iso_var_src = get_iso_var(*e_a.get_src(), *e_b.get_src());
    z3::expr iso_var_dst = get_iso_var(*e_a.get_dst(), *e_b.get_dst());
    z3::expr iso_eq = iso_var_src && iso_var_dst;

    return iso_eq;
  }


  string* IsoSolver::get_var_name(const char* prefix, long id1, long id2)
  {
    string* s;
    char* var_name;
    string id1_s = get_str(id1);
    string id2_s = get_str(id2);
    var_name = new char[sizeof(char) * id1_s.length() +
                        sizeof(char) * id2_s.length() +
                        sizeof(char) * std::strlen(prefix) +
                        3];
    sprintf(var_name, "%s_%s_%s",
            prefix,
            id1_s.c_str(),
            id2_s.c_str());

    s = new string(var_name);
    delete var_name;

    var_names.push_back(s);

    return s;
  }

  string IsoSolver::get_str(long id)
  {
    std::ostringstream ostr;
    ostr << id;
    return ostr.str();
  }

  long string_to_long(const string& str)
  {
    std::istringstream ss(str);
    long result;
    // ok, id are non-negatives
    bool ok = ss >> result;
    assert(ok);
    return result;
  }

  idPair IsoSolver::get_ids(const string& str)
  {
    size_t first_id;
    size_t second_id;

    first_id = str.find_first_of('_', 0);
    assert (first_id != string::npos);
    first_id = first_id + sizeof(char);
    second_id = str.find_first_of('_', first_id);
    assert (second_id != string::npos);
    second_id = second_id + sizeof(char);

    string first_id_str = str.substr(first_id, second_id - first_id - 1);
    string second_id_str = str.substr(second_id,
                                      (sizeof(char*) * str.length()) - second_id);

    idPair p = std::make_pair(string_to_long(first_id_str),
                              string_to_long(second_id_str));
    return p;
  }


  void Isomorphism::add_node_map(const long id_a, const long id_b)
  {
    node_mapping[id_a] = id_b;
  }

  void Isomorphism::add_edge_map(const long id_a, const long id_b)
  {
    edge_mapping[id_a] = id_b;
  }

  std::ostream& operator<<(std::ostream& stream, const Isomorphism& iso)
  {
    stream << "Isomorphism relation\nNode embeddings: ";

    for(std::map<long,long>::const_iterator it = iso.node_mapping.begin();
        it != iso.node_mapping.end(); it++) {
      if (it != iso.node_mapping.begin()) stream << ", ";
      const Node * a = iso.acdfg_a.getNodeFromID(it -> first);
      const Node * b = iso.acdfg_b.getNodeFromID(it -> second);
      assert( a != NULL && b != NULL);
      stream << "{" ;
      a -> prettyPrint(stream);
      stream << " and " ;
      b -> prettyPrint(stream);
      stream << "};" ;

    }

    stream << "\nEdges embeddings: ";
    for(std::map<long,long>::const_iterator it = iso.edge_mapping.begin();
        it != iso.edge_mapping.end(); it++) {
      if (it != iso.edge_mapping.begin()) stream << ", ";
      stream << "(" << it->first << "," << it->second << ")";
    }
    stream << std::endl;





    return stream;
  }

} // End of namespace
