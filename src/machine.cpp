#include "/Users/benmeyers/Desktop/turingViz/src/machine.h"

using helper::split;
using helper::trim;
using helper::toSym;
using helper::Direction;
using helper::symInd;
using helper::RIGHT;
using helper::LEFT;
using helper::NONE;
using helper::sdToNum;

TuringMachine::TuringMachine(Tape* tp, unsigned msPerState): tape(tp), sizeLimit(999), stateRate(msPerState){}

TuringMachine::~TuringMachine() {}

void TuringMachine::addConfig(Configuration* config){
    if (stateSymbolToConfig.size() == 0){currentConfig = config;}

    if (stateSymbolToConfig.count(config->name) == 0){
        unordered_map<Symbol, Configuration*>* func = new unordered_map<Symbol, Configuration*>();
        stateSymbolToConfig.emplace(config->name, func);
    }
    stateSymbolToConfig.at(config->name)->emplace(config->readSymbol, config);
}

TuringMachine* TuringMachine::fromStandardDescription(string description, Tape* tape, unsigned msPerState){
    TuringMachine* utm = new TuringMachine(tape, msPerState);

    for (const string& line : split(description, ';')){

        string nLine = line;
        trim(nLine);
        if (nLine == "#########"){break;}
        if (nLine.empty()){continue;}

        vector<string> parts = split(nLine, '-');
        if (parts.size() != 5){
            throw new std::invalid_argument("Invalid Congfiguration Definition!");
        }

        try{
            string state = parts[0];
            trim(state);

            string p1trim = parts[1];
            trim(p1trim);
            Symbol readSymbol = toSym.at(p1trim);

            string p2trim = parts[2];
            trim(p2trim);
            Symbol writeSymbol = toSym.at(p2trim);

            string p3trim = parts[3];
            trim(p3trim);
            Direction direction = p3trim == "R" ? Direction::RIGHT : p3trim == "L" ? Direction::LEFT : Direction::NONE;

            string nextState = parts[4];
            trim(nextState);

            unsigned idx;
            if (utm->stateSymbolToConfig.count(state) == 0){
                int currSize = utm->stateSymbolToConfig.size();
                idx = currSize;
            }
            else{
                idx = utm->stateSymbolToConfig.at(state)->begin()->second->index;
            }

            Configuration* config = new Configuration(idx, state, readSymbol, writeSymbol, direction, nextState);
            utm->configurations.emplace(config);
            utm->addConfig(config);
        }
        catch(std::invalid_argument e){
            throw new std::invalid_argument("Invalid symbol!");
        }
    }
    vector<string> colors = helper::generateColorSpectrum(utm->configurations.size());
    for (Configuration* c : utm->configurations){
        c->color = colors[c->index];

        c->sd = utm->sdify(c);
        c->sdSig = utm->sdifySig(c);
        c->sdNum = utm->sdint(c);

        utm->standardDesctription.push_back(c->sd);
        utm->fullSD += c->sd;
        utm->sdSignatures.push_back(c->sdSig);
    }
    return utm;
}

TuringMachine* TuringMachine::fromStandardDescription(fstream& file, Tape* tp, unsigned msPerState){
    string fileContent;
    try {
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        fileContent = buffer.str();
        
        size_t hashPos = fileContent.find("#########");
        if (hashPos != string::npos) {
            fileContent = fileContent.substr(hashPos + 9); // 9 is length of "#########"
        }
        
        replace(fileContent.begin(), fileContent.end(), '\t', ' ');
        replace(fileContent.begin(), fileContent.end(), '\n', ' ');
        
    } catch (const std::exception& e) {
        cerr << "Exception: " << e.what() << std::endl;
    }
    
    return fromStandardDescription(fileContent, tp, msPerState);
}

string TuringMachine::sdifyQ(Configuration* conf){
    stringstream ss;
    ss << 'D';
    for (unsigned i = 0; i < conf->index; i++){
        ss << 'C';
    }
    return ss.str();
}

string TuringMachine::sdifySS(Configuration* conf){
    stringstream ss;
    ss << 'D';
    for (unsigned i = 0; i < symInd.at(conf->readSymbol); i++){
        ss << 'A';
    }
    return ss.str();
}

string TuringMachine::sdifyWS(Configuration* conf){
    stringstream ss;
    ss << 'D';
    for (unsigned i = 0; i < symInd.at(conf->writeSymbol); i++){
        ss << 'A';
    }
    return ss.str();
}

string TuringMachine::sdifyMV(Configuration* conf){
    return conf->direction == RIGHT ? "R" : conf->direction == LEFT ? "L" : "N";
}

string TuringMachine::sdifyNC(Configuration* conf){
    unordered_map<Symbol, Configuration*>* nextFunc = stateSymbolToConfig.at(conf->nextState);
    stringstream ss;
    ss << 'D';
    for (unsigned i = 0; i < nextFunc->begin()->second->index; i++){
        ss << 'C';
    }
    return ss.str();
}

string TuringMachine::sdify(Configuration* conf){
    stringstream ss;
    ss << sdifyQ(conf) << sdifySS(conf) << sdifyWS(conf) << sdifyMV(conf) << sdifyNC(conf);
    return ss.str();
}

string TuringMachine::sdifyFunc(Configuration* conf){
    stringstream ss;
    ss << sdifyWS(conf) << sdifyMV(conf) << sdifyNC(conf);
    return ss.str();
}

string TuringMachine::sdifySig(Configuration* conf){
    stringstream ss;
    ss << 'D';
    for (unsigned i = 0; i < conf->index; i++){
        ss << 'C';
    }
    ss << 'D';
    for (unsigned i = 0; i < symInd.at(conf->readSymbol); i++){
        ss << 'A';
    }
    return ss.str();
}

string TuringMachine::sdint(Configuration* conf){
    string sd = sdify(conf);
    stringstream ss;
    for (unsigned i = 0; i < sd.size(); i++){
        ss << sdToNum.at(sd[i]);
    }
    return ss.str();
}

string TuringMachine::sdint(const string& sd){
    stringstream ss;
    for (unsigned i = 0; i < sd.size(); i++){
        ss << sdToNum.at(sd[i]);
    }
    return ss.str();
}

bool TuringMachine::update(unsigned elapsed){
    if (timeSinceUpdate < stateRate){
        timeSinceUpdate += elapsed; return true;
    }
    
    // write new sym
    tape->write(currentConfig->writeSymbol);
    // if we're operating on a white cell, add it to seen cells
    if (tape->cellColors[tape->getHead()] == graphics::WHITE){
        tape->cellsInUse++;
    }
    // update config stain regardless
    tape->cellColors[tape->getHead()] = currentConfig->color;
    
    // move
    if (currentConfig->direction == LEFT){
        tape->left();
    }
    else if (currentConfig->direction == RIGHT){
        tape->right();
    }

    // jump to next state
    currentState = currentConfig->nextState;
    currentConfig = stateSymbolToConfig.at(currentState)->at(tape->read());
    unusedConfig.erase(currentConfig);
    timeSinceUpdate = 0;

    steps++;
    return !(currentState == "HALT");
}

void TuringMachine::drawRunStats(Window* window){
    stringstream ss;
    ss << "Iteration #" << steps << ", on square #" << tape->getHead();
    drawShapeWithText(*window, ss.str(), window->getWidth()/2, window->getHeight() * 0.975, window->getWidth(), window->getHeight() * 0.05);
}

void TuringMachine::drawGenome(Window* window){
        stringstream ss;
        ss << "Turing Machine Genome: " << stateSymbolToConfig.size()-1 << " genes, " << fullSD.size() << " total nucleotides!";
        drawShapeWithText(*window, ss.str(), window->getWidth()/2, window->getHeight() * 0.0125, window->getWidth(), window->getHeight() * 0.025);
        int widdy = (int) window->getWidth()/stateSymbolToConfig.size();
        for (Configuration* c : configurations){
            drawShapeWithText(*window, "Q" + std::to_string(1+c->index), widdy/2 + (c->index * (window->getWidth()/stateSymbolToConfig.size())), window->getHeight() * 0.0425, widdy, window->getHeight() * 0.035, true, c->color);
            if (c == currentConfig){
                drawShapeAroundText(*window, c->sd, widdy/2 + (c->index * (window->getWidth()/stateSymbolToConfig.size())), window->getHeight() * 0.0775, window->getHeight() * 0.035, c->color, 2);
            }   
        }
    }

void TuringMachine::drawWholeTape(Window* window){

//     void vizWholeTape(graphics::Window& window, const string& headColor){
//         int wid = (int)(window.getWidth()/tape.cellsInUse);

//         // moving head:
//         string headthing;
//         int headX;
//         if (tape.getHead() < tape.cellsInUse) {
//             // Head is within the visible cells - show exact position
//             headthing = "HEAD ";
//             headX = (tape.getHead() + 0.5) * wid;
//         } else {
//             // Head is beyond visible cells - show arrow at right edge
//             headthing = "HEAD >> ";
//             headX = window.getWidth() - wid/2.0;
//         }
//         // min size
//         int cappedWid = std::max(wid, (int)(12 * headthing.size()));
//         graphics::drawShapeWithText(window, headthing, headX, window.getHeight() * 0.8, cappedWid, window.getHeight() * 0.027, true, headColor);

//         for (unsigned i = 0; i < tape.cellsInUse; i++){
//             // config-stained view
//             window.setColor(tape.cellColors.at(i));
//             window.fillRect(i * wid, window.getHeight() * 0.825, wid, window.getHeight() * 0.05);
//             window.setColor(graphics::BLACK);
//             window.drawRect(i * wid, window.getHeight() * 0.825, wid, window.getHeight() * 0.05);

//             // binary view
//             if (toStr.at(tape.readAt(i)) == '0'){
//                 window.setColor(graphics::DARK_GRAY);
//             }
//             else if (toStr.at(tape.readAt(i)) == '1'){
//                 window.setColor(graphics::BLACK);
//             }
//             else{
//                 window.setColor(dullerColor(tape.cellColors.at(i)));
//             }
//             window.fillRect(i * wid, window.getHeight() * 0.875, wid, window.getHeight() * 0.05);
//             window.setColor(graphics::BLACK);
//             window.drawRect(i * wid, window.getHeight() * 0.875, wid, window.getHeight() * 0.05);
//         }
//     }
}

void TuringMachine::drawBinding(Window* window){
        float movePercent = .54;
        float iterPercent = timeSinceUpdate / (1.0 * stateRate);
        // human signature
        string currSig = currentConfig->signature;
        // sd signature
        string currGene = sdifySig(currentConfig);
        // iterPercent --> if over movePercent, then put it in final state
        double realP = iterPercent > movePercent ? 1.0 : iterPercent/movePercent;

        // vertical distance scaled by realP
        int yAx = window->getHeight()/2 - (((window->getHeight()/2)-(window->getHeight() * 0.0425))*(1-realP));
        // horizontal is more complicated because we have midX, but same deal
        int widdy = (int) window->getWidth()/stateSymbolToConfig.size();
        int preXax = widdy/2 + (currentConfig->index * (window->getWidth()/stateSymbolToConfig.size()));
        int xAx;
        if (preXax >= window->getHeight()/2){
            xAx = preXax - ((preXax - window->getHeight()/2) * realP);
        }
        else{
            xAx = preXax + ((window->getHeight()/2 - preXax) * realP);
        }

        // signature
        int widSig = graphics::widthOfTextBox(currGene, 6);
        graphics::drawShapeAroundText(*window, currGene, xAx , yAx, window->getHeight() * 0.035, currentConfig->color, 6, 14, false);

        // write symbol
        int widWS = graphics::widthOfTextBox(sdifyWS(currentConfig), 3); // calc w padding
        //      3 padding x2      myself/2
        xAx += widSig/2.0 + widWS/2.0;
        graphics::drawShapeAroundText(*window, sdifyWS(currentConfig), xAx, yAx, window->getHeight() * 0.035, currentConfig->color, 3, 14, false); // write without padding
    
        // move direction
        int widMV = graphics::widthOfTextBox(sdifyMV(currentConfig), 3);
        xAx += widWS/2.0 + widMV/2.0;
        graphics::drawShapeAroundText(*window, sdifyMV(currentConfig), xAx, yAx, window->getHeight() * 0.035, currentConfig->color, 3, 14, false);

        // next config
        int widNC = graphics::widthOfTextBox(sdifyNC(currentConfig), 6);
        xAx += widMV/2.0 + widNC/2.0;
        graphics::drawShapeAroundText(*window, sdifyNC(currentConfig), xAx, yAx, window->getHeight() * 0.035, currentConfig->color, 6, 14, false);
    }


void TuringMachine::draw(Window* window) {
    tape->draw(window, currentConfig);

    drawRunStats(window);
    drawGenome(window);
    drawWholeTape(window);
    drawBinding(window);

}

unsigned TuringMachine::getStateRate(){
    return stateRate;
}

unordered_set<Configuration*> TuringMachine::getUnusedConfigs(){
    return unusedConfig;
}

