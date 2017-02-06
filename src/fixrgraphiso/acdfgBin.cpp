#include <ostream>
#include <fstream>
#include "acdfgBin.h"
#include "fixrgraphiso/isomorphismClass.h"

namespace fixrgraphiso {

  bool AcdfgBin::isACDFGBinSubsuming( AcdfgBin * b){
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

  void AcdfgBin::printInfo(std::ostream & out, bool printPopularity) const {
    if (printPopularity)
      out << "\t Frequency = " << getPopularity() <<  endl;
    for (const Acdfg * a : acdfgs){
      out << a -> getName() << endl;
    }
    
    for (const AcdfgBin * b : subsumingBins){
      b-> printInfo(out, false);
    }
    
  }

  void AcdfgBin::dumpToDot(string fileName) const{
    std::ofstream dot_out (fileName.c_str());
    assert(acdfgs.size() >= 1);
    Acdfg * repr = *(acdfgs.begin());
    repr -> dumpToDot(dot_out);
    dot_out.close();
  }
}
