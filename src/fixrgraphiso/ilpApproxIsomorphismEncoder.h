#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include "fixrgraphiso/acdfg.h"
#include "fixrgraphiso/milpProblem.h"
namespace fixrgraphiso {

  
  
  /*
   *  ILP encoding of isomorphism between two acdfg
   *
   *  1. Collect node pairs that are compatible with each other.
   *     These will be a map from a given node to a list of compatible nodes on the other graphs.
   *  2. Create variables for the isomorphism.
   *  3. Formulate constraints.
   *  4. Formulate the objective function.
   */
  using std::map;
  using std::vector;
  using std::string;
  
  
  typedef  std::map< node_id_t, vector<node_id_t> > compatible_node_map_t;
  typedef std::pair<edge_id_t, edge_id_t> edge_pair_t;
  
  class IlpApproxIsomorphism {
    
    
  
  public:
    
  IlpApproxIsomorphism(Acdfg* a, Acdfg* b ): acdfg_a(a), acdfg_b(b){};
    
    ~IlpApproxIsomorphism(){};
    
    
    void computeILPEncoding();

  private:

    Acdfg * acdfg_a; // The first graph
    Acdfg * acdfg_b; // The second graph
    compatible_node_map_t node_map_a_to_b; // Map each node from a to a compatible list of nodes from b.
    compatible_node_map_t node_map_b_to_a; // Reverse map each node from b to a compatible list of nodes from a.
    std::vector< edge_pair_t > compat_edges_a_to_b; // Let's just make a list of compatible edges from a to b
    MILProblem milp;
    
    void addCompatibleNodes(Node * a, Node* b); // Mark nodes from graph a and graph b as compatible
    void computeCompatibleNodes();
    void computeCompatibleEdges();
    void addAllCompatibleEdges(std::vector<edge_id_t> const & vA, std::vector<edge_id_t> const & vB);
    bool areCompatibleNodes(node_id_t aID, node_id_t bID);
    void insertCompatibleEdges(edge_id_t eA, edge_id_t eB);
    std::vector<node_id_t> getCompatibleNodeIDs(char a_or_b, node_id_t id);
    void initializeMILP();
    void createVariablesForCompatibleNodes();
    void createVariablesForCompatibleEdges();
    void addEdgeSourceTargetMatch();
    void addWeightOfCompatibleNodesEquation();
    void addDataNodeMustNotBeIsolated();
    void createObjectiveFunction();
    void addUniqueMatchingConstraint(char a_or_b);
  };
  

}
