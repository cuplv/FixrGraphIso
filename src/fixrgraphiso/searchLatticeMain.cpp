#include "fixrgraphiso/searchLattice.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/serializationLattice.h"

#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <unistd.h>

using std::ios;
using std::fstream;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::set;
using std::string;
using std::ofstream;
using std::ifstream;

using fixrgraphiso::SearchLattice;
using fixrgraphiso::SearchResult;
using fixrgraphiso::Acdfg;
using fixrgraphiso::Lattice;

namespace acdfg_protobuf = edu::colorado::plv::fixr::protobuf;

void printHelp() {
  cerr << "searchLatticeMain " <<
    "-q <query_acdfg> -l <lattice_file> -o <result_file>" << endl <<
    "\t <query_acdfg>: path to the acdfg file used as query" << endl <<
    "\t <lattice_file>: path to the file storing the lattice" << endl <<
    "\t <result_file>: path to the output file" << endl;
}

int search(string& queryFile, string& latticeFileName,
           string& outFileName)
{
  Lattice *lattice;

  lattice = fixrgraphiso::readLattice(latticeFileName);
  if (NULL == lattice) {
    cerr << "Cannot read the lattice in " << latticeFileName << endl;
    return 1;
  } else {
    Acdfg* query = fixrgraphiso::readAcdfg(queryFile);

    if (NULL == query) {
      cerr << "Cannot read acdfg " << queryFile << endl;
      return 1;
    } else {
      vector<SearchResult*> results;
      SearchLattice searchLattice(query, lattice);
      searchLattice.search(results);

      acdfg_protobuf::SearchResults* protoRes =
        searchLattice.toProto(results);

      searchLattice.printResult(results, cout);

      fstream outfile(outFileName.c_str(), ios::out | ios::binary | ios::trunc);
      protoRes->SerializeToOstream(&outfile);
      outfile.close();
      delete(protoRes);

      delete(query);
    }
    delete(lattice);
  }

  return 0;
}

int main(int argc, char * argv[]){
  extern char *optarg;
  extern int optind;

  string* acdfgFileName = NULL;
  string* latticeFileName = NULL;
  string* outFileName = NULL;

  char c;
  while ((c = getopt(argc, argv, "q:l:o:")) != -1) {
    switch (c){
    case 'q': {
      acdfgFileName = new string(optarg);
      break;
    }
    case 'l': {
      latticeFileName = new string(optarg);
      break;
    }
    case 'o': {
      outFileName = new string(optarg);
      break;
    }
    default:
      printHelp();
      return 1;
      break;
    }
  }

  if (NULL == acdfgFileName) {
    printHelp();
    return 1;
  }
  if (NULL == latticeFileName) {
    printHelp();
    return 1;
  }
  if (NULL == outFileName) {
    printHelp();
    return 1;
  }

  search(*acdfgFileName, *latticeFileName, *outFileName);

  delete(acdfgFileName);
  delete(latticeFileName);
  delete(outFileName);

  return 0;
}
