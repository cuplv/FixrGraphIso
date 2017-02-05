# FixrGraphIso
Implementation of the approximate graph isomorphism


# Build the project
Create a build folder: `$> mkdir build`

CD to the folder `$> cd build`

Run CMake `$> cmake ../ -DFIXR_GRAPH_EXTRACTOR_DIRECTORY=<path to the FixrGraphExtractor project>`

Note: CMake searches for the protobuf libraries. You can download and build them from https://developers.google.com/protocol-buffers/docs/downloads
You can install protobuf globally on your system (this allow CMake to find the libraries automatically) or you can manually set the variables CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH.

Build the tool: `$> make`

Run the executable: `$> ./src/fixrgraphiso/fixrgraphiso `

# External dependencies
The project assumes the following external dependencies:
- Protobuf (version >= 2.8.12)
- Z3 (release z3-4.4.1)



# Run clustering

TO_DEF:
GRAPH_FOLDER
FREQUENT_ITEMSET_BIN  frequentitemsets 
FREQUENT_SUBGRAPHS_BIN frequentsubgraphs
PROCESS_CLUSTER_SCRIPT ~/cuplv/FixrGraphIso/scripts/processClusters.py

A. Find the acdfg files in the filesToItemSetCluster folder
find <graph folder>s -name "*.acdfg.bin" > filesToItemSetCluster

B. Run frequent item sets
<frequentitemset_bin> -f 40 -m 3 -o clusters.txt filesToItemSetCluster

The above command takes a few minutes to run and dumps the clusters into the cluster.txt file in a format that my scripts understand

C. Create all_clusters file by running (the script below needs work)
mkdir all_clusters
~/cuplv/FixrGraphIso/scripts/processClusters.py <name of the output from previous command> <output directory> <cluser_id>

nclusters=`cat clusters.txt  | grep "I:" | wc -l` && nclusters=$((nclusters+1)); echo $nclusters
python <FIXR_ISO>/scripts/processClusters.py -f <FIXR_ISO> -n -a 1 -b $nclusters

This will create all the clusters and run the command to generate patterns for a particular cluster_id

Outputs:
- clusters.txt
- 
