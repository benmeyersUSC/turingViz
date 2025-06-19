#pragma once

#include "graphics.h"
#include "machine.h"
#include <fstream>
#include <chrono>

using namespace graphics;
using std::fstream;

class TuringMachineVisualization {
private:
    int buttonX, buttonY, buttonWidth, buttonHeight;
    bool isPointInButton(int x, int y);
    Window* window;
    TuringMachine* tm;
    
    // Slider state
    bool draggingSlider;
    int sliderX;
    int sliderY;
    int sliderWidth;
    int sliderHeight;
    
    void drawUI();
    void processEvents();
    void drawSpeedSlider();
    bool isPointInSlider(int x, int y);
    void updateSpeedFromMouse(int mouseX);
    
public:
    TuringMachineVisualization(fstream& file, unsigned stateRate = 100);
    bool update(long long elapsed);
    void draw();
    void run();
};








// #pragma once

// #include "machine.h"

// using std::string;
// using std::stringstream;
// using std::vector;
// using std::unordered_set;
// using std::unordered_map;
// using std::fstream;
// using std::ostream;
// using std::cout;
// using std::cin;
// using std::endl;
// using std::replace;
// using std::cerr;

// class TuringMachineVisualization{
//     private:
//         Window* window;
//         TuringMachine* tm;

//     public:
//         TuringMachineVisualization(fstream& file, unsigned stateRate = 1000);
//         bool update(long long elapsed);
//         void draw();
//         void run();
// };