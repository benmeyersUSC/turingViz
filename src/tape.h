#pragma once

#include "drawable.h"

using std::string;
using std::ostream;
using std::unordered_map;
using helper::Symbol;
using helper::Configuration;


class Tape : public Drawable{

    private:
    char* values;
    unsigned head;
    unsigned size;
    string tapeFill;

    public:
    unordered_map<unsigned, string> cellColors;
    unsigned cellsInUse;


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

    friend ostream& operator<<(ostream& ss, const Tape& tp);

    string toString(unsigned len, unsigned step) const ;

    void draw(graphics::Window& window, Configuration* config);
};