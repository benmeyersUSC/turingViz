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
        unordered_set<Drawable*> objs;

        unsigned MS_PER_TM_UPDATE = 1000;

    public:
        TuringMachineVisualization();
        void update(unsigned elapsed);
        void draw();
        void run();
};