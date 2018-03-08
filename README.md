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

# Executable compiled

- `fixrgraphiso`: computes a subgraph isomorphism relationship between two graphs

- `frequentitemsets`: performs the frequent itemset computation

- `frequentsubgraphs`: computes the set of frequent subgraphs


# External dependencies
The project assumes the following external dependencies:
- Protobuf (version >= 2.8.12) (https://developers.google.com/protocol-buffers/)

- Z3 (release z3-4.4.1) (https://github.com/Z3Prover/z3)

- Optional: Gurobi solver (http://www.gurobi.com) or GLPK solver (https://www.gnu.org/software/glpk/)

The optimization solver is used in the `fixrgraphiso` executable.

