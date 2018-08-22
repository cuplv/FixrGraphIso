// -*- C++ -*-
//
// Code used to serialize (read/write) objects
//
// Author: Sergio Mover
//


#ifndef SERIALIZATION_H_INCLUDED
#define SERIALIZATION_H_INCLUDED

#include "fixrgraphiso/acdfg.h"
#include "fixrgraphiso/acdfgBin.h"
#include "fixrgraphiso/proto_acdfg.pb.h"
#include "fixrgraphiso/proto_iso.pb.h"
#include "fixrgraphiso/proto_acdfg_bin.pb.h"

namespace fixrgraphiso {

  namespace acdfg_protobuf = edu::colorado::plv::fixr::protobuf;

  class AcdfgSerializer {
  public:
    /* convert the protobuf represenation to our representation */
    Acdfg* create_acdfg(acdfg_protobuf::Acdfg* proto_acdfg);

    /* Read the protobuf acdfg */
    acdfg_protobuf::Acdfg* read_protobuf_acdfg(const char* file_name);

    Acdfg * create_acdfg(acdfg_protobuf::Iso * proto_iso);

  private:
  };

  class LatticeSerializer {
  public:
    AcdfgBin* lattice_from_proto(acdfg_protobuf::Lattice* proto_acdfg_bin);
    acdfg_protobuf::Lattice* proto_from_lattice(AcdfgBin*);

    acdfg_protobuf::Lattice* read_protobuf(const char* file_name);
  private:
  };


} // end fixrgraphiso namespace

#endif // SERIALIZATION_H_INCLUDED
