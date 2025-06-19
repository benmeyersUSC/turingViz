#pragma once

#include "tape.h"

using std::string;
using std::stringstream;
using std::vector;
using std::unordered_set;
using std::map;
using std::unordered_map;
using std::fstream;
using std::ostream;
using std::cout;
using std::cin;
using std::endl;
using std::replace;
using std::cerr;

class TuringMachine{
    const unsigned MAX_TAPE = 999;
    private:
    unsigned stateRate;
    unsigned steps;

    unsigned timeSinceUpdate = std::numeric_limits<unsigned>::max();

    Configuration* lastConfig;

    map<string, unordered_map<Symbol, Configuration*>*> stateSymbolToConfig;
    vector<Configuration*> configurations;
    unordered_set<Configuration*> unusedConfig;

    Tape* tape;    
    unsigned sizeLimit;

    string fullSD;
    vector<string> standardDesctription;
    vector<string> sdSignatures;

    string currentState;
    Configuration* currentConfig;


    public:

    double getIterPercent();

    TuringMachine(Tape* tp, unsigned stateRate);

    void addConfig(Configuration* config);

    ~TuringMachine();

    static TuringMachine* fromStandardDescription(string description, Tape* tape, unsigned stateRate);

    static TuringMachine* fromStandardDescription(fstream& file, Tape* tp, unsigned stateRate);

    string sdifyQ(Configuration* conf);

    string sdifySS(Configuration* conf);
    
    string sdifyWS(Configuration* conf);
    
    string sdifyMV(Configuration* conf);
    
    string sdifyNC(Configuration* conf);
    
    string sdify(Configuration* conf);

    string sdifyFunc(Configuration* conf);

    string sdifySig(Configuration* conf);

    string sdint(Configuration* conf);

    string sdint(const string& sd);

    bool update(unsigned loops);

    void drawRunStats(Window* window, unsigned width, unsigned halfWidth, unsigned wHeight, double statSize);

    void drawGenome(Window* window, unsigned halfWidth, unsigned wWidth, unsigned wHeight, double genomeMult, unsigned widthPerState);

    void drawBinding(Window* window, double movePercent, unsigned fromY, unsigned toY, unsigned widthPerState, unsigned halfWidth, unsigned genomeHeight);

    void draw(Window* window);

    unsigned getStateRate();

    unordered_set<Configuration*> getUnusedConfigs();
};