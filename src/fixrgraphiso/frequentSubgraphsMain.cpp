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
    vector<int> subsumedIsos;
    int freq = 1;
    for (i = maximalIsos.size()-1; i >= 0;  --i){
      fixrgraphiso::IsomorphismClass & iso2 = maximalIsos[i];
      if (iso2.subsumes(&iso1)){
	iso2.incrFrequency();
	is_subsumed = true;
	break;
      }
      if (iso1.subsumes(&iso2)){
	subsumedIsos.push_back(i);
	freq = freq + iso2.getFrequency();
      }
    }
    
    if (!is_subsumed){
      iso1.setFrequency(freq);
      maximalIsos.push_back(iso1);
    }
    for (int j: subsumedIsos){
      assert (!is_subsumed);
      maximalIsos.erase(maximalIsos.begin() + j);
    }
    
  }
  // Sort the maximal Isos according to the frequencies
  std::sort(maximalIsos.begin(), maximalIsos.end(),
	    [](const fixrgraphiso::IsomorphismClass  & iso1, const fixrgraphiso::IsomorphismClass & iso2){
	      return iso1.getFrequency() > iso2.getFrequency();
	    });
  
  cout << "# Maximal Isos: " << maximalIsos.size() << endl;
  int count = 0;
  for (int k = 0; k < maximalIsos.size(); ++k){
    fixrgraphiso::IsomorphismClass & iso = maximalIsos[k];
    cout << iso.getIsoFilename () << endl;
    cout << "Freq = " << iso.getFrequency() << endl;
    string fname = string("iso_")+std::to_string(count)+".dot";
    std::ofstream out_file(fname.c_str());
    (iso.get_acdfg()) -> dumpToDot( out_file);
    out_file.close();
    count ++;
  }
  for (auto iso:allIsos){
    delete(iso.get_acdfg());
  }
  
}
  
  


