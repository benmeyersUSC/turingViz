#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <ostream>
#include <algorithm>

#include "/Users/benmeyers/Desktop/turingViz/src/util.hpp"
#include "/Users/benmeyers/Desktop/turingViz/src/tape.hpp"
#include "/Users/benmeyers/Desktop/turingViz/src/machine.hpp"
#include "/Users/benmeyers/Desktop/turingViz/src/drawable.hpp"
#include "/Users/benmeyers/Desktop/turingViz/src/graphics/graphics.h"

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
        TuringMachineVisualization(){
            window = new graphics::Window(1503, 819, "Turing Machine Visualization");
            Tape* tape = new Tape();
            objs.emplace(tape);
            tm = new TuringMachine(*tape);
        }
        void update(unsigned elapsed){
            tm->update(elapsed/MS_PER_TM_UPDATE);
        }
        void draw(){
            for (Drawable* obj : objs){
                obj->draw(*window);
            }
        }
        void run(){
            bool running = true;
            int lastTs;
            while (running){
                int ts;
                update(ts-lastTs);
                draw();
            }
        }
        
};