c++ -c *.cpp
rm  -f vanderplaats.o
c++ -o vanderplaats vanderplaats.cpp *.o
rm  -f *.o
vanderplaats vanderplaats_params.in vanderplaats_results.out
