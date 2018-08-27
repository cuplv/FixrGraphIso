#include <ostream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <map>
#include "acdfgBin.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/collectStats.h"

namespace fixrgraphiso {

  using std::ofstream;

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

  bool AcdfgBin::isACDFGEquivalent(Acdfg * b, IsoRepr* iso){
    IsoSubsumption dir_a (acdfgRepr, b);
    IsoSubsumption dir_b (b, acdfgRepr);
    if (! dir_a.checkNodeCounts() || ! dir_b.checkNodeCounts()){
      if (debug){
        cout << "Subsumption ruled out directly " << endl;
      }
      return false;
    }

    if (! dir_a.check()){
      if (debug){
        cout << "Subsumption bin -> b ruled out " << endl;
      }
      return false;
    }

    if (!dir_b.check()){
      if (debug){
        cout << "Subsumption b -> bin ruled out " << endl;
      }
      return false;
    }
    if (debug){
      cout << "Equivalent ACDFGs ! " <<endl;
    }

    return true;
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

  void AcdfgBin::computeImmediatelySubsumingBins(){
    set<AcdfgBin*> transitivelySubsuming;
    for (AcdfgBin * b : subsumingBins){
      b -> addSubsumingBinsToSet(transitivelySubsuming);
    }
    std::set_difference(subsumingBins.begin(), subsumingBins.end(), \
                        transitivelySubsuming.begin(), transitivelySubsuming.end(), \
                        std::inserter(immediateSubsumingBins, immediateSubsumingBins.begin()));

  }

  bool AcdfgBin::isAtFrontierOfPopularity(int freq_cutoff) const {
    int f = this -> getPopularity();
    if (f < freq_cutoff) return false;
    for (const AcdfgBin * b: immediateSubsumingBins){
      if (b -> getPopularity() >= freq_cutoff)
        return false;
    }
    return true;
  }

  void AcdfgBin::setPopular() {
    assert(! this -> subsuming);
    this -> popular = true;
    for (AcdfgBin * b: subsumingBins) {
      b -> popular = false;
      b -> subsuming = true;
    }

  }

  bool AcdfgBin::hasPopularAncestor() const{
    for (const AcdfgBin* b: subsumingBins) {
      if (b -> isPopular())
        return true;
    }
    return false;
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

  void Lattice::dumpAllBins(std::chrono::seconds time_taken,
                            const string & output_prefix,
                            const string & infoFileName) {
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
      a -> dumpToProtobuf(output_prefix + "/" + iso_bin_file_name);
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
      a -> dumpToProtobuf(output_prefix + "/" + iso_bin_file_name);
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
      a -> dumpToProtobuf(output_prefix + "/" + iso_bin_file_name);
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
}
