/*
 * Finds the common pattern of two different clusters.
 */
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/serializationLattice.h"

#include "fixrgraphiso/acdfgBin.h"


#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::pair;

using fixrgraphiso::Lattice;
using fixrgraphiso::AcdfgBin;
using fixrgraphiso::IsoRepr;

void findDuplicates(const Lattice &lattice_1, const int id_1,
                    const Lattice &lattice_2, const int id_2,
                    map<AcdfgBin*, int> &acdfgBin2id,
                    vector<pair<int, int>> &identicalBins)
{
  for (auto it_1 = lattice_1.beginPopular(); it_1 != lattice_1.endPopular();
       it_1++) {
    AcdfgBin* bin_1  = *it_1;

    for (auto it_2 = lattice_2.beginPopular(); it_2 != lattice_2.endPopular();
         it_2++) {
      AcdfgBin* bin_2  = *it_2;
      IsoRepr* iso;

      if (bin_1->isACDFGEquivalent(bin_2->getRepresentative(), iso)) {
        delete iso;

        identicalBins.push_back(std::make_pair(acdfgBin2id[bin_1],
                                               acdfgBin2id[bin_2]));
      }
    }
  }
}

void printHelp() {
  cerr << "findDuplicates " <<
    "-f <lattice_file_1> -i <id1> -s <lattice_file_2> -d <id2> -o <result_file>" << endl <<
    "\t <lattice_file_1>: path to the file storing the lattice" << endl <<
    "\t <id_1>: id of lattice 1 (used to back reference the lattice)" << endl <<
    "\t <lattice_file_2>: path to the file storing the lattice" << endl <<
    "\t <id_2>: id of lattice 2" << endl <<
    "\t <result_file>: path to the file containing patterns common to " <<
    "lattice 1 and lattice 2" << endl;
}

int main(int argc, char * argv[]){

  extern char *optarg;
  extern int optind;

  int id_1 = -1;
  int id_2 = -1;
  string* latticeFileName_1 = NULL;
  string* latticeFileName_2 = NULL;
  string* outFileName = NULL;

  char c;
  while ((c = getopt(argc, argv, "f:s:o:i:d:")) != -1) {
    switch (c){
    case 'f': {
      latticeFileName_1 = new string(optarg);
      break;
    }
    case 's': {
      latticeFileName_2 = new string(optarg);
      break;
    }
    case 'i': {
      id_1 = strtol(optarg, NULL, -1);
      break;
    }
    case 'd': {
      id_2 = strtol(optarg, NULL, -1);
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

  if (id_1 < 0 || id_2 < 0 || latticeFileName_1 == NULL ||
      latticeFileName_2 == NULL || outFileName == NULL) {
    printHelp();
    return 1;
  }


  {
    Lattice *lattice_1;
    Lattice *lattice_2;
    vector<pair<int,int>> identicalBins;

    map<AcdfgBin*, int> acdfgBin2id_1;
    map<AcdfgBin*, int> acdfgBin2id_2;

    lattice_1 = fixrgraphiso::readLattice(*latticeFileName_1,
                                          acdfgBin2id_1);
    if (NULL == lattice_1) return 1;
    lattice_2 = fixrgraphiso::readLattice(*latticeFileName_2,
                                          acdfgBin2id_1);
    if (NULL == lattice_2) return 1;

    // Find the duplicates
    acdfgBin2id_1.insert(acdfgBin2id_2.begin(), acdfgBin2id_2.end());
    findDuplicates(*lattice_1, id_1, *lattice_2, id_2,
                   acdfgBin2id_1, identicalBins);

    // Create the output

    delete latticeFileName_1;
    delete latticeFileName_2;
    delete outFileName;
    delete lattice_1;
    delete lattice_2;
  }

  return 0;
}
