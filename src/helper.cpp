#include "helper.h"

using std::getline;
using std::map;

namespace helper{
    Configuration::Configuration(){}

    Configuration::Configuration(const string& nm, Symbol rd, const Symbol wt, const Direction d, string nxt):
    name(nm),
    readSymbol(rd), 
    writeSymbol(wt), 
    direction(d), 
    nextState(nxt),
    signature(name + "{\'" + toStr.at(readSymbol) + "\'}")
    {}

    void Configuration::draw(Window* window, unsigned x, unsigned y, unsigned height, double mvP){
        int padding = 27;
        //nc
        string dNc = any_cast<bool>(window->params.at("protein_mode")) ? sdNC : nextState;
        int widNC = widthOfTextBox(dNc, padding);
        //mv
        int widMV = widthOfTextBox(sdMV, padding);
        //ws
        string s(1, toStr.at(writeSymbol));
        string dWs = any_cast<bool>(window->params.at("protein_mode")) ? sdWS : "\"" + s + "\"";
        int widWS = widthOfTextBox(dWs, padding); 
        //sig
        string dSig = any_cast<bool>(window->params.at("protein_mode")) ? sdSig : signature;
        int widSig = widthOfTextBox(dSig, padding);


        // signature
        if (mvP < 0.48){
            drawShapeAroundText(*window, dSig, x , y, height, color, padding, 14, false);
            x += widSig/2.0 + widWS/2.0;
        }

        // write symbol
        if (mvP < 0.69){
            drawShapeAroundText(*window, dWs, x, y, height, color, padding, 14, false); 
            x += widWS/2.0 + widMV/2.0;
        }

        // move direction
        if (mvP < 0.9){
            drawShapeAroundText(*window, sdMV, x, y, height, color, padding, 14, false);
            x += widMV/2.0 + widNC/2.0;
        }

        // next config
        drawShapeAroundText(*window, dNc, x, y, height + (2*height * (mvP > 0.9)), color, padding + (2*padding * (mvP > 0.9)), 14);

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

    vector<string> generateColorSpectrum(vector<Configuration*> confs, map<string, unordered_map<Symbol, Configuration*>*> ssc) {
        vector<string> spectrum;
        for (int i = 0; i < confs.size(); i++) {
            // double percentage = (numConfigs == 1) ? 0.0 : (double)i / (numConfigs - 1);
            
            double percentage = (1.0 / (double)ssc.size()) * (confs[i]->stateIndex + 0.5);
            spectrum.push_back(percentageToColor(percentage));
        }
        
        return spectrum;
    }   
}