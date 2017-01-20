import os
import os.path
import re

def runForCluster(clusterID):
    cmd_name = '/home/ubuntu/cuplv/FixrGraphIso/build/src/fixrgraphiso/frequentsubgraphs'
    dirName = './all_clusters/cluster_%d'%(clusterID)
    os.chdir(dirName)
    num_files = len([name for name in os.listdir('.') if os.path.isfile(name)])
    freq = int(0.85 * num_files)
    cmd = '%s -f %d *.acdfg.bin > ../run%d.out'%(cmd_name, freq, clusterID)
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

#clusters = processClusterFile('clusters.txt')
for cid in range(1,426):
    runForCluster(cid)
