#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ostream>

#include "graphics.h"
using graphics::drawShapeAroundText;
using graphics::drawShapeWithText;
using graphics::widthOfTextBox;
using graphics::Window;
using graphics::BLACK;
using graphics::DARK_GRAY;

using std::stringstream;
using std::to_string;
using std::vector;
using std::string;
using std::unordered_map;
using std::max;

namespace helper{
    enum Symbol{
    S_, S0, S1, R, L, N, SENTINEL, X, Y, Z, ASTR, Q, A, S, T, U, V
};


enum Direction{
    LEFT, RIGHT, NONE
};


const unordered_map<Symbol, char> toStr = {
    {S_, ' '},
    {S0, '0'},
    {S1, '1'},
    {R, 'R'},
    {L, 'L'},
    {N, 'N'},
    {SENTINEL, '@'},
    {X, 'X'},
    {Y, 'Y'},
    {Z, 'Z'},
    {Q, 'Q'},
    {A, 'A'},
    {S, 'S'},
    {T, 'T'},
    {U, 'U'},
    {V, 'V'},
    {ASTR, '*'}
};
const unordered_map<string, Symbol> toSym = {
    {"S_", S_},
    {"S0", S0},
    {"S1", S1},
    {"R", R},
    {"L", L},
    {"N", N},
    {"SENTINEL", SENTINEL},
    {"X", X},
    {"Y", Y},
    {"Z", Z},
    {"Q", Q},
    {"A", A},
    {"S", S},
    {"T", T},
    {"U", U},
    {"V", V},
    {"*", ASTR}
};
const unordered_map<Symbol, unsigned> symInd = {
    {S_, 0},
    {S0, 1},
    {S1, 2},
    {R, 3},
    {L, 4},
    {N, 5},
    {SENTINEL, 6},
    {X, 7},
    {Y, 8},
    {Z, 9},
    {ASTR, 10},
    {Q, 11},
    {A, 12},
    {S, 13},
    {T, 14},
    {U, 15},
    {V, 16}

};
const unordered_map<char, unsigned> sdToNum = {
    {'D', 1},
    {'C', 2},
    {'A', 3},
    {'R', 4},
    {'L', 5},
    {'N', 6}
};

struct Configuration{
    unsigned index;
    string name;
    Symbol readSymbol;
    Symbol writeSymbol;
    Direction direction;
    string nextState;
    string signature;

    string sd;
    string sdSig;
    string sdWS;
    string sdMV;
    string sdNC;
    string sdNum;

    string color;

    Configuration(const unsigned idx, const string& nm, Symbol rd, const Symbol wt, const Direction d, string nxt);

    void draw(Window* window, unsigned x, unsigned y, unsigned height);
};


vector<string> split(const string& str, char delimiter);

void ltrim(string &s);

void rtrim(string &s);

void trim(string &s);

struct RGB {
    int r, g, b;
    RGB(const std::string& hex);
};

string rgbToHex(int r, int g, int b);

string dullerColor(const std::string& color, double factor = 0.09);

string interpolateColor(const std::string& color1, const std::string& color2, double t);

vector<string> generateColorSpectrum(int numConfigs);

}