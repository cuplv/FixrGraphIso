// -*- C++ -*-
//
// Entry point
//
// Author: Sergio Mover <sergio.mover@colorado.edu>
//

#include <iostream>
#include <fstream>
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/acdfg.h"
//#include "fixrgraphiso/iso.h"
#include "fixrgraphiso/ilpApproxIsomorphismEncoder.h"
#include "fixrgraphiso/isomorphismResults.h"

using namespace fixrgraphiso;

extern Acdfg * createGraphA();
extern Acdfg * createGraphB();


void clean(acdfg_protobuf::Acdfg* proto_acdfg_a,
           acdfg_protobuf::Acdfg* proto_acdfg_b,
           Acdfg* acdfg_a, Acdfg* acdfg_b);

void process(std::string aName, Acdfg* acdfg_a, std::string bName, Acdfg* acdfg_b){

  std::string fStem = aName+"__"+bName;
  
  std::cout << "Acdfg a\n" << (*acdfg_a);
  std::cout << "\nAcdfg b\n" << (*acdfg_b);
  
  // /* Compute the isomorphism */
  // {
  //   IsoSolver solver(*acdfg_a, *acdfg_b);
  
  //   /* compute (precise) isomorphism */
  //   bool is_iso = solver.is_iso();
  //   std::cout << "Is a isomorphic to b? " <<
  //     is_iso << std::endl;
  //   if (is_iso) {
  //     std::cout << "Isomorphism:\n" <<
  //       solver.get_last_isomorphism() << std::endl;
  //   }
  
  //   bool is_approx_iso = solver.get_max_embedding();
  //   std::cout << "Is a approximate isomorphic to b? "
  //             << is_approx_iso << std::endl;
  //   if (is_approx_iso) {
  //     std::cout << "Approximate isomorphism:\n" <<
  //       solver.get_last_isomorphism() << std::endl;
  //   }
  // }
  
  IlpApproxIsomorphism ilp(acdfg_a, acdfg_b);
  ilp.computeILPEncoding();
  
  
  std::ofstream file(fStem+".dot");
  ilp.prettyPrintEncodingResultInDot(file);
  file.close();

  IsomorphismResults isoResults(aName,bName);
  ilp.populateResults(isoResults);
  isoResults.dumpProtobuf(fStem+".iso.bin");
}

int main (int argc, char *argv[])
{
  if (3 != argc) {
    Acdfg * acdfg_a = createGraphA();
    Acdfg * acdfg_b = createGraphB();
    process("a", acdfg_a,"b", acdfg_b);
 
    return 1;
  }
  else {
    AcdfgSerializer serializer;
    acdfg_protobuf::Acdfg* proto_acdfg_a = NULL;
    acdfg_protobuf::Acdfg* proto_acdfg_b = NULL;
    Acdfg* acdfg_a = NULL;
    Acdfg* acdfg_b = NULL;
    string graphAName (argv[1]);
    string graphBName (argv[2]);
    
    proto_acdfg_a = serializer.read_protobuf_acdfg(argv[1]);
    if (NULL == proto_acdfg_a) {
      std::cerr << "Error reading file " << argv[1] << "\n";
      clean(proto_acdfg_a, proto_acdfg_b, acdfg_a, acdfg_b);
      return 1;
    }

    proto_acdfg_b = serializer.read_protobuf_acdfg(argv[2]);
    if (NULL == proto_acdfg_b) {
      std::cerr << "Error reading file " << argv[2] << "\n";
      clean(proto_acdfg_a, proto_acdfg_b, acdfg_a, acdfg_b);
      return 1;
    }

    acdfg_a = serializer.create_acdfg(proto_acdfg_a);
    if (NULL == acdfg_a) {
      std::cerr << "Error building the acdfg for " << argv[1] << "\n";
      clean(proto_acdfg_a, proto_acdfg_b, acdfg_a, acdfg_b);
      delete(acdfg_a);
      return 1;
    }

    acdfg_b = serializer.create_acdfg(proto_acdfg_b);
    if (NULL == acdfg_b) {
      std::cerr << "Error building the acdfg for " << argv[2] << "\n";
      clean(proto_acdfg_a, proto_acdfg_b, acdfg_a, acdfg_b);
      delete(acdfg_b);
      return 1;
    }

    process(graphAName, acdfg_a, graphBName, acdfg_b);
    clean(proto_acdfg_a, proto_acdfg_b, acdfg_a, acdfg_b);
  }

  return 0;
}

void clean(acdfg_protobuf::Acdfg* proto_acdfg_a,
           acdfg_protobuf::Acdfg* proto_acdfg_b,
           Acdfg* acdfg_a, Acdfg* acdfg_b)
{
  if (NULL != proto_acdfg_a) delete proto_acdfg_a;
  if (NULL != proto_acdfg_b) delete proto_acdfg_b;
  if (NULL != acdfg_a) delete acdfg_a;
  if (NULL != acdfg_b) delete acdfg_b;

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
}
