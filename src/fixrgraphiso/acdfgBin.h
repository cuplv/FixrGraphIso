/*
 * acdfgBin.h
 *
 *  Created on: Feb 5, 2017
 *      Author: Sriram Sankaranarayanan
 */
#ifndef D__ACDFG__BIN__H_
#define D__ACDFG__BIN__H_
#include <vector>
#include <map>
#include <string>
#include <ostream>
#include <cassert>
#include <iostream>
#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso{
  using std::vector;
  using std::string;
  using std::endl;
  using std::cout;

  class AcdfgBin {
  public:
    AcdfgBin(Acdfg* a){
      acdfgs.push_back(a);
    }
    bool isACDFGEquivalent(Acdfg *b);
    void insertEquivalentACDFG(Acdfg * b){
      acdfgs.push_back(b);
    }
    int getFrequency() const {
      return acdfgs.size();
    }

    void printInfo(std::ostream & out) const;
    void dumpToDot(string fileName) const;
		
  protected:
    vector<Acdfg*> acdfgs;

  };

}
#endif


