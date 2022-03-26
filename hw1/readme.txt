1. Program: 2-Way F-M Circuit Partitioning

2. Language: c++

3. Compiler: c++11

4. File:
	(1) bin/: Executable file
	(2) src/: Source code including .h and .cpp file
	(3) testcase/: Input testcases
	(4) Makefile: File for building program automatically
	(5) readme.txt: The tutorial of this program
	(6) report.pdf: Report the data structures used in your program and the findings in this programming
	
5. Steps to run program:
	$ make
	$ ./bin/fm <input file name> <output file name>
	(Example: ./bin/fm input_1.dat output_1.dat)

6. Result:
	(1) The output file named "output_#.dat" will be generated in the current dictionary, and the content will be like:
	Cutsize = #1		<- Cut size of the final partition
	G1 #2			<- Total number of cell in partition G1
	c1 c2 ... c#2 ;		<- List all cell name in partition G1
	G2 #3			<- Total number of cell in partition G2
	c1 c2 ... c#3 ;		<- List all cell name in partition G2

	(2) The informatoin printed on the screen:
	==================== Summary ====================
 	Initial Cut Size: xxx					<- Cut size of the initial partition
 	Final Cut Size: xxx					<- Cut size of the final partition
 	Total cell number: xxx					<- Total number of cell
 	Total net number:  xxx					<- Total number of net
 	Cell Number of partition A: xxx				<- Total number of cell in partition A
	Cell Number of partition B: xxx				<- Total number of cell in partition B
	Total num of iteration: xxx				<- Total number of iteration
	=================================================
 