#include <iostream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <random>

#include "/Users/benmeyers/Desktop/turingViz/src/TuringMachine/turingMachine.hpp"
#include "/Users/benmeyers/Desktop/turingViz/src/graphics/graphics.h"


using std::cout;
using std::rand;
using std::endl;
using std::min;
using std::max;
using std::vector;
using std::string;

int main(int argc, const char* argv[]) {

    fstream file("/Users/benmeyers/Desktop/turingViz/src/TuringMachine/doubling.javaturing");
    if (!file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }
    
    Tape tape;
    
    TM* counting = TM::fromStandardDescription(file, tape, 999);
    
    counting->runStepWiseWindow();
    
    cout << tape << endl;

	cout << "Final tape size: " << tape.getSize() << endl;
	cout << "Final head position: " << tape.getHead() << endl;

    return 0;
}