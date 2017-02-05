#include "fixrgraphiso/itemSetDB.h"
#include <algorithm>

using std::cout;
using std::endl;

namespace fixrgraphiso {
  extern bool debug;
  
  ItemRecord::ItemRecord(std::string const & fname, set<int> const & r):record_contents(r), filename(fname)
  {}

  ItemRecord::~ItemRecord(){}

  FreqItemSet::FreqItemSet(set<int> const & iset,  vector<ItemRecord*> const & recs):s_int(iset),  idx_record(recs), mergedInto(false){}

  void FreqItemSet::prettyPrint(ostream & os, bool printRecords) const {
    // First print the items
    os << "I: ";
    string sep = " ";
    for (const auto it : s_string){
      os << sep << it;
      sep = ", ";
    }
    os << "( "<< idx_record.size()<< " )" << endl;
    // Second print the filenames that identify each record
    //vector<ItemRecord*> :: const_iterator jt;
    if (printRecords){
      for (const auto irec: idx_record){
	os << "F: " << irec -> get_filename() << endl;
      }
      os << "E" << endl;
    }
  }

  void FreqItemSet::mergeItemSet(FreqItemSet & what, set<ItemRecord*> & set_this){
    set<int> & what_s_int = what.get_int_set_ref();
    set<string> & what_s_string = what.get_string_set_ref();
    vector<ItemRecord*> what_idx_record = what.get_idx_record_ref();
    for (int t: what_s_int){
      s_int.insert(t);
    }

    for (string s: what_s_string){
      s_string.insert(s);
    }

    for (ItemRecord * w: what_idx_record){
      if (set_this.find(w) == set_this.end())
	idx_record.push_back(w);
    }

    what.mergedInto = true;
  }
  
  bool FreqItemSet::mergeCompatible(FreqItemSet & what)  {
    std::set<ItemRecord*> set_this, set_what;
    for (ItemRecord* it: idx_record){
      set_this.insert(it);
    }
    int count1=0;
    vector<ItemRecord*> what_idx_record = what.get_idx_record_ref();
    for (ItemRecord * it : what_idx_record){
      set_what.insert(it);
      if (set_this.find(it) != set_this.end())
	count1 ++;
    }
    if (count1 < set_what.size()/2)
      return false;
    
    int count2 = 0;
    for (ItemRecord * it : set_this){
      if (set_what.find(it) != set_what.end())
	count2++;
    }

    if (count2 < set_this.size()/2)
      return false;

    if (debug){
      cout << "Merging items --> ";
      what.prettyPrint(std::cout, false);
      cout << "into" ;
      this -> prettyPrint(std::cout, false);
      cout << endl;
    }
    this -> mergeItemSet(what, set_this);
    what.setMergedInto();
    return true;
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
    const auto it = item_ids.find(str);
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
    for (auto it = c.begin(); it != c.end(); ++it){
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
    for (auto it = rec.begin(); it != rec.end(); ++it){
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
    //vector< FreqItemSet > :: const_iterator it;
    for (auto it = all_sets.cbegin(); it != all_sets.cend(); ++it){
      if (set_contains(*it, s))
	return true;
    }

    return false;
  }

  void ItemSetDB::convertToStringSet(set<int> const & s, set<string> & res){
    for (auto it = s.begin(); it != s.end(); ++it){
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
    // Merge frequent item sets according to merge criteria
    int n = result.size();
    for (int i = 0; i < n-1; ++i){
      FreqItemSet & item_i = result[i];
      if (!item_i.hasBeenMerged()){
	for (int j  = i+1; j < n; ++j){
	  FreqItemSet & item_j = result[j];
	  item_i.mergeCompatible(item_j);
	 
	}
      }
    }
     
    // Detect potential anamolies for frequent item sets
    
    // Convert the result into vector of strings
    for (it = result.begin(); it != result.end(); ++it ){
      FreqItemSet & s = *it;
      if (!s.hasBeenMerged()){
	set<int> const & i_set = s.get_int_set_const_ref();
	set<string> & s_set = s.get_string_set_ref();
	this -> convertToStringSet(i_set, s_set);
	s.prettyPrint(out);
      }
    }
  }
  
}
