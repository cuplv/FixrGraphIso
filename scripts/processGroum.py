# Process the patterns obtained

import os
import re
import sys

from gatherResults import GenerateIndexPage

# Collect the statistics of a pattern
class PatternStats:
    def __init__(self,
                 cluster_id,
                 pattern_id,
                 size,
                 frequency,
                 method_bag,
                 files):
        self.cluster_id = cluster_id
        self.pattern_id = pattern_id
        self.size = size
        self.frequency = frequency
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
            return 0 # eq
        elif el_other == 0:
            return 1
        elif el_self == 0:
            return -1
        elif el_self > 0 and el_other > 0:
            return 2 # eq

    @staticmethod
    def readFromGroums(fname, cluster_id):
        f = open(fname, 'r')

        patterns = []
        pattern_id = -1
        size = -1
        frequency = -1
        method_bag = []
        files = []

        for line in f.readlines():
            line = line.strip()
            if (not line): continue

            m = re.match(r'/\* End a pattern \*/', line)
            if m:
                pattern = PatternStats(cluster_id,
                                       pattern_id,
                                       size,
                                       frequency,
                                       method_bag,
                                       files)
                patterns.append(pattern)

                pattern_id = -1
                size = -1
                frequency = -1
                method_bag = []
                files = []
                continue

            m = re.match(r'ID:\s*(\d+)', line)
            if m:
                pattern_id = m.group(1)
                continue

            m = re.match(r'Size:\s*(\d+)', line)
            if m:
                size = m.group(1)
                continue

            m = re.match(r'Frequency:\s*(\d+)', line)
            if m:
                frequency = m.group(1)
                continue

            m = re.match(r'File:\s*(.*)', line)
            if m:
                files.append(m.group(1))
                continue

            m = re.match(r'\s*\d+\s*\d+([^\s]+)\s*([^\s]+)\s*([^\s]+)\s.*', line)
            if m:
                method_name = "%s.%s" % (m.group(1), m.group(3))
                method_bag.append(method_name)
                frequency = m.group(1)
                continue

        return patterns


def get_from_dot(dotFileName):
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
                # TODO FIX INIT
                method_bag.append(method_name)


        elif ("shape=ellipse" in line):
            size = size + 1

        m = re.match(r':', line)
        if m:
            continue

    return (size, method_bag)

def parseInfoFile(cluster_folder, clusterID, type=1):
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
    patterns = parseInfoFile(folder, id)

    return patterns


# TODO:
# - read single dot
# - read get all the dot_i_pat.dot
# - get_all the patterns4graphs
#
# - DUMP:
# -  - histogram of pattern sizes
# -  - numbers for subsumptions
#

def main(argv):

    # hardcode everything
    graph_iso_folder="/home/sergio/works/projects/muse/repos/FixrGraph_experiments/msr_results/graphiso"
    groum_folder="/home/sergio/works/projects/muse/repos/FixrGraph_experiments/msr_results/grouminer"

    "Map from cluster id to pattern list"
    graphiso_patterns = {}
    groum_patterns = {}

    # Gather the patterns
    for id in range(1, 2):

        # read the graphiso pattern
        graphiso_patterns[id] = read_graphiso_patterns(os.path.join(graph_iso_folder, "all_clusters"), id)

        # read the grouminer data
        groum_f_name = os.path.join(groum_folder, "all_clusters", "cluster_%d"%id, "patterns4graph.txt")
        if (os.path.exists(groum_f_name)):
            groum_patterns[id]  = PatternStats.readFromGroums(groum_f_name, id)
        else:
            print "Missing file %s" % groum_f_name

    # 1. Compare the patterns
    print groum_patterns
    print graphiso_patterns

    # 2. Print the histogram file



if __name__ == '__main__':
    main(sys.argv)
