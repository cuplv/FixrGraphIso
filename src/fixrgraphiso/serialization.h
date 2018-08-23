// -*- C++ -*-
//
// Code used to serialize (read/write) objects
//
// Author: Sergio Mover
//


#ifndef SERIALIZATION_H_INCLUDED
#define SERIALIZATION_H_INCLUDED

#include "fixrgraphiso/acdfg.h"
#include "fixrgraphiso/proto_acdfg.pb.h"
#include "fixrgraphiso/proto_iso.pb.h"

namespace fixrgraphiso {

  namespace acdfg_protobuf = edu::colorado::plv::fixr::protobuf;

  class AcdfgSerializer {
  private:
    void proto_from_acdfg_label(acdfg_protobuf::Acdfg * protoAcdfg,
                                const Acdfg &acdfg,
                                Edge * edge);
  public:
    /* convert the protobuf represenation to our representation */
    Acdfg* create_acdfg(const acdfg_protobuf::Acdfg &proto_acdfg);


    void fill_proto_from_acdfg(const Acdfg &acdfg,
                               acdfg_protobuf::Acdfg*);

    /* Read the protobuf acdfg */
    acdfg_protobuf::Acdfg* read_protobuf_acdfg(const char* file_name);

    Acdfg * create_acdfg(acdfg_protobuf::Iso * proto_iso);

  private:
  };
} // end fixrgraphiso namespace

#endif // SERIALIZATION_H_INCLUDED
