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
using std::map;
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
    while ((c = getopt(argc, argv, "dm:f:t:o:i:zp:l:c:r:"))!= -1) {
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
      case 'r':
        // Use relative popularity (comulative frequency) to mark the popular pattern
        use_relative_popularity = true;
        relative_pop_threshold = std::stod(optarg, NULL);
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
    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it) {
      AcdfgBin * a = *it;
      for (auto jt = lattice.beginAllBins(); jt != lattice.endAllBins(); ++jt){
        AcdfgBin * b = *jt;
        if (a == b) continue;
        if (a -> isACDFGBinSubsuming(b)){
          // b subsumes a
          a -> addSubsumingBin(b);
        }
      }
    }
  }

  /**
   * Mark the popular patterns using the absolute frequency
   * as threshold and popularity as measure.
   *
   * This is the SANER2018 approach.
   */
  void FrequentSubgraphMiner::findPopularByAbsFrequency(Lattice &lattice) {
    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it){
      AcdfgBin * a = *it;

      a -> setCumulativeFrequency(a->getPopularity());
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
  }

  void FrequentSubgraphMiner::deleteTr(map<AcdfgBin*, set<AcdfgBin*>*> & tr) {
    for (auto const& x : tr) {
      delete x.second;
    }
  }

  /**
   * \brief Builds the immediate transition relation for the lattice
   *
   */
  void FrequentSubgraphMiner::buildTr(Lattice &lattice,
                                      map<AcdfgBin*, set<AcdfgBin*>*> & tr) {
    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it) {
      AcdfgBin* bin = *it;
      set<AcdfgBin*>* binReach = new set<AcdfgBin*>();
      for (auto it_succ : bin->getImmediateSubsumingBins()) {
        binReach->insert(it_succ);
      }
      tr[bin] = binReach;
    }
  }

  void FrequentSubgraphMiner::reverseTr(Lattice &lattice,
                                        map<AcdfgBin*, set<AcdfgBin*>*> & tr,
                                        map<AcdfgBin*, set<AcdfgBin*>*> & inverse) {
    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it)
      inverse[*it] = new set<AcdfgBin*>();

    for (auto x : tr) {
      for (auto to : *(x.second)) {
        // from -> to in tr, add to -> from in inverse
        inverse[to]->insert(x.first);
      }
    }

    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it) {
      AcdfgBin* bin = *it;
      set<AcdfgBin*>* binReach = new set<AcdfgBin*>();

      for (auto toBin : bin->getImmediateSubsumingBins()) {
        binReach->insert(toBin);
      }
      tr[bin] = binReach;
    }
  }

  /**
   * Compute the topological order of the lattice using the reverse
   * transition relation (i.e., starting from the nodes that are not
   * subsumed by any other nodes and going backward).
   */
  void FrequentSubgraphMiner::computeTopologicalOrder(Lattice &lattice,
                                                      vector<AcdfgBin*> &order) {
    map<AcdfgBin*, set<AcdfgBin*>*> tr;
    map<AcdfgBin*, set<AcdfgBin*>*> inverseTr;
    vector<AcdfgBin*> to_process;

    // Build the non-transitive transition relation
    // Note that the algorithm visits the DAG backward
    buildTr(lattice, tr);
    reverseTr(lattice, tr, inverseTr);

    // Find all the top elements of the lattice
    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it) {
      AcdfgBin* bin = *it;
      if (! inverseTr[bin]->empty())
        to_process.push_back(bin);
    }

    while (! to_process.empty()) {
      AcdfgBin* bin = to_process.back();
      to_process.pop_back();

      // Add bin to the topological order
      order.push_back(bin);

      set<AcdfgBin*>* succBins = inverseTr[bin];
      for (auto succ : (*succBins)) {
        /* remove (succ, bin) from tr */
        tr[succ]->erase(bin);

        /* if succ has no other incoming edges then add succ to to_process.
           At this point it's "safe" to process succ.
         */
        if (tr[succ]->empty()) {
          to_process.push_back(succ);
        }
      } // End of loop on successors
    } // End of loop on nodes

    deleteTr(tr);
    deleteTr(inverseTr);
  }

  /**
   * Compute the popularity of the bins in the lattice, marking patterns
   * as popular.
   *
   */
  void FrequentSubgraphMiner::computePopularity(Lattice &lattice,
                                                const vector<AcdfgBin*> &order,
                                                const bool no_subsumed_popular,
                                                const bool is_relative,
                                                const double popularity_threshold) {
    int totalFrequency = 0; // total number of samples in the lattice
    map<AcdfgBin*, int> cumulativeFrequency; // cumulativeFrequency for each bin
    map<AcdfgBin*, set<AcdfgBin*>*> notCountedSubsumedBinsMap;

    assert((! is_relative) || (popularity_threshold >= 0 && popularity_threshold <= 1));

    // Init data structures.
    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it) {
      AcdfgBin* bin = *it;
      notCountedSubsumedBinsMap[bin] = new set<AcdfgBin*>();
      totalFrequency += bin->getFrequency();
      cumulativeFrequency[bin] = 0;
    }

    for (auto bin : order) {
      set<AcdfgBin*>* notCountedSubsumedBins = notCountedSubsumedBinsMap[bin];
      set<AcdfgBin*> toRemove;

      notCountedSubsumedBins->insert(bin);

      for (auto toBin : bin->getImmediateSubsumingBins()) {
        for(auto toCount : (*notCountedSubsumedBinsMap[toBin])) {
          if (! toBin->isPopular()) {
            // "propagates" down in the lattice all the bins that are not popular
            notCountedSubsumedBins->insert(toCount);
          } else {
            // Removes all the bins already accounted for in a previous popular bin
            // Note: we remove them later from the notCountedSubsumedBins
            toRemove.insert(toBin);
          }
        }
      }

      for (auto toCount : toRemove)
        notCountedSubsumedBins->erase(toCount);

      {
        // Compute the popularity using the bins in notCountedSubsumedBins
        int cumulativeFrequencyBin = 0;
        for (auto toCount : *notCountedSubsumedBins) {
          cumulativeFrequencyBin += toCount->getFrequency();
        }
        cumulativeFrequency[bin] = cumulativeFrequencyBin;

        double to_compare = (double) cumulativeFrequencyBin;
        if (is_relative)
          to_compare = to_compare / totalFrequency;

        /* Popular if:
         *   - to_compare > popularity_treshold
         *   - if no_subsumed_popular is true, there are no
         *     subsuming bins of bin that are popular
         */
        if (to_compare > popularity_threshold &&
            ((! no_subsumed_popular) ||  (! bin->hasPopularAncestor())))
          bin->setPopular();
      }
    }

    // delete the sets
    // could free the memory earlier keeping track of the dependencies in the dag
    for (auto pair : notCountedSubsumedBinsMap) {
      delete pair.second;
    }

    for (auto elem : cumulativeFrequency)
      (elem.first)->setCumulativeFrequency(elem.second);
  }

  /**
   * Mark the popular patterns using the relative frequency
   * as threshold and popularity as measure.
   *
   * Furthermore, consider as popular bins that are not on the
   * "frontier" of popularity, but consider their total contribution to
   * poularity.
   *
   * For example, consider the following lattice with 4 bins and frequencies,
   * popularity, non-subsumed popularity, relative non-subsumed popularity
   *
   * B1 20, 40, 50.0%, 30, 75.0
   * B2  5,  5, 12.5%,  5, 12.5
   * B3  5,  5, 12.5%,  5, 12.5
   * B4 10, 10, 25.0%, 10, 25.0
   *
   * And the relation: (B1,B2), (B1,B3), (B1,B4)
   *
   * Relative and not absolute threshold: if the treshold is set to 5, then
   * B2, B3, B4 will be popular.
   *
   * If we consider a relative frequency (e.g., 20% of popularity), then we
   * get that B4 is popular.
   *
   * Do not just consider the frontier, but "how" much a bin contribute to a
   * pattern.
   *
   * If we consider the frontier, only B4 above will be popular. But B1 should
   * be popular too: there are other 75% of examples thare are not considered
   * in any popular pattern, but are really captured by B1.
   * So, not having B1 is not accetable.
   *
   * In this function we follow a different strategy to solve the above
   * problems"
   * - we consider a relative treshold for popularity.
   * - we compute a popular measure that takes into account how "much the"
   *   subsuming patterns are popular or not, instead of forgetting
   *   about them.
   */
  void FrequentSubgraphMiner::findPopularByRelFrequency(Lattice &lattice) {
    vector<AcdfgBin*> order;
    computeTopologicalOrder(lattice, order);
    computePopularity(lattice, order, false, true,
                      relative_pop_threshold);
  }

  void FrequentSubgraphMiner::classifyBins(Lattice &lattice) {
    // 1. Calculate the transitive reduction for each bin in the
    //    lattice and use it to judge popularity
    if (! use_relative_popularity) {
      findPopularByAbsFrequency(lattice);
    } else {
      findPopularByRelFrequency(lattice);
    }

    // 2. Now calculate the anomalous and isolated patterns
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

    // 1. Slice all the ACDFGs using the methods in the method names as the
    // target
    vector<Acdfg*> allSlicedACDFGs;
    sliceAcdfgs(filenames, methodnames, allSlicedACDFGs);

    // 2. Compute a binning of all the sliced ACDFGs using the exact isomorphism
    for (Acdfg* a: allSlicedACDFGs){
      bool acdfgSubsumed = false;

      for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it){
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

    // 4. Compute the lattice of bins
    calculateLatticeGraph(lattice);

    // 5. Classify the bin
    classifyBins(lattice);

    auto end = std::chrono::steady_clock::now();
    std::chrono::seconds time_taken =
      std::chrono::duration_cast<std::chrono::seconds>(end -start);

    // 6. Print all the  patterns
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

    // 4. Classify the bin
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
