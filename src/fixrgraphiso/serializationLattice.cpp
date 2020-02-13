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
  using namespace std;
  using std::map;
  using std::ifstream;
  using std::ofstream;
  using std::fstream;


  /**
   * Read a lattice from the protobuffer and create a lattice data structure
   */
  Lattice* LatticeSerializer::lattice_from_proto(acdfg_protobuf::Lattice* protoLattice,
                                                 std::map<AcdfgBin*, int> &acdfgBin2id) {
    Lattice* lattice = new Lattice();
    AcdfgSerializer serializer;

    /* 0. set the method names */
    for (int i = 0; i < protoLattice->method_names_size(); i++) {
      string methodName = protoLattice->method_names(i);
      lattice->addMethodName(methodName);
    }

    // 1. Create the all the AcdfgBins
    // It just creates the bins, ignoring their relations
    map<int, AcdfgBin*> id2AcdfgBinMap;
    for (int i = 0; i < protoLattice->bins_size(); i++) {
      const acdfg_protobuf::Lattice::AcdfgBin & protoAcdfgBin =
        protoLattice->bins(i);

      const acdfg_protobuf::Acdfg & protoRepr = protoAcdfgBin.acdfg_repr();
      Acdfg* repr = serializer.create_acdfg(protoRepr);

      AcdfgBin* acdfgBin = new AcdfgBin(repr, lattice->getStats());
      for (int j = 0; j < protoAcdfgBin.names_to_iso_size(); j++) {
        const acdfg_protobuf::Lattice::IsoPair & protoIso =
          protoAcdfgBin.names_to_iso(j);

        IsoRepr* iso = new IsoRepr(protoIso.iso());

        acdfgBin->insertEquivalentACDFG(protoIso.method_name(), iso);
      }

      if (protoAcdfgBin.anomalous()) acdfgBin->setAnomalous();
      if (protoAcdfgBin.subsuming()) acdfgBin->setSubsuming();
      if (protoAcdfgBin.popular()) acdfgBin->setPopular(true);
      if (protoAcdfgBin.isolated()) acdfgBin->setIsolated();

      if (protoAcdfgBin.has_cumulative_frequency())
        acdfgBin->setCumulativeFrequency(protoAcdfgBin.cumulative_frequency());
      else
        acdfgBin->setCumulativeFrequency(0);

      id2AcdfgBinMap[protoAcdfgBin.id()] = acdfgBin;
      acdfgBin2id[acdfgBin] = protoAcdfgBin.id();

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
    }

    for (auto bin : lattice->getAllBins())
      bin->computeImmediatelySubsumingBins();


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


    // 0. Assign the method names
    for (const string & methodName : lattice.getMethodNames()) {
      protoLattice->add_method_names(methodName);
    }

    // 1. Assign the IDs to the bins
    map<AcdfgBin*, int> acdfgBin2idMap;
    lattice.getAcdfgBin2id(acdfgBin2idMap);

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

      const map<string, IsoRepr*> names_to_iso = a->getAcdfgNameToIso();
      for (auto it = names_to_iso.begin(); it != names_to_iso.end(); it++) {
        acdfg_protobuf::Lattice::IsoPair* pair = proto_a->add_names_to_iso();
        pair->set_method_name(it->first);
        pair->set_allocated_iso(it->second->proto_from_iso());
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
      proto_a->set_isolated(a->isIsolated());
      proto_a->set_cumulative_frequency(a->getCumulativeFrequency());
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

  Lattice* readLattice(string latticeFile) {
    LatticeSerializer s;
    Lattice * res = NULL;

    acdfg_protobuf::Lattice * proto = s.read_protobuf(latticeFile.c_str());
    if (NULL != proto) {
      map<AcdfgBin*, int> acdfgBin2id;
      res = s.lattice_from_proto(proto, acdfgBin2id);
    }
    delete(proto);
    return res;
  }

  Lattice* readLattice(string latticeFile,
                       map<AcdfgBin*, int> &acdfgBin2id) {
    LatticeSerializer s;
    Lattice * res = NULL;

    acdfg_protobuf::Lattice * proto = s.read_protobuf(latticeFile.c_str());
    if (NULL != proto) {
      res = s.lattice_from_proto(proto, acdfgBin2id);
    }
    delete(proto);
    return res;
  }

  void writeLattice(const Lattice& lattice, string const& outFile) {
    LatticeSerializer s;
    acdfg_protobuf::Lattice * protoWrite = s.proto_from_lattice(lattice);

    fstream myfile(outFile.c_str(), ios::out | ios::binary | ios::trunc);
    protoWrite->SerializeToOstream(&myfile);
    myfile.close();
    delete(protoWrite);
  }
}
