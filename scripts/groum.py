from fixrgraph.annotator.protobuf.proto_acdfg_pb2 import Acdfg

class Groum(object):
    FIELD_TYPE = "0"
    METHOD_TYPE = "1"
    CONTROL_TYPE = "2"
    OTHER_TYPE = "3"

    class GroumNode(object):
        def __init__(self, node_id, node_type, class_name, object_name, method_name, start_line, end_line):
            self.node_id = node_id
            self.node_type = node_type
            self.class_name = class_name
            self.object_name = object_name
            self.method_name = method_name
            self.start_line = start_line
            self.end_line = end_line

        def __eq__(self, other):
            return self.node_id == other.node_id

        def __hash__(self):
            return hash(self.node_id)

    def __init__(self, pattern_id, frequency):
        self.pattern_id = pattern_id
        self.frequency = frequency
        self.nodes_id = set()
        self.nodes = set()
        # map from nodes to a list of nodes
        self.edges = {}

        self.files = []

    def has_node(self, node_id):
        return node_id in self.nodes_id

    def add_node(self, node):
        self.nodes.add(node)
        self.nodes_id.add(node.node_id)

    def add_edge(self, node_id_src, node_id_dst):
        assert node_id_src in self.nodes_id
        assert node_id_dst in self.nodes_id

        if node_id_src in self.edges:
            l = self.edges[node_id_src]
        else:
            l = []
            self.edges[node_id_src] = l
        l.append(node_id_dst)

    def add_file(self, fname):
        self.files.append(fname)

    def get_tot_edges(self):
        tot_edges = 0
        for (k,l) in self.edges.iteritems():
            tot_edges = tot_edges + len(l)
        return tot_edges

    def to_dot(self, dot_file):
        dot_file.write("""digraph isoX {
  node[shape=box,style="filled,rounded",penwidth=2.0,fontsize=13,];
  edge[ arrowhead=onormal,penwidth=2.0,];
""")

        for node in self.nodes:
            if node.node_type == Groum.FIELD_TYPE:
                style = "shape=ellipse,color=red,style=dashed"
            elif node.node_type == Groum.METHOD_TYPE:
                style = "shape=box, style=filled, color=lightgray"
            elif node.node_type == Groum.CONTROL_TYPE:
                style = "shape=box, style=filled, color=lightblue"
            elif node.node_type == Groum.OTHER_TYPE:
                style = "shape=box, style=filled, color=lightred"
            else:
                assert(False)

            dot_file.write(""""n_%s" [ %s, label="%s %s.%s"];
""" % (node.node_id, style, node.object_name, node.class_name, node.method_name))

        for (src_id, dst_list) in self.edges.iteritems():
            for dst_id in dst_list:
                dot_file.write(""""n_%s" -> "n_%s"[color=blue, penwidth=2];
""" % (src_id, dst_id))

        dot_file.write("""}""")


    def to_acdfg(self, acdfg_file):
        acdfg = Acdfg() # create a new acdfg

        for node in self.nodes:
            if node.node_type == Groum.FIELD_TYPE:
                data_node = acdfg.data_node.add()
                data_node.id = int(node.node_id)
                data_node.name =  "%s.%s" % (node.class_name, node.object_name)
                data_node.type = "UNKNOWN"
                data_node.data_type = Acdfg.DataNode.DATA_VAR
            elif node.node_type == Groum.METHOD_TYPE:
                method_node = acdfg.method_node.add()
                method_node.id = int(node.node_id)

                # No assignee
                # method_node.assignee =
                # TODO: assignee should be the data node associated to node.object_name
                # method_node.invokee  = 2;

                # CHECK: check if the final name matches what we have in graphiso
                # Most likely not
                method_node.name = "%s.%s" % (node.class_name, node.method_name)
                # Arguments should be data nodes - we don't have them
                # method_node.arguments

            elif node.node_type == Groum.CONTROL_TYPE:
                misc_node = acdfg.misc_node.add()
                misc_node.id = int(node.node_id)
            elif node.node_type == Groum.OTHER_TYPE:
                assert False
            else:
                assert(False)

        # TODO Create the control/use edges

        return acdfg
