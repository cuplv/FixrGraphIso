#include "fixrgraphiso/frequentSubgraphs.h"

int main(int argc, char * argv[]){
  fixrgraphiso::FrequentSubgraphMiner miner;

  return miner.mine(argc, argv);
}
