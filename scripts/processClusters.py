import os
import os.path
import sys
import re
import getopt

class ClusterProcessor:
    def __init__(self):
        pass

    def runForCluster(self, clusterID, fun_names):
        fixr_graph_iso_home = self.fixr_root_directory
        cmd_name = fixr_graph_iso_home+'/build/src/fixrgraphiso/frequentsubgraphs'
        dirName = './%s/cluster_%d'%(self.outputRootName, clusterID)
        os.chdir(dirName)
        num_files = len([name for name in os.listdir('.') if os.path.isfile(name)])
        freq = int(0.25 * num_files)
        pop_cutoff = int(0.15 * num_files)
        fun_list0 = [s.strip() for s in fun_list.split()]
        fun_list1 = ["-m "+s for s in fun_list0]
        fun_string = ''.join(fun_list1)
        cmd = '%s -f %d -g %d -o ../cluster_%d_info.txt %s *.acdfg.bin > ../run%d.out'%(cmd_name, freq, pop_cutoff, clusterID, fun_string, clusterID)
        print ('Running %s'%(cmd))
        os.system(cmd)
        os.chdir('../..')


    def copyFile(self, filename, clusterID):
        filename = filename.strip()
        (path, acdfgname) = os.path.split(filename)
        newfile = '%s/cluster_%d/%s'%(self.outputRootName, clusterID, acdfgname)
        cmd = 'cp %s %s'%(filename, newfile)
        print(cmd)
        os.system(cmd)

    def makeDirectory(self, clusterID):
        os.system('mkdir %s/cluster_%d'%(self.outputRootName,clusterID))

    def processClusterFile(self):
        fname = self.cluster_file_name
        fhandle = open(fname, 'r')
        count = 0
        list_of_clusters=[]
        for line in fhandle:
            m = re.match(r'I:\s*(.*)\(\s*(\d+)\s*\)', line)
            if m:
                count = count + 1
                functionNames = m.group(1)
                print (functionNames)
                self.makeDirectory(count)
                list_of_clusters.append((count, functionNames))
            m = re.match(r'F:\s*([\S\$]*)$', line)
            if m:
                print (m.group(1))
                if (self.run_cluster_copy):
                    self.copyFile(m.group(1), count)
        return list_of_clusters

    def help_message(self):
        print ("processClusters.py [options]")
        print ("\t -a | --start <starting cluster id> default: 1")
        print ("\t -b | --end <ending cluster id> default: 426")
        print ("\t -f | --fixr < fixr root directory> default: %s"%(self.fixr_root_directory))
        print ("\t -n | --nocopy Skip the copying step default: off")
        print ("\t -c | --cluster-file-name <name of the file with cluster results> default: clusters.txt")

    def main(self,argv):
        self.start_range = 1
        self.end_range = 426
        self.run_cluster_copy=True
        self.fixr_root_directory='/Users/macuser/Projects/git/FixrGraphIso'
        self.cluster_file_name = 'clusters.txt'
        self.outputRootName = 'all_clusters'
        try:
            opts, args = getopt.getopt(argv[1:],"a:b:hf:nc:d:",["fixr-path=","nocopy","start=","end=","help","cluster-file-name=","output-dir="])
        except getopt.GetoptError:
            self.help_message()
            sys.exit(2)
        for (o,a) in opts:
            print (o,a)
            if o in ("-a","--start"):
                self.start_range = int(a)
            if o in ("-b","--end"):
                self.end_range = int(a)
            if o in ("-n", "--nocopy"):
                self.run_cluster_copy = False
            if o in ("-f","--fixr-path"):
                self.fixr_root_directory = a
            if o in ("-c","--cluster-file-name"):
                self.cluster_file_name = a
            if o in ("-d","--output-dir"):
                self.rootName = a
            if o in ("-h","--help"):
                self.help_message()

        clusters = self.processClusterFile()
        if (self.end_range > self.start_range):
            for (cid, fun_names) in range(self.start_range, self.end_range):
                self.runForCluster(cid, fun_names)
        else:
            print( 'start range (%d) must be less than end range (%d) '%(self.start_range, self.end_range))


if __name__ == '__main__':
    p = ClusterProcessor()
    p.main(sys.argv)
