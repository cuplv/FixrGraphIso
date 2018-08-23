/*
 */
#ifndef D__SEARCH_LATTICE__H_
#define D__SEARCH_LATTICE__H_

#include <vector>
#include <map>
#include <string>
#include <set>
#include "fixrgraphiso/acdfgBin.h"
#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso {

  class SearchLattice {
  public:
    SearchLattice(Acdfg* query, Lattice* lattice) {
      this->query = query;
      this->lattice = lattice;
    }

    void search();

  private:
    Lattice* lattice;
    Acdfg* query;
  };
}


#endif
