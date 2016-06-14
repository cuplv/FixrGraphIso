// -*- C++ -*-
//
// Encodes the approximate graph isomorphism in MaxSAT
//
// Author: Sergio Mover
//

#ifndef ISO_H_INCLUDED
#define ISO_H_INCLUDED

#include "fixrgraphiso/acdfg.h"
#include"z3++.h"

namespace fixrgraphiso {

class IsoSolver {
public:
  IsoSolver(Acdfg& a, Acdfg& b) : acdfg_a(a), acdfg_b(b) {};

  // TODO: return an object that represent the isomorphism
  bool is_iso();
  // TODO: return an object that represent the isomorphism
  bool get_max_embedding();

private:
  Acdfg& acdfg_a;
  Acdfg& acdfg_b;
  z3::context z3_context;

  void get_encoding(std::vector<z3::expr>& nodes_iso,
                    std::vector<z3::expr>& edges_iso);
  z3::expr get_iso_var(const Node &n_a, const Node &n_b);
  z3::expr get_iso_var(const Edge &e_a, const Edge &e_b);
  bool may_match(const Node& n_a, const Node& n_b);
  bool may_match(const DataNode& n_a, const DataNode& n_b);
  bool may_match(const MethodNode& n_a, const MethodNode& n_b);
  bool may_match(const Edge& e_a, const Edge& e_b);
  z3::expr get_iso_eq(const Node& n_a, const Node& n_b);
  z3::expr get_iso_eq(const MethodNode& n_a, const MethodNode& n_b);
  z3::expr get_iso_eq(const Edge& e_a, const Edge& e_b);
  char* get_var_name(const char* prefix, long id1, long id2);
  string get_str(long id);
};

} // end namespace fixrgraphiso

#endif // ISO_H_INCLUDED
