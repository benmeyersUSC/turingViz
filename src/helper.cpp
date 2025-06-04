#include "helper.h"

using std::getline;

namespace helper{
    Configuration::Configuration(const string& nm, Symbol rd, const Symbol wt, const Direction d, string nxt):
    name(nm),
    readSymbol(rd), 
    writeSymbol(wt), 
    direction(d), 
    nextState(nxt),
    signature(name + "{\'" + toStr.at(readSymbol) + "\'}")
    {}

    void Configuration::draw(Window* window, unsigned x, unsigned y, unsigned height){
        // signature
        int widSig = widthOfTextBox(sdSig, 6);
        drawShapeAroundText(*window, sdSig, x , y, height, color, 6, 14, false);

        // write symbol
        int widWS = widthOfTextBox(sdWS, 3); 
        //      3 padding x2      myself/2
        x += widSig/2.0 + widWS/2.0;
        drawShapeAroundText(*window, sdWS, x, y, height, color, 3, 14, false); 
    
        // move direction
        int widMV = widthOfTextBox(sdMV, 3);
        x += widWS/2.0 + widMV/2.0;
        drawShapeAroundText(*window, sdMV, x, y, height, color, 3, 14, false);

        // next config
        int widNC = widthOfTextBox(sdNC, 6);
        x += widMV/2.0 + widNC/2.0;
        drawShapeAroundText(*window, sdNC, x, y, height, color, 6, 14, false);
    }

    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    void ltrim(string &s) {
        s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !isspace(ch);
        }));
    }

    void rtrim(string &s) {
        s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !isspace(ch);
        }).base(), s.end());
    }

    void trim(string &s) {
        ltrim(s);
        rtrim(s);
    }
 
    RGB::RGB(const std::string& hex) {
        r = std::stoi(hex.substr(1, 2), nullptr, 16);
        g = std::stoi(hex.substr(3, 2), nullptr, 16);
        b = std::stoi(hex.substr(5, 2), nullptr, 16);
    }

    string rgbToHex(int r, int g, int b) {
        char buf[8];
        snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
        return std::string(buf);
    }

    string dullerColor(const std::string& color, double factor) {
        RGB rgb(color);
        return rgbToHex(
            rgb.r + (255 - rgb.r) * (1 - factor),
            rgb.g + (255 - rgb.g) * (1 - factor),
            rgb.b + (255 - rgb.b) * (1 - factor)
        );
    }

string percentageToColor(double t) {
    // t is 0.0 to 1.0
    // Simple HSV approach: vary hue from 0 (red) to 300 (magenta)
    double hue = t * 300.0;
    
    // Convert to RGB using simplified HSV->RGB (S=1, V=1)
    double h = hue / 60.0;
    int hi = (int)h;
    double f = h - hi;
    
    int r, g, b;
    switch(hi) {
        case 0: r = 255; g = (int)(255 * f); b = 0; break;        // Red to Yellow
        case 1: r = (int)(255 * (1-f)); g = 255; b = 0; break;    // Yellow to Green
        case 2: r = 0; g = 255; b = (int)(255 * f); break;        // Green to Cyan
        case 3: r = 0; g = (int)(255 * (1-f)); b = 255; break;    // Cyan to Blue
        case 4: r = (int)(255 * f); g = 0; b = 255; break;        // Blue to Magenta
        default: r = 255; g = 0; b = 255; break;                   // Magenta
    }
    
    return rgbToHex(r, g, b);
}

vector<string> generateColorSpectrum(int numConfigs) {
    vector<string> spectrum;
    
    for (int i = 0; i < numConfigs; i++) {
        double percentage = (numConfigs == 1) ? 0.0 : (double)i / (numConfigs - 1);
        spectrum.push_back(percentageToColor(percentage));
    }
    
    return spectrum;
}
}