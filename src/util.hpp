#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <ostream>
#include <algorithm>

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
    Symbol readSymbol;
    Symbol writeSymbol;
    Direction direction;
    string nextConfig;
    string signature;

    string sd;
    string sdSig;
    string sdNum;

    Configuration(const unsigned idx, Symbol rd, const Symbol wt, const Direction d, const string nxt):
    index(idx),
    readSymbol(rd), 
    writeSymbol(wt), 
    direction(d), 
    nextConfig(nxt),
    signature(nextConfig + "{\'" + toStr.at(readSymbol) + "\'}")
    {}
};



// HELPER BELOW


static vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

static void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
}

static void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
}

static void trim(string &s) {
    ltrim(s);
    rtrim(s);
}

struct RGB {
    int r, g, b;
    RGB(const std::string& hex) {
        r = std::stoi(hex.substr(1, 2), nullptr, 16);
        g = std::stoi(hex.substr(3, 2), nullptr, 16);
        b = std::stoi(hex.substr(5, 2), nullptr, 16);
    }
};

std::string rgbToHex(int r, int g, int b) {
    char buf[8];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
    return std::string(buf);
}

std::string dullerColor(const std::string& color, double factor = 0.09) {
    RGB rgb(color);
    return rgbToHex(
        rgb.r + (255 - rgb.r) * (1 - factor),
        rgb.g + (255 - rgb.g) * (1 - factor),
        rgb.b + (255 - rgb.b) * (1 - factor)
    );
}

// Interpolate between two colors
std::string interpolateColor(const std::string& color1, const std::string& color2, double t) {
    RGB c1(color1);
    RGB c2(color2);
    
    int r = c1.r + (int)((c2.r - c1.r) * t);
    int g = c1.g + (int)((c2.g - c1.g) * t);
    int b = c1.b + (int)((c2.b - c1.b) * t);
    
    return rgbToHex(r, g, b);
}

// Generate color spectrum for N configurations
std::vector<std::string> generateColorSpectrum(int numConfigs) {
    std::vector<std::string> spectrum;
    
    // Define key colors for biological look
    std::vector<std::string> keyColors = {
        "#FF0000",  // Red
        "#FF7F00",  // Orange  
        "#FFFF00",  // Yellow
        "#00FF00",  // Green
        "#00FFFF",  // Cyan
        "#0000FF",  // Blue
        "#FF00FF"   // Magenta
    };
    
    if (numConfigs <= keyColors.size()) {
        // If few configs, just use the key colors
        for (int i = 0; i < numConfigs; i++) {
            spectrum.push_back(keyColors[i]);
        }
    } else {
        // Interpolate between key colors
        double segmentSize = (double)(numConfigs - 1) / (keyColors.size() - 1);
        
        for (int i = 0; i < numConfigs; i++) {
            double position = i / segmentSize;
            int colorIndex = (int)position;
            double localT = position - colorIndex;
            
            if (colorIndex >= keyColors.size() - 1) {
                spectrum.push_back(keyColors.back());
            } else {
                spectrum.push_back(interpolateColor(
                    keyColors[colorIndex], 
                    keyColors[colorIndex + 1], 
                    localT
                ));
            }
        }
    }
    
    return spectrum;
}