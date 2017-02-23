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
from groum import Groum

MIN_FREQUENCY = 20
STATS_FILE_NAME = "patterns_stats"

class PatternStats:
    """ keeps the statistics of the patterns
    """
    def __init__(self,
                 cluster_id,
                 pattern_id,
                 size,
                 frequency,
                 method_bag,
                 files,
                 is_anomaly = False,
                 violated_pattern = False,
                 rareness = -1):
        self.cluster_id = cluster_id
        self.pattern_id = pattern_id
        self.size = int(size)
        self.frequency = int(frequency)
        self.method_bag = method_bag
        self.files = files

        self.is_anomaly = is_anomaly
        self.violated_pattern = violated_pattern
        self.rareness = rareness

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

def readFromGroums(fname, cluster_id, is_anomaly=False):
    f = open(fname, 'r')

    patterns = []
    groums = []
    pattern_id = -1
    size = -1
    frequency = -1
    violated_pattern = -1
    rareness = -1
    method_bag = []
    files = []

    read_adjacency_list = False
    begin_pattern = False
    groum = None
    for line in f.readlines():
        line = line.strip()
        if (not line):
            if (is_anomaly and read_adjacency_list):
                read_adjacency_list = False
            continue

        if ((not is_anomaly) and line.startswith("Graph")):
            read_adjacency_list = True
            continue
        if (is_anomaly and line.startswith("Edges:")):
            read_adjacency_list = True
            continue
        if ((not is_anomaly) and line.startswith("/* Begin a pattern */")):
            begin_pattern = True
            continue
        if (is_anomaly and line.startswith("Anomalies in pattern ")):
            m = re.match(r'Anomalies\sin\spattern\s(\d+)\srareness:\s(\d+.\d+)', line)
            assert m
            pattern_id = m.group(1)
            rareness = float(m.group(2))
            begin_pattern = True
            groum = Groum(pattern_id, frequency)
            continue

        if not begin_pattern:
            continue

        if (not is_anomaly):
            m = re.match(r'/\* End a pattern \*/', line)
        else:
            m = re.match(r'According to pattern (\d+)', line)
        if m:
            if is_anomaly:
                violated_pattern = m.group(1)

            if (is_anomaly or int(frequency) >= MIN_FREQUENCY):
                pattern = PatternStats(cluster_id,
                                       pattern_id,
                                       size,
                                       frequency,
                                       method_bag,
                                       files,
                                       is_anomaly,
                                       violated_pattern,
                                       rareness)
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


        m = re.match(r'File:\s*(.*)', line)
        if m:
            fpath = m.group(1)
            files.append(fpath)

            assert None != groum
            groum.add_file(fpath)

            continue

        if (not is_anomaly):
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

            if read_adjacency_list:
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
        else:
            assert is_anomaly
            m = re.match(r'(\d+)\sNode:\s.*', line)
            if m:
                size = int(m.group(1))
                continue

            # TODO FIX
            # Node: 333430 - Label: PSIActivity.this.findViewById - 102	Lines: 186-->186
            m = re.match(r'Node:\s(\d+)\s-\sLabel:\s([a-zA-z.$_<>]+)\s-\s(\d+)\s.*', line)
            if (m):
                node_id = m.group(1)
                node_label = m.group(2)

                # non correct - no other info now
                node_type = Groum.METHOD_TYPE
                class_name = ".".join(node_label.split(".")[:-2])
                object_name = ".".join(node_label.split(".")[-2])
                method_name = ".".join(node_label.split(".")[-1])

                # hack for init
                # OnItemClickListener.OnItemClickListener.new
                # android.widget.ListView.lv.setOnItemClickListener
                if (method_name == "new"):
                    method_name = object_name
                    object_name = ""

                start_line = -1
                end_line = -1

                node = Groum.GroumNode(node_id, node_type, class_name, object_name,
                                       method_name, start_line, end_line)
                assert groum is not None
                groum.add_node(node)

                method_name = "%s.%s" % (class_name, method_name)
                method_bag.append(method_name)
                continue

            if read_adjacency_list:
                assert groum is not None
                node_list = line.split(" ")
                assert(len(node_list) > 0)

                for elem in node_list:
                    m = re.match(r'(\d+)<--(\d+)', elem)
                    assert m
                    src_node_id = m.group(1)
                    dst_node_id = m.group(2)

                    if (groum.has_node(dst_node_id) and
                        groum.has_node(src_node_id)):
                        groum.add_edge(src_node_id, dst_node_id)
                read_adjacency_list = False


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

            acdfg_file = os.path.join(base_path, "%s_%d.acdfg.bin" % (prefix, bin_id))
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
               "anom", "Anomalous Bin", groums_anomalies)

    cluster_file.close()
    map_file.close()

def read_data(stats, key, prefix, line):
    if (line.startswith(prefix)):
        new_prefix = prefix.replace("(","\(")
        new_prefix = new_prefix.replace(")","\)")

        m = re.match(r'%s\s(\d+)' % new_prefix, line)
        if m:
            stats[key] = int(m.group(1))
            return True
        else:
            return False
    else:
        return False

def read_graphiso_times(in_file):
    stats = {}
    for line in in_file:
        line = line.strip()
        if line is None: continue

        if read_data(stats, "time", "Total Time (s):", line): continue
        if read_data(stats, "graphs", "# Graphs :", line): continue
        if read_data(stats, "avg_nodes", "Average # of nodes:", line): continue
        if read_data(stats, "avg_edges", "Average # of edges:", line): continue
        if read_data(stats, "max_nodes", "Max. # of nodes :", line): continue
        if read_data(stats, "max_edges", "Max. # of edges :", line): continue
        if read_data(stats, "subs", "# Subsumption checks:", line): continue
        if read_data(stats, "sat", "# SAT calls:", line): continue
        if read_data(stats, "sat_time", "# satSolverTime (ms):", line): continue

    return stats

def get_stat_val(stats, key):
    if key in stats:
        return stats[key]
    else:
        return "NA"

def write_times_graph(all_stats, out_file):
    out_file.write("cluster_id time graphs avg_nodes avg_edges max_nodes max_edges subs sat sat_time\n")

    for (cluster_id, stats) in all_stats.iteritems():
        out_file.write("%d " % cluster_id +
                       "%d " % get_stat_val(stats,"time") +
                       "%d " % get_stat_val(stats,"graphs") +
                       "%d " % get_stat_val(stats,"avg_nodes") +
                       "%d " % get_stat_val(stats,"avg_edges") +
                       "%d " % get_stat_val(stats,"max_nodes") +
                       "%d " % get_stat_val(stats,"max_edges") +
                       "%d " % get_stat_val(stats,"subs") +
                       "%d " % get_stat_val(stats,"sat") +
                       "%d\n" % get_stat_val(stats,"sat_time"))

def read_groum_times(in_file):
    stats = {}
    for line in in_file:
        line = line.strip()
        if line is None: continue

        if read_data(stats, "mining_time", "Finish mining", line): continue
        if read_data(stats, "filtering_time", "Finish filtering", line): continue
        if read_data(stats, "files", "Number of files:", line): continue
        if read_data(stats, "methods", "Number of methods:", line): continue
        if read_data(stats, "groums", "Number of groums:", line): continue
        if read_data(stats, "max_nodes", "Maximum groum size:", line): continue
        if read_data(stats, "avg_nodes", "Average groum size:", line): continue
        if read_data(stats, "tot_patterns", "Total number of patterns:", line): continue
        if read_data(stats, "max_patterns_nodes", "Maximum pattern size:", line): continue
        if read_data(stats, "max_patterns_nodes", "Average pattern size:", line): continue
        if read_data(stats, "time", "Running time", line): continue

    return stats

def write_times_groum(all_stats, out_file):
    out_file.write("cluster_id time mining_time filtering_time files methods groums max_nodes avg_nodes tot_patterns max_patterns_nodes max_patterns_nodes\n")
    for (cluster_id, stats) in all_stats.iteritems():
        out_file.write("%d " % cluster_id +
                       "%d " % get_stat_val(stats,"time") +
                       "%d " % get_stat_val(stats,"mining_time") +
                       "%d " % get_stat_val(stats,"filtering_time") +
                       "%d " % get_stat_val(stats,"files") +
                       "%d " % get_stat_val(stats,"methods") +
                       "%d " % get_stat_val(stats,"groums") +
                       "%d " % get_stat_val(stats,"max_nodes") +
                       "%d " % get_stat_val(stats,"avg_nodes") +
                       "%d " % get_stat_val(stats,"tot_patterns") +
                       "%d " % get_stat_val(stats,"max_patterns_nodes") +
                       "%d\n" % get_stat_val(stats,"max_patterns_nodes"))


def count_stats_old(m1, m2, filter_fun):
    """ Old function that also compares containment using the names of the methods in the graph.
    Not currently used.
    """
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


def write_stats(pattern_stats_list, out_file):
    """ Write the statistics collected for the patterns and anomalies on file.

    """
    def _write_single_stat(pattern_stats, out_file):
        out_file.write(("%d %s %d %d " %
                        (pattern_stats.cluster_id,
                         pattern_stats.pattern_id,
                         pattern_stats.size,
                         pattern_stats.frequency)) +
                       ("%d %d %d %s " %
                        (len(pattern_stats.method_bag),
                         len(pattern_stats.files),
                         pattern_stats.is_anomaly,
                         pattern_stats.violated_pattern)) +
                       "%f\n" % pattern_stats.rareness)

    # write header
    out_file.write("cluster_id pattern_id nodes frequency " +
                   "meth_bag_size files_in_patterns is_anmoaly violated_pattern " +
                   "rareness\n")

    for pattern_stat in pattern_stats_list:
        _write_single_stat(pattern_stat, out_file)


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

    groum_time_stats = {}
    graphiso_time_stats = {}

    print "IGNORING GROUMS WITH FREQUENCY < %d..." % MIN_FREQUENCY

    # Gather the patterns and the statistics
    for id in range(1,195):
        # read the graphiso pattern
        graphiso_patterns[id] = read_graphiso_patterns(os.path.join(graph_iso_folder, "all_clusters"), id)
        graphiso_base_path = os.path.join(graph_iso_folder, "all_clusters", "cluster_%d"%id)
        # read the execution statistics
        graphiso_cluster_info = os.path.join(graphiso_base_path, "cluster_%d_info.txt" % id)
        with open(graphiso_cluster_info, 'r') as in_file:
            graphiso_time_stats[id] = read_graphiso_times(in_file)
        # Writes the pattern statistics
        with open(os.path.join(graphiso_base_path, "%s_%d.txt" % (STATS_FILE_NAME, id)), 'w') as out_file:
            write_stats(graphiso_patterns[id], out_file)
            out_file.close()


        # read the grouminer patterns
        groum_base_path = os.path.join(groum_folder, "all_clusters", "cluster_%d"%id)
        groum_f_name = os.path.join(groum_base_path, "patterns4graph.txt")
        groum_anomalies = os.path.join(groum_base_path, "rankedAnomalies_6_-1.txt")
        if (os.path.exists(groum_f_name)):
            # read the patterns
            (patterns_stats, groums) = readFromGroums(groum_f_name, id)
            groum_patterns[id]  = patterns_stats
            # writes the patterns
            with open(os.path.join(groum_base_path, "%s_%d.txt" % (STATS_FILE_NAME, id)), 'w') as out_file:
                write_stats(patterns_stats, out_file)
                out_file.close()

            # read the anomalies
            anomalies = []
            if (os.path.exists(groum_anomalies)):
                (anomalies_stats, anomalies) = readFromGroums(groum_anomalies, id, True)
            # Write the dot and acdfg files on disk
            write_cluster_info(groum_base_path, id, groums, anomalies)

            # read the execution statistics
            groum_cluster_info = os.path.join(groum_base_path, "log_6_-1.txt")
            with open(groum_cluster_info, 'r') as in_file:
                groum_time_stats[id] = read_groum_times(in_file)

        else:
            print "Missing file %s" % groum_f_name
            continue

    # Print the overall statistics for the cluster
    with open(os.path.join(groum_folder, "all_clusters", "%s.txt" % (STATS_FILE_NAME)), 'w') as out_file:
        all_patterns = []
        for value in groum_patterns.values(): all_patterns.extend(value)
        write_stats(all_patterns, out_file)
        out_file.close()

    # Print pattern stats for graphiso
    with open(os.path.join(graph_iso_folder, "all_clusters", "%s.txt" % (STATS_FILE_NAME)), 'w') as out_file:
        all_patterns = []
        for value in graphiso_patterns.values(): all_patterns.extend(value)
        write_stats(all_patterns, out_file)
        out_file.close()

    # Print times for groum
    with open(os.path.join(groum_folder, "all_clusters", "times.txt"), 'w') as out_file:
        write_times_groum(groum_time_stats, out_file)
        out_file.close()


    # Print times for graph iso
    with open(os.path.join(graph_iso_folder, "all_clusters", "times.txt"), 'w') as out_file:
        write_times_graph(graphiso_time_stats, out_file)
        out_file.close()



if __name__ == '__main__':
    main(sys.argv)
