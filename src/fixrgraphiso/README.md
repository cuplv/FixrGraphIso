# Structure of the code

## Data structure
- acdfg.h, acdfg.cpp: Data structure for the ACDFG
- serialization.h, serialization.cpp: Serialize/de-serialize from ACDFG to protobuf

## Frequent itemset computation
- frequentItemSetsMain.cpp: Entry point for the computation of the frequent itemsets
- itemSetDB.h, itemSetDB.cpp: Implementation of the frequent itemset algorithm

## Frequent subgraph mining computation
- frequentSubgraphsMain.cpp: Entry point for the computation of the frequent subgraphs
- acdfgBin.h, acdfgBin.cpp  : Data strcuture storing a Bin
- isomorphismClass.h, isomorphismClass.cpp: Implementation of exact sugraph isomorphism and isomorphism
- isomorphismResults.h: Results of the isomorphism, serialization to protobuf
- collectStats.h: Store the run-time statistics for the subsumption check

## Approximate isomorphism computation using ILP
- ilpApproxIsomorphismEncoder.h, ilpApproxIsomorphismEncoder.cpp: ILP encoding of the subgraph isomorphism
- milpProblem.h, milpProblem.cpp: 
- main.cpp: Compute approximate isomorphism between two graphs
- iso.cpp, iso.h: Implementation approximate isomorphism using MaxSAT (not used)
