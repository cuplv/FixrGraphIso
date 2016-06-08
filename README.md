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


