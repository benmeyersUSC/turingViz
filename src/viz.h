#pragma once

#include "machine.h"

using std::string;
using std::stringstream;
using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::fstream;
using std::ostream;
using std::cout;
using std::cin;
using std::endl;
using std::replace;
using std::cerr;

class TuringMachineVisualization{
    private:
        graphics::Window* window;
        TuringMachine* tm;

    public:
        TuringMachineVisualization(fstream& file, unsigned stateRate = 1000);
        bool update(long long elapsed);
        void draw();
        void run();
};