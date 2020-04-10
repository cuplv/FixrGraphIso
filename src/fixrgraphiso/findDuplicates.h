#include <string>
#include <vector>
#include <tuple>

#ifndef D__FIND_DUPLICATES__H_
#define D__FIND_DUPLICATES__H_

using std::string;
using std::vector;
using std::tuple;

namespace fixrgraphiso {

  using dup_tuple = vector<tuple<int, int, int, int>>;

  int findDuplicates(const string &latticeFileName_1, const int id_1,
                     const string &latticeFileName_2, const int id_2,
                     dup_tuple &identicalBins);

  int findDuplicatesList(const string &latticeListFileName,
                         dup_tuple &identicalBins);

  int writeDuplicateList(const dup_tuple &identicalBins,
                         const string outFileName);
}

#endif
