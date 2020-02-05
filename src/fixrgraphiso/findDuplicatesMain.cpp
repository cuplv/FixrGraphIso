/*
 * Finds the common pattern of two different clusters.
 */

#include "fixrgraphiso/findDuplicates.h"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;


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
    int res = fixrgraphiso::findDuplicates(*latticeFileName_1, id_1,
                                           *latticeFileName_2, id_2,
                                           *outFileName);
    delete latticeFileName_1;
    delete latticeFileName_2;
    delete outFileName;

    return res;
  }
}
