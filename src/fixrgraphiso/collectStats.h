#ifndef D__COLLECT_STATS__H__
#define D__COLLECT_STATS__H__

#include <iostream>
#include <chrono>

namespace fixrgraphiso{

  struct stats_struct{
    int numSATCalls;
    int numSubsumptionChecks;
    int totalGraphs;
    int totalNodes;
    int totalEdges;
    int maxNodes;
    int maxEdges;
    int minNodes;
    int minEdges;

    std::chrono::milliseconds satSolverTime;

  stats_struct():numSATCalls(0), numSubsumptionChecks(0), totalEdges(0),
      totalGraphs(0), totalNodes(0), maxNodes(0), maxEdges(0), minNodes(0),
      minEdges(0), satSolverTime(0) {}

  };

  extern stats_struct all_stats;


  static void addSubsumptionCheck(){
    all_stats.numSubsumptionChecks++;
  }

  static void addSATCallStat(std::chrono::milliseconds t){
    all_stats.numSATCalls++;
    all_stats.satSolverTime = all_stats.satSolverTime + t;
  }

  static void addGraphStats(int n_nodes, int n_edges){
    all_stats.totalGraphs++;
    all_stats.totalNodes += n_nodes;
    all_stats.totalEdges += n_edges;
    if (n_nodes >= all_stats.maxNodes){
      all_stats.maxNodes = n_nodes;
    }

    if (n_edges >= all_stats.maxEdges){
      all_stats.maxEdges = n_edges;
    }
  }

  static void printStats(std::ostream & out){
    out << "# Graphs : " << all_stats.totalGraphs << std::endl;
    if (all_stats.totalGraphs > 0){
      out << "Average # of nodes: " << all_stats.totalNodes/all_stats.totalGraphs << std::endl;
      out << "Average # of edges: " << all_stats.totalEdges/all_stats.totalGraphs << std::endl;
      out << "Max. # of nodes : " << all_stats.maxNodes << std::endl;
      out << "Max. # of edges : " << all_stats.maxEdges << std::endl;
    }
    out << "# Subsumption checks: " << all_stats.numSubsumptionChecks << std::endl << std::endl;

    out << "# SAT calls: " << all_stats.numSATCalls << std::endl;

    out << "# satSolverTime (ms): " << all_stats.satSolverTime.count() << std::endl;
  }
}

#endif
