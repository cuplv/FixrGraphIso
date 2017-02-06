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
        freq = self.freq_cutoff
        fun_list0 = [s.strip() for s in fun_names.split(',')]
        fun_list1 = [s.replace('$','\$') for s in fun_list0]
        fun_list2 = [s.replace('<','\<') for s in fun_list1]
        fun_list3 = [s.replace('>','\>') for s in fun_list2]
        method_file = 'methods_%d.txt'%(clusterID)
        fil = open(method_file, 'wt')
        for s in fun_list3:
            print(s, file=fil)
        fil.close()
        cmd = '%s -f %d -o ./cluster_%d_info.txt -m %s *.acdfg.bin >& ./run%d.out'%(cmd_name, freq,  clusterID, method_file, clusterID)
        print ('%s'%(cmd))
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
                file_old = m.group(1)
                file_new = file_old.replace('$','\$')
                if (self.run_cluster_copy):
                    self.copyFile(file_new, count)
        return list_of_clusters

    def help_message(self):
        print ("processClusters.py [options]")
        print ("\t -a | --start <starting cluster id> default: 1")
        print ("\t -b | --end <ending cluster id> default: 426")
        print ("\t -p | --fixr < fixr root directory> default: %s"%(self.fixr_root_directory))
        print ("\t -n | --nocopy Skip the copying step default: off")
        print ("\t -c | --cluster-file-name <name of the file with cluster results> default: clusters.txt")
        print ("\t -d | --output-dir <output dir name> default: all_clusters")
        print ("\t -f | --freq <frequency cutoff > default: 20 ")

    def main(self,argv):
        self.start_range = 1
        self.end_range = 426
        self.run_cluster_copy=True
        self.fixr_root_directory='/Users/macuser/Projects/git/FixrGraphIso'
        self.cluster_file_name = 'clusters.txt'
        self.outputRootName = 'all_clusters'
        self.freq_cutoff  =20
        try:
            opts, args = getopt.getopt(argv[1:],"a:b:hp:f:nc:d:",["fixr-path=","nocopy","start=","end=","help","cluster-file-name=","output-dir=", "freq="])
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
            if o in ("-p","--fixr-path"):
                self.fixr_root_directory = a
            if o in ("-c","--cluster-file-name"):
                self.cluster_file_name = a
            if o in ("-d","--output-dir"):
                self.outputRootName = a
            if o in ("-f","--freq"):
                self.freq_cutoff= int(a)
            if o in ("-h","--help"):
                self.help_message()
                sys.exit(1)

        clusters = self.processClusterFile()
        if (self.end_range > self.start_range):
            for (cid, fun_names) in clusters:
                self.runForCluster(cid, fun_names)
        else:
            print( 'start range (%d) must be less than end range (%d) '%(self.start_range, self.end_range))


if __name__ == '__main__':
    p = ClusterProcessor()
    p.main(sys.argv)
