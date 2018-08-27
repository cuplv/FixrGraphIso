// -*- C++ -*-
//
// Code used to serialize (read/write) objects
//
// Author: Sergio Mover
//


#include <iostream>
#include <fstream>
#include <map>
#include <typeinfo>
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/serializationLattice.h"

namespace fixrgraphiso {
  /**
   * Read a lattice from the protobuffer and create a lattice data structure
   */
  Lattice* LatticeSerializer::lattice_from_proto(acdfg_protobuf::Lattice* protoLattice) {
    Lattice* lattice = new Lattice();
    AcdfgSerializer serializer;

    /* 0. set the method names */
    for (int i = 0; i < protoLattice->method_names_size(); i++) {
      string methodName = protoLattice->method_names(i);
      lattice->addMethodName(methodName);
    }

    // 1. Create the all the AcdfgBins
    // It just creates the bins, ignoring their relations
    std::map<int, AcdfgBin*> id2AcdfgBinMap;
    for (int i = 0; i < protoLattice->bins_size(); i++) {
      const acdfg_protobuf::Lattice::AcdfgBin & protoAcdfgBin =
        protoLattice->bins(i);

      const acdfg_protobuf::Acdfg & protoRepr = protoAcdfgBin.acdfg_repr();
      Acdfg* repr = serializer.create_acdfg(protoRepr);

      AcdfgBin* acdfgBin = new AcdfgBin(repr);
      // TODO - fix
      // for (int j = 0; j < protoAcdfgBin.acdfg_names_size(); j++) {
      //   acdfgBin->insertEquivalentACDFG(protoAcdfgBin.acdfg_names(j));
      // }

      if (protoAcdfgBin.anomalous()) acdfgBin->setAnomalous();
      if (protoAcdfgBin.subsuming()) acdfgBin->setSubsuming();
      if (protoAcdfgBin.popular()) acdfgBin->setPopular();

      id2AcdfgBinMap[protoAcdfgBin.id()] = acdfgBin;

      lattice->addBin(acdfgBin);
    }

    // 2. Create links between bins
    for (int i = 0; i < protoLattice->bins_size(); i++) {
      const acdfg_protobuf::Lattice::AcdfgBin & protoAcdfgBin =
        protoLattice->bins(i);
      AcdfgBin* currentBin = id2AcdfgBinMap[protoAcdfgBin.id()];

      for (int j = 0; j < protoAcdfgBin.subsuming_bins_size(); j++) {
        int otherId = protoAcdfgBin.subsuming_bins(j);
        AcdfgBin* other = id2AcdfgBinMap[otherId];
        currentBin->addSubsumingBin(other);
      }

      for (int j = 0; j < protoAcdfgBin.incoming_edges_size(); j++) {
        int otherId = protoAcdfgBin.incoming_edges(j);
        AcdfgBin* other = id2AcdfgBinMap[otherId];
        currentBin->insertIncomingEdge(other);
      }

      currentBin->computeImmediatelySubsumingBins();
    }


    // 3. Populate popular/anomalous/isolated list
    for (int i = 0; i < protoLattice->popular_bins_size(); i++) {
      AcdfgBin* other = id2AcdfgBinMap[protoLattice->popular_bins(i)];
      lattice->addPopular(other);
    }

    for (int i = 0; i < protoLattice->anomalous_bins_size(); i++) {
      AcdfgBin* other = id2AcdfgBinMap[protoLattice->anomalous_bins(i)];
      lattice->addAnomalous(other);
    }

    for (int i = 0; i < protoLattice->isolated_bins_size(); i++) {
      AcdfgBin* other = id2AcdfgBinMap[protoLattice->isolated_bins(i)];
      lattice->addIsolated(other);
    }

    return lattice;
  }

  /**
   * Serialize a lattice structure in a protobuffer
   */
  acdfg_protobuf::Lattice* LatticeSerializer::proto_from_lattice(const Lattice & lattice) {
    acdfg_protobuf::Lattice* protoLattice = new acdfg_protobuf::Lattice();

    std::map<AcdfgBin*, int> acdfgBin2idMap;

    // 0. Assign the method names
    for (const string & methodName : lattice.getMethodNames()) {
      protoLattice->add_method_names(methodName);
    }

    // 1. Assign the IDs to the bins
    {
      int id = -1;
      for (auto it = lattice.beginAllBins();
           it != lattice.endAllBins(); ++it) {
        AcdfgBin * a = *it;
        id += 1;
        acdfgBin2idMap[a] = id;
      };
    }

    // 2. Populate the bins field
    for (auto it = lattice.beginAllBins();
         it != lattice.endAllBins(); ++it) {
      AcdfgBin * a = *it;
      int id = acdfgBin2idMap[a];

      acdfg_protobuf::Lattice::AcdfgBin* proto_a =
        protoLattice->add_bins();

      proto_a->set_id(id);

      acdfg_protobuf::Acdfg* proto_repr = proto_a->mutable_acdfg_repr();
      AcdfgSerializer serializer;
      Acdfg* reprAcdfg = (a->getRepresentative());
      serializer.fill_proto_from_acdfg((const Acdfg&) *reprAcdfg, proto_repr);

      for (const string & acdfgName : a->getAcdfgNames()) {
        proto_a->add_acdfg_names(acdfgName);
      }

      for (AcdfgBin* subsuming : a->getSubsumingBins()) {
        int id = acdfgBin2idMap[subsuming];
        proto_a->add_subsuming_bins(id);
      }

      for (AcdfgBin* incoming : a->getIncomingEdges()) {
        int id = acdfgBin2idMap[incoming];
        proto_a->add_incoming_edges(id);
      }

      proto_a->set_subsuming(a->isSubsuming());
      proto_a->set_anomalous(a->isAnomalous());
      proto_a->set_popular(a->isPopular());
    }

    for (auto it = lattice.beginPopular(); it != lattice.endPopular(); ++it) {
      AcdfgBin * a = *it;
      protoLattice->add_popular_bins(acdfgBin2idMap[a]);
    }

    for (auto it = lattice.beginAnomalous();
         it != lattice.endAnomalous(); ++it) {
      AcdfgBin * a = *it;
      protoLattice->add_anomalous_bins(acdfgBin2idMap[a]);
    }

    for (auto it = lattice.beginIsolated();
         it != lattice.endIsolated(); ++it) {
      AcdfgBin * a = *it;
      protoLattice->add_isolated_bins(acdfgBin2idMap[a]);
    }

    return protoLattice;
  }

  acdfg_protobuf::Lattice* LatticeSerializer::read_protobuf(const char* file_name) {
    acdfg_protobuf::Lattice* lattice = new acdfg_protobuf::Lattice();

    std::fstream input_stream (file_name, std::ios::in | std::ios::binary);
    if (input_stream.is_open()) {
      if (lattice->ParseFromIstream(&input_stream)) {
        return lattice;
      } else {
        return NULL;
      }
    } else {
      return NULL;
    }
  }
}