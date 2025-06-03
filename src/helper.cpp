#include "helper.h"

using std::getline;

namespace helper{
    Configuration::Configuration(const unsigned idx, const string& nm, Symbol rd, const Symbol wt, const Direction d, string nxt):
    index(idx),
    name(nm),
    readSymbol(rd), 
    writeSymbol(wt), 
    direction(d), 
    nextState(nxt),
    signature(name + "{\'" + toStr.at(readSymbol) + "\'}")
    {}

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

    string interpolateColor(const std::string& color1, const std::string& color2, double t) {
        RGB c1(color1);
        RGB c2(color2);
        
        int r = c1.r + (int)((c2.r - c1.r) * t);
        int g = c1.g + (int)((c2.g - c1.g) * t);
        int b = c1.b + (int)((c2.b - c1.b) * t);
        
        return rgbToHex(r, g, b);
    }

    vector<string> generateColorSpectrum(int numConfigs){
        vector<string> spectrum;
        
        // Define key colors for biological look
        vector<string> keyColors = {
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
}