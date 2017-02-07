#ifndef D__ITEM_SET_DB__H__
#define D__ITEM_SET_DB__H__
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <cassert>
#include <ostream>

namespace fixrgraphiso {

  using std::set;
  using std::string;
  using std::map;
  using std::vector;
  using std::ostream;
  class ItemRecord;
  class FreqItemSet;
  class ItemSetDB;
  
  class ItemRecord {
  protected:
    std::set<int> record_contents;
    std::string filename;
  public:
    ItemRecord(std::string const & fname, std::set<int> const & r);
    ~ItemRecord();
    std::set<int> const & get_contents() const { return record_contents;}
    string get_filename() const {return filename; }
    int numberOfCommonItems(set<int> const & itemSet) const;
  };

  typedef map<std::string, int> str_int_map_t;

  class FreqItemSet {
  protected:
    set<int> s_int;
    set<string> s_string;
    set<ItemRecord*> idx_record;
    bool mergedInto;
    void mergeItemSet(FreqItemSet & what);
  public:
    FreqItemSet(set<int> const & iset,  vector<ItemRecord*> const & recs);
    FreqItemSet(set<int> const & iset,  set<ItemRecord*> const & recs);
    void prettyPrint(ostream & os, bool printRecords = true) const;
    set<string> & get_string_set_ref(){ return s_string; }
    set<int> const & get_int_set_const_ref() const { return s_int; }
    set<int> & get_int_set_ref() { return s_int; }
    set<ItemRecord*> & get_idx_record_ref() { return idx_record;}
    bool hasBeenMerged() const {return mergedInto; }
    bool mergeCompatible(FreqItemSet & what, int min_size_cutoff);
    void setMergedInto() { mergedInto = true;}
    int getFrequency() const { return idx_record.size(); }
    void addItemRecord(ItemRecord * it){ idx_record.insert(it); }
  };
  
  class ItemSetDB {
  protected:
    std::map<std::string, int> item_ids;
    std::vector<std::string> item_names;
    std::vector< ItemRecord * > records;
    std::map<int, int> item_frequencies;
    std::map<int, std::vector< ItemRecord*> > idx_records;
    std::set<string> excluded_functions;
    int nItems;
    string findItemName(int j);
    int findItemID(std::string const & str);
    void insertRecordAndUpdateFrequencies(ItemRecord * i_rec);
    void incrFrequency(int j);
    bool findFrequentItemSetsRecursive(int freq_cutoff,
				       int min_size_cutoff,
				       int last_id,
				       std::set<int> sets_so_far, 
				       std::vector< FreqItemSet> & all_sets);
    void addIndex(int j, ItemRecord * i_rec);
    void convertToStringSet(set<int> const & s, set<string> & res);
    bool is_excluded (string const & s) const;
    void addRemainingRecordsToFrequentItemSets(vector<FreqItemSet> & result);
    
  public:
    
    ItemSetDB();
    ~ItemSetDB();
    void addRecord(std::string const & filename, std::set<std::string> const & rec);
    void computeFrequentItemSets(int freq_cutoff, int min_size_cutoff, vector< FreqItemSet > & result, ostream & out);
    
  };

 
}

#endif
