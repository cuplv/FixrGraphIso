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
#include <chrono>
#include "fixrgraphiso/acdfg.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/collectStats.h"

namespace fixrgraphiso {
  using std::vector;
  using std::string;
  using std::endl;
  using std::cout;
  using std::set;

  class AcdfgBin {
  public:

  enum SubsRel
  {
    EQUIVALENT,
    SUBSUMED,
    SUBSUMING,
    NONE
  };

  AcdfgBin(Acdfg* a, Stats* stats) : subsuming(false),
      anomalous(false), popular(false), isolated(false) {
    acdfgRepr = a;
    IsoRepr* iso = new IsoRepr(a);
    insertEquivalentACDFG(a, iso);
    isImmediateSubsumingUpdate = true;
    this->stats = stats;
  }

  ~AcdfgBin() {
    /* free the isomorphism relation */
    for (auto it = acdfgNameToIso.begin();
         it != acdfgNameToIso.end(); it++) {
    }

    // TODO: Free the acdfgs
  }

  bool isACDFGEquivalent(Acdfg *b, IsoRepr* iso);
  SubsRel compareACDFG(Acdfg *b,  IsoRepr* iso,
                       const bool canSubsume,
                       const bool canBeSubsumed);

  void insertEquivalentACDFG(const string b, IsoRepr* iso){
    /* cout << "ACDFG REPR: " << b << */
    /*   " -- SIZE: " << (iso->getNodesRel()).size() << endl; */
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
    isImmediateSubsumingUpdate = false;
  }
  bool hasSubsumingBin(AcdfgBin* b){
    return (subsumingBins.find(b) != subsumingBins.end());
  }
  void addSubsumingBin(AcdfgBin * b){
    subsumingBins.insert(b);
    b->insertIncomingEdge(this);
    isImmediateSubsumingUpdate = false;
  }

  void computeImmediatelySubsumingBins();

  static void getReachable(const set<AcdfgBin*> &initial,
                           set<AcdfgBin*> &reachable,
                           const bool invert);

  bool isSubsuming() const { return subsuming; }
  void setSubsuming() { subsuming = true; }
  bool isAnomalous() const {return anomalous; }
  void setAnomalous() { anomalous = true; }
  bool isIsolated() const {return isolated; }
  void setIsolated() { isolated = true; }
  void setPopular();
  void setPopular(bool allowSubsuming);
  bool isPopular() const { return popular;}

  int getCumulativeFrequency() const { return cumulativeFrequency;}
  void setCumulativeFrequency(const int cumulativeFrequency) {
    this->cumulativeFrequency = cumulativeFrequency;}

  bool isClassified() const { return popular || anomalous || isolated;}

  const std::vector<string>  & getAcdfgNames() const { return acdfgNames; }
  bool isAtFrontierOfPopularity(int freq_cutoff);
  bool hasPopularAncestor() const;

  const std::set<AcdfgBin*> & getSubsumingBins() const {
    return subsumingBins;
  }

  const std::set<AcdfgBin*> & getImmediateSubsumingBins() {
    if (! isImmediateSubsumingUpdate) {
      computeImmediatelySubsumingBins();
    }

    return immediateSubsumingBins;
  }

  const std::set<AcdfgBin*> & getIncomingEdges() const {
    return incomingEdges;
  }

  const map<string, IsoRepr*> & getAcdfgNameToIso() const {
    return acdfgNameToIso;
  }

  void getRepr(std::ostream& out) const {
    out << this <<
      ", " << this->getCumulativeFrequency() <<
      ", " << this->getFrequency() <<
      ", " << this->isPopular() <<
      ", " << this->getRepresentative()->source_info.package_name <<
      ", " << this->getRepresentative()->source_info.method_name <<
      endl;
  }


  void resetClassification();

  Stats* getStats() { return stats; }

  protected:
  void addSubsumingBinsToSet(set<AcdfgBin*> & what) ;

  /* List of acdfgs contained in the Bin */
  Acdfg* acdfgRepr;
  vector<string> acdfgNames;
  map<string, IsoRepr*> acdfgNameToIso;

  /* List of bins that subsumes this bin
     I.e., all the bins that contains this bin
     {b | SUBSUMES(b, this)},
   */
  set<AcdfgBin*> subsumingBins;
  /* List of bins that directly subsumes by this bin
     i.e. the set {b2 | SUB(this,b2) and does not exist a
     b3 such that SUB(this,b3) and SUB(b3,b2)}
  */
  set<AcdfgBin*> immediateSubsumingBins;
  /* List of bins that are subsumed this bin */
  set<AcdfgBin*> incomingEdges;

  /* True if the bin subsumes a popular bin */
  bool subsuming;
  bool anomalous;
  bool isolated;
  bool popular;

  // store the cumulative frequency of the bin
  int cumulativeFrequency;
  bool isImmediateSubsumingUpdate;

  Stats *stats;
  };

  class Lattice {
  public:
    Lattice() {};
    Lattice(const vector<string> & methodNames);
    Lattice(const Stats stats);
    ~Lattice();

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

    const vector<AcdfgBin*> & getAllBins() const {return allBins;};
    const vector<AcdfgBin*> & getPopularBins() const {return popularBins;};
    const vector<AcdfgBin*> & getAnomalousBins() const {return anomalousBins;};
    const vector<AcdfgBin*> & getIsolatedBins() const {return isolatedBins;};

    void buildTr(map<AcdfgBin*, set<AcdfgBin*>*> & tr) const;
    void reverseTr(const map<AcdfgBin*, set<AcdfgBin*>*> & tr,
                   map<AcdfgBin*, set<AcdfgBin*>*> & inverse) const;
    static void deleteTr(map<AcdfgBin*, set<AcdfgBin*>*> & tr);
    void computeTopologicalOrder(vector<AcdfgBin*> &order) const;

    void makeClosure();

    void sortByFrequency();

    void sortAllByFrequency();

    void resetClassification();

    void dumpAllBins(std::chrono::seconds time_taken,
                     const string & output_prefix,
                     const string & infoFileName,
                     const string & latticeFileName);

    void dumpToDot(const string & dotFile,
                   const bool onlyClassified);

    void getAcdfgBin2id(map<AcdfgBin*, int> &acdfgBin2idMap) const;

    int countCommonMethods(const Lattice &other) const;

    bool isValid() const;

    Stats* getStats() { return &stats; }
    const Stats getStats() const { return stats; };

  private:
    vector<string> methodNames;
    vector<AcdfgBin*> allBins;
    vector<AcdfgBin*> popularBins;
    vector<AcdfgBin*> anomalousBins;
    vector<AcdfgBin*> isolatedBins;
    Stats stats;
  };

}
#endif
