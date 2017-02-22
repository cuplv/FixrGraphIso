# The script does two things:
#  - Process the GROUM patterns and anomalies.
#    - It creates a dot file for the pattern/anomaly
#    - It creates a ACDFG for each pattern/anomaly
#    - It creates the cluster_n_info.txt for the pattenrs and anomalies
#
#  - Collects and prints the statistics for the patterns and the anomalies.
#

import os
import re
import sys

from gatherResults import GenerateIndexPage
from fixrgraph.annotator.protobuf.proto_acdfg_pb2 import Acdfg
from groum import Groum

MIN_FREQUENCY = 20

class PatternStats:
    """ keeps the statistics of the patterns
    """
    def __init__(self,
                 cluster_id,
                 pattern_id,
                 size,
                 frequency,
                 method_bag,
                 files):
        self.cluster_id = cluster_id
        self.pattern_id = pattern_id
        self.size = int(size)
        self.frequency = int(frequency)
        self.method_bag = method_bag
        self.files = files

        # fully qualified name package.Class.methodName (list of method names)
        self.methods_bag = method_bag
        self.methods_bag.sort()

    def get_containment_relation(self, otherPattern):
        """ Returns:
        -1: self is contained in other
         0: self eq other
         1: self contains in other
         2: neither one of the above
        """
        eq_elements = 0
        el_self = 0
        el_other = 0

        i = 0
        j = 0
        while i < len(self.methods_bag) or j < len(otherPattern.methods_bag):
            inc_self = False
            inc_other = False

            if i < len(self.methods_bag) and j == len(otherPattern.methods_bag):
                inc_self = True
            elif i == len(self.methods_bag) and j < len(otherPattern.methods_bag):
                inc_other = True
            else:
                if self.methods_bag[i] < otherPattern.methods_bag[j]:
                    inc_self = True
                elif self.methods_bag[i] > otherPattern.methods_bag[j]:
                    inc_other = True
                else:
                    inc_self = True
                    inc_other = True

            if (inc_self and inc_other):
                eq_elements = eq_elements + 1
                i = i + 1
                j = j + 1
            if (inc_self and not inc_other):
                el_self = el_self + 1
                i = i + 1
            elif (inc_other and not inc_self):
                el_other = el_other + 1
                j = j + 1

        if el_self == 0 and el_other == 0:
            return "EQUAL" # eq
        elif el_other == 0:
            return "CONTAINS ANOTHER"
        elif el_self == 0:
            return "CONTAINED IN ANOTHER"
        elif el_self > 0 and el_other > 0:
            return "INCOMPARABLE" # eq

def readFromGroums(fname, cluster_id):
    f = open(fname, 'r')

    patterns = []
    groums = []
    pattern_id = -1
    size = -1
    frequency = -1
    method_bag = []
    files = []

    read_adjacency_list = False
    begin_pattern = False
    groum = None
    for line in f.readlines():
        line = line.strip()
        if (not line): continue

        if (line.startswith("Graph")):
            read_adjacency_list = True
            continue
        if (line.startswith("/* Begin a pattern */")):
            begin_pattern = True
            continue

        if not begin_pattern:
            continue

        m = re.match(r'/\* End a pattern \*/', line)
        if m:
            if (int(frequency) >= MIN_FREQUENCY):
                pattern = PatternStats(cluster_id,
                                       pattern_id,
                                       size,
                                       frequency,
                                       method_bag,
                                       files)
                patterns.append(pattern)
                groums.append(groum)

            pattern_id = -1
            size = -1
            frequency = -1
            method_bag = []
            files = []
            groum = None
            read_adjacency_list = False
            begin_pattern = False
            continue


        m = re.match(r'ID:\s*(\d+)', line)
        if m:
            pattern_id = m.group(1)
            continue

        m = re.match(r'Size:\s*(\d+)\s*$', line)
        if m:
            size = int(m.group(1))
            continue

        m = re.match(r'Frequency:\s*(\d+)\s*$', line)
        if m:
            assert pattern_id > -1
            frequency = int(m.group(1))

            groum = Groum(pattern_id, frequency)

            continue

        m = re.match(r'File:\s*(.*)', line)
        if m:
            fpath = m.group(1)
            files.append(fpath)

            assert None != groum
            groum.add_file(fpath)

            continue

        # if line.startswith("Nodes: "):
        #     line = line.replace("Nodes: ","")
        #     nodes_ids = line.split(" ")
        #     for id in nodes_id:
        #         groum.add_node_id()

        # Example
        # 49603	1	android.view.ViewGroup	mDashboardContainer	addView	32	32 
        m = re.match(r'(\d+)\s+(\d+)\s+([a-zA-z.$_<>]+)\s+([a-zA-z.$_<>]+)\s*([a-zA-z.$_<>]+)\s+(\d+)\s+(\d+)', line)

        if (m):
            node_id = m.group(1)
            node_type = m.group(2)
            class_name = m.group(3)
            object_name = m.group(4)
            method_name = m.group(5)
            start_line = m.group(6)
            end_line = m.group(7)

            node = Groum.GroumNode(node_id, node_type, class_name, object_name,
                                   method_name, start_line, end_line)
            assert groum is not None
            groum.add_node(node)

            method_name = "%s.%s" % (class_name, method_name)
            method_bag.append(method_name)
            continue

        if (read_adjacency_list):
            assert groum is not None
            node_list = line.split(" ")
            assert(len(node_list) > 0)
            m = re.match(r'(\d+)', node_list[0])
            assert m
            src_node_id = m.group(1)

            for i in range(len(node_list) - 1):
                m = re.match(r'(\d+)', node_list[i+1])
                assert m
                dst_node_id = m.group(1)

                if (groum.has_node(dst_node_id) and
                    groum.has_node(src_node_id)):
                    groum.add_edge(src_node_id, dst_node_id)


    return (patterns, groums)


def get_from_dot(dotFileName):
    """ Get the statistics of the patterns/anomalies that we compute
    from the dot representation.
    """
    f = open(dotFileName, "rt")

    size = 0
    method_bag = []

    for line in f:
        line = line.strip()

        if ("shape=box" in line):
            size = size + 1

            m = re.match(r'.*label="(.*)\[(.*)', line)
            if m:
                method_name = m.group(1)
                method_name = method_name.strip()

                if "<init>" in method_name:
                    splitted = method_name.split(".")
                    method_name = ".".join(splitted) + "." + splitted[-2]

                method_bag.append(method_name)


        elif ("shape=ellipse" in line):
            size = size + 1

        m = re.match(r':', line)
        if m:
            continue

    return (size, method_bag)

def parseInfoFile(cluster_folder, clusterID, type=1):
    """ Parse the cluster_n_info.txt file to thet statistics on the
    produced patterns.
    """
    filename = '%s/cluster_%d/cluster_%d_info.txt'%(cluster_folder, clusterID, clusterID)

    patterns = []

    try:
        f = open(filename, 'rt')
        patternList = []
        patternType = 1
        patternID = -1
        patternFrequency = -1
        dotFilename = None
        for line in f:
            line = line.strip()
            m = re.match(r'Popular\s*Bins:', line)
            if m:
                continue

            m = re.match(r'(\w+)\s*Bin\s*#\s*(\d+)', line)
            if m:
                if (patternID >= 0):
                    if (patternType == type):
                        dotfile = '%s/cluster_%d/%s'%(cluster_folder, clusterID, dotFileName)
                        (size, method_bag) = get_from_dot(dotfile)

                        pattern = PatternStats(clusterID, patternID,
                                               size,
                                               patternFrequency,
                                               method_bag,
                                               patternList)
                        patterns.append(pattern)
                    patternList=[]
                    dotFileName = None
                    patternFrequency=None
                patternDescr = m.group(1)
                if (patternDescr == 'Popular'):
                    patternType = 1
                elif (patternDescr == 'Anomalous'):
                    patternType = 2
                else:
                    patternType = 3
                patternID = int(m.group(2))
                continue
            m = re.match(r'Dot:\s*(.*.dot)', line)
            if m:
                dotFileName = m.group(1)
                continue
            # Otherwise it is a filename
            m = re.match(r'Frequency\s*:\s*(\d+),\s*(\d+)', line)
            if m:
                patternFrequency = int(m.group(1)) + int(m.group(2))
                continue
            m = re.match(r'Frequency\s*:\s*(\d+)(.*)$',line)
            if m:
                patternFrequency = int(m.group(1))
                patternList.append(m.group(2))
                continue
            patternList.append(line)

        f.close()

    except IOError:
        print('Error: could not open file --', filename)

    return patterns

def read_graphiso_patterns(folder, id):
    """ Read the statistics for the graphiso patterns and anomalies
    """
    patterns = parseInfoFile(folder, id)
    return patterns


def write_cluster_info(base_path,
                       cluster_id,
                       groums_patterns,
                       groums_anomalies):
    """ Writes the cluster info file from groum
    Given the base path where cluster_<cluster_id>_info.txt must be created,
    the id of the cluster, a list of groums patters, a list of groums anomalies,
    writes the correspondent cluster file.
    """

    def write_list(base_path,
                   cluster_file,
                   map_file,
                   prefix,
                   description,
                   groums):
        # writes the popular groums
        bin_id = 0
        for groum in groums:
            bin_id = bin_id + 1
            map_file.write("%d %s\n" % (bin_id, groum.pattern_id))
            cluster_file.write("%s # %d\n" % (description, bin_id))
            cluster_file.write("Dot: %s_%d.dot\n" % (prefix, bin_id))
            cluster_file.write("Frequency: %d, %d\n" % (groum.frequency, len(groum.files)))
            for f in groum.files:
                cluster_file.write("%s\n" % f)

            dot_file_name = os.path.join(base_path, "%s_%d.dot" % (prefix,bin_id))
            with open(dot_file_name, 'wt') as dot_file:
                groum.to_dot(dot_file)
                dot_file.close()

            acdfg_file = "%s_%d.acdfg.bin" % (prefix, bin_id)
            acdfg = groum.to_acdfg(acdfg_file)
            f = open(acdfg_file, "wb")
            f.write(acdfg.SerializeToString())
            f.close()

    cluster_info_file_path = os.path.join(base_path, "cluster_%d_info.txt" % cluster_id)
    cluster_file = open(cluster_info_file_path, "wt")
    map_file = open(os.path.join(base_path, "patterns_ids_to_groum_id_%d.txt" % cluster_id), "wt")

    cluster_file.write("Popualar Bins:\n")
    map_file.write("Popualar Bins:\n")
    write_list(base_path,cluster_file, map_file, "pop",
               "Popualar Bins", groums_patterns)

    map_file.write("Anomalous Bin:\n")
    write_list(base_path, cluster_file, map_file,
               "anom", "Anomalous Bin", groums_patterns)

    cluster_file.close()
    map_file.close()




def count_stats(m1, m2):
    res = {}
    tot = 0

    mins = None
    maxs = None

    for (g1,pglist) in m1.iteritems():
        for pg  in pglist:
            if mins is None:
                mins = pg.size
            mins = min(mins,pg.size)
            if maxs is None:
                maxs = pg.size
            maxs = max(maxs,pg.size)

            tot = tot + 1

            for (g2,pilist) in m2.iteritems():
                for pi in pilist:
                    rel = pg.get_containment_relation(pi)
                    if rel not in res:
                        res[rel] = set()
                    res[rel].add(pg)

    return (res, tot, mins, maxs, 0)

def count_stats(m1, m2, filter_fun):
    res = {}
    tot = 0

    mins = None
    maxs = None

    for (g1,pglist) in m1.iteritems():
        for pg  in pglist:
            if not filter_fun(pg): continue
            if mins is None:
                mins = pg.size
            mins = min(mins,pg.size)
            if maxs is None:
                maxs = pg.size
            maxs = max(maxs,pg.size)
            tot = tot + 1

            # local_res["EQUAL"] = 0
            # local_res["CONTAINS ANOTHER"] = 0
            # local_res["CONTAINED IN ANOTHER"] = 0
            # local_res["INCOMPARABLE"] = 0

            for (g2,pilist) in m2.iteritems():
                for pi in pilist:
                    if not filter_fun(pi): continue
                    rel = pg.get_containment_relation(pi)
                    if rel not in res:
                        res[rel] = 0
                    res[rel] = res[rel] + 1

    return (res, tot, mins, maxs, 0)


def print_hist(patternmap, fname):
    fout = open(fname, "w")

    fout.write("size frequency clusterid patternid\n")
    for k,l in patternmap.iteritems():
        for pattern in l:
            assert pattern.frequency >= 20
            fout.write("%s %s %s %s\n" % (str(pattern.size), str(pattern.frequency), str(pattern.cluster_id), str(pattern.pattern_id)))
    fout.close()

def filter_none(pattern):
    return True

def filter_by_meth(pattern, method_bag):
    for a in method_bag:
        if a not in pattern.method_bag:
            return False
    return True


def filter_1_sql(pattern):
    methods = ["android.database.sqlite.SQLiteDatabase.beginTransaction",
               "android.database.sqlite.SQLiteDatabase.endTransaction",
               "android.database.sqlite.SQLiteDatabase.setTransactionSuccessful"]

    return filter_by_meth(pattern, methods)


def main(argv):

    # hardcode everything
    graph_iso_folder="/home/sergio/works/projects/muse/repos/FixrGraph_experiments/msr_results/graphiso"
    groum_folder="/home/sergio/works/projects/muse/repos/FixrGraph_experiments/msr_results/grouminer"
    groum_folder="/home/sergio/works/projects/muse/repos/FixrGraph_experiments/GROUM_test"

    "Map from cluster id to pattern list"
    graphiso_patterns = {}
    groum_patterns = {}
    groums = {}


    print "IGNORING GROUMS WITH FREQUENCY < %d..." % MIN_FREQUENCY

    # Gather the patterns
    for id in range(1,195):
        # read the graphiso pattern
        graphiso_patterns[id] = read_graphiso_patterns(os.path.join(graph_iso_folder, "all_clusters"), id)

        # read the grouminer patterns
        groum_base_path = os.path.join(groum_folder, "all_clusters", "cluster_%d"%id)
        groum_f_name = os.path.join(groum_base_path, "patterns4graph.txt")
        if (os.path.exists(groum_f_name)):
            (patterns_stats, groums) = readFromGroums(groum_f_name, id)
            groum_patterns[id]  = patterns_stats

            # Write the dot and acdfg files on disk
            write_cluster_info(groum_base_path, id, groums, [])
        else:
            print "Missing file %s" % groum_f_name
            continue


    # filters = [("", filter_none)]
    # #filters = [("01_", filter_1_sql)]
    # for (name,filt) in filters:
    #     print "Filter " + name
    #     # Compare the patterns
    #     (res, tot, mins, maxs, avg) = count_stats(groum_patterns, graphiso_patterns, filt)
    #     print "GROUM vs ISO"
    #     for (k,v) in res.iteritems():
    #         print ("%s = %s " % (k ,v))
    #     print "Tot " + str(tot)
    #     print "Min size " + str(mins)
    #     print "Max size " + str(maxs)

    #     (res, tot, mins, maxs, avg) = count_stats(graphiso_patterns, groum_patterns, filt)
    #     print "ISO vs GROUM"
    #     for (k,v) in res.iteritems():
    #         print ("%s = %s " % (k ,v))
    #     print "Tot " + str(tot)
    #     print "Min size " + str(mins)
    #     print "Max size " + str(maxs)

    #     # 2. Print the histogram file
    #     print_hist(graphiso_patterns, "%spattern_iso" % name)
    #     print_hist(groum_patterns, "%spattern_groum" % name)




if __name__ == '__main__':
    main(sys.argv)
