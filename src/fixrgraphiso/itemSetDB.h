#ifndef D__ITEM_SET_DB__H__
#define D__ITEM_SET_DB__H__
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <cassert>


namespace fixrgraphiso {

  using std::set;
  using std::string;
  using std::map;
  using std::vector;
  
  class ItemRecord {
  protected:
    std::set<int> record_contents;
    std::string filename;
  public:
    ItemRecord(std::string const & fname, std::set<int> const & r);
    ~ItemRecord();
    std::set<int> const & get_contents() const { return record_contents;}
  };

  typedef map<std::string, int> str_int_map_t;
  
  class ItemSetDB {
  protected:
    std::map<std::string, int> item_ids;
    std::vector<std::string> item_names;
    std::vector< ItemRecord * > records;
    std::map<int, int> item_frequencies;
    std::map<int, std::vector< ItemRecord*> > idx_records;
    int nItems;
    string findItemName(int j);
    int findItemID(std::string const & str);
    void insertRecordAndUpdateFrequencies(ItemRecord * i_rec);
    void incrFrequency(int j);
    bool findFrequentItemSetsRecursive(int freq_cutoff,
				       int min_size_cutoff,
				       int last_id,
				       std::set<int> sets_so_far, 
				       std::vector< std::set<int> > & all_sets);
    void addIndex(int j, ItemRecord * i_rec);
    
  public:
    
    ItemSetDB();
    ~ItemSetDB();
    void addRecord(std::string const & filename, std::set<std::string> const & rec);

    void computeFrequentItemSets(int freq_cutoff, int min_size_cutoff, vector< set<string> > & result);
    
  };

 
}

#endif
