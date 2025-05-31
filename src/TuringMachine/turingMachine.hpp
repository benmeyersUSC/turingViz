#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <ostream>
#include <algorithm>

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

// Helper to convert hex color to RGB components
struct RGB {
    int r, g, b;
    RGB(const std::string& hex) {
        r = std::stoi(hex.substr(1, 2), nullptr, 16);
        g = std::stoi(hex.substr(3, 2), nullptr, 16);
        b = std::stoi(hex.substr(5, 2), nullptr, 16);
    }
};

// Convert RGB back to hex string
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

enum Symbol{
    S_, S0, S1, R, L, N, SENTINEL, X, Y, Z, ASTR, Q, A, S, T, U, V
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

class Tape{

    private:

    char* values;
    unsigned head;
    unsigned size;
    string tapeFill;
    public:
    unordered_map<unsigned, string> cellColors;
    unsigned cellsInUse;


    unsigned getHead(){return head;}

    Tape(unsigned sz, const string tf) : head(0), size(sz), values(new char[sz]), tapeFill(tf) {
        for (unsigned i = 0; i < size; i++) {
            values[i] = toStr.at(S_);
            cellColors[i] = graphics::WHITE;
        }
    }
    
    Tape(const string tf) : head(0), size(54), values(new char[54]), tapeFill(tf) {
        for (unsigned i = 0; i < size; i++) {
            values[i] = toStr.at(S_);
            cellColors[i] = graphics::WHITE;
        }
    }

    Tape(unsigned sz) : head(0), size(sz), values(new char[54]), tapeFill("S_") {
        for (unsigned i = 0; i < size; i++) {
            values[i] = toStr.at(S_);
            cellColors[i] = graphics::WHITE;
        }
    }
    
    Tape() : head(0), size(54), values(new char[54]), tapeFill("S_") {
        for (unsigned i = 0; i < size; i++) {
            values[i] = toStr.at(S_);
            cellColors[i] = graphics::WHITE;
        }
    }
    
    ~Tape() {
        delete[] values;
    }
    
    Tape(const Tape& other) : head(other.head), size(other.size), tapeFill(other.tapeFill) {
        values = new char[size]; 
        for (unsigned i = 0; i < size; i++) {
            values[i] = other.values[i]; 
            cellColors[i] = graphics::WHITE;
        }
    }
    
    Tape& operator=(const Tape& other) {
        if (this != &other) { 
            delete[] values; 
            
            head = other.head;
            size = other.size;
            tapeFill = other.tapeFill;
            
            values = new char[size]; 
            for (unsigned i = 0; i < size; i++) {
                values[i] = other.values[i]; 
            }
        }
        return *this;
    }

    Symbol read(){
        char vl = values[head];
        for (const auto& pair : toStr){
            if (pair.second == vl){return pair.first;}
        }
        return toSym.at("S_");
    }

    Symbol readAt(unsigned i){
        char vl = values[i];
        for (const auto& pair : toStr){
            if (pair.second == vl){return pair.first;}
        }
        return toSym.at("S_");
    }

    string readStr(int displacement = 0){
        return string(1, values[std::min(std::max(((int)head + displacement), 0), (const int) size)]);
    }

    void write(Symbol s){
        values[head] = toStr.at(s);
    }

    void right(){
        if (head + 1 == size){
            size += 10;
            char* newArr = new char[size];
            for (unsigned i = 0; i < size; i++){
                if (i < size - 10){
                    newArr[i] = values[i];
                }
                else{
                    newArr[i] = toStr.at(toSym.at(tapeFill));
                    cellColors[i] = graphics::WHITE;
                }
            }
            delete[] values;
            values = newArr;
        }
        head++;
    }

    void left(){
        if (head == 0){
            size += 10;
            char* newArr = new char[size];
            
            for (int i = 0; i < 10; i++){
                newArr[i] = toStr.at(toSym.at(tapeFill));
                cellColors[i] = graphics::WHITE;
            }
            
            for (unsigned i = 10; i < size; i++){
                newArr[i] = values[i-10];
            }
            
            unordered_map<unsigned, string> newColors;
            for (auto& [idx, color] : cellColors) {
                if (idx < size - 10) {
                    newColors[idx + 10] = color;
                }
            }
            cellColors = newColors;
            
            delete[] values;
            values = newArr;
            head = 9;
        }
        else{
            head--;
        }
    }

    unsigned getSize(){
        return size;
    }

    friend ostream& operator<<(ostream& ss, const Tape& tp){
        ss << tp.toString(tp.size, 1);
        return ss;
    }

    string toString(unsigned len, unsigned step) const {
        stringstream ss;
        
        if (len >= size) {
            ss << '|';
            
            for (unsigned i = 0; i < size; i += step) {
                if (i == head) {
                    ss << "{\\  " << values[i] << "  /}";
                } else {
                    ss << values[i];
                }
                ss << '|';
            }
            
            return ss.str();
        }
        
        unsigned halfLen = len / 2;
        unsigned start = (head > halfLen) ? (head - halfLen) : 0;
        unsigned end = std::min(head + halfLen + 1, size);
        
        if (start > 0) {
            ss << "......[" << start << "]......";
        }
        
        ss << '|';
        
        for (unsigned i = start; i < end; i += step) {
            if (i == head) {
                ss << "{\\  " << values[i] << "  /}";
            } else {
                ss << values[i];
            }
            ss << '|';
        }
        
        if (end < size) {
            ss << "......[" << (size - end) << "]......";
        }
        
        return ss.str();
    }

};

class TM{
    const unsigned MAX_TAPE = 999; 

    enum Direction{
        LEFT, RIGHT, NONE
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

    private:

    unordered_map<string, unordered_map<Symbol, Configuration>> head;
    string currentState;
    Tape& tape;    
    unsigned sizeLimit;

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

    TM(Tape& tp) : tape(tp), sizeLimit(999) {}

    TM(Tape& tp, unsigned szLmt) : tape(tp), sizeLimit(szLmt) {}

    ~TM() {}

    static TM* fromStandardDescription(string description, Tape& tape, unsigned szLmt){
        bool foundInit = false;
        TM* utm = new TM(tape, szLmt);

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
                Direction direction = p3trim == "R" ? RIGHT : p3trim == "L" ? LEFT : NONE;

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
                utm->sdSignatures.push_back(config.sdSig);
                utm->SDNum.push_back(config.sdNum);
            }
            catch(std::invalid_argument e){
                throw new std::invalid_argument("Invalid symbol!");
            }
        }
        return utm;
    }

    static TM* fromStandardDescription(fstream& file, Tape& tp, unsigned szLmt){
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

    void run(){
        cout << "Starting TM run..." << endl;
        cout << "  Initial head position: " << tape.getHead() << endl;
        cout << "  Initial tape state: " << tape.toString(20, 1) << endl;

        unsigned steps = 0;

        while (currentState != "HALT" && tape.getSize() < sizeLimit){
            cout << tape.toString(10, 1) << endl;
            Symbol currentSymbol = tape.read();

            Configuration configuration = head.at(currentState).at(currentSymbol);
            
            tape.write(configuration.writeSymbol);

            if (configuration.direction == LEFT){
                tape.left();
            }
            else if (configuration.direction == RIGHT){
                tape.right();
            }

            currentState = configuration.nextConfig;
            steps++;
        }
        cout << "Halting...Steps taken: " << steps << endl;
    }

    void runStepwise(int step){
        unsigned steps = 0;
       
        while (currentState != "HALT" && tape.getSize() < sizeLimit){
            
            Symbol currentSymbol = tape.read();

            Configuration configuration = head.at(currentState).at(currentSymbol);

            if(steps % step == 0){
                cout << "@ " << steps << ": SIGNATURE = " << configuration.signature << endl;
                cout << tape.toString(10, 1) << endl << endl;
                cin.get();
            }
            
            tape.write(configuration.writeSymbol);

            if (configuration.direction == LEFT){
                tape.left();
            }
            else if (configuration.direction == RIGHT){
                tape.right();
            }

            currentState = configuration.nextConfig;
            steps++;
        }
        cout << "Halting...Steps taken: " << steps << endl;
    }

    void runStepWiseWindow(unsigned pauze = 300, unsigned wWidth = 1430, unsigned wHeight = 810){
        unsigned steps = 0;
        graphics::Window window(wWidth, wHeight, "Turing Machine Visualization");
        initializeColors((int)(window.getWidth()));
        window.clear();
        unsigned midX = window.getWidth()/2;
        unsigned midY = window.getHeight()/2;
        unsigned tapeY = 5 * (window.getHeight()/6);
        graphics::pause(100);

        string lastState = "null";
        int animator = 0;
        int animatorLoop = 27;
        while (currentState != "HALT" && tape.getSize() < sizeLimit && window.isOpen()){
            animator++;
            Symbol currentSymbol = tape.read();

            Configuration configuration = head.at(currentState).at(currentSymbol);

            stringstream prnt;
            prnt << tape.toString(99, 1);
            window.clear();
            vizTape(window, midX, tapeY, tape.getHead(), configuration);
            vizRunStats(window, steps, tape.getHead(), midX, lastState);
            vizGenome(window, configuration.signature, sdifySig(configuration));
            vizWholeTape(window);
            if (animator % animatorLoop != 0){
                vizBinding(window, configuration, midX, ((animator%animatorLoop)/((double)animatorLoop)));
            }
            window.update();
            
            if(animator % animatorLoop == 0){
                graphics::pause(pauze);
                tape.write(configuration.writeSymbol);

                if (tape.cellColors[tape.getHead()] == graphics::WHITE){tape.cellsInUse++;}
                tape.cellColors[tape.getHead()] = sigToColor.at(configuration.signature);

                if (configuration.direction == LEFT){
                    tape.left();
                }
                else if (configuration.direction == RIGHT){
                    tape.right();
                }

                lastState = currentState + " {\"" + tape.readStr() + "\"}";
                currentState = configuration.nextConfig;
                steps++;
            }
            else{
                graphics::pause(pauze*(1/animatorLoop));
            }
        }
    }

    void vizTape(graphics::Window& window, unsigned x, unsigned y, unsigned squarePos, Configuration config){
        int wid = 51;
        int hi = 51;
        int flank = 11;

        // current square
        graphics::drawShapeWithText(window, tape.readStr(), x, y, (int)(wid*1.25), (int)(hi*1.25), true, tape.cellColors.at(squarePos));  
        // head      
        // graphics::drawShapeWithText(window, currentState + " {\"" + tape.readStr() + "\"} = " + config.sdSig, x, y-((int)(1.25*hi)), (int)(wid*1.25) * 5, hi, true, sigToColor.at(config.signature));
        graphics::drawShapeAroundText(window, currentState + " {\"" + tape.readStr() + "\"} = " + config.sdSig, x, y-((int)(1.25*hi)), hi, sigToColor.at(config.signature));
        

        // edges
        stringstream rs;
        stringstream ls;
        rs << "   ... << [" << tape.getHead() - 1 << "]...";
        ls << "   ...[" <<tape.getSize() - tape.getHead() << "] >> ...";
        for (unsigned i = 0; i <= flank; i++){
            if (i!= flank){
                graphics::drawShapeWithText(window, tape.readStr(-i), x-((int)(wid*1.25))-(wid*(std::max(0, int(i-1)))), y, wid, hi, true, 
                            tape.cellColors.at(std::max((int)(squarePos - i), 0)));
                graphics::drawShapeWithText(window, tape.readStr(i), x+((int)(wid*1.25))+(wid*(std::max(0, int(i-1)))), y, wid, hi, true,
                            tape.cellColors.at(std::min(squarePos + i, tape.getSize()-1)));
                        }
            else{
                graphics::drawShapeWithText(window, rs.str(), x-((int)(wid*1.25))-(wid*(std::max(0, int(i-1)))) - wid, y, wid*2, hi, true, 
                            tape.cellColors.at(std::max((int)(squarePos - i), 0)));
                graphics::drawShapeWithText(window, ls.str(), x+((int)(wid*1.25))+(wid*(std::max(0, int(i-1)))) + wid, y, wid*2, hi, true,
                            tape.cellColors.at(std::min(squarePos + i, tape.getSize()-1)));
            }
        }

    }

    void vizRunStats(graphics::Window& window, int numIters, int sqarePos, unsigned midX, string lastState){
        stringstream ss;
        ss << "Iteration #" << numIters << ", on sqaure #" << sqarePos << ", last state: " << lastState;
        graphics::drawShapeWithText(window, ss.str(), midX, window.getHeight()-18, window.getWidth(), 25);
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

    void initializeColors(unsigned width) {
        std::vector<std::string> colors = generateColorSpectrum(signatures.size());
        
        for (size_t i = 0; i < signatures.size(); i++) {
            sigToColor[signatures[i]] = colors[i];
        }
        
        int configWidth = width / signatures.size();
        
        for (size_t i = 0; i < signatures.size(); i++) {
            scaleToGene[i] = i * configWidth + configWidth/2; // center of each slot
        }
    }

    void vizGenome(graphics::Window& window, const string& currSig, const string& currGene){
        int x;
        for (const string& s : standardDesctription){
            x += s.size();
        }

        stringstream ss;
        ss << "Turing Machine Genome: " << configs.size()-1 << " genes, " << x << " total nucleotides!";
        graphics::drawShapeWithText(window, ss.str(), window.getWidth()/2, 15, 540, 27);
        int widdy = (int) window.getWidth()/signatures.size();
        for (string s : signatures){
            graphics::drawShapeWithText(window, std::to_string(1+signatureToCongifIndex.at(s)), scaleToGene.at(sigToScale.at(s)), 45, widdy, 30, true, sigToColor.at(s));
            if (s == currSig){
                // graphics::drawShapeWithText(window, sdint(currGene), scaleToGene.at(sigToScale.at(s)), 75, 630, 30, true, sigToColor.at(s));
                graphics::drawShapeAroundText(window, sdint(currGene), scaleToGene.at(sigToScale.at(s)), 75, 30, sigToColor.at(s), 2);
                // graphics::drawShapeWithText(window, currGene, scaleToGene.at(sigToScale.at(s)), 105, 630, 30, true, sigToColor.at(s));
                graphics::drawShapeAroundText(window, currGene, scaleToGene.at(sigToScale.at(s)), 105, 30, sigToColor.at(s), 2);
                // graphics::drawShapeWithText(window, s, scaleToGene.at(sigToScale.at(s)), 135, widdy*5, 30, true, sigToColor.at(s));
                graphics::drawShapeAroundText(window, s, scaleToGene.at(sigToScale.at(s)), 135, 30, sigToColor.at(s), 2);
            }   
        }
    }

    void vizBinding(graphics::Window& window, const Configuration& config, unsigned midX, double iterPercent){
        string currSig = config.signature;
        string currGene = sdifySig(config);
        double realP = iterPercent > 0.6 ? 1.0 : iterPercent/0.6;
        int widdy = (int) window.getWidth()/signatures.size() * 5;
        int yBind = 4.5 * (window.getHeight()/6);
        int yAx = yBind - ((yBind-195)*(1-realP));
        int xAx;
        if (scaleToGene.at(sigToScale.at(currSig)) >= midX){
            xAx = scaleToGene.at(sigToScale.at(currSig)) - ((scaleToGene.at(sigToScale.at(currSig)) - midX) * realP);
        }
        else{
            xAx = scaleToGene.at(sigToScale.at(currSig)) + ((midX - scaleToGene.at(sigToScale.at(currSig))) * realP);
        }

        // signature
        int widSig = graphics::widthOfTextBox(currGene, 0);
        xAx += graphics::widthOfTextBox(currSig + " = ", 0)/2.0;
        graphics::drawShapeAroundText(window, currGene, xAx , yAx, 30, sigToColor.at(currSig));


    }

    void vizWholeTape(graphics::Window& window){
        int wid = (int)(window.getWidth()/tape.cellsInUse);

        for (unsigned i = 0; i < tape.cellsInUse; i++){
            if (i == tape.getHead()){
                window.fillRect(i * wid + (wid * 0.25), window.getHeight()-99, wid/2, 10);
            }
            window.setColor(tape.cellColors.at(i));
            window.fillRect(i * wid, window.getHeight()-90, wid, 27);
            window.setColor(graphics::BLACK);
            window.drawRect(i * wid, window.getHeight()-90, wid, 27);


            if (toStr.at(tape.readAt(i)) == '0'){
                window.setColor(graphics::LIGHT_GRAY);
            }
            else if (toStr.at(tape.readAt(i)) == '1'){
                window.setColor(graphics::BLACK);
            }
            else{
                window.setColor(dullerColor(tape.cellColors.at(i)));
                // window.setColor(graphics::WHITE);
            }
            window.fillRect(i * wid, window.getHeight()-62, wid, 27);
            window.setColor(graphics::BLACK);
            window.drawRect(i * wid, window.getHeight()-62, wid, 27);
        }
    }
};


/*

make the binding animation better

first: make a draw text function auto fit the element size around the text

second: helper fn to draw genomic thing (either signature or full one)...just split by D, then concatenate them as chunks

third: binding animation will actually show the segments coming apart, matching, fitting, staying!

*/