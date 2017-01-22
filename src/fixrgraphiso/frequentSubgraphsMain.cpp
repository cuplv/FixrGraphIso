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
#include "fixrgraphiso/ilpApproxIsomorphismEncoder.h"
#include "fixrgraphiso/serialization.h"
namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::string;
using std::ofstream;


namespace fixrgraphiso{
  bool debug = false;
  int freq_cutoff=10;
  int subsumption_freq_cutoff =20;
  double gurobi_timeout = 30.0;
  string info_file_name = "cluster-info.txt";
  
  void loadACDFGFromFilename(string filename, std::vector<Acdfg*> & allACDFGs){
    AcdfgSerializer s;
    iso_protobuf::Acdfg * proto_acdfg = s.read_protobuf_acdfg(filename.c_str());
    Acdfg * acdfg = s.create_acdfg(proto_acdfg);
    acdfg -> setName(filename);
    if (acdfg -> node_count() >= 100 || acdfg -> edge_count() >= 200){
      std::cerr << "ACDFG " << filename << " too large # nodes: " << acdfg -> node_count() << " # edges : " << acdfg -> edge_count() << std::endl;
      delete(acdfg);
    } else {
      allACDFGs.push_back(acdfg);
    }
  }

  void processCommandLine(int argc, char * argv[] , vector<string> & filenames){
    
    char c;
    int index;
    while ((c = getopt(argc, argv, "df:t:o:g:"))!= -1){
	switch (c){
	case 'd':
	  fixrgraphiso::debug = true;
	  break;
	case 'f':
	  freq_cutoff = strtol(optarg, NULL, 10);  
	  break;
	case 't':
	  gurobi_timeout = strtof(optarg, NULL);
	  cout << "Setting Gurobi timeout to : " << gurobi_timeout << std::endl;
	  break;
	case 'o':
	  info_file_name = string(optarg);
	  cout << "Info will be dumped to : " << info_file_name << std::endl;
	  break;
	case 'g':
	  subsumption_freq_cutoff = strtol(optarg, NULL, 10);
	  break;
	default:
	  break;
	}
    }

    for (index = optind; index < argc; ++index){
      string fname(argv[index]);
      filenames.push_back(fname);
      std::cout << index <<  "--> " << fname << std::endl;
    }
    
    if (filenames.size() <= 0){
      cout << "Usage:" << argv[0] << " -f [frequency cutoff] -t [gurobi timeout] -o [output info filename] -g [popularity cutoff] [list of iso.bin files] " << endl;
      return;
    }
  }

  void computeAllPairwiseIsos(vector<Acdfg*> & allACDFGs){
    int n = allACDFGs.size();
    if (debug){
      cout << "# of ACDFGs loaded: " << n << endl;
    }
    for (int i = 1; i < n ; ++i){
      for (int j = 0; j +i < n; ++j){
	Acdfg * a = allACDFGs[j];
	Acdfg * b = allACDFGs[j+i];
	if (debug){
	  cout << a -> getName() << " vs. " << b -> getName() << std::endl; 
	}

	IlpApproxIsomorphism ilp(a, b);
	bool stat = ilp.computeILPEncoding();
	if (stat)
	  ilp.populateFrequencies();
      }
    }
  }

  void computeSubsumedACDFGs(std::vector<IsomorphismClass> & allIsos, std::vector<IsomorphismClass> & maximalIsos, std::vector<string> & anamolies){
    for (auto iso1: allIsos){
      bool is_subsumed = false;
      int i;
      vector<int> subsumedIsos;
      int freq = 1;
      Acdfg * a = iso1.get_acdfg();
      if (a -> method_node_count() < 3){
	std::cout << "IGNORING: " << a -> getName() << std::endl;
	anamolies.push_back(a -> getName());
	continue;
      }
      for (i = maximalIsos.size()-1; i >= 0;  --i){
	fixrgraphiso::IsomorphismClass & iso2 = maximalIsos[i];

	if (iso1.subsumes(&iso2)){
	  is_subsumed = true;
	  iso2.addSubsumingACDFG(iso1.get_acdfg() -> getName());
	  iso2.incrFrequency();
	  break;
	}
	if (iso2.subsumes(&iso1)){
	  subsumedIsos.push_back(i);
	  iso1.copySubsumingACDFGs(iso2);
	  freq = freq + iso2.getFrequency();
	}
	// } else {
	//   if (iso2.subsumes(&iso1)){
	//     iso2.incrFrequency();
	//     is_subsumed = true;
	//     break;
	//   }
	//   if (iso1.subsumes(&iso2)){
	//     subsumedIsos.push_back(i);
	//     freq = freq + iso2.getFrequency();
	//   }
	// }
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
    
  }
  
  void dumpAllIsos(std::vector<IsomorphismClass> & maximalIsos, std::vector<std::string> & anamolous, std::string infoFileName){
    std::ofstream out_file(infoFileName.c_str());
    
    out_file << "Number of Minimal Isos: " << maximalIsos.size() << endl;
    for (int k = 0; k < maximalIsos.size(); ++k){
      fixrgraphiso::IsomorphismClass & iso = maximalIsos[k];
      if (iso.getFrequency() >= subsumption_freq_cutoff){
	out_file << "Popular Pattern #"<< k << " Frequency = " << iso.getFrequency() << endl;
      } else {
	out_file << "Unpopular Pattern #" << k << " Frequency = " << iso.getFrequency() << endl;
      }
      out_file << "F: " << iso.getIsoFilename () << endl;
      std::vector<std::string> const & fnames = iso.getSubsumingACDFGs();
      for (const auto s: fnames){
	out_file << "F: "<< s << endl; 
      }
      string fname = string("iso_")+std::to_string(k)+".dot";
      out_file << "DOT: " << fname << endl;
      std::ofstream dot_file(fname.c_str());
      (iso.get_acdfg()) -> dumpToDot( dot_file);
      dot_file.close();
    }

    out_file << "Anamolous ACDFGs Frequency = " << anamolous.size() << endl;
    for (string s: anamolous){
      out_file << "X:" << s << endl;
    }
    
    out_file.close();
    
  }

  void frequentSubgraphsMain(int argc, char * argv [] ){
    vector<string> filenames;
    
    processCommandLine(argc, argv, filenames);
    vector<Acdfg*> allACDFGs;
    for (string s: filenames){
      loadACDFGFromFilename(s, allACDFGs);
    }
    computeAllPairwiseIsos(allACDFGs);
    vector<IsomorphismClass> aboveCutoff;
    vector<IsomorphismClass> maximalIsos;
    for (Acdfg * a: allACDFGs){
      Acdfg * b = a-> extractSubgraphWithFrequencyCutoff(freq_cutoff);
      b -> setName( a -> getName() );
      IsomorphismClass iso (b);
      aboveCutoff.push_back (iso);
    }
    vector<string> anamolous;
    computeSubsumedACDFGs(aboveCutoff, maximalIsos,  anamolous);
    dumpAllIsos(maximalIsos, anamolous,info_file_name);
    for (auto iso:aboveCutoff){
      delete(iso.get_acdfg());
    }
    
  }

  
}

int old_main(int argc, char * argv[]){
  
  vector<string> filenames;
  vector<fixrgraphiso::IsomorphismClass> allIsos;
  fixrgraphiso::processCommandLine(argc, argv, filenames);
  
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
  
  return 1;
}


int main(int argc, char * argv[]){
  fixrgraphiso::frequentSubgraphsMain(argc, argv);
  return 1;
}



