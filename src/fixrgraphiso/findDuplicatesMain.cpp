/*
 * Finds the common pattern of two different clusters.
 */

#include "fixrgraphiso/findDuplicates.h"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;

void printHelp() {
  cerr << "findDuplicates " <<
    "-f <lattice_file_list>" << endl <<
    "\t <result_file>: path to the file containing patterns common to " <<
    "lattice 1 and lattice 2" << endl;
}

int main(int argc, char * argv[]) {

  extern char *optarg;
  extern int optind;

  string* latticeFileList = NULL;
  string* outFileName = NULL;

  fixrgraphiso::dup_tuple identicalBins;

  char c;
  while ((c = getopt(argc, argv, "f:o:")) != -1) {
    switch (c){
    case 'f': {
      latticeFileList = new string(optarg);
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

  if (latticeFileList == NULL || outFileName == NULL) {
    printHelp();
    return 1;
  }

  int res = fixrgraphiso::findDuplicatesList(*latticeFileList,
                                             identicalBins);

  delete latticeFileList;
  delete outFileName;

  return res;
}
