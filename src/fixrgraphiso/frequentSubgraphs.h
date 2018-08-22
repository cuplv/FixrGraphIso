#include <stdlib.h>
#include <vector>
#include "fixrgraphiso/acdfgBin.h"

namespace fixrgraphiso {
  using std::string;
  using std::vector;

  class FrequentSubgraphMiner {
    private:

    protected:
    void processCommandLine(int argc, char * argv[],
                            vector<string> & filenames,
                            vector<string> & methodNames);

    void calculateLatticeGraph(vector<AcdfgBin*> & allBins);

    void classifyBins(vector<AcdfgBin*> & allBins,
                      vector<AcdfgBin*> & popular,
                      vector<AcdfgBin*> & anomalous,
                      vector<AcdfgBin*> & isolated);

    void computePatternsThroughSlicing(vector<string> & filenames,
                                       vector<string> & methodnames);

    void testPairwiseSubsumption(vector<string> & filenames,
                                 vector<string> & methodnames);

    void dumpAllBins(vector<AcdfgBin*> & popular,
                     vector<AcdfgBin*> & anomalous,
                     vector<AcdfgBin*> & isolated,
                     std::chrono::seconds time_taken,
                     const string & output_prefix,
                     const string & infoFileName);


    public:
    FrequentSubgraphMiner();
    void mine(int argc, char * argv []);

    void mine(int freqCutoff,
              string methodNames,
              string outputPrefix,
              string acdfgFileName);


    private:
    int freq_cutoff = 20;
    string info_file_name = "cluster-info.txt";
    string output_prefix = ".";
    int minTargetSize = 2;
    int maxTargetSize = 100;
    int maxEdgeSize = 400;
    int anomalyCutOff = 5;
    bool runTestOfSubsumption = false;
  };


}
