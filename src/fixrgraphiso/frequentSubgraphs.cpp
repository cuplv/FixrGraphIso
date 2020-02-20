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


#define INCR_LATTICE_COMPUTATION 0

namespace fixrgraphiso {
  namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;

  bool debug = false;

  void loadNamesFromFile (string filename, vector<string> & listOfNames){
    ifstream ifile(filename.c_str());
    string line;
    while (std::getline(ifile, line)){
      line.erase( std::remove_if( line.begin(),
                                  line.end(),
                                  [](char x){ return std::isspace(x);}),
                  line.end());
      listOfNames.push_back(line);
      //std::cout << "\t Adding: " << line << endl;
    }
  }

  void filterFileNames(Lattice& lattice, vector<string>& fileNames) {
    set<string> existing;
    vector<string> newfiles;
    for (auto bin : lattice.getAllBins()) {
      for (auto name : bin->getAcdfgNames())
        existing.insert(name);
    }

    for (auto name : fileNames) {
      if (existing.find(name) == existing.end()) {
        newfiles.push_back(name);
      }
    }

    fileNames.clear();

    cout << "Incremental computation, skipping " <<
      fileNames.size() - newfiles.size() <<
      " already computed graphs." << endl;

    for (auto name : newfiles) {
      fileNames.push_back(name);
    }
  }

  Acdfg * loadACDFGFromFilename(string filename){
    AcdfgSerializer s;
    iso_protobuf::Acdfg * proto_acdfg = s.read_protobuf_acdfg(filename.c_str());
    Acdfg * acdfg = s.create_acdfg((const iso_protobuf::Acdfg &) *proto_acdfg);
    acdfg -> setName(filename);
    delete proto_acdfg;
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

  void FrequentSubgraphMiner::saveState(Lattice &lattice, bool toSave) {
    if (toSave) {
      cout << "Saving lattice... " << endl;
      fixrgraphiso::writeLattice(lattice, lattice_filename);
    }
  }

  int FrequentSubgraphMiner::processCommandLine(int argc, char * argv[],
                                                vector<string> & filenames,
                                                vector<string> & methodNames) {
    char c;
    int index;
    while ((c = getopt(argc, argv, "dm:f:t:o:i:zp:l:cr:sa"))!= -1) {
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
      case 's':
        incremental = true;
        std::cout << "Incremental lattice computation..." << endl;
        break;
      case 'a':
        anytimeComputation = true;
        std::cout << "Use anytime computation..." << endl;
        break;
      case 'c':
        rerunClassification = true;
        break;
      case 'd':
        fixrgraphiso::debug = true;
        break;
      case 'f':
        freq_cutoff = strtol(optarg, NULL, 10);
        std::cout << "Using frequency: " << freq_cutoff << endl;
        break;
      case 'i':
        {
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
        std::cout << "Using relative frequency " << relative_pop_threshold << endl;
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
        "-a " <<
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
                                          Lattice& lattice,
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
          lattice.getStats()->addGraphStats(new_acdfg->node_count(), new_acdfg->edge_count());
          allSlicedACDFGs.push_back(new_acdfg);
        }
      }
      delete(orig_acdfg);
    }
  }

  void FrequentSubgraphMiner::calculateLatticeGraph(Lattice & lattice) {
    // complete the graph adding all the subsuming relations
    int i = 0;
    for (auto it = lattice.beginAllBins(); it != lattice.endAllBins(); ++it) {
      AcdfgBin * a = *it;

      for (auto jt = lattice.beginAllBins(); jt != lattice.endAllBins(); ++jt) {
        AcdfgBin * b = *jt;
        if (a == b) continue;

        if (a -> isACDFGBinSubsuming(b)){
          // b subsumes a
          a -> addSubsumingBin(b);
        }

        i += 1;
        saveState(lattice, i % 10000 == 0 && incremental);
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
      totalFrequency += bin->getFrequency();
      cumulativeFrequency[bin] = 0;
    }

    for (auto bin : order) {
      assert(notCountedSubsumedBinsMap.find(bin) == notCountedSubsumedBinsMap.end());
      notCountedSubsumedBinsMap[bin] = new set<AcdfgBin*>();

      set<AcdfgBin*>* notCountedSubsumedBins = notCountedSubsumedBinsMap[bin];
      set<AcdfgBin*> toRemove;

      notCountedSubsumedBins->insert(bin);

      for (auto toBin : bin->getImmediateSubsumingBins()) {
        assert(notCountedSubsumedBinsMap.find(toBin) != notCountedSubsumedBinsMap.end());

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
            ((! no_subsumed_popular) ||  (! bin->hasPopularAncestor()))) {
          if (debug){
            std::cout << "Found popular: " << bin << endl <<
              "Cumulative frequency: " << cumulativeFrequencyBin << endl <<
              "Compared with: " << to_compare << endl <<
              "Frequency: " << bin->getFrequency() << endl;
          }
          bin->setPopular(true);
        }
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
    lattice.computeTopologicalOrder(order);
    computePopularity(lattice, order, false, true,
                      relative_pop_threshold);
  }

  void FrequentSubgraphMiner::classifyBins(Lattice &lattice) {
    // 1. Calculate the transitive reduction for each bin in the
    //    lattice and use it to judge popularity
    if (! use_relative_popularity) {
      if (debug)
        cout << "Classifying using absolute frequency..." << endl;
      findPopularByAbsFrequency(lattice);
    } else {
      if (debug)
        cout << "Classifying using relative frequency..." << endl;
      findPopularByRelFrequency(lattice);
    }

    // 2. Now calculate the anomalous and isolated patterns
    for (auto it = lattice.beginAllBins();
         it != lattice.endAllBins(); ++it){
      AcdfgBin * a = *it;

      if ((! use_relative_popularity) && a -> isSubsuming()) continue;
      if (a -> isPopular()) {
        lattice.addPopular(a);
      } else if (a -> getFrequency() <= anomalyCutOff &&
                 a -> hasPopularAncestor()){
        a -> setAnomalous();
        lattice.addAnomalous(a);
      } else if ((! a -> isSubsuming()) &&
                 a -> getFrequency() <= anomalyCutOff){
        a->setIsolated();
        lattice.addIsolated(a);
      }
    }

    return;
  }

  std::chrono::seconds diff_times(std::chrono::time_point<std::chrono::steady_clock> start,
                                  std::chrono::time_point<std::chrono::steady_clock> end) {
    return std::chrono::duration_cast<std::chrono::seconds>(end -start);
  }

  std::chrono::milliseconds diff_times_ms(std::chrono::time_point<std::chrono::steady_clock> start,
                                  std::chrono::time_point<std::chrono::steady_clock> end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end -start);
  }


  /**
   * Prune at the frontier of subsumption
   */
  void FrequentSubgraphMiner::pruneFrontiers(Lattice &lattice,
                                             Acdfg* acdfgToInsert,
                                             set<AcdfgBin*> &notSubsumedBins,
                                             set<AcdfgBin*> &notSubsumingBins)

  {
    for (auto bin : lattice.getAllBins()) {
      Acdfg* binAcdfg = bin->getRepresentative();

      if (not acdfgToInsert->canSubsumeB(*binAcdfg)) {
        notSubsumedBins.insert(bin);
      }

      if (not binAcdfg->canSubsumeB(*acdfgToInsert)) {
        notSubsumingBins.insert(bin);
      }
    }
  }

  /**
   * Add the Acdfg to the lattice.
   */
  void FrequentSubgraphMiner::binAndSubs(Lattice &lattice, Acdfg* acdfgToInsert) {
    vector<AcdfgBin*> frontier;
    set<AcdfgBin*> visited;
    vector<AcdfgBin*> subsumedBins;
    vector<AcdfgBin*> subsumingBins;

    // set of bins acdfgToInsert cannot subsume
    set<AcdfgBin*> notSubsumedBins;
    // set of bins that cannot subsume acdfgToInsert
    set<AcdfgBin*> notSubsumingBins;

    // Try to prune the bins that are "easily" not subsumed or subsuming
    pruneFrontiers(lattice, acdfgToInsert,
                   notSubsumedBins, notSubsumingBins);

    // Get all non-subsumed bins
    for (auto bin : lattice.getAllBins())
      if (bin->getIncomingEdges().size() == 0)
        frontier.push_back(bin);

    int i = 0;
    while (frontier.size() > 0) {
      AcdfgBin* next_bin = frontier.back();
      IsoRepr* isoRepr = new IsoRepr(acdfgToInsert,
                                     next_bin->getRepresentative());

      frontier.pop_back();

      // avoid duplicates
      if (visited.find(next_bin) != visited.end())
        continue;
      visited.insert(next_bin);

      bool canBeSubsumed;
      bool canSubsume;

      canBeSubsumed = notSubsumingBins.find(next_bin) == notSubsumingBins.end();
      canSubsume = notSubsumedBins.find(next_bin) == notSubsumedBins.end();

      AcdfgBin::SubsRel compareRes = next_bin->compareACDFG(acdfgToInsert,
                                                            isoRepr,
                                                            canSubsume,
                                                            canBeSubsumed);
      switch(compareRes) {
      case AcdfgBin::EQUIVALENT:
        // Do not visit any other bin, the search ends here
        next_bin->insertEquivalentACDFG(acdfgToInsert, isoRepr);
        return;
      case AcdfgBin::SUBSUMED:
        // acdfgToInsert is subsumed by bin
        subsumingBins.push_back(next_bin);

        // Removes all the bins subsuming next_bin from the visit
        {
          set<AcdfgBin*> reachable;
          next_bin->getReachable(next_bin->getSubsumingBins(),
                                 reachable, false);
          for(auto upperBins : reachable) {
            subsumingBins.push_back(upperBins);
            visited.insert(upperBins);
          }
        }
        break;
      case AcdfgBin::SUBSUMING:
        // acdfgToInsert subsumes the bin
        subsumedBins.push_back(next_bin);
        {
          set<AcdfgBin*> reachable_prev;
          next_bin->getReachable(next_bin->getIncomingEdges(),
                                 reachable_prev, true);
          for(auto lowerBins : reachable_prev) {
            subsumedBins.push_back(lowerBins);
            visited.insert(lowerBins);

            for (auto bin : lowerBins->getImmediateSubsumingBins()) {
              if (visited.find(bin) == visited.end()) {
                frontier.push_back(bin);
              }
            }
          }
        }

        for(auto upperBins : next_bin->getImmediateSubsumingBins())
          if (visited.find(upperBins) == visited.end()) {
            frontier.push_back(upperBins);
          }

        break;
      case AcdfgBin::NONE:
        /* From this result and transitivity we infer the following:
         * 1. acdfgToInsert cannot subsume any bin subsumed by next_bin
         *   - next_bin <= bin and bin <= acdfgToInsert implies
         *     next_bin <= acdfgToInsert
         *     This would contradict ! (next_bin <= acdfgToInsert)
         *
         * 2. acdfgToInsert cannot be subsumed by any bin that next_bin subsumes
         *   - acdfgToInsert <= bin and bin <= next_bin implies
         *     acdfgToInsert <= next_bin
         *     This would contradict ! (acdfgToInsert <= next_bin)
         */

        {
          // 1. set of bins acdfgToInsert cannot subsume
          set<AcdfgBin*> reachable;
          next_bin->getReachable(next_bin->getImmediateSubsumingBins(),
                                 reachable, false);
          for (auto bin : reachable)
            notSubsumedBins.insert(bin);

          // 2. set of bins that cannot subsume acdfgToInsert
          set<AcdfgBin*> reachable_prev;
          next_bin->getReachable(next_bin->getIncomingEdges(),
                                 reachable_prev, true);
          for (auto bin : reachable_prev)
            notSubsumingBins.insert(bin);

          // Visit all the children, we enforce what we learned
          // using the sets
          for(auto upperBins : next_bin->getImmediateSubsumingBins())
            frontier.push_back(upperBins);
        }
        break;
      }

      delete isoRepr;
    } // end of reachability on lattice


    // we have to create a new bin
    AcdfgBin * newbin = new AcdfgBin(acdfgToInsert, lattice.getStats());
    lattice.addBin(newbin);

    // newBin subsumes subsumed
    for (auto subsumed : subsumedBins)
      subsumed->addSubsumingBin(newbin);

    // subsuming subsumes newBin
    for (auto subsuming : subsumingBins)
      newbin->addSubsumingBin(subsuming);
  }


  int get_acdfg_counts(Acdfg* b) {
    return 
      b->node_count() +
      b->edge_count() +
      b->data_node_count() +
      b->method_node_count() +
      b->control_edge_count() +
      b->use_edge_count() +
      b->def_edge_count() +
      b->exceptional_edge_count();
  }

  bool compareBins(Acdfg* b1, Acdfg* b2)
  {
    return get_acdfg_counts(b1) < get_acdfg_counts(b2);
  }

  /**
   * Add the Acdfgs to the lattice.
   */
  void FrequentSubgraphMiner::binAndSubs(Lattice &lattice,
                                         vector<Acdfg*> & allSlicedACDFGs) {
    int i = 0;

    for (Acdfg* a: allSlicedACDFGs) {
      i++;

      if (i % 10 == 0) {
        cout << "Processing acdfg " << i << "/" <<
          allSlicedACDFGs.size() << ".." << endl;
      }

      binAndSubs(lattice, a);

      saveState(lattice, i % 1000 == 0 && incremental);
    }

    // Compute the transitive closure of the lattice
    lattice.makeClosure();
  }

  void FrequentSubgraphMiner::computePatternsThroughSlicing(Lattice & lattice,
                                                            vector<string> & filenames,
                                                            vector<string> & methodnames) {
    // compute the elapsed real time for the computation (no cpu time)
    auto start = std::chrono::steady_clock::now();

    // 1. Slice all the ACDFGs using the methods in the method names as the
    // target
    vector<Acdfg*> allSlicedACDFGs;
    sliceAcdfgs(filenames, methodnames, lattice, allSlicedACDFGs);
    std::sort(allSlicedACDFGs.begin(), allSlicedACDFGs.end(), compareBins);

    auto end_slicing = std::chrono::steady_clock::now();
    cout << "Slicing took " << diff_times(start, end_slicing).count() << endl;

    if (anytimeComputation) {
      // Compute bins and lattice at the same time
      binAndSubs(lattice, allSlicedACDFGs);
      lattice.sortByFrequency();

      auto end_binning = std::chrono::steady_clock::now();
      cout << "Binning and lattice computation took " <<
        diff_times(end_slicing, end_binning).count() << endl;
    } else {
      // 2. Compute a binning of all the sliced ACDFGs using the exact
      // isomorphism
      int i = 0;
      for (Acdfg* a: allSlicedACDFGs){
        i++;
        bool acdfgSubsumed = false;
        bool delete_a = true;

        std::cerr << "Acdfg " << i << "/" <<
          allSlicedACDFGs.size() << ".." << std::endl;

        for (auto it = lattice.beginAllBins();
             it != lattice.endAllBins(); ++it) {
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
          AcdfgBin * newbin = new AcdfgBin(a, lattice.getStats());
          lattice.addBin(newbin);
        }

        saveState(lattice, i % 1000 == 0 && incremental);
      }

      auto end_binning = std::chrono::steady_clock::now();
      cout << "Binning took " <<
        diff_times(end_slicing, end_binning).count() << endl;

      lattice.sortByFrequency();

      // Compute the lattice of bins
      calculateLatticeGraph(lattice);
      lattice.makeClosure();

      auto end_lattice = std::chrono::steady_clock::now();
      cout << "Lattice took " <<
        diff_times(end_binning, end_lattice).count() << endl;
    }

    assert(lattice.isValid()); // To run in debug mode

    cout << "Total bins " << lattice.getAllBins().size() << endl;

    // Classify the bin
    classifyBins(lattice);

    auto end = std::chrono::steady_clock::now();
    std::chrono::seconds time_taken =
      std::chrono::duration_cast<std::chrono::seconds>(end -start);

    // Print all the patterns
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
      delete orig_acdfg;
      new_acdfg -> setName(f);
      allACDFGs.push_back(new_acdfg);
    }

    Stats stats;
    for (Acdfg * a : allACDFGs){
      for (Acdfg * b : allACDFGs){
        if (a == b) continue;
        IsoSubsumption isoSub(a, b, &stats);
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
        Lattice *lattice_ptr;

        if (incremental) {
          lattice_ptr = fixrgraphiso::readLattice(lattice_filename);
          if (NULL == lattice_ptr) {
            lattice_ptr = new Lattice(methodnames);
          } else {
            filterFileNames(*lattice_ptr, filenames);
          }
        } else {
          lattice_ptr = new Lattice(methodnames);
        }

        computePatternsThroughSlicing(*lattice_ptr, filenames, methodnames);

        delete lattice_ptr;
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
    filterFileNames(lattice, fileNames);

    computePatternsThroughSlicing(lattice, fileNames, methodNames);

    return 0;
  }
}
