#include "fixrgraphiso/searchLattice.h"

#include <fstream>
#include <iostream>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::set;
using std::string;
using std::ofstream;
using std::ifstream;


void printHelp() {
  cerr << "searchLatticeMain " <<
    "-q <query_acdfg> -l <lattice_file> -o <result_file>" << endl <<
    "\t <query_acdfg>: path to the acdfg file used as query" << endl <<
    "\t <lattice_file>: path to the file storing the lattice" << endl <<
    "\t <result_file>: path to the output file" << endl;
}

int main(int argc, char * argv[]){
  //  fixrgraphiso::SearchLattice search;

  string acdfgFileName;
  string latticeFileName;
  string outFileName;

  bool error = false;
  char c;
  while ((c = getopt(argc, argv, "q:l:o")) != -1 &&
         ! error) {
    switch (c){
    case 'q': {
      acdfgFileName = optarg;
      break;
    }
    case 'l': {
      latticeFileName = optarg;
      break;
    }
    case 'o': {
      outFileName = optarg;
      break;
    }
    default:
      error = true;
      break;
    }
  }

  if (error) {
    printHelp();
    return 1;
  } else {
  }

  return 0;
}
