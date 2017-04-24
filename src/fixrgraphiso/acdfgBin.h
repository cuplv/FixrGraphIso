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
#include <set>
#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso{
  using std::vector;
  using std::string;
  using std::endl;
  using std::cout;
  using std::set;
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


    int getPopularity() const;

    const Acdfg* getRepresentative() const{
      assert(acdfgs.size() > 0);
      return *(acdfgs.begin());
    }

    Acdfg * getRepresentative(){
      assert(acdfgs.size() > 0);
      return *(acdfgs.begin());
    }

    void printInfo(std::ostream & out, bool printAbove = true) const;

    void dumpToDot(string fileName) const;
    void dumpToProtobuf(string fileName) const;

    bool isACDFGBinSubsuming(AcdfgBin * b);
    void insertIncomingEdge(AcdfgBin * c){
      incomingEdges.insert(c);
    }
    bool hasSubsumingBin(AcdfgBin* b){
      return (subsumingBins.find(b) != subsumingBins.end());
    }
    void addSubsumingBin(AcdfgBin * b){
      subsumingBins.insert(b);
      b-> insertIncomingEdge(this);
    }

    void computeImmediatelySubsumingBins();

    bool isSubsuming() const { return subsuming; }
    void setSubsuming()  {subsuming = true; }

    bool isAnomalous() const {return anomalous; }
    void setAnomalous() { anomalous = true; }

    void setPopular() ;
    bool isPopular() const { return popular;}

    const std::vector<Acdfg*>  & getACDFGs() const { return acdfgs; }
    bool isAtFrontierOfPopularity(int freq_cutoff) const;
    bool hasPopularAncestor() const;

  protected:

    void addSubsumingBinsToSet(set<AcdfgBin*> & what) ;

    vector<Acdfg*> acdfgs;
    set<AcdfgBin*> subsumingBins;
    set<AcdfgBin*> immediateSubsumingBins;
    set<AcdfgBin*> incomingEdges;
    bool subsuming;
    bool anomalous;
    bool popular;

  };

}
#endif


