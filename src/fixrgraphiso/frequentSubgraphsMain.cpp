#include <fstream>
#include <iostream>
#include <cassert>
#include <set>
#include <vector>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include "fixrgraphiso/proto_iso.pb.h"
#include "fixrgraphiso/proto_acdfg.pb.h"
#include "fixrgraphiso/isomorphismClass.h"

namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::string;
using std::ofstream;


namespace fixrgraphiso{
  bool debug = true;
}
int main(int argc, char * argv[]){
  vector<string> filenames;
  char c;
  while (optind < argc){
    if ((c = getopt(argc, argv, "d"))!= -1){
      switch (c){
      case 'd':
	fixrgraphiso::debug = true;
	break;
      default:
	break;
      }
    } else {
      string fname(argv[optind]);
      filenames.push_back(fname);
      optind++;
    }
  }
  if (filenames.size() <= 0){
    cout << "Usage:" << argv[0] << " [list of iso.bin files] " << endl;
    return 1;
  }

  vector<fixrgraphiso::IsomorphismClass> allIsos;
  for (string fname: filenames){
    fixrgraphiso::IsomorphismClass iso(fname);
    allIsos.push_back(iso);
  }

  // for (auto iso1: allIsos){
  //   for (auto iso2: allIsos){
  //     if (iso1.getIsoFilename() != iso2.getIsoFilename()){
  // 	cout << "Subsumption test: " << iso1.getIsoFilename() << " vs. " << iso2.getIsoFilename() << endl;
  // 	iso1.subsumes(&iso2);
  //     }
  //   }
  // }
  vector<fixrgraphiso::IsomorphismClass> maximalIsos;
  for (auto iso1: allIsos){
    bool is_subsumed = false;
    int i;
    for (i =0; i < maximalIsos.size(); ++i){
      fixrgraphiso::IsomorphismClass & iso2 = maximalIsos[i];
      if (iso2.subsumes(&iso1)){
	is_subsumed = true;
	break;
      }
      if (iso1.subsumes(&iso2)){
	break;
      }
    }

    if ( i >= maximalIsos.size()) maximalIsos.push_back(iso1);
    else if (!is_subsumed){
      maximalIsos[i] = iso1;
    }
  }
  
  cout << "# Maximal Isos: " << maximalIsos.size() << endl;
  int count = 0;
  for (const auto iso: maximalIsos){
    cout << iso.getIsoFilename () << endl;
    string fname = string("iso_")+std::to_string(count);
    std::ofstream out_file(fname.c_str());
    (iso.get_acdfg()) -> dumpToDot( out_file);
    out_file.close();
    count ++;
  }
  for (auto iso:allIsos){
    delete(iso.get_acdfg());
  }
  
}
  
  


