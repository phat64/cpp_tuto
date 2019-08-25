all:
	g++ -std=c++11 01_testsmartpointer.cpp -o 01_testsmartpointer
	g++ -std=c++11 02_testbenchmarkduration.cpp -o 02_testbenchmarkduration
	g++ -std=c++11 03_testexitcallback.cpp -o 03_testexitcallback
	g++ -std=c++11 04_testmemorymanager.cpp -o 04_testmemorymanager
	g++ -std=c++11 05_testlistzip.cpp -o 05_testlistzip
	g++ -std=c++11 06_testimage2ascii.cpp -o 06_testimage2ascii
	gcc 07_testoop.c -o 07_testoop
	g++ 08_testcalculator.cpp -o 08_testcalculator
	g++ 09_testinterpreter.cpp -o 09_testinterpreter
