#include "viz.h"

TuringMachineVisualization::TuringMachineVisualization(){
    window = new graphics::Window(1503, 819, "Turing Machine Visualization");
    Tape* tape = new Tape();
    objs.emplace(tape);
    tm = new TuringMachine(tape);
    objs.emplace(tm);
}

void TuringMachineVisualization::update(unsigned elapsed){
    tm->update(elapsed/MS_PER_TM_UPDATE);
}

void TuringMachineVisualization::draw(){
    window->clear();
    for (Drawable* obj : objs){
        obj->draw(*window);
    }
    window->update();
}

void TuringMachineVisualization::run(){
    bool running = true;
    int lastTs;
    while (running){
        int ts;
        update(ts-lastTs);
        draw();
    }
}