Hyperbolicity
=============

A plug-in based infrastructure for running algorithms to find the delta-hyperbolicity of a graph.


Solutions
=========

Graph - Code infrastructure for graphs and graph algorithms.
DeltaHyperbolicity - Contains 2 projects:
	1.	SDK for creating algorithms.
	2.	Executable that can process graphs with algorithms given as shared-library plug-ins.
Algorithms - Different algorithm implementations using the SDK just mentioned.


Compilation
===========

All project were compiled and tested using MSVC 2012. The code relies on Boost (http://www.boost.org). It was compiled and tested with Boost 1.53, however it *may* work with earlier versions as well.

1. Compile Boost (version 1.53 or higher). Optionally compile a 64-bit version as well.
2. In Boost's root folder, there should be a folder named "boost" with header files, and a folder named "lib". Inside the "lib" folder there should be a "win32" folder with the compiled 32-bit files, and optionally "x64" with the 64-bit files.
3. Define an environment variable %BOOST_ROOT% that points to Boost's root folder.
4. Compile the solutions in the following order:
	4.a. Graph (Graph/build/msvc-110/Graph.sln).
	4.b. DeltaHyperbolicity (DeltaHyperbolicity/build/msvc-110/DeltaHyperbolicity.sln).
	4.c. Algorithms (Algorithms/build/msvc-110/Algorithms.sln).
	
	
Running
=======

After compiling successfully, run DeltaHyperbolicity\bin\<platform>\<configuration>\DeltaHyperbolicity.exe and use the UI to process graphs with algorithms. Note that there is also a command-line interface - type "DeltaHyperbolicity.exe --help" for help.


Writing New Algorithms
======================

Under the DeltaHyperbolicity solution, note the IGraphAlg.h header file. A new algorithm must inherit this interface and implement its pure virtual methods. See interface documentation in file for further details, and see other implemented algorithms in the "Algorithms" solution for examples.