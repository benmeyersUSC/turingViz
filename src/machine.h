#pragma once

#include "tape.h"

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

using helper::Configuration;
using helper::Symbol;


class TuringMachine{
    const unsigned MAX_TAPE = 999;
    private:

    // vector<Configuration*> configurations;
    unordered_map<string, unordered_map<Symbol, Configuration*>*> stateSymbolToConfig;
    Tape* tape;    
    unsigned sizeLimit;

    string fullSD;
    vector<string> standardDesctription;
    vector<string> sdSignatures;

    string currentState;
    Configuration* currentConfig;


    public:

    TuringMachine(Tape* tp);

    void addConfig(Configuration* config);

    ~TuringMachine();

    static TuringMachine* fromStandardDescription(string description, Tape* tape);

    static TuringMachine* fromStandardDescription(fstream& file, Tape* tp);

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

    void draw(graphics::Window* window);
};