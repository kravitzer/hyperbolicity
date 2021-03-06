Eran Kravitz and Yuval Shavitt.  
January 2014  
https://github.com/kravitzer/hyperbolicity  
kravitzer@gmail.com  

The program is a framework for running plug-in algorithms on graph input files.  
Provided are several algorithms discussed in our paper.  
The program is provided 'as is' with no warranty.  

You may use and distribute it, provided you cite the web page above, and the paper:  

Scalable and Efficient Calculation of Gromov \delta-Hyperbolicity of Complex Networks  
Eran Kravitz and Yuval Shavitt,  
2014.  

Hyperbolicity
=============

A plug-in based infrastructure for running algorithms to find the delta-hyperbolicity of a graph.  


Solutions
=========

Graph - Code infrastructure for graphs and graph algorithms.  
DeltaHyperbolicity - Contains 2 projects:  
	1.	SDK for creating algorithms.  
	2.	Executable that can process graphs with algorithms given as shared-library plug-ins.  
Algorithms - Different algorithm implementations using the SDK mentioned above.  


Compilation
===========

All projects were compiled and tested using MSVC 2012. The code relies on Boost (http://www.boost.org). It was compiled and tested with Boost 1.53, however it *may* work with earlier versions as well.  

1. Compile Boost (version 1.53 or higher). Optionally compile a 64-bit version as well.  
2. Create a root folder for boost.  
2.1. Create a sub-folder named "boost" with Boost's header files.  
2.2. Create a sub-folder named "lib", inside which create a "win32" folder with the compiled 32-bit files, and optionally "x64" with the 64-bit compiled files.  
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
