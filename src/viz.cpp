#include "viz.h"
#include <cmath>
#include <sstream>
#include <iomanip>

TuringMachineVisualization::TuringMachineVisualization(fstream& file, unsigned stateRate){
    window = new Window(1503, 819, "Turing Machine Visualization");
    window->params.emplace("protein_mode", true);
    
    Tape* tape = new Tape();
    tm = TuringMachine::fromStandardDescription(file, tape, stateRate);
    
    // Initialize slider state
    draggingSlider = false;
        sliderHeight = 30;

    sliderWidth = 300;
    sliderX = window->getWidth() * 0.75 - sliderWidth/2.0;
    sliderY = window->getHeight() - sliderHeight;

    buttonWidth = 160;
    buttonX = window->getWidth()*0.25 - buttonWidth*0.5;
    buttonHeight = 35;
    buttonY = window->getHeight() - buttonHeight*1.1;
}

bool TuringMachineVisualization::isPointInButton(int x, int y) {
    return x >= buttonX && x <= buttonX + buttonWidth &&
           y >= buttonY && y <= buttonY + buttonHeight;
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
    // Draw protein mode toggle button
    bool proteinMode = any_cast<bool>(window->params.at("protein_mode"));
    std::string modeText = proteinMode ? "Protein Mode: ON" : "Protein Mode: OFF";
    
    // Draw button background
    window->setColor(proteinMode ? "#00AA00" : "#AA0000");
    window->fillRect(buttonX, buttonY, buttonWidth, buttonHeight);
    window->setColor(graphics::BLACK);
    window->drawRect(buttonX, buttonY, buttonWidth, buttonHeight);
    
    // Draw button text
    window->setColor(graphics::WHITE);
    window->drawLabel(modeText, buttonX + buttonWidth/2 - modeText.length() * 4, buttonY + buttonHeight/2 + 5);
    
    // Draw speed slider
    drawSpeedSlider();
}

void TuringMachineVisualization::drawSpeedSlider() {
    // Draw slider background
    window->setColor(graphics::DARK_GRAY);
    window->fillRect(sliderX, sliderY, sliderWidth, sliderHeight);
    window->setColor(graphics::BLACK);
    window->drawRect(sliderX, sliderY, sliderWidth, sliderHeight);
    
    // Calculate slider position based on current speed
    unsigned currentSpeed = tm->getStateRate();
    // Map speed (1-3000) to position (0-1)
    double normalized = (std::log(currentSpeed) - std::log(1.0)) / (std::log(3000.0) - std::log(1.0));
    int knobX = sliderX + 10 + (sliderWidth - 20) * normalized;
    
    // Draw slider track
    window->setColor(graphics::GRAY);
    window->fillRect(sliderX + 10, sliderY + sliderHeight/2, sliderWidth - 20, 4);
    
    // Draw slider knob
    window->setColor(draggingSlider ? graphics::ORANGE : graphics::BLUE);
    window->fillCircle(knobX, sliderY + sliderHeight/2, 8);
    window->setColor(graphics::BLACK);
    window->drawCircle(knobX, sliderY + sliderHeight/2, 8);
    
    // Draw speed label
    std::stringstream ss;
    ss << 1000.0/currentSpeed << " states/second";
    drawShapeAroundText(*window, ss.str(), sliderX + sliderWidth/2.0 + widthOfTextBox(ss.str(), 27, 12) * 1.5, sliderY + sliderHeight/2, 20, 
                       graphics::WHITE, 27, 12, true, graphics::BLACK, graphics::BLACK);
                       
}

bool TuringMachineVisualization::isPointInSlider(int x, int y) {
    // Check if point is within slider bounds (with some padding for easier clicking)
    return x >= sliderX - 10 && x <= sliderX + sliderWidth + 10 &&
           y >= sliderY - 10 && y <= sliderY + sliderHeight + 10;
}

void TuringMachineVisualization::updateSpeedFromMouse(int mouseX) {
    // Clamp mouse position to slider bounds
    int effectiveX = std::max(sliderX + 10, std::min(mouseX, sliderX + sliderWidth - 10));
    
    // Calculate normalized position (0-1)
    double normalized = (double)(effectiveX - sliderX - 10) / (sliderWidth - 20);
    
    // Map to logarithmic scale (1-3000)
    double logMin = std::log(1.0);
    double logMax = std::log(3000.0);
    double logValue = logMin + normalized * (logMax - logMin);
    unsigned newSpeed = std::round(std::exp(logValue));
    
    // Update the state rate
    tm->setStateRate(newSpeed);
}

void TuringMachineVisualization::processEvents() {
    while (window->hasEvents()) {
        Event e = window->getEvent();
        
        if (e.Type == EventType::KeyDown) {
            // Check for 'p' or 'P' key press
            if (e.Event.Key.Code == 'p' || e.Event.Key.Code == 'P') {
                // Toggle protein mode
                bool currentMode = any_cast<bool>(window->params.at("protein_mode"));
                window->params["protein_mode"] = !currentMode;
            }
        }
        else if (e.Type == EventType::MouseBtnDown) {
            if (e.Event.Mouse.Button == MouseButton::Left) {
                // Check button click
                if (isPointInButton(e.Event.Mouse.X, e.Event.Mouse.Y)) {
                    bool currentMode = any_cast<bool>(window->params.at("protein_mode"));
                    window->params["protein_mode"] = !currentMode;
                }
                // Check slider
                else if (isPointInSlider(e.Event.Mouse.X, e.Event.Mouse.Y)) {
                    draggingSlider = true;
                    updateSpeedFromMouse(e.Event.Mouse.X);
                }
                else if (draggingSlider) {
                    // Continue dragging even if mouse moves outside slider
                    updateSpeedFromMouse(e.Event.Mouse.X);
                }
            }
        }
        else if (e.Type == EventType::MouseBtnUp) {
            if (e.Event.Mouse.Button == MouseButton::Left) {
                draggingSlider = false;
            }
        }
    }
    
    // Handle mouse dragging (this needs to check current mouse position)
    // For now, we'll handle drag on mouse down events
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
