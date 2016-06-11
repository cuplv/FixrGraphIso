// -*- C++ -*-
//
// Entry point
//
// Author: Sergio Mover <sergio.mover@colorado.edu>
//

#include <iostream>

#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/acdfg.h"

using namespace fixrgraphiso;

void clean(acdfg_protobuf::Acdfg* proto_acdfg_1,
           acdfg_protobuf::Acdfg* proto_acdfg_2,
           Acdfg* acdfg_1, Acdfg* acdfg_2);

int main (int argc, char *argv[])
{
  if (3 != argc) {
    std::cerr << "usage: fixrgraphiso <proto-graph> <proto-graph>\n";
    return 1;
  }
  else {
    AcdfgSerializer serializer;
    acdfg_protobuf::Acdfg* proto_acdfg_1 = NULL;
    acdfg_protobuf::Acdfg* proto_acdfg_2 = NULL;
    Acdfg* acdfg_1 = NULL;
    Acdfg* acdfg_2 = NULL;

    proto_acdfg_1 = serializer.read_protobuf_acdfg(argv[1]);
    if (NULL == proto_acdfg_1) {
      std::cerr << "Error reading file " << argv[1] << "\n";
      clean(proto_acdfg_1, proto_acdfg_2, acdfg_1, acdfg_2);
      return 1;
    }

    proto_acdfg_2 = serializer.read_protobuf_acdfg(argv[2]);
    if (NULL == proto_acdfg_2) {
      std::cerr << "Error reading file " << argv[2] << "\n";
      clean(proto_acdfg_1, proto_acdfg_2, acdfg_1, acdfg_2);
      return 1;
    }

    acdfg_1 = serializer.create_acdfg(proto_acdfg_1);
    if (NULL == acdfg_1) {
      std::cerr << "Error building the acdfg for " << argv[1] << "\n";
      clean(proto_acdfg_1, proto_acdfg_2, acdfg_1, acdfg_2);
      delete(acdfg_1);
      return 1;
    }

    acdfg_2 = serializer.create_acdfg(proto_acdfg_2);
    if (NULL == acdfg_2) {
      std::cerr << "Error building the acdfg for " << argv[2] << "\n";
      clean(proto_acdfg_1, proto_acdfg_2, acdfg_1, acdfg_2);
      delete(acdfg_2);
      return 1;
    }

    std::cout << "Acdfg 1\n" << (*acdfg_1);
    std::cout << "\nAcdfg 2\n" << (*acdfg_2);

    clean(proto_acdfg_1, proto_acdfg_2, acdfg_1, acdfg_2);
  }

  return 0;
}

void clean(acdfg_protobuf::Acdfg* proto_acdfg_1,
           acdfg_protobuf::Acdfg* proto_acdfg_2,
           Acdfg* acdfg_1, Acdfg* acdfg_2)
{
  if (NULL != proto_acdfg_1) delete proto_acdfg_1;
  if (NULL != proto_acdfg_2) delete proto_acdfg_2;
  if (NULL != acdfg_1) delete acdfg_1;
  if (NULL != acdfg_2) delete acdfg_2;
}
