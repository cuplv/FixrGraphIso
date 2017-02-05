/*--
    1. Load a list of computed isomorphisms from a file
    2. For each iso get the feature list
    3. Build frequent item sets
--*/


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
#include "fixrgraphiso/itemSetDB.h"


namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::string;
using std::ofstream;

namespace fixrgraphiso{
  bool loadACDFG = true;
  int freq = 50;
  int min_size = 4;
  string outputFileName = "item_sets_out.txt";
  bool debug = false;
  
  iso_protobuf::Acdfg* loadACDFGFromFile(std::string const & file_name){
    std::fstream inp_file(file_name.c_str(), std::ios::in | std::ios::binary);
    iso_protobuf::Acdfg * acdfg = new iso_protobuf::Acdfg();
    if (inp_file.is_open()){
      acdfg -> ParseFromIstream(&inp_file);
    } else {

      assert(false);
    }
    inp_file.close();
    return acdfg;
  }

  void captureItemSetFromACDFG(iso_protobuf::Acdfg * acdfg, const string & filename, ItemSetDB* items){
    /* -- Use the contents of method nodes to extract the record --*/
    std::set<string> mCalls;
    std::string sep = " ";
    for (int j = 0; j < acdfg -> method_node_size(); ++j){
      const iso_protobuf::Acdfg_MethodNode & proto_node = acdfg -> method_node(j);
      std::string const & str = proto_node.name();
      if (debug) cout << sep << str ;
      sep = ", ";
      mCalls.insert(str);
    }
    if (debug) cout << endl;
    items -> addRecord(filename, mCalls);
  }

#ifdef OLD__UNUSED_CODE

  iso_protobuf::Iso * loadIsomorphismFromFile(std::string const & file_name){
    std::ifstream inp_file(file_name.c_str(), std::ios::in | std::ios::binary);
    iso_protobuf::Iso * proto_iso = new iso_protobuf::Iso();
    if (inp_file.is_open())
      proto_iso -> ParseFromIstream (&inp_file);
    else
      assert (false);
    inp_file.close();
    return proto_iso;
  }

  void captureItemSetFromIsomorphism(iso_protobuf::Iso * proto_iso, const string & filename, ItemSetDB* items ){
    int n = proto_iso -> methodcallnames_size();
    int i;
    std::set<string> mCalls;
    string sep="";
    if (debug) cout << endl;
    for (i = 0; i < n ; ++i){
      std::string const & str = proto_iso -> methodcallnames(i);
      if (debug) cout << sep<< str ;
      mCalls.insert(str);
    }
    if (debug) cout << endl;
    items-> addRecord(filename, mCalls);
  }
  
#endif
  
}




int main (int argc, char *argv[]) {
   /* input should be a name of a list of iso files to be processed */
  
  const char * fName = NULL;
  while	(optind	< argc){
    char c;
    if ( (c = getopt(argc, argv, "f:m:o:di")) != -1){
      switch (c){
      case 'f':
	fixrgraphiso::freq = strtol(optarg, NULL, 10);
	cout << "Setting minimum frequency to " << fixrgraphiso::freq << endl;
	break;
      case 'm':
	fixrgraphiso::min_size = strtol(optarg, NULL, 10);
	cout << "Setting minimum set size cutoff to " << fixrgraphiso::min_size << endl;
	break;
      case 'd':
	fixrgraphiso::debug = true;
	
	break;
#ifdef OLD__UNUSED_CODE
      case 'i':
	fixrgraphiso::loadACDFG = false;
	break;
#endif
      case 'o':
	fixrgraphiso::outputFileName = string(optarg);
	cout << "Setting output file to : " << fixrgraphiso::outputFileName << endl;
	break;
      default:
	break;
      }
    } else {
      fName = argv[optind];
      optind++;
    }
  }


  if (fName == NULL){
    cout << "Usage: " << argv[0] << "[-f freq -m min_size -o out_file_name -d] name_of_file" << endl;
    return 1;
  }
  

  std::ifstream inp_file(fName);
  std::string line;
  fixrgraphiso::ItemSetDB allItems;
  
  while (std::getline(inp_file, line)){
    if (!line.empty() && line[line.length() -1] == '\n')
      line.erase(line.length() -1);
    if (fixrgraphiso::debug) cout << "Reading file" << line << endl;
#ifndef OLD__UNUSED_CODE
    assert(fixrgraphiso::loadACDFG);
#endif
    if (fixrgraphiso::loadACDFG){
      iso_protobuf::Acdfg * acdfg = fixrgraphiso::loadACDFGFromFile(line);
      fixrgraphiso::captureItemSetFromACDFG(acdfg,line, &allItems);
    } else {
#ifdef OLD__UNUSED_CODE
      iso_protobuf::Iso * iso = fixrgraphiso::loadIsomorphismFromFile(line);
      fixrgraphiso::captureItemSetFromIsomorphism(iso,line, &allItems);
#endif
    }
  }
  
  /* output should be a list of frequent item sets and corresponding
     list of the isomorphisms under those */
  ofstream out_file (fixrgraphiso::outputFileName.c_str());
  cout << " Frequent Item Sets computed " << endl;
  vector< fixrgraphiso::FreqItemSet > result;
  allItems.computeFrequentItemSets(fixrgraphiso::freq,fixrgraphiso::min_size,result, out_file);
  out_file.close();
  google::protobuf::ShutdownProtobufLibrary();
  return 1;
}
