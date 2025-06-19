#include "viz.h"

TuringMachineVisualization::TuringMachineVisualization(fstream& file, unsigned stateRate){
    window = new Window(1503, 819, "Turing Machine Visualization");
    window->params.emplace("protein_mode", true);
    
    Tape* tape = new Tape();
    tm = TuringMachine::fromStandardDescription(file, tape, stateRate);
}

bool TuringMachineVisualization::update(long long elapsed){
    return tm->update(elapsed);
}

void TuringMachineVisualization::draw(){
    window->clear();
    tm->draw(window);
    
    // Draw UI overlay showing current mode and instructions
    drawUI();
    
    window->update(); // graphics method, not our update()
}

void TuringMachineVisualization::drawUI() {
    // Draw protein mode indicator
    bool proteinMode = any_cast<bool>(window->params.at("protein_mode"));
    std::string modeText = proteinMode ? "Protein Mode: ON" : "Protein Mode: OFF";
    
    // Position in top-right corner
    int textWidth = widthOfTextBox(modeText, 10);
    int x = window->getWidth() - textWidth/2 - 20;
    int y = 30;
    
    drawShapeAroundText(*window, modeText, x, y, 25, 
                       proteinMode ? "#00AA00" : "#AA0000", 
                       10, 14, true, graphics::BLACK, graphics::WHITE);
    
    // Draw instructions
    std::string instructions = "Press 'P' to toggle Protein Mode";
    int instrWidth = widthOfTextBox(instructions, 5);
    drawShapeAroundText(*window, instructions, window->getWidth() - instrWidth/2 - 20, 60, 20, 
                       graphics::LIGHT_GRAY, 5, 12, true, graphics::DARK_GRAY, graphics::BLACK);
}

void TuringMachineVisualization::processEvents() {
    while (window->hasEvents()) {
        Event e = window->getEvent();
        
        if (e.Type == EventType::KeyDown) {
            std::cout << "Key pressed: " << (char)e.Event.Key.Code << " (code: " << e.Event.Key.Code << ")" << std::endl;
            
            // Check for 'p' or 'P' key press
            if (e.Event.Key.Code == 'p' || e.Event.Key.Code == 'P') {
                // Toggle protein mode
                bool currentMode = any_cast<bool>(window->params.at("protein_mode"));
                window->params["protein_mode"] = !currentMode;
                std::cout << "Toggled protein mode to: " << (!currentMode ? "ON" : "OFF") << std::endl;
            }
        }
    }
}

void TuringMachineVisualization::run(){
    bool running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();
    int xxx = 0;
    
    while (running && window->isOpen()){
        auto currentTime = std::chrono::high_resolution_clock::now();
        long long delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        
        // Process any keyboard/mouse events
        processEvents();
        
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





















// #include "viz.h"

// TuringMachineVisualization::TuringMachineVisualization(fstream& file, unsigned stateRate){
//     window = new Window(1503, 819, "Turing Machine Visualization");
//     window->params.emplace("protein_mode", true);
//     Tape* tape = new Tape();
//     tm = TuringMachine::fromStandardDescription(file, tape, stateRate);
// }

// bool TuringMachineVisualization::update(long long elapsed){
//     return tm->update(elapsed);
// }

// void TuringMachineVisualization::draw(){
//     window->clear();

//     tm->draw(window);

//     window->update();               // graphics method, not our update()
// }

// void TuringMachineVisualization::run(){
//     bool running = true;
//     auto lastTime = std::chrono::high_resolution_clock::now();

//     int xxx = 0;
//     while (running && window->isOpen()){
//         auto currentTime = std::chrono::high_resolution_clock::now();
//         long long delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();              
//         running = update(delta);
//         draw();
//         lastTime = currentTime;
//         xxx++;
//         if (xxx > 9000){running = false;}
//     }
//     for (Configuration* conf : tm->getUnusedConfigs()){
//         cout << conf->signature << endl;
//     }
// }