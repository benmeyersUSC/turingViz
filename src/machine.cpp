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

TuringMachine::TuringMachine(Tape* tp) : tape(tp), sizeLimit(999), currentConfig(){}

TuringMachine::~TuringMachine() {}

void TuringMachine::addConfig(Configuration* config){
    if (stateSymbolToConfig.size() == 0){currentConfig = config;}

    if (stateSymbolToConfig.count(config->name) == 0){
        unordered_map<Symbol, Configuration*>* func = new unordered_map<Symbol, Configuration*>();
        stateSymbolToConfig.emplace(config->name, func);
    }
    stateSymbolToConfig.at(config->name)->emplace(config->readSymbol, config);

    config->sd = sdify(config);
    config->sdSig = sdifySig(config);
    config->sdNum = sdint(config);

    standardDesctription.push_back(config->sd);
    fullSD += config->sd;
    sdSignatures.push_back(config->sdSig);
}

TuringMachine* TuringMachine::fromStandardDescription(string description, Tape* tape){
    TuringMachine* utm = new TuringMachine(tape);

    for (const string& line : split(description, ';')){

        string nLine = line;
        trim(nLine);
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
            utm->addConfig(config);
        }
        catch(std::invalid_argument e){
            throw new std::invalid_argument("Invalid symbol!");
        }
    }
    return utm;
}

TuringMachine* TuringMachine::fromStandardDescription(fstream& file, Tape* tp){
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
    
    return fromStandardDescription(fileContent, tp);
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

void TuringMachine::update(unsigned loops){
    for (unsigned i = 0; i < loops; i++){
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
    }
}

void TuringMachine::draw(graphics::Window& window) {
    tape->draw(window, currentConfig);
}