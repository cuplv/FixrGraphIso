import os
import os.path
import sys
import re
import getopt

def runForCluster(fixr_graph_iso_home, clusterID):
    cmd_name = fixr_graph_iso_home+'/build/src/fixrgraphiso/frequentsubgraphs'
    dirName = './all_clusters/cluster_%d'%(clusterID)
    os.chdir(dirName)
    num_files = len([name for name in os.listdir('.') if os.path.isfile(name)])
    freq = int(0.5 * num_files)
    cmd = '%s -f %d -g %d -o ../cluster_%d_info.txt *.acdfg.bin > ../run%d.out'%(cmd_name, freq, freq, clusterID, clusterID)
    print ('Running %s'%(cmd))
    os.system(cmd)
    os.chdir('../..')


def copyFile(filename, clusterID):
    filename = filename.strip()
    (path, acdfgname) = os.path.split(filename)
    newfile = 'all_clusters/cluster_%d/%s'%(clusterID, acdfgname)
    cmd = 'cp %s %s'%(filename, newfile)
    print(cmd)
    os.system(cmd)

def makeDirectory(clusterID):
    os.system('mkdir all_clusters/cluster_%d'%(clusterID))

def processClusterFile(fname):
    fhandle = open(fname, 'r')
    count = 0
    list_of_clusters=[]
    for line in fhandle:
        m = re.match(r'I:\s*(.*)\(\s*(\d+)\s*\)', line)
        if m:
            count = count + 1
            print (m.group(1))
            makeDirectory(count)
            list_of_clusters.append(count)
        m = re.match(r'F:\s*(\S*)$', line)
        if m:
            print (m.group(1))
            copyFile(m.group(1), count)
    return list_of_clusters


def main(argv):
    try:
        opts, args = getopt.getopt(argv,"a:b:hf:",["fixr=","nocopy","start=","end=","help"])
    except getopt.GetoptError:
        print 'test.py -i <inputfile> -o <outputfile>'
        sys.exit(2)
    start_range = 1
    end_range = 426
    run_cluster_copy=True
    fixr_root_directory='/Users/macuser/Projects/git/FixrGraphIso'
    for (o,a) in opts:
        if o in ("-a","--start"):
            start_range = int(a)
        if o in ("-b","--end"):
            end_range = int(b)
        if o in ("-n", "--nocopy"):
            run_cluster_copy = False
        if o in ("-f","--fixr"):
            fixr_root_directory = a
        if o in ("-h","--help"):
            print ("processClusters.py [options]")
            print ("\t -a | --start <starting cluster id> default: 1")
            print ("\t -b | --end <ending cluster id> default: 426")
            print ("\t -f | --fixr < fixr root directory> default: %s"%(fixr_root_directory))
            print ("\t -n | --nocopy Skip the copying step default: off")
            sys.exit(2)
    
    clusters = processClusterFile('clusters.txt')
    if (end_range > start_range):
        for cid in range(start_range, end_range):
            runForCluster(fixr_root_directory, cid)
    else:
        print( 'start range (%d) must be larger than end range (%d) '%(start_range, end_range))
    


main(sys.argv)
