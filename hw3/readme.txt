1. Program: Global Placement

2. Language: c++

3. Compiler: c++11

4. Server: 
	IP:port: edaunion.ee.ntu.edu.tw:40056
	Name: edaU6
	Type: HPE ProLiant DL360 Gen10
	CPU: Intel Xeon
	CPU Clock: 2.1 GHz * 32
	Memory: 251 G
	OS: Ubuntu 20.04

5. File:
	(1) bin/: Executable file
	(2) src/: Source code including .h and .cpp file
	(3) lib/: Static library providing additional placement tool
	(3) benchmark/: Input testcases
	(4) Makefile: File for building program automatically
	(5) readme.txt: The tutorial of this program
	(6) report.pdf: Report the data structures used in your program and the findings in this programming
	
6. Steps to run program:

	!!!!!
	Due to some compatible issues, please run the program on below server. Thank you.
	IP:port: edaunion.ee.ntu.edu.tw:40056
	Name: edaU6
	Type: HPE ProLiant DL360 Gen10
	OS: Ubuntu 20.04
	!!!!!

	(1) Put the input file in benchmark/
	(2) Go to the current dictionary (same path with readme.txt)
	(3) $ make clean
	(4) $ make
	(5) Executable file "place" will be generated in the current dictionary
	(6) $ ./place -aux <inputFile.aux>
	(ex: ./place -aux benchmark/ibm01/ibm01-cu85.aux)

7. Result:
	(1) The information showed in the screen

/********************************************************************************************************************
	
	Benchmark: ibm09-cu90							<- input benchmark
	HPWL: 26045716								<- initial HPWL
	Memory usage: 38.8 MB							<- memory usage
	Core region: (-62172,-61936)-(62238,62048)				<- die boundary

	////// Global Placement ///////
	Objective = 2.5974e+07 ||grad|| = 3.28197				<- global placement optimizing HPWL and density
	Objective = 2.37399e+07 ||grad|| = 3.15095
	.
	.
	.
	Objective = 1.59107e+07 ||grad|| = 2.71715

	HPWL: 1424582846 (53.15%)						<- HPWL after legalization

	////// Detail Placement ///////
 	run: 0 HPWL=1270769727 (-10.797%)(-10.797%)   time: 0 sec   all: 0 sec	<- detail placement optimizing HPWL
 	run: 1 HPWL=1197512871 (-5.765%)(-15.939%)   time: 0 sec   all: 0 sec
 	run: 2 HPWL=1153547470 (-3.671%)(-19.026%)   time: 0 sec   all: 1 sec
 	.
	.
	.
 	run:19 HPWL=1025893890 (-0.256%)(-27.986%)   time: 1 sec   all: 17 sec

	HPWL: 1025893890 (-27.99%)						<- detail placement optimizing wirelength


	////////////////////
	Benchmark: ibm09-cu90							<- input benchmark

	Global HPWL: 930169521   Time:  375.0 sec (6.2 min)			<- HPWL after global placement
 	Legal HPWL: 1424582846   Time:   10.0 sec (0.2 min)			<- HPWL after legalization
	Detail HPWL: 1025893890   Time:   23.0 sec (0.4 min)			<- HPWL after detail placement
 	===================================================================
       	HPWL: 1025893890   Time:  408.0 sec (6.8 min)				<- final HPWL

********************************************************************************************************************/