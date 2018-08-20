#include <fstream>
#include <iostream>
#include <cassert>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <unistd.h>
#include "fixrgraphiso/proto_iso.pb.h"
#include "fixrgraphiso/proto_acdfg.pb.h"
#include "fixrgraphiso/collectStats.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/acdfgBin.h"
namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::string;
using std::ofstream;
using std::ifstream;

namespace fixrgraphiso {
  bool debug = false;
  int freq_cutoff = 20;
  string info_file_name = "cluster-info.txt";
  string output_prefix = ".";
  int minTargetSize = 2;
  int maxTargetSize = 100;
  int maxEdgeSize = 400;
  int anomalyCutOff = 5;
  bool runTestOfSubsumption = false;
  stats_struct all_stats;

  void loadNamesFromFile (string filename, vector<string> & listOfNames){
    ifstream ifile(filename.c_str());
    std::string line;
    while (std::getline(ifile, line)){
      line.erase( std::remove_if( line.begin(),
                                  line.end(),
                                  [](char x){ return std::isspace(x);}),
                  line.end());
      listOfNames.push_back(line);
      std::cout << "\t Adding :" << line << std::endl;
    }
  }

  Acdfg * loadACDFGFromFilename(string filename){
    AcdfgSerializer s;
    iso_protobuf::Acdfg * proto_acdfg = s.read_protobuf_acdfg(filename.c_str());
    Acdfg * acdfg = s.create_acdfg(proto_acdfg);
    acdfg -> setName(filename);
    return acdfg;
  }

  void loadACDFGFromFilename(string filename, std::vector<Acdfg*> & allACDFGs){
    Acdfg * acdfg = loadACDFGFromFilename(filename);
    //  if (acdfg -> node_count() >= 100 || acdfg -> edge_count() >= 1000){
    //    std::cerr << "ACDFG " << filename << " too large # nodes: " << acdfg -> node_count() << " # edges : " << acdfg -> edge_count() << std::endl;
    //    delete(acdfg);
    //  } else
    {
      allACDFGs.push_back(acdfg);
    }
  }

  void processCommandLine(int argc, char * argv[] , vector<string> & filenames,
                          vector<string> & methodNames){
    char c;
    int index;
    while ((c = getopt(argc, argv, "dm:f:t:o:i:zp:"))!= -1){
      switch (c){
      case 'm': {
        string methodNamesFile = optarg;
        cout << "Loading methods" << endl;
        loadNamesFromFile(methodNamesFile, methodNames);
      }
        break;
      case 'z':
        runTestOfSubsumption = true;
        break;
      case 'd':
        fixrgraphiso::debug = true;
        break;
      case 'f':
        freq_cutoff = strtol(optarg, NULL, 10);
        break;
      case 'i':{
        string inputFileName = optarg;
        cout << "Loading ACDFGs " << endl;
        loadNamesFromFile(inputFileName, filenames);
      }
        break;
      case 'o':
        info_file_name = string(optarg);
        cout << "Info will be dumped to : " << info_file_name << std::endl;
        break;
      case 'p':
        output_prefix = string(optarg);
        cout << "Output patterns will be dumped in: " << output_prefix << std::endl;
        break;
      default:
        break;
      }
    }

    for (index = optind; index < argc; ++index){
      string fname(argv[index]);
      filenames.push_back(fname);
      std::cout << index <<  "--> " << fname << std::endl;
    }

    if (filenames.size() <= 0){
      cout << "Usage:" << argv[0] <<
        " -f [frequency cutoff] -o [output info filename] " <<
        "-m [file with method names] -i [file with acdfg names] " <<
        "[list of iso.bin files]" << endl;
      return;
    }
  }

  void dumpAllBins(std::vector<AcdfgBin*> & popular,
                   std::vector<AcdfgBin*> & anomalous,
                   std::vector<AcdfgBin*> & isolated,
                   std::chrono::seconds time_taken,
                   const std::string & output_prefix,
                   const std::string & infoFileName){
    std::ofstream out_file(infoFileName.c_str());
    int count = 1;
    string iso_file_name;
    string iso_bin_file_name;
    out_file << "Popular Bins: " << endl;
    for (AcdfgBin * a: popular){
      assert (a -> isPopular());
      iso_file_name = string("pop_")+std::to_string(count)+".dot";
      iso_bin_file_name = string("pop_")+std::to_string(count)+".acdfg.bin";
      out_file << "Popular Bin # " << count << endl;
      out_file << "Dot: " << iso_file_name << endl;
      out_file << "Bin: " << iso_bin_file_name << endl;
      out_file << "Frequency: " << a -> getFrequency() << ", "
               << a-> getPopularity() << std::endl;
      a -> dumpToDot(output_prefix + "/" + iso_file_name);
      a -> dumpToProtobuf(output_prefix + "/" + iso_bin_file_name);
      a -> printInfo(out_file);
      count ++;
    }

    count = 1;

    for (AcdfgBin * a: anomalous){
      assert(a -> isAnomalous());
      iso_file_name = string("anom_")+std::to_string(count)+".dot";
      iso_bin_file_name = string("anom_")+std::to_string(count)+".acdfg.bin";
      out_file << "Anomalous Bin # " << count << endl;
      out_file << "Dot: " << iso_file_name << endl;
      out_file << "Bin: " << iso_bin_file_name << endl;
      out_file << "Frequency: " << a -> getFrequency()<< std::endl;
      a -> dumpToDot(output_prefix + "/" + iso_file_name);
      a -> dumpToProtobuf(output_prefix + "/" + iso_bin_file_name);
      a -> printInfo(out_file, false);
      count ++;
    }

    count = 1;
    for (AcdfgBin * a: isolated){
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

  void calculateLatticeGraph(std::vector<AcdfgBin*> & allBins){
    for (auto it = allBins.begin(); it != allBins.end(); ++it){
      AcdfgBin * a = *it;
      for (auto jt = allBins.begin(); jt != allBins.end(); ++jt){
        AcdfgBin * b = *jt;
        if (a == b) continue;
        if (a -> isACDFGBinSubsuming(b)){
          a -> addSubsumingBin(b);
        }
      }
    }
  }

  void classifyBins(std::vector<AcdfgBin*> & allBins,
                    std::vector<AcdfgBin*> & popular,
                    std::vector<AcdfgBin*> & anomalous,
                    std::vector<AcdfgBin*> & isolated){

    calculateLatticeGraph(allBins);
    // 1. Calculate the transitive reduction for each node and use
    //    it to judge popularity
    for (AcdfgBin * a: allBins){
      a -> computeImmediatelySubsumingBins();
      if (a -> isSubsuming()) continue;
      if (a -> isAtFrontierOfPopularity(freq_cutoff)){
        a -> setPopular();
        if (debug){
          std::cout << "Found popular bin with frequency : " <<
            a -> getPopularity() << std::endl;
        }
      }
    }

    // 2. Now calculate the anomalous patterns
    for (AcdfgBin * a: allBins){
      if (a -> isSubsuming()) continue;
      if (a -> isPopular()) {
        popular.push_back(a);
      } else  if (a -> getFrequency() <= anomalyCutOff &&
                  a -> hasPopularAncestor()){
        a -> setAnomalous();
        anomalous.push_back(a);
      } else if (a -> getFrequency() <= anomalyCutOff){
        isolated.push_back(a);
      }
    }

    return;
  }

  void computePatternsThroughSlicing(vector<string> & filenames,
                                     vector<string> & methodnames){
    // 1. Slice all the ACDFGs using the methods in the
    // method names as the target
    vector<Acdfg*> allSlicedACDFGs;
    auto start = std::chrono::steady_clock::now();
    for (string f: filenames){
      Acdfg * orig_acdfg = loadACDFGFromFilename(f);
      vector<MethodNode*> targets;
      orig_acdfg->getMethodsFromName(methodnames, targets);

      if (targets.size() < minTargetSize){
        // File has too few methods, something is not correct.
        std::cerr << "Warning: filename = " << f \
                  << " Could not find " << minTargetSize \
                  << " methods from the list of method names" \
                  << " -- Ignoring this file." << std::endl;
      } else if (targets.size() >= maxTargetSize){
        std::cerr << "Warning: filename = " << f \
                  << "too many matching methods found -- " \
                  << targets.size() \
                  << " -- Ignoring this file." << std::endl;
      } else {
        Acdfg * new_acdfg = orig_acdfg->sliceACDFG(targets);
        if (new_acdfg -> edge_count() >= maxEdgeSize){
          std::cerr << "Warning: Filename = " << f \
                    << "too many edges found -- " << new_acdfg->edge_count() \
                    << "-- Ignorning this file." << std::endl;
          delete(new_acdfg);
        } else {
          new_acdfg -> setName(f);
          addGraphStats(new_acdfg->node_count(), new_acdfg->edge_count());
          allSlicedACDFGs.push_back(new_acdfg);
        }
      }
      delete(orig_acdfg);
    }

    // 2. Compute a binning of all the sliced ACDFGs using the exact isomorphism
    std::vector<AcdfgBin*> allBins;
    for (Acdfg* a: allSlicedACDFGs){
      bool acdfgSubsumed = false;
      for (AcdfgBin * bin: allBins){
        if (bin -> isACDFGEquivalent(a)){
          bin -> insertEquivalentACDFG(a);
          acdfgSubsumed = true;
          break;
        }
      }
      if (!acdfgSubsumed){
        AcdfgBin * newbin = new AcdfgBin(a);
        allBins.push_back(newbin);
      }
    }

    // 3. Print all the popular patterns.
    std::sort(allBins.begin(), allBins.end(),
              [](const AcdfgBin  * iso1, const AcdfgBin * iso2){
                return iso1 -> getFrequency() > iso2 -> getFrequency();
              });

    std::vector<AcdfgBin*> popular, anomalous, isolated;
    classifyBins(allBins,popular,anomalous,isolated);
    auto end = std::chrono::steady_clock::now();
    std::chrono::seconds time_taken =
      std::chrono::duration_cast<std::chrono::seconds>(end -start);

    dumpAllBins(popular, anomalous, isolated,
                time_taken, output_prefix, info_file_name);
  }

  void testPairwiseSubsumption(std::vector<string> & filenames,
                               std::vector<string> & methodnames){
    vector<Acdfg * > allACDFGs;

    for (string f: filenames){
      Acdfg * orig_acdfg = loadACDFGFromFilename(f);
      vector<MethodNode*> targets;
      orig_acdfg -> getMethodsFromName(methodnames, targets);
      Acdfg * new_acdfg = orig_acdfg -> sliceACDFG(targets);
      new_acdfg -> setName(f);
      allACDFGs.push_back(new_acdfg);
    }

    for (Acdfg * a : allACDFGs){
      for (Acdfg * b : allACDFGs){
        if (a == b) continue;
        IsoSubsumption isoSub(a, b);
        if (isoSub.check()){
          std:: cout << a -> getName() << " subsumes "
                     << b -> getName() << endl;
        }
      }
    }
    return ;
  }

  void frequentSubgraphsMain(int argc, char * argv [] ){
    vector<string> filenames;
    vector<string> methodnames;
    processCommandLine(argc, argv, filenames, methodnames);

    if (runTestOfSubsumption){
      testPairwiseSubsumption(filenames, methodnames);
    } else {
      computePatternsThroughSlicing(filenames, methodnames);
    }
  }
}

int main(int argc, char * argv[]){
  fixrgraphiso::frequentSubgraphsMain(argc, argv);
  return 0;
}
