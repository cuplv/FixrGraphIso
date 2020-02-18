// -*- C++ -*-
//
// Code used to serialize (read/write) the 
//
// Author: Sergio Mover
//


#ifndef SERIALIZATION_LATTICE_H_INCLUDED
#define SERIALIZATION_LATTICE_H_INCLUDED

#include "fixrgraphiso/acdfgBin.h"
#include "fixrgraphiso/proto_acdfg_bin.pb.h"

namespace fixrgraphiso {
  namespace acdfg_protobuf = edu::colorado::plv::fixr::protobuf;

  class LatticeSerializer {
  public:
    Lattice* lattice_from_proto(acdfg_protobuf::Lattice* proto_lattice,
                                std::map<AcdfgBin*, int> &acdfgBin2id);
    acdfg_protobuf::Lattice* proto_from_lattice(const Lattice & lattice);
    acdfg_protobuf::Lattice* read_protobuf(const char* file_name);
  private:
  };

  Lattice* readLattice(string latticeFile);
  Lattice* readLattice(string latticeFile,
                       std::map<AcdfgBin*, int> &acdfgBin2id);
  void writeLattice(const Lattice& lattice, string const& outFile);

} // end fixrgraphiso namespace

#endif // SERIALIZATION_LATTICE_H_INCLUDED

