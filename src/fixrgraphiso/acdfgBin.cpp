#include <ostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include "acdfgBin.h"
#include "fixrgraphiso/isomorphismClass.h"

namespace fixrgraphiso {

  bool AcdfgBin::isACDFGBinSubsuming( AcdfgBin * b){
    // First check the graph so far
    for (AcdfgBin * c : incomingEdges){
      if (c -> hasSubsumingBin(b)){
	return true;
      }
    }
    
    Acdfg * repr_a = getRepresentative();
    Acdfg * repr_b = b -> getRepresentative();

    IsoSubsumption d(repr_b, repr_a);
    return d.check();
  }

  

  bool AcdfgBin::isACDFGEquivalent(Acdfg * b){
    Acdfg * repr = *(acdfgs.begin());
    IsoSubsumption dir_a (repr, b);
    IsoSubsumption dir_b (b, repr);
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
    


    for (const Acdfg * a : acdfgs){
      out <<  a -> getName() << endl;
    }
    if (printAbove){
    
      for (const AcdfgBin * b : subsumingBins){
	const std::vector<Acdfg*> & sub_acdfgs = b -> getACDFGs();
	for (const Acdfg * a: sub_acdfgs){
	  out << a -> getName() << endl;
	}
      }
    }
    
  }

  void AcdfgBin::dumpToDot(string fileName) const{
    std::ofstream dot_out (fileName.c_str());
    assert(acdfgs.size() >= 1);
    Acdfg * repr = *(acdfgs.begin());
    repr -> dumpToDot(dot_out);
    dot_out.close();
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
    std::set_difference(subsumingBins.begin(), subsumingBins.end(),	\
			transitivelySubsuming.begin(), transitivelySubsuming.end(), \
			std::inserter(immediateSubsumingBins, immediateSubsumingBins.begin()));
    
  }

  bool AcdfgBin::isAtFrontierOfPopularity(int freq_cutoff) const {
    int f = this -> getPopularity();
    if (f < freq_cutoff) return false;
    const Acdfg * a = this -> getRepresentative();
    if (a -> method_node_count() < 3) return false;
    bool hasUnpopularParent = false;
    for (const AcdfgBin * b: immediateSubsumingBins){
      if (b -> getPopularity() >= freq_cutoff)
	return false;
    }
    return true;
  }

  void AcdfgBin::setPopular() {
    assert(! this -> subsuming);
    this -> popular = true;
    for (AcdfgBin * b: subsumingBins){
      b -> popular = false;
      b -> subsuming = true;
    }
    
  }

  bool AcdfgBin::hasPopularAncestor() const{
    for (const AcdfgBin* b: subsumingBins){
      if (b -> isPopular())
	return true;
    }
    return false;
  }
  
}
