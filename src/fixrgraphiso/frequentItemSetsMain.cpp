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
#include "fixrgraphiso/proto_iso.pb.h"
#include "fixrgraphiso/proto_acdfg.pb.h"
#include "fixrgraphiso/itemSetDB.h"


namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::string;

namespace fixrgraphiso{
  bool loadACDFG = true;
  int freq = 200;
  int min_size = 4;

  iso_protobuf::Acdfg* loadACDFGFromFile(std::string const & file_name){
    std::fstream inp_file(file_name, std::ios::in | std::ios::binary);
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
      const iso_protobuf::Acdfg_MethodNode & proto_node =
	acdfg -> method_node(j);
      std::string const & str = proto_node.name();
      cout << sep << str ;
      sep = ", ";
      mCalls.insert(str);
    }
    cout << endl;
    items -> addRecord(filename, mCalls);
  }

  iso_protobuf::Iso * loadIsomorphismFromFile(std::string const & file_name){
    std::ifstream inp_file(file_name, std::ios::in | std::ios::binary);
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
    cout << endl;
    for (i = 0; i < n ; ++i){
      std::string const & str = proto_iso -> methodcallnames(i);
      cout << str <<",";
      mCalls.insert(str);
    }
    cout << endl;
    items-> addRecord(filename, mCalls);
  }
  
  
}


int main (int argc, char *argv[]) {
   /* input should be a name of a list of iso files to be processed */
  if (argc <= 1){
    std::cout << " Usage " << argv[0] << " [name of input file] " << std::endl;
    return 1;
  }

  const char * fName = argv[1];
  std::ifstream inp_file(fName);
  std::string line;
  fixrgraphiso::ItemSetDB allItems;
  
  while (std::getline(inp_file, line)){
    if (!line.empty() && line[line.length() -1] == '\n')
      line.erase(line.length() -1);
    cout << "Reading file" << line << endl;
    if (fixrgraphiso::loadACDFG){
      iso_protobuf::Acdfg * acdfg = fixrgraphiso::loadACDFGFromFile(line);
      fixrgraphiso::captureItemSetFromACDFG(acdfg,line, &allItems);
    } else {
      iso_protobuf::Iso * iso = fixrgraphiso::loadIsomorphismFromFile(line);
      fixrgraphiso::captureItemSetFromIsomorphism(iso,line, &allItems);
    }
  }
  
  /* output should be a list of frequent item sets and corresponding
     list of the isomorphisms under those */
  vector< set<string> > result;
  allItems.computeFrequentItemSets(fixrgraphiso::freq,fixrgraphiso::min_size,result);
  google::protobuf::ShutdownProtobufLibrary();
  
}
