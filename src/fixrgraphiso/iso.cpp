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
#include "fixrgraphiso/iso.h"

namespace fixrgraphiso {

IsoSolver::~IsoSolver()
{
  if (last_isomorphism != NULL) {
    delete last_isomorphism;
  }
}

bool IsoSolver::is_iso()
{
  z3::solver solver(z3_context);

  std::vector<z3::expr> nodes_iso;
  std::vector<z3::expr> edges_iso;

  // Get the conjuncts of the encodings
  get_encoding(nodes_iso, edges_iso);

  // assert the conjuncts
  for (std::vector<z3::expr>::const_iterator it = nodes_iso.begin();
       it != nodes_iso.end(); ++it) {
    z3::expr expr = *it;
    solver.add(expr);
  }
  for (std::vector<z3::expr>::const_iterator it = edges_iso.begin();
       it != edges_iso.end(); ++it) {
    z3::expr expr = *it;
    solver.add(expr);
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

bool IsoSolver::get_max_embedding()
{
  z3::optimize opt(z3_context);

  std::vector<z3::expr> nodes_iso;
  std::vector<z3::expr> edges_iso;

  // Get the conjuncts of the encodings
  get_encoding(nodes_iso, edges_iso);

  // assert the conjuncts
  for (std::vector<z3::expr>::const_iterator it = nodes_iso.begin();
       it != nodes_iso.end(); ++it) {
    z3::expr expr = *it;
    opt.add(expr, 1);
  }
  for (std::vector<z3::expr>::const_iterator it = edges_iso.begin();
       it != edges_iso.end(); ++it) {
    z3::expr expr = *it;
    opt.add(expr, 1);
  }

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

void IsoSolver::get_encoding(std::vector<z3::expr>& nodes_iso,
                             std::vector<z3::expr>& edges_iso)
{
  // 1. Declare encoding variables

  // 1. Encode constraints of iso_node_v0_v1
  /* For all pairs of nodes (that could match) iso_node_v0_v1
   * We have a single variable for (v0,v1) and (v1,v0) under the.
   * injectivity assumption
   *
   * iso_node_v0_v1 -> (label(v0) = label(v1))
   */
  nodes_iso.clear();
  for (nodes_t::const_iterator it_a =  acdfg_a.begin_nodes();
       it_a != acdfg_a.end_nodes(); ++it_a) {
    Node& node_a = *(*it_a);

    for (nodes_t::const_iterator it_b =  acdfg_b.begin_nodes();
         it_b != acdfg_b.end_nodes(); ++it_b) {
      Node& node_b = *(*it_b);

      if (may_match(node_a, node_b)) {
        z3::expr iso_var = get_iso_var(node_a, node_b);

        z3::expr iso_eq = get_iso_eq(node_a, node_b);

        nodes_iso.push_back((! iso_var) || iso_eq);
      }
    }
  }

  // 3. Encode the constraints of iso_edge_e0_e1
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

      if (may_match(edge_a, edge_b)) {
        z3::expr iso_var = get_iso_var(edge_a, edge_b);

        z3::expr iso_eq = get_iso_eq(edge_a, edge_b);

        edges_iso.push_back((! iso_var) || iso_eq);
      }
    }
  }
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
      // TODO: fix
      std::map<z3::expr, idPair>::iterator node_iter = var2nodes.find(e);
      if (var2nodes.end() != node_iter) {
        /* mapping for nodes */
        iso->add_node_map(((*node_iter).second).first,
                          ((*node_iter).second).second);
      }
      else {
        // TODO: fix
        std::map<z3::expr, idPair>::iterator edge_iter = var2edges.find(e);
        if (var2edges.end() != edge_iter) {
          /* mapping for edges */
          iso->add_edge_map(((*edge_iter).second).first,
                            ((*edge_iter).second).second);
        }
        else {
          /* it is never the case that we cannot find the pair
             correspondent to a variable in the current encoding */
          assert(false);
        }
      }
    }
  }

  return iso;
}

z3::expr IsoSolver::get_iso_var(const Node &n_a, const Node &n_b) {
  char* var_name = get_var_name("iso_node_",
                                n_a.get_id(),
                                n_b.get_id());
  z3::expr iso_var = z3_context.bool_const(var_name);
  delete var_name;

  idPair p = std::make_pair(n_a.get_id(), n_b.get_id());
  var2nodes[iso_var] = p;

  return iso_var;
}

z3::expr IsoSolver::get_iso_var(const Edge &e_a, const Edge &e_b)
{
  char* var_name = get_var_name("iso_edge_",
                                e_a.get_id(),
                                e_b.get_id());
  z3::expr iso_var = z3_context.bool_const(var_name);
  delete var_name;

  idPair p = std::make_pair(e_a.get_id(), e_b.get_id());
  var2edges[iso_var] = p;

  return iso_var;
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

  return false;
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
  assert(false);
  return z3_context.bool_val(true);
}


char* IsoSolver::get_var_name(const char* prefix, long id1, long id2)
{
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

  return var_name;
}

string IsoSolver::get_str(long id)
{
  std::ostringstream ostr;
  ostr << id;
  return ostr.str();
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
    stream << "(" << it->first << "," << it->second << ")";
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
