#include <string>

#ifndef D__FIND_DUPLICATES__H_
#define D__FIND_DUPLICATES__H_

using std::string;

namespace fixrgraphiso {

  int findDuplicates(const string &latticeFileName_1, const int id_1,
                     const string &latticeFileName_2, const int id_2,
                     const string &outFileName);

}

#endif
