#include "fixrgraphiso/findDuplicates.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/serializationLattice.h"
#include "fixrgraphiso/acdfgBin.h"

#include <tuple>
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
using std::tuple;

using fixrgraphiso::Lattice;
using fixrgraphiso::AcdfgBin;
using fixrgraphiso::IsoRepr;
using fixrgraphiso::dup_tuple;

namespace fixrgraphiso {

  void findDuplicatesAux(const Lattice &lattice_1, const int id_1,
                         const Lattice &lattice_2, const int id_2,
                         map<AcdfgBin*, int> &acdfgBin2id,
                         dup_tuple &identicalBins)
  {
    for (auto it_1 = lattice_1.beginPopular(); it_1 != lattice_1.endPopular();
         it_1++) {
      AcdfgBin* bin_1  = *it_1;

      for (auto it_2 = lattice_2.beginPopular(); it_2 != lattice_2.endPopular();
           it_2++) {
        AcdfgBin* bin_2  = *it_2;
        IsoRepr* iso = new IsoRepr(bin_1->getRepresentative(),
                                   bin_2->getRepresentative());

        if (bin_1->isACDFGEquivalent(bin_2->getRepresentative(), iso)) {
          delete iso;

          identicalBins.push_back(std::make_tuple(id_1, acdfgBin2id[bin_1],
                                                  id_2, acdfgBin2id[bin_2]));
        }
      }
    }
  }

  int findDuplicates(const string &latticeFileName_1, const int id_1,
                     const string &latticeFileName_2, const int id_2,
                     dup_tuple &identicalBins) {
    Lattice *lattice_1;
    Lattice *lattice_2;

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
    findDuplicatesAux(*lattice_1, id_1, *lattice_2, id_2,
                      acdfgBin2id_1, identicalBins);


    // Create the output
    delete lattice_1;
    delete lattice_2;

    return 0;
  }

  int findDuplicatesList(const vector<pair<string,int>> &latticeNamesList,
                         dup_tuple &identicalBins) {

    for (auto i1 = latticeNamesList.begin();
         i1 != latticeNamesList.end(); ++i1) {
      Lattice *lattice_1;
      map<AcdfgBin*, int> acdfgBin2id_1;

      lattice_1 = fixrgraphiso::readLattice(i1->first, acdfgBin2id_1);
      if (NULL == lattice_1) return 1;

      for (auto i2 = i1 + 1;
           i2 != latticeNamesList.end(); ++i2) {
        Lattice *lattice_2;
        map<AcdfgBin*, int> acdfgBin2id_2;
        lattice_2 = fixrgraphiso::readLattice(i2->first, acdfgBin2id_2);
        if (NULL == lattice_2) {
          return 1;
          delete lattice_1;
        }

        if (lattice_1->countCommonMethods(*lattice_2) >= 2) {
          // Find the duplicates
          acdfgBin2id_2.insert(acdfgBin2id_1.begin(), acdfgBin2id_1.end());
          findDuplicatesAux(*lattice_1, i1->second,
                            *lattice_2, i2->second,
                            acdfgBin2id_2, identicalBins);
        }

        delete lattice_2;
      }

      delete lattice_1;
    }

    return 0;
  }


  int findDuplicatesList(const string &latticeListFileName,
                         dup_tuple &identicalBins) {

    vector<pair<string,int>> latticeNamesList;

    std::ifstream listFile(latticeListFileName);

    if (listFile.is_open()) {
      string line;
      while (getline(listFile, line)) {
        std::size_t pos_delim = line.find(" ");

        if (std::string::npos == pos_delim ||
            pos_delim < 1 ||
            (line.size() -1) == pos_delim) {
          return 1;
          listFile.close();
        }

        string id_str = line.substr(0, pos_delim);
        int id = std::stoi(id_str);
        string latticeFileName = line.substr(pos_delim + 1, line.size() -1);

        latticeNamesList.push_back(make_pair(latticeFileName, id));
      }
      listFile.close();
    } else {
      cerr << "Cannot read file " << latticeListFileName << std::endl;
      return 1;
    }


    return findDuplicatesList(latticeNamesList, identicalBins);
  }

  int writeDuplicateList(const dup_tuple &identicalBins,
                         const string outFileName) {
    std::ofstream outFile(outFileName);

    if (outFile.is_open()) {
      for (auto t : identicalBins) {
        outFile <<
          std::get<0>(t) << "," <<
          std::get<1>(t) << "," <<
          std::get<2>(t) << "," <<
          std::get<3>(t) << endl;
      }
    }
    else {
      cout << "Unable to create output file";
      return 0;
    }

    outFile.close();

    return 0;
  }
}
