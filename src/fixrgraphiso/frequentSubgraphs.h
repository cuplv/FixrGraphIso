#include <stdlib.h>
#include <vector>
#include "fixrgraphiso/acdfgBin.h"

namespace fixrgraphiso {
  using std::string;
  using std::vector;

  class FrequentSubgraphMiner {
    private:

    void computePopularity(Lattice &lattice,
                           const vector<AcdfgBin*> &order,
                           const bool no_subsumed_popular,
                           const bool is_relative,
                           const double popularity_threshold);

    protected:
    int processCommandLine(int argc, char * argv[],
                           vector<string> & filenames,
                           vector<string> & methodNames);

    void sliceAcdfgs(const vector<string> & filenames,
                     const vector<string> & methodnames,
                     Lattice& lattice,
                     vector<Acdfg*> & allSlicedACDFGs);

    void pruneFrontiers(Lattice &lattice,
                        Acdfg* acdfgToInsert,
                        set<AcdfgBin*> &notSubsumedBins,
                        set<AcdfgBin*> &notSubsumingBins);
    void binAndSubs(Lattice &lattice, Acdfg* a);
    void binAndSubs(Lattice &lattice,
                    vector<Acdfg*> &allSlicedACDFGs);

    void calculateLatticeGraph(Lattice & lattice);

    void findPopularByAbsFrequency(Lattice &lattice);

    void findPopularByRelFrequency(Lattice &lattice);

    void classifyBins(Lattice & lattice);

    void reClassifyBins();

    void computePatternsThroughSlicing(Lattice & lattice,
                                       vector<string> & filenames,
                                       vector<string> & methodnames);

    void testPairwiseSubsumption(vector<string> & filenames,
                                 vector<string> & methodnames);

    public:
    FrequentSubgraphMiner();
    int mine(int argc, char * argv []);

    int mine(Lattice & lattice,
             int freqCutoff,
             string methodNames,
             string outputPrefix,
             string acdfgFileName);


    private:
    int freq_cutoff = 20;
    string info_file_name = "cluster-info.txt";
    string lattice_filename = "lattice.bin";
    string output_prefix = ".";
    int minTargetSize = 2;
    int maxTargetSize = 100;
    int maxEdgeSize = 400;
    int anomalyCutOff = 5;
    bool runTestOfSubsumption = false;
    bool rerunClassification = false;

    bool use_relative_popularity = false;
    double relative_pop_threshold = 0.2;
  };


}
