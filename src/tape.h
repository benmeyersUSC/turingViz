#pragma once

#include "helper.h"

using std::string;
using std::ostream;
using std::unordered_map;
using helper::Symbol;
using helper::Configuration;


class Tape{

    private:
    char* values;
    unsigned head;
    unsigned size;
    string tapeFill;
    unsigned cellsInUse;


    public:
    unordered_map<unsigned, string> cellColors;


    unsigned getHead();
    
    Tape();
    
    ~Tape();
    
    Tape(const Tape& other);
    
    Tape& operator=(const Tape& other);

    Symbol read();

    Symbol readAt(unsigned i);

    string readStr(int displacement = 0);

    void write(Symbol s);

    void right();

    void left();

    unsigned getSize();

    void incrCellsInUse();

    unsigned getCellsInUse();

    friend ostream& operator<<(ostream& ss, const Tape& tp);

    string toString(unsigned len, unsigned step) const ;

    void draw(Window* window, Configuration* config);

    void drawWhole(Window* window, Configuration* config, unsigned wWidth, unsigned wHeight, double heightMult);
};