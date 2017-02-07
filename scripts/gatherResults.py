#! /usr/bin/python3
from __future__ import print_function
import re
import sys
import os
import os.path

class MinedPattern:
    def __init__ (self, clusterID, image, listOfMethods, patternID, isPopular):
        self.clusterID = clusterID
        self.imageName = image
        self.listOfMethods = listOfMethods
        self.isPopular = isPopular
        self.patternID = patternID

    def printToHTML(self, f):
        if self.isPopular:
            patternType = 'Popular'
        else:
            patternType = 'Anomalous'
        listOfMethods1 = [s.replace('<','\<')  for s in listOfMethods]
        listOfMethods2 = [s.replace('>','\>') for s in listOfMethods1]
        listOfMethods3 = ['<h3>'+s+'</h3>' for s in listOfMethods2]
        methods_str = '<br>'.join(self.listOfMethods3)

        s1 = """ <h2> %s Pattern %d </h2> """%(patternType, self.patternID)
        s2 = """ <img src=\"%s\" alt=\"DOT Image\" style=\"width:100%%;border:2px solid black;\"> """%(self.imageName)
        s3 = """ <p><p><div style=\"width: 800px; height: 300px; background-color:lightblue; overflow-y: scroll;\" class = \"listNameBlock\"> <span id =\"cluster_%d_pattern_%d\">
                  %s
              </span> </div>"""%(self.clusterID, self.patternID, methods_str)
        print(s1, file = f)
        print(s2, file = f)
        print(s3, file = f)

class GenerateIndexPage:
    def __init__(self, outputRootDirectory, htmlOutputDirectory):
        self.htmlOutputDir = htmlOutputDirectory
        self.outputRootDir = outputRootDirectory
        self.clusterInfo = {}
        self.minedPatternsByClusterID = {}
        self.clusterMethods = {}
        self.clusterPages = {}
        self.runDotLocally = False

    def makeIndexFile(self):
        f = open('%s/index.html'%(self.htmlOutputDir), 'w')
        print('<html><body> <h1> Cluster Index Page </h1>\n<ul>\n', file = f)
        for (cID,mList) in clusterMethods.items():
            assert(cID in clusterPages)
            cluster_pg = clusterPages[cID]
            listOfMethods1 = [s.replace('<','\<')  for s in listOfMethods]
            listOfMethods2 = [s.replace('>','\>') for s in listOfMethods1]
            listOfMethods3 = ['<h3>'+s+'</h3>' for s in listOfMethods2]
            methods_str = ', '.join(self.listOfMethods3)
            s = '<li>  %s <a href=\"%s\"> page </a>'%(methods_str, cluster_pg)
            print(s,file=f)
        print('</ul></body></html>')
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
                self.clusterPages[clusterID] = fName
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


    def registerPattern(self, clusterID, dot_file_name, patternList, patternID, isPopular):
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
        pat = MinedPattern(clusterID, png_file_name, patternList, patternID,  isPopular)
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
            popularPattern = True
            patternID = -1
            patternFrequency = -1
            dotFilename = None
            for line in f:
                line = line.strip()
                m = re.match(r'Popular\s*Bins:', line)
                if m:
                    popularPattern = True
                    continue

                m = re.match(r'Anomalous\s*Bins:',line)
                if m:
                    if (patternID >= 0):
                        # register the previous pattern
                        self.registerPattern(clusterID, dotFileName, patternList,
                                             patternID, popularPattern)
                        patternList=[]
                        dotFileName = None
                        patternID = -1
                    popularPattern = False
                    continue

                m = re.match(r'Bin\s*#\s*(\d+)', line)
                if m:
                    if (patternID >= 0):
                        # register the previous pattern
                        self.registerPattern(clusterID, dotFileName, patternList,
                                             patternID, popularPattern)
                        patternList=[]
                        dotFileName = None
                    patternID = int(m.group(1))
                    continue
                m = re.match(r'Dot:\s*(.*.dot)\s*Frequency\s*=\s*(\d+)', line)
                if m:
                    dotFileName = m.group(1)
                    patternFrequency = m.group(2)
                    continue
                # Otherwise it is a filename
                patternList.append(line)

            f.close()

        except IOError:
            print('Error: could not open file --', filename)

def main(argv):
    start_range = 1
    end_range = 65
    outputRootName = 'new_clusters'
    htmlOutputDir = 'html_files'
    g = GenerateIndexPage(outputRootName, htmlOutputDir)
    for id in range(start_range, end_range+1):
        g.parseInfoFile(id)
        g.generatePageForCluster(id)
    g.makeIndexFile()


if __name__ == '__main__':
    main(sys.argv)
