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
#include "fixrgraphiso/frequentSubgraphs.h"
#include "fixrgraphiso/serializationLattice.h"

using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::string;
using std::ofstream;
using std::ifstream;

namespace fixrgraphiso {
  namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;

  bool debug = false;
  stats_struct all_stats;

  void loadNamesFromFile (string filename, vector<string> & listOfNames){
    ifstream ifile(filename.c_str());
    string line;
    while (std::getline(ifile, line)){
      line.erase( std::remove_if( line.begin(),
                                  line.end(),
                                  [](char x){ return std::isspace(x);}),
                  line.end());
      listOfNames.push_back(line);
      std::cout << "\t Adding: " << line << endl;
    }
  }

  Acdfg * loadACDFGFromFilename(string filename){
    AcdfgSerializer s;
    iso_protobuf::Acdfg * proto_acdfg = s.read_protobuf_acdfg(filename.c_str());
    Acdfg * acdfg = s.create_acdfg((const iso_protobuf::Acdfg &) *proto_acdfg);
    acdfg -> setName(filename);
    return acdfg;
  }

  void loadACDFGFromFilename(string filename, vector<Acdfg*> & allACDFGs){
    Acdfg * acdfg = loadACDFGFromFilename(filename);
    //  if (acdfg -> node_count() >= 100 || acdfg -> edge_count() >= 1000){
    //    std::cerr << "ACDFG " << filename << " too large # nodes: " << acdfg -> node_count() << " # edges : " << acdfg -> edge_count() << endl;
    //    delete(acdfg);
    //  } else
    {
      allACDFGs.push_back(acdfg);
    }
  }

  int FrequentSubgraphMiner::processCommandLine(int argc, char * argv[],
                                                vector<string> & filenames,
                                                vector<string> & methodNames) {
    char c;
    int index;
    while ((c = getopt(argc, argv, "dm:f:t:o:i:zp:l:c"))!= -1) {
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
      case 'c':
        rerunClassification = true;
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
        cout << "Info will be dumped to : " << info_file_name << endl;
        break;
      case 'l':
        lattice_filename = string(optarg);
        cout << "Lattice file name will be written to : " << lattice_filename << endl;
        break;
      case 'p':
        output_prefix = string(optarg);
        cout << "Output patterns will be dumped in: " << output_prefix << endl;
        break;
      default:
        break;
      }
    }

    for (index = optind; index < argc; ++index){
      string fname(argv[index]);
      filenames.push_back(fname);
      std::cout << index <<  "--> " << fname << endl;
    }

    if (filenames.size() <= 0 && (! rerunClassification) ){
      cout << "Usage --- (default) mine frequent patterns: " << argv[0] <<
        " -f [frequency cutoff] -o [output info filename] " <<
        "-l [lattice file protobuf] " <<
        "-m [file with method names] -i [file with acdfg names] " <<
        "-p [output path for the found patterns] " <<
        "[list of acdfg.bin files to mine]" << endl <<
        //
        "Usage --- classify bins, re-run the bin classification: " << argv[0] <<
        "-c " <<
        "-f [frequency cutoff] -o [output info filename] " <<
        "-l [lattice file protobuf] " <<
        "-p [output path for the found patterns] " << endl <<
        //
        "Usage --- subsumption test:" << argv[0] <<
        " -z [frequency cutoff]" <<
        " -z [frequency cutoff]" <<
        "-m [file with method names] -i [file with acdfg names] " <<
        "[list of acdfg.bin files to mine]" << endl;
      return 1;
    } else {
      return 0;
    }
  }

  void FrequentSubgraphMiner::sliceAcdfgs(const vector<string> & filenames,
                                          const vector<string> & methodnames,
                                          vector<Acdfg*> & allSlicedACDFGs) {
    set<int> ignoreMethodIds;
    for (string f: filenames){
      Acdfg * orig_acdfg = loadACDFGFromFilename(f);
      vector<MethodNode*> targets;
      orig_acdfg->getMethodsFromName(methodnames, targets);

      if (targets.size() < minTargetSize){
        // File has too few methods, something is not correct.
        std::cerr << "Warning: filename = " << f \
                  << " Could not find " << minTargetSize \
                  << " methods from the list of method names" \
                  << " -- Ignoring this file." << endl;
      } else if (targets.size() >= maxTargetSize){
        std::cerr << "Warning: filename = " << f \
                  << "too many matching methods found -- " \
                  << targets.size() \
                  << " -- Ignoring this file." << endl;
      } else {
        Acdfg * new_acdfg = orig_acdfg->sliceACDFG(targets, ignoreMethodIds);
        new_acdfg -> setName(f);
        if (new_acdfg -> edge_count() >= maxEdgeSize){
          std::cerr << "Warning: Filename = " << f \
                    << "too many edges found -- " << new_acdfg->edge_count() \
                    << "-- Ignorning this file." << endl;
          delete(new_acdfg);
        } else {
          addGraphStats(new_acdfg->node_count(), new_acdfg->edge_count());
          allSlicedACDFGs.push_back(new_acdfg);
        }
      }
      delete(orig_acdfg);
    }
  }

  void FrequentSubgraphMiner::calculateLatticeGraph(Lattice & lattice) {
    // complete the graph adding all the subsuming relations
    for (auto it = lattice.beginAllBins();
         it != lattice.endAllBins(); ++it) {
      AcdfgBin * a = *it;
      for (auto jt = lattice.beginAllBins();
           jt != lattice.endAllBins(); ++jt){
        AcdfgBin * b = *jt;
        if (a == b) continue;
        if (a -> isACDFGBinSubsuming(b)){
          a -> addSubsumingBin(b);
        }
      }
    }
  }

  void FrequentSubgraphMiner::classifyBins(Lattice &lattice) {

    // 1. Compute the lattice of bins
    calculateLatticeGraph(lattice);

    // 2. Calculate the transitive reduction for each bin in the
    //    lattice and use it to judge popularity
    for (auto it = lattice.beginAllBins();
         it != lattice.endAllBins(); ++it){
      AcdfgBin * a = *it;

      a -> computeImmediatelySubsumingBins();
      if (a -> isSubsuming()) continue;
      if (a -> isAtFrontierOfPopularity(freq_cutoff)){
        a -> setPopular();
        if (debug){
          std::cout << "Found popular bin with frequency : " <<
            a -> getPopularity() << endl;

          std::cout << "Equivalent bins" <<
            (a->getAcdfgNames()).size() << endl;

          for (const string& inbin: a->getAcdfgNames()) {
            cout << inbin << endl;
          }
        }
      }
    }

    // 3. Now calculate the anomalous and isolated patterns
    for (auto it = lattice.beginAllBins();
         it != lattice.endAllBins(); ++it){
      AcdfgBin * a = *it;

      if (a -> isSubsuming()) continue;
      if (a -> isPopular()) {
        lattice.addPopular(a);
      } else if (a -> getFrequency() <= anomalyCutOff &&
                 a -> hasPopularAncestor()){
        a -> setAnomalous();
        lattice.addAnomalous(a);
      } else if (a -> getFrequency() <= anomalyCutOff){
        a->setIsolated();
        lattice.addIsolated(a);
      }
    }

    return;
  }

  void FrequentSubgraphMiner::computePatternsThroughSlicing(Lattice & lattice,
                                                            vector<string> & filenames,
                                                            vector<string> & methodnames) {
    // compute the elapsed real time for the computation (no cpu time)
    auto start = std::chrono::steady_clock::now();

    // 1. Slice all the ACDFGs using the methods in the method names as the target
    vector<Acdfg*> allSlicedACDFGs;
    sliceAcdfgs(filenames, methodnames, allSlicedACDFGs);

    // 2. Compute a binning of all the sliced ACDFGs using the exact isomorphism
    for (Acdfg* a: allSlicedACDFGs){
      bool acdfgSubsumed = false;

      for (auto it = lattice.beginAllBins();
           it != lattice.endAllBins(); ++it){
        AcdfgBin * bin = *it;
        IsoRepr* iso = new IsoRepr(a, bin->getRepresentative());
        if (bin -> isACDFGEquivalent(a, iso)) {
          bin->insertEquivalentACDFG(a, iso);
          acdfgSubsumed = true;
          break;
        } else {
          delete(iso);
        }
      }
      if (! acdfgSubsumed) {
        AcdfgBin * newbin = new AcdfgBin(a);
        lattice.addBin(newbin);
      }
    }

    // 3. Sort the bins by frequency
    lattice.sortByFrequency();

    // 4. Classify the bin trough lattice construction
    classifyBins(lattice);

    auto end = std::chrono::steady_clock::now();
    std::chrono::seconds time_taken =
      std::chrono::duration_cast<std::chrono::seconds>(end -start);

    // 5. Print all the  patterns
    lattice.dumpAllBins(time_taken, output_prefix,
                        info_file_name,
                        lattice_filename);
  }

  /**
   * \brief Rerun the classification of the bin on an existing lattice
   */
  void FrequentSubgraphMiner::reClassifyBins() {

    // 1. Read the lattice
    Lattice *lattice_ptr;
    lattice_ptr = fixrgraphiso::readLattice(lattice_filename);
    if (NULL == lattice_ptr) {
      std::cerr << "Cannot read the lattice in " << lattice_filename << endl;
      return;
    }
    Lattice& lattice = *lattice_ptr;

    // 2. Reset the classification in the lattice
    lattice.resetClassification();

    // 3. Sort the bins by frequency
    lattice.sortByFrequency();

    // 4. Classify the bin trough lattice construction
    classifyBins(lattice);

    // 5. Print all the  patterns
    // Just print 0 in the info file name
    // This means we will not have a mining time in the output file
    auto start = std::chrono::steady_clock::now();
    std::chrono::seconds time_taken =
      std::chrono::duration_cast<std::chrono::seconds>(start -start);
    lattice.dumpAllBins(time_taken, output_prefix,
                        info_file_name,
                        lattice_filename);
    delete lattice_ptr;
  }


  /**
   * \brief For every possible pair (a,b) of acdfg in filenames tests if a subsumes b
   */
  void FrequentSubgraphMiner::testPairwiseSubsumption(vector<string> & filenames,
                                                      vector<string> & methodnames) {
    vector<Acdfg * > allACDFGs;
    set<int> ignoreMethodIds;

    for (string f: filenames){
      Acdfg * orig_acdfg = loadACDFGFromFilename(f);
      vector<MethodNode*> targets;
      orig_acdfg -> getMethodsFromName(methodnames, targets);
      Acdfg * new_acdfg = orig_acdfg -> sliceACDFG(targets,
                                                   ignoreMethodIds);
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

  FrequentSubgraphMiner::FrequentSubgraphMiner() {}

  int FrequentSubgraphMiner::mine(int argc, char * argv [] ){
    vector<string> filenames;
    vector<string> methodnames;
    if (0 == processCommandLine(argc, argv, filenames, methodnames)) {
      if (runTestOfSubsumption){
        testPairwiseSubsumption(filenames, methodnames);
      } else if (rerunClassification) {
        reClassifyBins();
      } else {
        Lattice lattice(methodnames);
        computePatternsThroughSlicing(lattice, filenames, methodnames);
      }
      return 0;
    } else {
      return 1;
    }
  }

  int FrequentSubgraphMiner::mine(Lattice & lattice,
                                  int freqCutoff,
                                  string methodNamesFile,
                                  string outputPrefix,
                                  string acdfgFileName) {
    vector<string> fileNames;
    vector<string> methodNames;

    this->freq_cutoff = freqCutoff;

    cout << "Loading methods from " << methodNamesFile << endl;
    loadNamesFromFile(methodNamesFile, methodNames);

    const vector<string> & latticeMethods = lattice.getMethodNames();
    for (string methodName : methodNames) {
      bool hasName = std::find(latticeMethods.begin(),
                               latticeMethods.end(),
                               methodName) != latticeMethods.end();
      if (! hasName)
        lattice.addMethodName(methodName);
    }

    this->output_prefix = outputPrefix;

    cout << "Loading ACDFGs from " << acdfgFileName << endl;
    loadNamesFromFile(acdfgFileName, fileNames);

    computePatternsThroughSlicing(lattice, fileNames, methodNames);

    return 0;
  }
}
