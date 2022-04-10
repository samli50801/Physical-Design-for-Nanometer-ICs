1. Program: Fixed-outline Floorplanning

2. Language: c++

3. Compiler: c++11

4. File:
	(1) bin/: Executable file
	(2) src/: Source code including .h and .cpp file
	(3) input_pa2/: Input testcases
	(4) Makefile: File for building program automatically
	(5) readme.txt: The tutorial of this program
	(6) report.pdf: Report the data structures used in your program and the findings in this programming
	
5. Steps to run program:

	/* Note: Please make sure the environment has installed GNUplot */

	(1) Put the input file (ex: .block and .nets files) in input_pa2/
	(2) Go to the current dictionary (same path with readme.txt)
	(3) $ make clean
	(4) $ make
	(5) Executable file named "fp" will be generated in the current dictionary
	(6) $ ./fp <α value> <input.block name> <input.net name> <output file name> (ex: ./fp 0.5 ami49.block ami49.nets output.rpt)
	(7) If GNUplot has been installed, however, the program still cannot run successfully, please delete "fp->plot(BEST_FLOORPLAN);" in src/main.cpp:47, and then go back to step4.

6. Result:
	(1) The output file named "output.rpt" will be generated in the current dictionary, and the content is like:
 
		<final cost>				// Cost = α*Area + (1-α)*Wirelength
		<total wirelength>			// Total HPWL summation of nets
		<chip_area>				// area = (chip_width) * (chip_height)
		<chip_width> <chip_height>		//resulting chip width and height
		<program_runtime>			//report the runtime in seconds
		<macro_name> <x1> <y1> <x2> <y2>	// (x1, y1): lower-left corner, (x2, y2): upper-right corner
		<macro_name> <x1> <y1> <x2> <y2> 
		... More macros
 	
	(2) The visualized result "floorplan.png" will be generated in the current dictionary.