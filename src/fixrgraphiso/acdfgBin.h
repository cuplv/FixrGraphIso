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

namespace fixrgraphiso {
  using std::vector;
  using std::string;
  using std::endl;
  using std::cout;
  using std::set;

  class AcdfgBin {
  public:

  AcdfgBin(Acdfg* a) : subsuming(false), anomalous(false), popular(false) {
    acdfgRepr = a;
    acdfgNames.push_back(a->getName());
  }

  bool isACDFGEquivalent(Acdfg *b);

  void insertEquivalentACDFG(Acdfg * b){
    acdfgNames.push_back(b->getName());
  }

  int getFrequency() const {
    return acdfgNames.size() ;
  }

  int getPopularity() const;

  const Acdfg* getRepresentative() const{
    return acdfgRepr;
  }

  Acdfg * getRepresentative(){
    return acdfgRepr;
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
    b->insertIncomingEdge(this);
  }

  void computeImmediatelySubsumingBins();

  bool isSubsuming() const { return subsuming; }

  bool isAnomalous() const {return anomalous; }
  void setAnomalous() { anomalous = true; }

  void setPopular() ;
  bool isPopular() const { return popular;}

  const std::vector<string>  & getAcdfgNames() const { return acdfgNames; }
  bool isAtFrontierOfPopularity(int freq_cutoff) const;
  bool hasPopularAncestor() const;

  protected:
  void addSubsumingBinsToSet(set<AcdfgBin*> & what) ;

  /* List of acdfgs contained in the Bin */
  Acdfg* acdfgRepr;
  vector<string> acdfgNames;
  /* List of bins subsumed by this bin */
  set<AcdfgBin*> subsumingBins;
  /* List of bins that are directly subsumed by this bin
     i.e. the set {b2 | SUB(this,b2) and does not exist a
     b3 such that SUB(this,b3) and SUB(b3,b2)}
  */
  set<AcdfgBin*> immediateSubsumingBins;
  /* List of bins that subsume this bin */
  set<AcdfgBin*> incomingEdges;

  /* */
  bool subsuming;
  bool anomalous;
  bool popular;
  };

  class Lattice {
  public:
    Lattice() {
    }

    void addBin(AcdfgBin* bin);
    void addPopular(AcdfgBin* bin);
    void addAnomalous(AcdfgBin* bin);
    void addIsolated(AcdfgBin* bin);

    using acdfgbins_t = vector<AcdfgBin*>;
    using bin_iterator = acdfgbins_t::const_iterator;
    bin_iterator beginAllBins() const { return allBins.begin(); }
    bin_iterator endAllBins() const { return allBins.end(); }
    bin_iterator beginPopular() const { return popularBins.begin(); }
    bin_iterator endPopular() const { return popularBins.end(); }
    bin_iterator beginAnomalous() const { return anomalousBins.begin(); }
    bin_iterator endAnomalous() const { return anomalousBins.end(); }
    bin_iterator beginIsolated() const { return isolatedBins.begin(); }
    bin_iterator endIsolated() const { return isolatedBins.end(); }

    void sortByFrequency();

    void dumpAllBins(std::chrono::seconds time_taken,
                     const string & output_prefix,
                     const string & infoFileName);

  private:
    vector<AcdfgBin*> allBins;
    vector<AcdfgBin*> popularBins;
    vector<AcdfgBin*> anomalousBins;
    vector<AcdfgBin*> isolatedBins;
  };

}
#endif


