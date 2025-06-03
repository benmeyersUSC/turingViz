#include "viz.h"

TuringMachineVisualization::TuringMachineVisualization(fstream& file, unsigned msPerState) : stateRate(msPerState){
    window = new graphics::Window(1503, 819, "Turing Machine Visualization");
    Tape* tape = new Tape();
    tm = TuringMachine::fromStandardDescription(file, tape);
}

bool TuringMachineVisualization::update(long long elapsed){
    return tm->update(elapsed/stateRate);
}

void TuringMachineVisualization::draw(){
    window->clear();

    tm->draw(window);

    window->update();
}

void TuringMachineVisualization::run(){
    bool running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();
    while (running){
        auto currentTime = std::chrono::high_resolution_clock::now();
        long long delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

        cout << delta << endl;
              
        // running = update(delta);
        draw();
        graphics::pause(stateRate);
        running = false;

        lastTime = currentTime;
    }
    cout << "YOO" << endl;
}