#! /bin/bash

echo "filename, total time, # subsumption checks, # SAT calls , # sat solve time (ms), # graphs, avg # of nodes, avg. # of edges, max # of nodes, max # of edges"
for i in `cat allFiles`
do
    echo -n $i ", "
    echo -n `grep "Total Time" $i | awk '{print $4}'`
    echo -n ", "
    echo -n `grep "Subsumption checks" $i | awk '{print $4}'`
    echo -n ", "
    echo -n `grep "SAT calls" $i | awk '{print $4}'`
    echo -n ", "
    echo -n `grep "satSolverTime" $i | awk '{print $4}'`
    echo -n ", "
    echo -n `grep "Graphs" $i | awk '{print $4}'`
    echo -n ", "
    echo -n `grep "Average # of nodes" $i | awk '{print $5}'`
    echo -n ", "
    echo -n `grep "Average # of edges" $i | awk '{print $5}'`
    echo -n ", "
    echo -n `grep "Max. # of nodes" $i | awk '{print $6}'`
    echo -n ", "
    echo -n `grep "Max. # of edges" $i | awk '{print $6}'`
    
    
    echo ""
done
