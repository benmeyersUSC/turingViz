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

class TuringMachine {
    const unsigned MAX_TAPE = 999;
    private:

    unordered_map<string, unordered_map<Symbol, Configuration>> head;
    string currentState;
    Tape& tape;    
    unsigned sizeLimit;

    string fullSD;

    unordered_set<string> configs;
    unordered_map<string, unsigned> configIds;
    vector<string> signatures;
    unordered_map<string, int> signatureToCongifIndex;
    vector<string> standardDesctription;
    vector<string> sdSignatures;
    vector<string> SDNum;
    unordered_map<string, unsigned> sigToScale; // signature -> signatureIndex
    unordered_map<unsigned, unsigned> scaleToGene; // sigScale -> x coordinate on genome (genome now a bar up top)
    unordered_map<string, string> sigToColor;
    
    int sliderValue = 500;
    bool draggingSlider = false;

    public:

    TuringMachine(Tape& tp) : tape(tp), sizeLimit(999) {}

    TuringMachine(Tape& tp, unsigned szLmt) : tape(tp), sizeLimit(szLmt) {}

    ~TuringMachine() {}

    static TuringMachine* fromStandardDescription(string description, Tape& tape, unsigned szLmt){
        bool foundInit = false;
        TuringMachine* utm = new TuringMachine(tape, szLmt);

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

                if (!foundInit){
                    utm->currentState = state;
                    foundInit = true;
                }

                unsigned idx;
                if (utm->configs.count(state) == 0){
                    idx = utm->configs.size();
                    utm->configs.emplace(state);
                    utm->configIds.emplace(state, idx);
                }
                else{
                    idx = utm->configIds.at(state);
                }
                Configuration config = Configuration(idx, readSymbol, writeSymbol, direction, nextState);
            
                if (utm->sigToScale.find(config.signature) == utm->sigToScale.end()) {
                    utm->sigToScale.emplace(config.signature, utm->signatures.size());
                    utm->signatures.push_back(config.signature);
                    utm->signatureToCongifIndex.emplace(config.signature, config.index);
                }

                config.sd = utm->sdify(config);
                config.sdSig = utm->sdifySig(config);
                config.sdNum = utm->sdint(config);

                utm->addConfiguration(state, config);

                utm->standardDesctription.push_back(config.sd);
                utm->fullSD += config.sd;
                utm->sdSignatures.push_back(config.sdSig);
                utm->SDNum.push_back(config.sdNum);
            }
            catch(std::invalid_argument e){
                throw new std::invalid_argument("Invalid symbol!");
            }
        }
        return utm;
    }

    static TuringMachine* fromStandardDescription(fstream& file, Tape& tp, unsigned szLmt){
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
        
        return fromStandardDescription(fileContent, tp, szLmt);
    }

    void addConfiguration(const string& state, const Configuration& config) {
        if (head.find(state) == head.end()) {
            head.insert({state, unordered_map<Symbol, Configuration>()});
        }
        
        auto& stateMap = head.at(state);
        
        auto it = stateMap.find(config.readSymbol);
        if (it != stateMap.end()) {
            stateMap.erase(it);
        }
        
        stateMap.insert({config.readSymbol, config});
    }

    string sdifyQ(Configuration conf){
        stringstream ss;
        ss << 'D';
        for (unsigned i = 0; i < conf.index; i++){
            ss << 'C';
        }
        return ss.str();
    }

    string sdifySS(Configuration conf){
        stringstream ss;
        ss << 'D';
        for (unsigned i = 0; i < symInd.at(conf.readSymbol); i++){
            ss << 'A';
        }
        return ss.str();
    }
    
    string sdifyWS(Configuration conf){
        stringstream ss;
        ss << 'D';
        for (unsigned i = 0; i < symInd.at(conf.writeSymbol); i++){
            ss << 'A';
        }
        return ss.str();
    }
    
    string sdifyMV(Configuration conf){
        return conf.direction == RIGHT ? "R" : conf.direction == LEFT ? "L" : "N";
    }
    
    string sdifyNC(Configuration conf){
        stringstream ss;
        ss << 'D';
        int nextInd;
        if (configIds.count(conf.nextConfig) == 0){
            nextInd = configs.size();
        }
        else{
            nextInd = configIds.at(conf.nextConfig);
        }
        for (unsigned i = 0; i < nextInd; i++){
            ss << 'C';
        }
        return ss.str();
    }
    
    string sdify(Configuration conf){
        stringstream ss;
        ss << sdifyQ(conf) << sdifySS(conf) << sdifyWS(conf) << sdifyMV(conf) << sdifyNC(conf);
        return ss.str();
    }

    string sdifyFunc(Configuration conf){
        stringstream ss;
        ss << sdifyWS(conf) << sdifyMV(conf) << sdifyNC(conf);
        return ss.str();
    }

    string sdifySig(Configuration conf){
        stringstream ss;
        ss << 'D';
        for (unsigned i = 0; i < conf.index; i++){
            ss << 'C';
        }
        ss << 'D';
        for (unsigned i = 0; i < symInd.at(conf.readSymbol); i++){
            ss << 'A';
        }
        return ss.str();
    }

    string sdint(Configuration conf){
        string sd = sdify(conf);
        stringstream ss;
        for (unsigned i = 0; i < sd.size(); i++){
            ss << sdToNum.at(sd[i]);
        }
        return ss.str();
    }

    string sdint(const string& sd){
        stringstream ss;
        for (unsigned i = 0; i < sd.size(); i++){
            ss << sdToNum.at(sd[i]);
        }
        return ss.str();
    }

    void update(unsigned loops){
        for (unsigned i = 0; i < loops; i++){
            // where are we?
            Symbol currentSymbol = tape.read();
            Configuration configuration = head.at(currentState).at(currentSymbol);

            // write new sym
            tape.write(configuration.writeSymbol);
            // if we're operating on a white cell, add it to seen cells
            if (tape.cellColors[tape.getHead()] == graphics::WHITE){
                tape.cellsInUse++;
            }
            // update config stain regardless
            tape.cellColors[tape.getHead()] = sigToColor.at(configuration.signature);
            
            // move
            if (configuration.direction == LEFT){
                tape.left();
            }
            else if (configuration.direction == RIGHT){
                tape.right();
            }
            // update state
            currentState = configuration.nextConfig;
        }
    }

};