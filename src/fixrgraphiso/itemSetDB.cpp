#include "fixrgraphiso/itemSetDB.h"
#include <algorithm>

using std::cout;
using std::endl;

namespace fixrgraphiso {
  
  
  ItemRecord::ItemRecord(std::string const & fname, set<int> const & r):record_contents(r), filename(fname)
  {}

  ItemRecord::~ItemRecord(){}

  FreqItemSet::FreqItemSet(set<int> const & iset,  vector<ItemRecord*> const & recs):s_int(iset),  idx_record(recs){}

  void FreqItemSet::prettyPrint(ostream & os) const {
    // First print the items
    os << "I: ";
    string sep = " ";
    set<string>::const_iterator it;
    for (it = s_string.begin(); it != s_string.end(); ++it){
      os << sep << *it;
      sep = ", ";
    }
    os << "( "<< idx_record.size()<< " )" << endl;
    // Second print the filenames that identify each record
    vector<ItemRecord*> :: const_iterator jt;
    for (jt = idx_record.begin(); jt != idx_record.end(); ++jt){
      ItemRecord const * irec = *jt;
      os << "F: " << irec -> get_filename() << endl;
    }
    os << "E" << endl;
  }
  
  ItemSetDB::ItemSetDB(): nItems(0){
    excluded_functions.insert(string("NEQ"));
    excluded_functions.insert(string("EQ"));
    excluded_functions.insert(string("LT"));
    excluded_functions.insert(string("GT"));
    excluded_functions.insert(string("LE"));
    excluded_functions.insert(string("GE"));
  }
  
  ItemSetDB::~ItemSetDB(){}

  int ItemSetDB::findItemID(string const & str){
    str_int_map_t::const_iterator it = item_ids.find(str);
    if (it == item_ids.end()){
      item_ids[str] = nItems;
      item_names.push_back(str);
      nItems ++;
      assert(item_names.size() == nItems);
      return (nItems -1 );
    } else {
      return it -> second;
    }
  }

  string ItemSetDB::findItemName(int j){
    assert(0 <= j && j < item_names.size());
    return this -> item_names[j];
  }
  
  void ItemSetDB::incrFrequency(int j){
    if (item_frequencies.find(j) == item_frequencies.end())
      item_frequencies[j] = 1;
    else
      item_frequencies[j] += 1;
  }

  void ItemSetDB::addIndex(int j, ItemRecord* i_rec){
    if (idx_records.find(j) == idx_records.end()){
      vector<ItemRecord *> nVec;
      nVec.push_back(i_rec);
      idx_records[j] = nVec;
    } else {
      idx_records[j].push_back(i_rec);
    }
  }

  void ItemSetDB::insertRecordAndUpdateFrequencies(ItemRecord * i_rec){
    records.push_back(i_rec);
    set<int> const & c = i_rec -> get_contents();
    set<int> :: const_iterator it;
    for (it = c.begin(); it != c.end(); ++it){
      this -> addIndex(*it, i_rec);
      this -> incrFrequency(*it);
    }
  }

  bool ItemSetDB::is_excluded(string const & s) const{
    return (excluded_functions.find(s) != excluded_functions.end());
  }
  
  void ItemSetDB::addRecord(string const & fname, set<string> const & rec){
    // 1. Compute the indices for each of strings converting them to integers
    set<int> r;
    set<string> :: const_iterator it;

    for (it = rec.begin(); it != rec.end(); ++it){
      if (! is_excluded(*it))
	r.insert( this -> findItemID(*it));
    }
    // 2. Add a record and update individual item frequencies for later.
    ItemRecord * irec = new ItemRecord(fname, r);
    this -> insertRecordAndUpdateFrequencies(irec);
  }

  bool set_contains(FreqItemSet const & f, set<int> const & b){
    set<int> const & a = f.get_int_set_const_ref();
    return std::includes(a.begin(), a.end(), b.begin(), b.end() );
  }
  
  bool is_subset(vector< FreqItemSet > const & all_sets, set<int> const & s){
    vector< FreqItemSet > :: const_iterator it;
    for (it = all_sets.begin(); it != all_sets.end(); ++it){
      if (set_contains(*it, s))
	return true;
    }

    return false;
  }

  void ItemSetDB::convertToStringSet(set<int> const & s, set<string> & res){
    set<int>::const_iterator it;
    for (it = s.begin(); it != s.end(); ++it){
      int j = *it;
      assert (j >= 0);
      assert (j < nItems);
      res.insert( this -> item_names[j]);
    }
  }
  
  bool ItemSetDB::findFrequentItemSetsRecursive(int freq_cutoff,
						int min_size_cutoff,
						int last_id,
						set<int> set_so_far,
						vector< FreqItemSet > & all_sets){
    /*-- At the top level, iterate through the hash map of all item frequencies and
      collect those whose frequencies are above the minimum cutoff --*/
    map<int, vector<ItemRecord*> > ::const_iterator it;
    bool something = false; 
    for (it = idx_records.begin(); it != idx_records.end(); ++it){
      int id = it -> first;
      if (id > last_id){
	vector<ItemRecord *> const & v = it -> second;
	if (v.size() >= freq_cutoff){
	  /*-- Create a new item db with the records under v --*/
	  set<int> new_set_so_far = set_so_far;
	  new_set_so_far.insert(id);
	  ItemSetDB childDB;
	  vector<ItemRecord*> :: const_iterator jt;
	  for (jt = v.begin(); jt != v.end(); ++jt){
	    childDB.insertRecordAndUpdateFrequencies(*jt);
	  }
	  bool s = childDB.findFrequentItemSetsRecursive(freq_cutoff, min_size_cutoff, id, new_set_so_far, all_sets);
	  if (s)
	    something = true;
	}
      }
    }
    
    if (!something && set_so_far.size() >= min_size_cutoff){
      if (!is_subset(all_sets, set_so_far)) {
	FreqItemSet f( set_so_far, this -> records);
	all_sets.push_back(f);
	return true;
      }
    }
    return false;
  }

  void ItemSetDB::computeFrequentItemSets(int freq_cutoff, int min_size_cutoff, vector< FreqItemSet > & result, ostream & out ){
    set<int> set_so_far;
    this -> findFrequentItemSetsRecursive(freq_cutoff, min_size_cutoff, -1, set_so_far, result);
    vector< FreqItemSet > :: iterator it;
    // Convert the result into vector of strings
    for (it = result.begin(); it != result.end(); ++it ){
      FreqItemSet & s = *it;
      set<int> const & i_set = s.get_int_set_const_ref();
      set<string> & s_set = s.get_string_set_ref();
      this -> convertToStringSet(i_set, s_set);
      s.prettyPrint(out);
    }

    
  }
  
}
