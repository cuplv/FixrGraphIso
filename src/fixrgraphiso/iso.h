// -*- C++ -*-
//
// Encodes the approximate graph isomorphism in MaxSAT
//
// Author: Sergio Mover
//

#ifndef ISO_H_INCLUDED
#define ISO_H_INCLUDED

#include <string>
#include <map>
#include <vector>
#include "z3++.h"
#include "fixrgraphiso/acdfg.h"


namespace fixrgraphiso {

typedef std::pair<long, long> idPair;

struct exprCompare {
  bool operator()(const z3::expr& a, const z3::expr& b) const {
    check_context(a, b);

    return Z3_get_ast_id(a.ctx(), a) < Z3_get_ast_hash(b.ctx(), b);
  }
};

struct pairsCompare {
  bool operator()(const idPair& a, const idPair& b) const {
    if (a.first != b.first) {
      return a.first < b.first;
    }
    else {
      return a.second < b.second;
    }
  }
};

/**
   \brief Represent a (possibly partial) isomorphism between two
   graphs.
 */
class Isomorphism {
public:
  Isomorphism(Acdfg& a, Acdfg& b) : acdfg_a(a), acdfg_b(b) {};

  void add_node_map(const long id_a, const long id_b);
  void add_edge_map(const long id_a, const long id_b);

  friend std::ostream& operator<<(std::ostream&, const Isomorphism&);

private:
  Acdfg& acdfg_a;
  Acdfg& acdfg_b;

  /* Map from nodes of a to nodes of b */
  std::map<long, long> node_mapping;
  /* Map from edges of a to edges of b */
  std::map<long, long> edge_mapping;
};

/**
  \brief Implement the solver for approximate isomorphism.
*/
class IsoSolver {
public:
  IsoSolver(Acdfg& a, Acdfg& b) : acdfg_a(a), acdfg_b(b)
  {
    last_isomorphism = NULL;
  };
  ~IsoSolver();

  // TODO: return an object that represent the isomorphism
  bool is_iso();
  // TODO: return an object that represent the isomorphism
  bool get_max_embedding();

  /* Get the last isomorphism */
  Isomorphism& get_last_isomorphism();

private:
  Acdfg& acdfg_a;
  Acdfg& acdfg_b;
  z3::context z3_context;

  std::vector<string*> var_names;
  std::map<idPair, string*, pairsCompare> nodes2varname;
  std::map<unsigned, idPair> z3id2nodes;

  std::map<idPair, string*, pairsCompare> edges2varname;
  std::map<unsigned, idPair> z3id2edges;

  Isomorphism* last_isomorphism;

  Isomorphism* get_isomorphism(const z3::model model);
  void get_encoding(std::vector<z3::expr>& nodes_iso,
                    std::vector<z3::expr>& edges_iso,
                    std::vector<z3::expr>& uniqueness_constraints,
                    std::vector<z3::expr>& nodes_iso_vars,
                    std::vector<z3::expr>& edges_iso_vars);

  void set_last_iso(Isomorphism* new_iso);
  z3::expr get_iso_var(const Node &n_a, const Node &n_b);
  z3::expr get_iso_var(const Edge &e_a, const Edge &e_b);
  bool may_match(const Node& n_a, const Node& n_b);
  bool may_match(const DataNode& n_a, const DataNode& n_b);
  bool may_match(const MethodNode& n_a, const MethodNode& n_b);
  bool may_match(const Edge& e_a, const Edge& e_b);
  z3::expr get_iso_eq(const Node& n_a, const Node& n_b);
  z3::expr get_iso_eq(const MethodNode& n_a, const MethodNode& n_b);
  z3::expr get_iso_eq(const Edge& e_a, const Edge& e_b);
  string* get_var_name(const char* prefix, long id1, long id2);
  string get_str(long id);
  idPair get_ids(const string& str);
  z3::expr exactly_n(std::vector<z3::expr>& formulas,
                     const int n);
};

} // end namespace fixrgraphiso

#endif // ISO_H_INCLUDED
