#ifndef D__COLLECT_STATS__H__
#define D__COLLECT_STATS__H__

#include <iostream>
#include <chrono>

namespace fixrgraphiso{
  class Stats {
    public:
    Stats() : numSATCalls(0), numSubsumptionChecks(0), totalEdges(0),
      totalGraphs(0), totalNodes(0), maxNodes(0), maxEdges(0), minNodes(0),
      minEdges(0), satSolverTime(0) {}

    void addSubsumptionCheck(){
      this->numSubsumptionChecks++;
    }

    void addSATCallStat(std::chrono::milliseconds t){
      this->numSATCalls++;
      this->satSolverTime = this->satSolverTime + t;
    }

    void addGraphStats(int n_nodes, int n_edges){
      this->totalGraphs++;
      this->totalNodes += n_nodes;
      this->totalEdges += n_edges;
      if (n_nodes >= this->maxNodes){
        this->maxNodes = n_nodes;
      }

      if (n_edges >= this->maxEdges){
        this->maxEdges = n_edges;
      }
    }

    void print(std::ostream & out){
      out << "# Graphs : " << this->totalGraphs << std::endl;
      if (this->totalGraphs > 0){
        out << "Average # of nodes: " << this->totalNodes/this->totalGraphs << std::endl;
        out << "Average # of edges: " << this->totalEdges/this->totalGraphs << std::endl;
        out << "Max. # of nodes : " << this->maxNodes << std::endl;
        out << "Max. # of edges : " << this->maxEdges << std::endl;
      }
      out << "# Subsumption checks: " << this->numSubsumptionChecks << std::endl << std::endl;

      out << "# SAT calls: " << this->numSATCalls << std::endl;

      out << "# satSolverTime (ms): " << this->satSolverTime.count() << std::endl;
    }

    private:
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
  };
}

#endif
