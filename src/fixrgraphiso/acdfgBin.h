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
#include "fixrgraphiso/isomorphismClass.h"

namespace fixrgraphiso {
  using std::vector;
  using std::string;
  using std::endl;
  using std::cout;
  using std::set;

  class AcdfgBin {
  public:

  AcdfgBin(Acdfg* a) : subsuming(false),
      anomalous(false), popular(false), isolated(false) {
    acdfgRepr = a;
    acdfgNames.push_back(a->getName());
  }

  ~AcdfgBin() {
    /* free the isomorphism relation */
    for (auto it = acdfgNameToIso.begin();
         it != acdfgNameToIso.end(); it++) {
      delete(it->second);
    }
  }

  bool isACDFGEquivalent(Acdfg *b, IsoRepr* iso);

  void insertEquivalentACDFG(const string b, IsoRepr* iso){
    acdfgNames.push_back(b);
    acdfgNameToIso[b] = iso;
  }

  void insertEquivalentACDFG(Acdfg * b, IsoRepr* iso){
    insertEquivalentACDFG(b->getName(), iso);
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
  void setSubsuming() { subsuming = true; }
  bool isAnomalous() const {return anomalous; }
  void setAnomalous() { anomalous = true; }
  bool isIsolated() const {return isolated; }
  void setIsolated() { isolated = true; }
  void setPopular();
  bool isPopular() const { return popular;}

  bool isClassified() const { return popular || anomalous || isolated;}

  const std::vector<string>  & getAcdfgNames() const { return acdfgNames; }
  bool isAtFrontierOfPopularity(int freq_cutoff) const;
  bool hasPopularAncestor() const;

  const std::set<AcdfgBin*> & getSubsumingBins() const {
    return subsumingBins;
  }

  const std::set<AcdfgBin*> & getImmediateSubsumingBins() const {
    return immediateSubsumingBins;
  }

  const std::set<AcdfgBin*> & getIncomingEdges() const {
    return incomingEdges;
  }

  const map<string, IsoRepr*> & getAcdfgNameToIso() const {
    return acdfgNameToIso;
  }

  protected:
  void addSubsumingBinsToSet(set<AcdfgBin*> & what) ;

  /* List of acdfgs contained in the Bin */
  Acdfg* acdfgRepr;
  vector<string> acdfgNames;
  map<string, IsoRepr*> acdfgNameToIso;

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
  bool isolated;
  bool popular;
  };

  class Lattice {
  public:
    Lattice() {};
    Lattice(const vector<string> & methodNames);

    void addMethodName(const string& methodName) { methodNames.push_back(methodName);}
    const vector<string> & getMethodNames() const { return methodNames; }

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

    const vector<AcdfgBin*> getAllBins() const {return allBins;};
    const vector<AcdfgBin*> getPopularBins() const {return popularBins;};
    const vector<AcdfgBin*> getAnomalousBins() const {return anomalousBins;};
    const vector<AcdfgBin*> getIsolatedBins() const {return isolatedBins;};

    void sortByFrequency();

    void dumpAllBins(std::chrono::seconds time_taken,
                     const string & output_prefix,
                     const string & infoFileName);

    void dumpToDot(const string & dotFile,
                   const bool onlyClassified);

  private:
    vector<string> methodNames;
    vector<AcdfgBin*> allBins;
    vector<AcdfgBin*> popularBins;
    vector<AcdfgBin*> anomalousBins;
    vector<AcdfgBin*> isolatedBins;
  };

}
#endif


