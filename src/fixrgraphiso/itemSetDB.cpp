#include "fixrgraphiso/itemSetDB.h"

using std::cout;
using std::endl;
namespace fixrgraphiso {
  
  
  ItemRecord::ItemRecord(std::string const & fname, set<int> const & r):record_contents(r), filename(fname)
  {}

  ItemRecord::~ItemRecord(){}

  ItemSetDB::ItemSetDB(): nItems(0){}
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
  
  void ItemSetDB::addRecord(string const & fname, set<string> const & rec){
    // 1. Compute the indices for each of strings converting them to integers
    set<int> r;
    set<string> :: const_iterator it;

    for (it = rec.begin(); it != rec.end(); ++it){
      r.insert( this -> findItemID(*it));
    }
    // 2. Add a record and update individual item frequencies for later.
    ItemRecord * irec = new ItemRecord(fname, r);
    this -> insertRecordAndUpdateFrequencies(irec);
  }

  bool ItemSetDB::findFrequentItemSetsRecursive(int freq_cutoff,
						int min_size_cutoff,
						int last_id,
						set<int> set_so_far,
						vector< set<int> > & all_sets){
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
      all_sets.push_back(set_so_far);
      return true;
    }
    return false;
  }

  void ItemSetDB::computeFrequentItemSets(int freq_cutoff, int min_size_cutoff, vector< set<string> > & result){
    vector< set<int> > all_sets ;
    set<int> set_so_far;
    this -> findFrequentItemSetsRecursive(freq_cutoff, min_size_cutoff, -1, set_so_far, all_sets);
    vector< set<int> > :: const_iterator it;
    for (it = all_sets.begin(); it != all_sets.end(); ++it ){
      set<int> const & s = *it;
      set<int>::const_iterator jt;
      set<string> str_set;
      cout << "{ " ;
      for (jt =  s.begin(); jt != s.end(); ++jt){
	int j = *jt;
	string s= this -> findItemName(j);
	str_set.insert( s );
	cout << s << ", " ;
      }
      cout << "}" << endl;
      result.push_back(str_set);
    }
  }
  
}
