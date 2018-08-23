#include "fixrgraphiso/frequentSubgraphs.h"

int main(int argc, char * argv[]){
  fixrgraphiso::FrequentSubgraphMiner miner;

  miner.mine(argc, argv);

  return 0;
}
