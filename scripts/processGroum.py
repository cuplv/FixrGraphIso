# Process the patterns obtained

# Collect the statistics of a pattern
class PatternStats:
    def __init__(self,
                 cluster_id,
                 pattern_id,
                 size,
                 frequency,
                 method_bag,
                 files):
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

                patterns = []
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
                method_name = "%s.%s" % (m.group(1) + m.group(3))
                method_bag.append(method_name)
                frequency = m.group(1)
                continue

        return pattens
