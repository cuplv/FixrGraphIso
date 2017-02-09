#ifndef D__COLLECT_STATS__H__
#define D__COLLECT_STATS__H__

#include <iostream>
#include <chrono>

namespace fixrgraphiso{
  
  extern int numSATCalls;
  extern int numSubsumptionChecks;
  extern std::chrono::milliseconds satSolverTime;

  static void initializeStats(){
    numSATCalls = 0;
    numSubsumptionChecks = 0;
  }
  
  static void addSubsumptionCheck(){
    numSubsumptionChecks++;
  }
  
  static void addSATCallStat(std::chrono::milliseconds t){
    numSATCalls++;
    satSolverTime = satSolverTime + t;
  }
  
  static void printStats(std::ostream & out){
    out << "# Subsumption checks" << numSubsumptionChecks << std::endl;
    out << "# SAT calls" << numSATCalls << std::endl;
    out << "# satSolverTime (ms) " << satSolverTime.count()<< std::endl;
  }
  
  
}

#endif
