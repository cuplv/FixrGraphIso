#! /usr/bin/python3
from __future__ import print_function
import re
import sys
import os
import os.path
import getopt
class MinedPattern:
    def __init__ (self, clusterID, image, listOfMethods, patternID, patternType, patternFrequency, acdfg2java_map=None):
        self.clusterID = clusterID
        self.imageName = image
        self.listOfMethods = listOfMethods
        self.isPopular = patternType
        self.patternID = patternID
        self.acdfg2java_map = acdfg2java_map
        self.patternFrequency = patternFrequency

    def printToHTML(self, f):
        if self.isPopular == 1:
            patternType = 'Popular'
        elif self.isPopular == 2:
            patternType = 'Anomalous'
        else:
            patternType = 'Isolated'


        methods_str = ""
        for m in self.listOfMethods:
            m_to_print = m.replace('<','((')
            m_to_print = m.replace('>','))')
            m_to_print = m_to_print

            htmlfile = os.path.join("/home/sergio/works/projects/muse/repos/FixrGraph_experiments/dataset/provenance", m.replace(os.path.basename(m), ""), os.path.basename(m))

            try:
                javafile = self.acdfg2java_map[m]
                methods_str = methods_str + """
                <b><a href="%s">[Java]</a> <a href="%s">[html]</a> %s</b></br>
                """ % (javafile, htmlfile, m_to_print)
            except:
                methods_str = methods_str + """
                <a href="%s">[html]</a> %s </b></br>
                """ % (htmlfile, m_to_print)

        # methods_str = ', '.join(listOfMethods3)

        s1 = """ <h2> %s Pattern %d (Frequency %d) </h2> """%(patternType, self.patternID, self.patternFrequency)
        s2 = """ <img src=\"%s\" alt=\"DOT Image\" style=\"width:100%%;border:2px solid black;\"> """%(self.imageName)
        s3 = """ <p><p><div style=\"width: 800px; height: 300px; background-color:lightblue; overflow-y: scroll;\" class = \"listNameBlock\"> <span id =\"cluster_%d_pattern_%d\">
                  %s
              </span> </div>"""%(self.clusterID, self.patternID, methods_str)
        print(s1, file = f)
        print(s2, file = f)
        print(s3, file = f)

class GenerateIndexPage:
    def __init__(self, outputRootDirectory, htmlOutputDirectory, acdfg2java_map_file=None, sourceCodePath=None):
        self.htmlOutputDir = htmlOutputDirectory
        self.outputRootDir = outputRootDirectory
        self.clusterInfo = {}
        self.minedPatternsByClusterID = {}
        self.clusterMethods = {}
        self.clusterPages = {}
        self.runDotLocally = False

        self.acdfg2java_map = {}
        if (acdfg2java_map_file is not None and sourceCodePath is not None):
            with open(acdfg2java_map_file, 'r') as f:
                for l in f.readlines():
                    splitted = l.split(" ")
                    acdfg_name = splitted[0][splitted[0].rindex("/")+1:]
                    java_fname = l.split(" ")[1]
                    self.acdfg2java_map[acdfg_name] = os.path.join(sourceCodePath,java_fname)


    def makeIndexFile(self):
        f = open('%s/index.html'%(self.htmlOutputDir), 'w')
        print('<html><body> <h1> Cluster Index Page </h1>\n<ul>\n', file = f)
        for (cID,mList) in self.clusterMethods.items():
            if (cID in self.clusterPages):
                cluster_pg = self.clusterPages[cID]
                listOfMethods1 = [s.replace('<','\<')  for s in mList]
                listOfMethods2 = [s.replace('>','\>') for s in listOfMethods1]
                listOfMethods3 = ['<b>'+s+'</b>' for s in listOfMethods2]
                methods_str = ', '.join(listOfMethods3)
                s = '<li>  %s <a href=\"%s\"> page </a>'%(methods_str, cluster_pg)
                print(s,file=f)
        print('</ul></body></html>', file = f)
        f.close()

    def printClusterHeader(self, f, clusterID, cluster_methods):
        cluster_method_str = '</b>, <b> '.join(cluster_methods)
        cluster_method_str = '<b>'+ cluster_method_str + '</b>'
        s ="""<html> <body>
        <h1> Cluster %d </h1>
        <div class = \"methodNameBlocks\">
        %s
        </div>
        """%(clusterID, cluster_method_str)
        print(s, file = f)

    def printClusterTrailer(self, f, clusterID):
        s = """ </body> </html> """
        print(s, file = f)



    def generatePageForCluster(self, clusterID):
        fName = '%s/cluster_%d.html'%(self.htmlOutputDir,clusterID)
        if (clusterID in self.minedPatternsByClusterID):
            f = open(fName, 'w')
            try:
                cluster_methods = []
                if (clusterID in self.clusterMethods):
                    cluster_methods = self.clusterMethods[clusterID]
                self.printClusterHeader(f, clusterID, cluster_methods)
                # Print each pattern
                pattern_list = self.minedPatternsByClusterID[clusterID]
                for pat in pattern_list:
                    pat.printToHTML(f)
                self.printClusterTrailer(f, clusterID)
                self.clusterPages[clusterID] = 'cluster_%d.html'%(clusterID)
                f.close()
            except IOError:
                print('Could not open file for writing %s'%(fName), file=sys.stderr)
                raise

    def loadMethodsFile(self, clusterID):
        fName = '%s/cluster_%d/methods_%d.txt'%(self.outputRootDir,clusterID, clusterID)
        try:
            methodNames= []
            f = open(fName, 'r')
            for line in f:
                line = line.strip()
                methodNames.append(line)
                self.clusterMethods[clusterID] = methodNames
            f.close()
        except IOError:
            print('Could not open file %s'%(fName), file=sys.stderr)
            raise


    def registerPattern(self, clusterID, dot_file_name, patternList, patternID, isPopular, patternFrequency):
        #1. Convert the dot_file to a png file
        stem = os.path.splitext(dot_file_name)[0]
        png_file_name='cluster_%d_%s.png'%(clusterID,stem)
        if self.runDotLocally:
            cmd = 'dot -Tpng %s/cluster_%d/%s -o %s/%s'%(self.outputRootDir, clusterID, dot_file_name, self.htmlOutputDir, png_file_name)
            print('Running ', cmd)
            os.system(cmd)
        else:
            tgt_dot_file ='cluster_%d_%s.dot'%(clusterID, stem)
            cmd = 'cp %s/cluster_%d/%s %s/%s'%(self.outputRootDir, clusterID, dot_file_name, self.htmlOutputDir, tgt_dot_file)
            os.system(cmd)
        #2. Add a link to that PNG file
        pat = MinedPattern(clusterID, png_file_name, patternList, patternID,  isPopular, patternFrequency, self.acdfg2java_map)
        if clusterID in self.minedPatternsByClusterID:
            lst = self.minedPatternsByClusterID[clusterID]
            lst.append(pat)
        else:
            self.minedPatternsByClusterID[clusterID] = [pat]


    def parseInfoFile(self, clusterID):
        filename = '%s/cluster_%d/cluster_%d_info.txt'%(self.outputRootDir, clusterID, clusterID)
        self.loadMethodsFile(clusterID)
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
                        # register the previous pattern
                        self.registerPattern(clusterID, dotFileName, patternList,
                                             patternID, patternType, patternFrequency)
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

def help_message():
    print ("gatherResults.py [options]")
    print ("\t -a | --start <starting cluster id> default: 1")
    print ("\t -b | --end <ending cluster id> default: 65")
    print ("\t -d | -o | --output-dir <output dir name> default: html_files")
    print ("\t -i | --input-dir <output dir name> default: new_clusters")


def main(argv):
    start_range = 1
    end_range = 65
    outputRootName = 'new_clusters'
    htmlOutputDir = 'html_files'
    sourceCodePath = None
    try:
        opts, args = getopt.getopt(argv[1:],"a:b:hd:i:o:s:m:",["fixr-path=","nocopy","start=","end=","help","cluster-file-name=","output-dir=", "freq="])
    except getopt.GetoptError:
        help_message()
        sys.exit(2)
    for (o,a) in opts:
        print (o,a)
        if o in ("-a","--start"):
            start_range = int(a)
        if o in ("-b","--end"):
            end_range = int(a)
        if o in ("-o","-d","--output-dir"):
            htmlOutputDir = a
        if o in ("-i","--input-dir"):
            outputRootName = a
        if o in ("-m","--acdfgmap"):
            acdfg_map = a
        if o in ("-s","--source-code-dir"):
            sourceCodePath = a
        if o in ("-h","--help"):
            help_message()
            sys.exit(1)
    g = GenerateIndexPage(outputRootName, htmlOutputDir, acdfg_map, sourceCodePath)
    for id in range(start_range, end_range+1):
        g.parseInfoFile(id)
        g.generatePageForCluster(id)
    g.makeIndexFile()


if __name__ == '__main__':
    main(sys.argv)
