#ifndef D__ISOMORPHISM_CLASS_H__
#define D__ISOMORPHISM_CLASS_H__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "fixrgraphiso/proto_unweighted_iso.pb.h"
#include "z3++.h"
#include "fixrgraphiso/acdfg.h"

namespace fixrgraphiso {
  using std::string;
  using std::vector;
  using std::cout;
  using std::endl;
  using std::pair;
  using std::map;

  using edu::colorado::plv::fixr::protobuf::UnweightedIso;

  extern bool debug;

  typedef std::pair<long, long> id_pair_t;


  /**
   * Encodes the subgraph isomorphism problem in Z3
   */
  class IsoEncoder {
  protected:
    z3::context ctx;
    z3::solver s;
    bool satisfiable;
    bool alreadySolved;
  public:

    typedef z3::expr var_t;
    IsoEncoder();
    ~IsoEncoder();
    var_t createBooleanVariable(const string & vName);
    void atmostOne(const vector<var_t>  & what);
    void atleastOne(const vector<var_t>  & what);
    void exactlyOne(const vector<var_t> & what);
    void addImplication(var_t a, var_t b);
    void solve();
    bool isSat();
    void resetSatState();
    bool getTruthValuation(var_t x);
  };

  /**
   * Stores the isomorphism relation between two ACDFGs
   */
  class IsoRepr {
    public:
    IsoRepr(Acdfg* acdfg_1, Acdfg* acdfg_2) {
      this->acdfg_1 = acdfg_1;
      this->acdfg_2 = acdfg_2;
    }

    IsoRepr(const UnweightedIso& iso);

    void addNodeRel(node_id_t node_1, node_id_t node_2) {
      nodesRel.insert(id_pair_t(node_1,node_2));
    }
    void addEdgeRel(edge_id_t edge_1, edge_id_t edge_2) {
      edgesRel.insert(id_pair_t(edge_1,edge_2));
    }

    const set<id_pair_t> & getNodesRel() const {return nodesRel;}
    const set<id_pair_t> & getEdgesRel() const {return edgesRel;}

    UnweightedIso* proto_from_iso() const;

    private:
    Acdfg* acdfg_1;
    Acdfg* acdfg_2;

    set<id_pair_t> nodesRel;
    set<id_pair_t> edgesRel;
  };

  /**
   * Check if acdfg_a subsumes acdfg_b
   *
   * WARNING: the encoding is correct ONLY if the acdfg is sliced now
   * i.e. it does not contain regular nodes
   */
  class IsoSubsumption {

  protected:
    typedef long node_id_t;
    typedef long edge_id_t;
    typedef map< id_pair_t, IsoEncoder::var_t> id_pair_map_t;
    typedef map< node_id_t, vector<node_id_t> > compat_nodes_map_t;
    typedef map< edge_id_t, vector<edge_id_t> > compat_edges_map_t;

    Acdfg * acdfg_a;
    Acdfg * acdfg_b;
    IsoEncoder e;
    // Encoding variables
    id_pair_map_t node_pairs_to_var;
    id_pair_map_t edge_pairs_to_var; // We need two maps because edge ids may duplicate node ids
    // Book keeping for the encoding
    compat_nodes_map_t nodes_a_to_b;
    compat_nodes_map_t nodes_b_to_a;
    compat_edges_map_t edges_a_to_b;
    compat_edges_map_t edges_b_to_a;

    IsoEncoder::var_t getNodePairVar(node_id_t a, node_id_t b) const;
    IsoEncoder::var_t getEdgePairVar(edge_id_t a, edge_id_t b) const ;
    bool isCompatibleNodePair(node_id_t a, node_id_t b) const;
    bool isCompatibleNodePair(const Node * na, const Node  * nb) const {
      return isCompatibleNodePair(na -> get_id(), nb -> get_id());
    }

    bool staticCheckMethodNodeCompatible(const MethodNode * ma, const MethodNode * mb) const;
    void addCompatibleNodePair(Node * na, Node * nb);
    void addCompatibleEdgePair(Edge * ea, Edge * eb);

    void createEncodingVariables();
    void addIsoEncoding();

    void buildIsoRepr(IsoRepr* iso);

  public:
    IsoSubsumption(Acdfg * a, Acdfg * b);
    ~IsoSubsumption(){}

    bool checkNodeCounts() const;
    bool findCompatibleMethodNodes();
    bool findCompatibleDataNodes();
    bool findCompatibleEdgePairs();
    void makeEncoding();
    bool check();
    bool check(IsoRepr *iso);
    bool check_iso(IsoRepr *iso);
    bool canSubsume();
  };


  /**
   * Check if acdfg_a is isomorphic to acdfg_b
   */
  class IsomorphismClass {
  protected:
    Acdfg * acdfg;
    string filename_a;
    string filename_b;
    string iso_filename;
    int freq;
    std::vector<std::string> subsumingACDFGs;
  public:

    IsomorphismClass(Acdfg * what);
    IsomorphismClass(string const & iso_filename);
    ~IsomorphismClass();
    bool subsumes(IsomorphismClass const * what) const;
    Acdfg * get_acdfg() const { return acdfg; }
    string getIsoFilename() const { return iso_filename;}
    int getFrequency() const {return freq; }
    void setFrequency(int f){ this -> freq = f ; }
    void incrFrequency(){ freq++; }
    void addSubsumingACDFG(std::string const & what){
      subsumingACDFGs.push_back(what);
    }

    std::vector<std::string> const & getSubsumingACDFGs(){ return subsumingACDFGs; }

    void copySubsumingACDFGs(IsomorphismClass const & what){
      std::vector<std::string> const & v = what.subsumingACDFGs;
      for (const auto s: v){
        (this -> subsumingACDFGs).push_back(s);
      }
    }

  };
}


#endif
