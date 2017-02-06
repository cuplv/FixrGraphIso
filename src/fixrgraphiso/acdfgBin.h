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

  AcdfgBin(Acdfg* a):subsuming(false), anomalous(false), popular(false){
      acdfgs.push_back(a);
    }
    
    bool isACDFGEquivalent(Acdfg *b);

    void insertEquivalentACDFG(Acdfg * b){
      acdfgs.push_back(b);
    }

    int getFrequency() const {
      return acdfgs.size() ;
    }

    int getSubsumingFrequency() const {
      return subsumingBins.size();
    }

    int getPopularity() const{
      return getFrequency() + getSubsumingFrequency();
    }

    Acdfg * getRepresentative(){
      assert(acdfgs.size() > 0);
      return *(acdfgs.begin());
    }

    void printInfo(std::ostream & out) const;

    void dumpToDot(string fileName) const;

    bool isACDFGBinSubsuming(AcdfgBin * b);

    void addSubsumingBin(AcdfgBin * b){
      subsumingBins.push_back(b);
    }

    bool isSubsuming() const { return subsuming; }
    void setSubsuming()  {subsuming = true; }

    bool isAnomalous() const {return anomalous; }
    void setAnomalous() { anomalous = true; }

    void setPopular() { popular = true;}
    bool isPopular() { return popular;}

  protected:

    vector<Acdfg*> acdfgs;
    vector<AcdfgBin*> subsumingBins;
    bool subsuming;
    bool anomalous;
    bool popular;
    
  };

}
#endif


