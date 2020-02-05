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

namespace fixrgraphiso {

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

  int findDuplicates(const string &latticeFileName_1, const int id_1,
                     const string &latticeFileName_2, const int id_2,
                     const string &outFileName) {
    Lattice *lattice_1;
    Lattice *lattice_2;
    vector<pair<int,int>> identicalBins;

    map<AcdfgBin*, int> acdfgBin2id_1;
    map<AcdfgBin*, int> acdfgBin2id_2;

    lattice_1 = fixrgraphiso::readLattice(latticeFileName_1,
                                          acdfgBin2id_1);
    if (NULL == lattice_1) return 1;
    lattice_2 = fixrgraphiso::readLattice(latticeFileName_2,
                                          acdfgBin2id_1);
    if (NULL == lattice_2) return 1;

    // Find the duplicates
    acdfgBin2id_1.insert(acdfgBin2id_2.begin(), acdfgBin2id_2.end());
    findDuplicates(*lattice_1, id_1, *lattice_2, id_2,
                   acdfgBin2id_1, identicalBins);


    // Create the output
    delete lattice_1;
    delete lattice_2;

    return 0;
  }

}
