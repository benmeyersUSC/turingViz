#include "viz.h"

TuringMachineVisualization::TuringMachineVisualization(fstream& file, unsigned stateRate){
    window = new graphics::Window(1503, 819, "Turing Machine Visualization");
    Tape* tape = new Tape();
    tm = TuringMachine::fromStandardDescription(file, tape, stateRate);
}

bool TuringMachineVisualization::update(long long elapsed){
    return tm->update(elapsed);
}

void TuringMachineVisualization::draw(){
    window->clear();

    tm->draw(window);

    window->update();
}

void TuringMachineVisualization::run(){
    bool running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    int xxx = 0;
    while (running && window->isOpen()){
        auto currentTime = std::chrono::high_resolution_clock::now();
        long long delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();              
        running = update(delta);
        draw();
        lastTime = currentTime;
        xxx++;
        if (xxx > 9000){running = false;}
    }
    for (Configuration* conf : tm->getUnusedConfigs()){
        cout << conf->signature << endl;
    }
}