#include <ostream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <map>
#include "acdfgBin.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/collectStats.h"
#include "fixrgraphiso/serializationLattice.h"

namespace fixrgraphiso {

  using std::ofstream;

  /**
   * Return true iff the bin b subsumes this bin
   * (i.e., this bin is submsumed by b)
   */
  bool AcdfgBin::isACDFGBinSubsuming(AcdfgBin * b){
    // First check the graph so far
    for (AcdfgBin * c : subsumingBins){
      if (c -> hasSubsumingBin(b)){
        return true;
      }
    }

    Acdfg * repr_a = getRepresentative();
    Acdfg * repr_b = b -> getRepresentative();

    IsoSubsumption d(repr_b, repr_a);
    return d.check();
  }

  bool USE_INC_ISO = true;
  bool AcdfgBin::isACDFGEquivalent(Acdfg * b, IsoRepr* iso) {
    if (! USE_INC_ISO) {
      IsoSubsumption dir_a (acdfgRepr, b);
      IsoSubsumption dir_b (b, acdfgRepr);

      if (! dir_a.checkNodeCounts() || ! dir_b.checkNodeCounts()){
        if (debug){
          cout << "Subsumption ruled out directly " << endl;
        }

        /* [SM] HACK */
        addSubsumptionCheck(); /* count a direction */
        addSubsumptionCheck(); /* count b direction */

        return false;
      }

      if (! dir_a.check()){
        if (debug){
          cout << "Subsumption bin -> b ruled out " << endl;
        }

        /* [SM] HACK */
        addSubsumptionCheck(); /* count b direction */

        return false;
      }

      if (! dir_b.check(iso)){
        if (debug){
          cout << "Subsumption b -> bin ruled out " << endl;
        }
        return false;
      }
    } else {
      IsoSubsumption dir_b (b, acdfgRepr);
      if (! dir_b.check_iso(iso)) {
        if (debug){
          cout << "Subsumption b -> bin ruled out " << endl;
        }
        return false;
      }
    }

    if (debug){
      cout << "Equivalent ACDFGs ! " <<endl;
    }

    return true;
  }

  AcdfgBin::SubsRel AcdfgBin::compareACDFG(Acdfg *b,  IsoRepr* iso,
                                           const bool canSubsume,
                                           const bool canBeSubsumed) {
    bool bin_subsumes_b;
    bool b_subsumes_bin;

    IsoSubsumption dir_a (acdfgRepr, b); // bin subsumes b
    IsoSubsumption dir_b (b, acdfgRepr); // b subsumes bin

    // The order depends on the search algorithm
    addSubsumptionCheck();
    if ((!canSubsume) || (! dir_b.checkNodeCounts())) {
      b_subsumes_bin = false;
    } else {
      b_subsumes_bin = dir_b.check(iso);
    }

    addSubsumptionCheck();
    if ( (! canBeSubsumed) || (! dir_a.checkNodeCounts())) {
      bin_subsumes_b = false;
    } else {
      bin_subsumes_b = dir_a.check();
    }

    if (bin_subsumes_b && b_subsumes_bin)
      return AcdfgBin::EQUIVALENT;
    else if (bin_subsumes_b)
      return AcdfgBin::SUBSUMED;
    else if (b_subsumes_bin)
      return AcdfgBin::SUBSUMING;
    else
      return AcdfgBin::NONE;
  }

  int AcdfgBin::getPopularity() const {
    int f = getFrequency();
    for (const AcdfgBin* a : subsumingBins){
      f += a -> getFrequency();
    }
    return f;
  }

  void AcdfgBin::printInfo(std::ostream & out, bool printAbove) const {

    for (const string acdfgName : acdfgNames){
      out <<  acdfgName << endl;
    }
    if (printAbove){

      for (const AcdfgBin * b : subsumingBins){
        const std::vector<string> & sub_acdfgs = b -> getAcdfgNames();
        for (const string a_name: sub_acdfgs){
          out << a_name << endl;
        }
      }
    }

  }

  void AcdfgBin::dumpToDot(string fileName) const{
    std::ofstream dot_out (fileName.c_str());
    acdfgRepr -> dumpToDot(dot_out);
    dot_out.close();
  }

  void AcdfgBin::dumpToProtobuf(string fileName) const{
    std::fstream output(fileName, std::ios::out | std::ios::trunc | std::ios::binary);

    acdfgRepr -> dumpToAcdfgProto(output);
    output.close();
  }

  void AcdfgBin::addSubsumingBinsToSet(set<AcdfgBin*> & what) {
    for (AcdfgBin* b: subsumingBins){
      assert(b != this);
      what.insert(b);
    }
  }

  /**
   * Get the bins that are immediately reachable (recall that
   * subsumingBin contains the transitive closure...)
   *
   * Store the result in the immediateSubsumingBins set.
   */
  void AcdfgBin::computeImmediatelySubsumingBins(){
    // set of bins reached transitively through another bin
    set<AcdfgBin*> transitivelySubsuming;
    for (AcdfgBin * b : subsumingBins){
      b -> addSubsumingBinsToSet(transitivelySubsuming);
    }

    immediateSubsumingBins.clear();
    std::set_difference(subsumingBins.begin(), subsumingBins.end(), \
                        transitivelySubsuming.begin(), transitivelySubsuming.end(), \
                        std::inserter(immediateSubsumingBins, immediateSubsumingBins.begin()));
    isImmediateSubsumingUpdate = true;
  }

  /**
   * Returns true is the bin popularity is over the treshold,
   * and there are no bin subsuming this one such that
   * their popularity is over the treshold
   */
  bool AcdfgBin::isAtFrontierOfPopularity(int freq_cutoff) {
    int f = this -> getPopularity();
    if (f < freq_cutoff) return false;
    for (const AcdfgBin * b: getImmediateSubsumingBins()){
      if (b -> getPopularity() >= freq_cutoff)
        return false;
    }
    return true;
  }

  void AcdfgBin::setPopular(bool allowSubsuming) {
    assert(allowSubsuming || (! this -> subsuming));
    this -> popular = true;
    for (AcdfgBin * b: subsumingBins) {
      if (!allowSubsuming)
        b -> popular = false;
      b -> subsuming = true;
    }
  }

  void AcdfgBin::setPopular() {
    setPopular(false);
  }

  bool AcdfgBin::hasPopularAncestor() const{
    for (const AcdfgBin* b: subsumingBins) {
      if (b -> isPopular())
        return true;
    }
    return false;
  }

  void AcdfgBin::resetClassification() {
    subsuming = false;
    anomalous = false;
    isolated = false;
    popular = false;
    cumulativeFrequency = 0;
  }

  void AcdfgBin::getReachable(const set<AcdfgBin*> &initial,
                              set<AcdfgBin*> &reachable,
                              const bool invert) {
    vector<AcdfgBin*> to_visit;
    set<AcdfgBin*> visited;

    for (auto b : initial)
      to_visit.push_back(b);

    while (to_visit.size() > 0) {
      AcdfgBin* b = to_visit.back();
      to_visit.pop_back();

      if (reachable.find(b) != reachable.end())
        continue;

      reachable.insert(b);

      if (! invert) {
        for (AcdfgBin* next : b->getSubsumingBins()) {
          to_visit.push_back(next);
        }
      } else {
        for (AcdfgBin* prev : b->getIncomingEdges()) {
          to_visit.push_back(prev);
        }
      }
    }
  }


  Lattice::Lattice(const vector<string> & methodNames) {
    for (string s : methodNames) {
      this->methodNames.push_back(s);
    }
  }


  void Lattice::addBin(AcdfgBin* bin) {
    allBins.push_back(bin);
  }

  void Lattice::addPopular(AcdfgBin* popular) {
    popularBins.push_back(popular);
  }

  void Lattice::addAnomalous(AcdfgBin* anomalous) {
    anomalousBins.push_back(anomalous);
  }

  void Lattice::addIsolated(AcdfgBin* isolated) {
    isolatedBins.push_back(isolated);
  }

  void Lattice::sortByFrequency() {
    std::sort(allBins.begin(), allBins.end(),
              [](const AcdfgBin  * bin1, const AcdfgBin * bin2){
                return bin1 -> getFrequency() > bin2 -> getFrequency();
              });

  }

  void sortHelp(vector<AcdfgBin*> &binVector) {
    std::sort(binVector.begin(), binVector.end(),
              [](const AcdfgBin  * bin1, const AcdfgBin * bin2){
                return bin1 -> getFrequency() > bin2 -> getFrequency();
              });
  }

  void Lattice::sortAllByFrequency() {
    sortHelp(allBins);
    sortHelp(popularBins);
    sortHelp(anomalousBins);
    sortHelp(isolatedBins);
  }


  void Lattice::resetClassification() {
    popularBins.clear();
    anomalousBins.clear();
    isolatedBins.clear();

    for (AcdfgBin * acdfgBin : allBins){
      acdfgBin->resetClassification();
    }
  }

  void Lattice::dumpAllBins(std::chrono::seconds time_taken,
                            const string & output_prefix,
                            const string & infoFileName,
                            const string & latticeFileName) {

    fixrgraphiso::writeLattice((const Lattice&) *this, latticeFileName);

    ofstream out_file(infoFileName.c_str());
    int count = 1;
    string iso_file_name;
    string iso_bin_file_name;
    out_file << "Popular Bins: " << endl;

    for (AcdfgBin * a: popularBins){
      assert (a -> isPopular());
      iso_file_name = string("pop_")+std::to_string(count)+".dot";
      iso_bin_file_name = string("pop_")+std::to_string(count)+".acdfg.bin";
      out_file << "Popular Bin # " << count << endl;
      out_file << "Dot: " << iso_file_name << endl;
      out_file << "Bin: " << iso_bin_file_name << endl;
      out_file << "Frequency: " << a -> getFrequency() << ", "
               << a-> getPopularity() << endl;
      a -> dumpToDot(output_prefix + "/" + iso_file_name);
      a -> printInfo(out_file);
      count ++;
    }

    count = 1;

    for (AcdfgBin * a: anomalousBins) {
      assert(a -> isAnomalous());
      iso_file_name = string("anom_")+std::to_string(count)+".dot";
      iso_bin_file_name = string("anom_")+std::to_string(count)+".acdfg.bin";
      out_file << "Anomalous Bin # " << count << endl;
      out_file << "Dot: " << iso_file_name << endl;
      out_file << "Bin: " << iso_bin_file_name << endl;
      out_file << "Frequency: " << a -> getFrequency()<< endl;
      a -> dumpToDot(output_prefix + "/" + iso_file_name);
      a -> printInfo(out_file, false);
      count ++;
    }

    count = 1;
    for (AcdfgBin * a: isolatedBins) {
      iso_file_name = string("isol_")+std::to_string(count)+".dot";
      iso_bin_file_name = string("isol_")+std::to_string(count)+".acdfg.bin";
      out_file << "Isolated Bin # " << count << endl;
      out_file << "Dot: " << iso_file_name << endl;
      out_file << "Bin: " << iso_bin_file_name << endl;
      out_file << "Frequency: " << a -> getFrequency() ;
      a -> dumpToDot(output_prefix + "/" + iso_file_name);
      a -> printInfo(out_file, false);
      count ++;
    }

    out_file << "Total Time (s): " << time_taken.count()<< endl;
    printStats(out_file);

    out_file.close();
  }

  void Lattice::dumpToDot(const string & dotFile,
                          const bool onlyClassified) {
    ofstream out_file(dotFile.c_str());

    out_file << "digraph { " << endl;

    // Print nodes
    int id = -1;
    std::map<AcdfgBin*,int> toNodeId;
    for (AcdfgBin* bin : allBins) {
      if (onlyClassified && !bin->isClassified()) {
        continue;
      }
      id++;
      toNodeId[bin] = id;

      string color;
      if (bin->isPopular()) {
        color = "green";
      } else if (bin->isAnomalous()) {
        color = "red";
      } else if (bin->isIsolated()) {
        color = "yellow";
      } else {
        color = "gray";
      }

      out_file << "node [shape = circle, style=filled, ";
      out_file << "color=" << color << ",";
      out_file << "label = \"" << id;
      out_file << "\", ] node_" << id << ";" << endl;
    }

    for (AcdfgBin* bin : allBins) {
      if (onlyClassified && !bin->isClassified())
        continue;

      for (AcdfgBin* subsumed : bin->getSubsumingBins()) {
        if (onlyClassified && !subsumed->isClassified()) {
          continue;
        }

        out_file << "node_" << toNodeId[bin] << " -> node_" << toNodeId[subsumed] << ";" << endl;
      }
    }

    // Print edges
    out_file << "}" << endl;
  }

  void Lattice::getAcdfgBin2id(map<AcdfgBin*, int> &acdfgBin2idMap) const {
    int id = -1;
    for (auto it = beginAllBins();
         it != endAllBins(); ++it) {
      AcdfgBin * a = *it;
      id += 1;
      acdfgBin2idMap[a] = id;
    };
  }

  Lattice::~Lattice() {
    for (auto bin : allBins)
      delete bin;
  }

  void Lattice::deleteTr(map<AcdfgBin*, set<AcdfgBin*>*> & tr) {
    for (auto x : tr) {
      set<AcdfgBin*>* set = x.second;
      delete set;
    }
  }

  /**
   * \brief Builds the immediate transition relation for the lattice
   *
   */
  void Lattice::buildTr(map<AcdfgBin*, set<AcdfgBin*>*> & tr) const {
    for (auto bin : allBins) {
      set<AcdfgBin*>* binReach = new set<AcdfgBin*>();

      for (AcdfgBin* it_succ : bin->getImmediateSubsumingBins()) {
        binReach->insert(it_succ);
      }
      tr[bin] = binReach;
    }
  }

  void Lattice::reverseTr(const map<AcdfgBin*, set<AcdfgBin*>*> & tr,
                          map<AcdfgBin*, set<AcdfgBin*>*> & inverse) const {
    for (auto bin : allBins)
      inverse[bin] = new set<AcdfgBin*>();

    for (auto x : tr) {
      for (auto to : *(x.second)) {
        // from -> to in tr, add to -> from in inverse
        inverse[to]->insert(x.first);
      }
    }
  }

  /**
   * Compute the topological order of the lattice using the reverse
   * transition relation (i.e., starting from the nodes that are not
   * subsumed by any other nodes and going backward).
   */
  void Lattice::computeTopologicalOrder(vector<AcdfgBin*> &order) const {
    map<AcdfgBin*, set<AcdfgBin*>*> tr;
    map<AcdfgBin*, set<AcdfgBin*>*> inverseTr;
    vector<AcdfgBin*> to_process;

    // Build the non-transitive transition relation
    // Note that the algorithm visits the DAG backward
    buildTr(tr);
    reverseTr(tr, inverseTr);

    // Find all the top elements of the lattice
    for (auto bin : allBins) {
      if (tr[bin]->empty())
        to_process.push_back(bin);
    }

    while (! to_process.empty()) {
      AcdfgBin* bin = to_process.back();
      to_process.pop_back();

      // Add bin to the topological order
      order.push_back(bin);

      set<AcdfgBin*>* succBins = inverseTr[bin];
      for (auto succ : (*succBins)) {
        set<AcdfgBin*>* predOfSucc = tr[succ];
        if (predOfSucc->empty()) // already visited
          continue;

        /* remove (succ, bin) from tr */
        predOfSucc->erase(bin);

        /* if succ has no other incoming edges then add succ to to_process.
           At this point it's "safe" to process succ.
         */
        if (predOfSucc->empty()) {
          to_process.push_back(succ);
        }
      } // End of loop on successors
    } // End of loop on nodes

    assert(order.size() == allBins.size());

    // // Check topological order -- DEBUG
    // Lattice::deleteTr(tr);
    // buildTr(tr);
    // {
    //   set<AcdfgBin*> visited;

    //   for (auto bin : order) {
    //     visited.insert(bin);

    //     cout << "map is " << tr[bin] << 
    //       " " << tr[bin]->size() << endl;
    //     for(AcdfgBin* bin2 : bin->getImmediateSubsumingBins()) {

    //       cout << "TR FIND " << bin << " " << bin2 << endl;

    //       assert(tr[bin]->find(bin2) != tr[bin]->end());
    //       assert(inverseTr[bin2]->find(bin) != inverseTr[bin2]->end());
    //       assert(visited.find(bin2) != visited.end());
    //     }
    //   }
    // }

    Lattice::deleteTr(tr);
    Lattice::deleteTr(inverseTr);
  }

  /**
   * Compute the transitive closure of the lattice
   *
   * Could be more efficient, exploiting the lattice is a DAG
   * using topological sort
   */
  void Lattice::makeClosure() {
    for (AcdfgBin* a : allBins) {
      set<AcdfgBin*> reachable;
      a->getReachable(a->getSubsumingBins(),
                      reachable, false);

      for (auto b : reachable) {
        a->addSubsumingBin(b);
      }
    }
  }

  int Lattice::countCommonMethods(const Lattice &other) const {
    vector<string> v(methodNames.size() + other.methodNames.size());
    vector<string>::iterator it;

    it = set_intersection(methodNames.begin(), 
                          methodNames.end(), 
                          other.methodNames.begin(), 
                          other.methodNames.end(), 
                          v.begin());
    return v.size();

  }

  /**
   * Check the validity of the lattice
   *
   * A lattice is valid if:
   *  - There are no two bins that are equivalent
   *  - if there is an edge from b1 to b2:
   *    - then b2 subsumes b1
   *    - b2 has an incoming edge from b1
   *
   * Not complete
   */
  bool Lattice::isValid() const {
    bool valid = true;

    // There are no duplicate bins
    for (auto b1 : allBins) {
      for (auto b2 : allBins) {
        if (b1 != b2) {

          IsoRepr* isoRepr = new IsoRepr(b2->getRepresentative(),
                                         b1->getRepresentative());
          if (b1->isACDFGEquivalent(b2->getRepresentative(), isoRepr)) {
            cout << "Found duplicate (" << b1 << "," << b2 <<
              ")" << endl;

            delete isoRepr;
            return false;
          }
        }
      }
    }

    for (auto b1 : allBins) {
      for (auto b2 : b1->getSubsumingBins()) {
        if (b1 == b2) {
          cout << "Self loops!" << endl;
          return false; // no self loops
        }

        if (! b1->isACDFGBinSubsuming(b2)) {
          // b1 not subsumed by b2
          cout << "No propser subsumption!" << endl;

          return false;
        }

        if (b2->getIncomingEdges().find(b1) == b2->getIncomingEdges().end()) {
          // no incoming edge
          cout << "Missing back edge!" << endl;
          return false;
        }
      }
    }

    for (auto b1 : allBins) {
      for (auto b2 : allBins) {
        if (b1->getSubsumingBins().find(b2) == b1->getSubsumingBins().end() &&
            b1->getIncomingEdges().find(b2) == b1->getIncomingEdges().end()) {

          if (b1 == b2)
            continue;

          IsoRepr* isoRepr = new IsoRepr(b2->getRepresentative(),
                                         b1->getRepresentative());

          if (b1->compareACDFG(b2->getRepresentative(), isoRepr,
                               true, true) !=
              AcdfgBin::NONE) {
            cout << b1->getRepresentative() << " unrelated to " <<
              b2->getRepresentative() << endl;

            cout << "Missing relationship" << endl;

            cout << "b2 subsumes b1 " << b1->isACDFGBinSubsuming(b2) << endl;
            cout << "b1 subsumes b2 " << b2->isACDFGBinSubsuming(b1) << endl;

            return false;
          }
        }
      }
    }

    return valid;
  }
}
