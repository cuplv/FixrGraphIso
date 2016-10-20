
#include "fixrgraphiso/acdfg.h"
using namespace fixrgraphiso;

ControlEdge* addControlEdge(Acdfg * G, MethodNode * n1, MethodNode * n2){
  int edgeID = G -> edge_count();
  ControlEdge * ce = new ControlEdge(edgeID, n1, n2);
  G -> add_edge(ce);
  return ce;
}

DefEdge* addDefEdge(Acdfg * G, DataNode * d1, MethodNode * n2){
  int edgeID = G -> edge_count();
  DefEdge * ce = new DefEdge(edgeID, n2, d1);
  G -> add_edge(ce);
  
  return ce;
}

UseEdge* addUseEdge(Acdfg * G, DataNode * d1, MethodNode * n2){
  int edgeID = G -> edge_count();
  UseEdge * ce = new UseEdge(edgeID, d1, n2);
  G -> add_edge(ce);
  return ce;
}


DataNode* addDataNode(Acdfg * G, string name, string type){
  int nodeID = G -> node_count();
  DataNode * d = new DataNode(nodeID, name, type, DATA_NODE_VAR);
  G -> add_node(d);
  return d;
}


MethodNode * addMethodNode(Acdfg* G, string methName, DataNode* rcv, std::vector<DataNode*>  & args){
  int nodeID = G -> node_count();
  MethodNode * n = new MethodNode(nodeID, methName, rcv, args, NULL);
  G -> add_node(n);
  return n;
}

Acdfg * createGraphA() {

  Acdfg * acdfg_a = new Acdfg();
  /* add data nodes */
  /*-- Graph A
    y0 = a.build0();
    y1 = y0.build1();
    y2 = y1.build2();
    y3 = y2.build3();
    y4 = y3.build4();
    y5 = y4.buildFinal();
    --*/
  DataNode * d0 = new DataNode(0, "a", "type1", DATA_NODE_VAR);
  DataNode * d1 = new DataNode(1, "y0", "type1", DATA_NODE_VAR);
  DataNode * d2 = new DataNode(2, "y1", "type1", DATA_NODE_VAR);
  DataNode * d3 = new DataNode(3, "y2", "type1", DATA_NODE_VAR);
  DataNode * d4 = new DataNode(4, "y3", "type1", DATA_NODE_VAR);
  DataNode * d5 = new DataNode(5, "y4", "type1", DATA_NODE_VAR);
  DataNode * d6 = new DataNode(6, "y5", "type1", DATA_NODE_VAR);
  std::vector<DataNode*> args;
  MethodNode *  n0 = new MethodNode(7, "build0", d0, args, NULL);
  MethodNode *  n1 = new MethodNode(8, "build1", d1, args, NULL);
  MethodNode *  n2 = new MethodNode(9, "build2", d2, args, NULL);
  MethodNode *  n3 = new MethodNode(10, "build3", d3, args, NULL);
  MethodNode *  n4 = new MethodNode(11, "build4", d4, args, NULL);
  MethodNode *  n5 = new MethodNode(12, "buildFinal", d5, args, NULL);

  acdfg_a-> add_node(d0);
  acdfg_a-> add_node(d1);
  acdfg_a-> add_node(d2);
  acdfg_a-> add_node(d3);
  acdfg_a-> add_node(d4);
  acdfg_a-> add_node(d5);
  acdfg_a-> add_node(d6);
  acdfg_a-> add_node(n0);
  acdfg_a-> add_node(n1);
  acdfg_a-> add_node(n2);
  acdfg_a-> add_node(n3);
  acdfg_a-> add_node(n4);
  acdfg_a-> add_node(n5);

  addControlEdge(acdfg_a, n0,n1);
  addControlEdge(acdfg_a, n0,n2);
  addControlEdge(acdfg_a, n0,n3);
  addControlEdge(acdfg_a, n0,n4);
  addControlEdge(acdfg_a, n0,n5);
  addControlEdge(acdfg_a, n1,n2);
  addControlEdge(acdfg_a, n1,n3);
  addControlEdge(acdfg_a, n1,n4);
  addControlEdge(acdfg_a, n1,n5);
  addControlEdge(acdfg_a, n2,n3);
  addControlEdge(acdfg_a, n2,n4);
  addControlEdge(acdfg_a, n2,n5);
  addControlEdge(acdfg_a, n3,n4);
  addControlEdge(acdfg_a, n3,n5);
  addControlEdge(acdfg_a, n4,n5);

  addUseEdge(acdfg_a,d0, n0);
  addDefEdge(acdfg_a,d1, n0);
  addUseEdge(acdfg_a,d1, n1);
  addDefEdge(acdfg_a,d2, n1);
  addUseEdge(acdfg_a,d2, n2);
  addDefEdge(acdfg_a,d3, n2);
  addUseEdge(acdfg_a,d3, n3);
  addDefEdge(acdfg_a,d4, n3);
  addUseEdge(acdfg_a,d4, n4);
  addDefEdge(acdfg_a,d5, n4);
  addUseEdge(acdfg_a,d5, n5);
  addDefEdge(acdfg_a,d6, n5);

  return acdfg_a;
}

Acdfg * createGraphB(){
  Acdfg* acdfg_b = new Acdfg();
  /*==
    z0 = b.build0();
    z1  = z0.build3();
    z2 = z1.build1();
    z3 = z2.buildFinal();
    ==*/
  DataNode * b = addDataNode(acdfg_b, "b", "type1");
  DataNode * z0 = addDataNode(acdfg_b, "z0", "type1");
  DataNode * z1 = addDataNode(acdfg_b, "z1", "type1");
  DataNode * z2 = addDataNode(acdfg_b, "z2", "type1");
  DataNode * z3 = addDataNode(acdfg_b, "z3", "type1");
  std::vector<DataNode*> args;
  MethodNode * m0 = addMethodNode(acdfg_b, "build0", b, args);
  MethodNode * m1 = addMethodNode(acdfg_b, "build3", z0, args);
  MethodNode * m2 = addMethodNode(acdfg_b, "build1", z1, args);
  MethodNode * m3 = addMethodNode(acdfg_b, "buildFinal", z2, args);

  addControlEdge(acdfg_b, m0, m1);
  addControlEdge(acdfg_b, m0, m2);
  addControlEdge(acdfg_b, m0, m3);
  addControlEdge(acdfg_b, m1, m2);
  addControlEdge(acdfg_b, m1, m3);
  addControlEdge(acdfg_b, m2, m3);

  addUseEdge(acdfg_b, b,m0);
  addDefEdge(acdfg_b, z0, m0);
  addUseEdge(acdfg_b,z0, m1);
  addDefEdge(acdfg_b,z1, m1);
  addUseEdge(acdfg_b,z1, m2);
  addDefEdge(acdfg_b,z2, m2);
  addUseEdge(acdfg_b,z2, m3);
  addDefEdge(acdfg_b,z3, m3);

  return acdfg_b;

  
}
