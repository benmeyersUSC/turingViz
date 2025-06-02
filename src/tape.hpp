#include <string>
#include <unordered_map>

#include "/Users/benmeyers/Desktop/turingViz/src/util.hpp"
#include "/Users/benmeyers/Desktop/turingViz/src/drawable.hpp"

using std::string;
using std::unordered_map;


class Tape : public Drawable{

    private:
    char* values;
    unsigned head;
    unsigned size;
    string tapeFill;

    public:
    unordered_map<unsigned, string> cellColors;
    unsigned cellsInUse;


    unsigned getHead(){return head;}
    
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

    void draw(graphics::Window& window){
        unsigned midX = window.getWidth()/2;
        unsigned midY = window.getHeight()/2;
        unsigned tapeY = window.getHeight()/2;
        unsigned squareWid = window.getHeight()/10;
        unsigned squareHi = squareWid;
        float scannedSquareMult = 1.25;

        // squares on either side
        int flank = 1 + ((window.getWidth() - (squareWid*(scannedSquareMult + 4)))/2.0)/squareWid;

        // current square
        graphics::drawShapeWithText(window, tape.readStr(), x, y, sqWid*mult, sqHi*mult, true, 
                            tape.cellColors.at(squarePos));

        // head      
        int sigWid = graphics::widthOfTextBox(config.sdSig, 3);
        graphics::drawShapeAroundText(window, config.sdSig, 
            // y: - scann sq height - half my own height
            x, y - (mult*sqHi*0.5) - window.getHeight() * 0.0175, window.getHeight() * 0.035, sigToColor.at(config.signature), 3);
        
        // index-based signature       
        string indexBasedSig = "Q" + std::to_string(config.index) + "{'S" + std::to_string(symInd.at(config.readSymbol)) + "'} ";            
        int ibSigWid = graphics::widthOfTextBox(indexBasedSig, 0);                                                    
        graphics::drawShapeWithText(window, indexBasedSig, x, y - (mult*sqHi*0.5) - window.getHeight() * 0.05, std::max(ibSigWid, sigWid), window.getHeight() * 0.035, true, sigToColor.at(config.signature));
        
        // human signature                                                                                    
        string humanSig = config.signature + " ";            
        int humSigWid = graphics::widthOfTextBox(humanSig, 0);                                                    
        graphics::drawShapeWithText(window, humanSig, x, y - (mult*sqHi*0.5) - window.getHeight() * 0.085, std::max(humSigWid, sigWid), window.getHeight() * 0.035, true, sigToColor.at(config.signature));

        // edges
        stringstream rs;
        stringstream ls;
        rs << "   ... << [" << tape.getHead() - 1 << "]...";
        ls << "   ...[" <<tape.getSize() - tape.getHead() << "] >> ...";

        for (unsigned i = 0; i <= flank; i++){
            if (i!= flank){
                // actual squares, i to the right and left
                graphics::drawShapeWithText(window, tape.readStr(-i), 
                    x-((int)(sqWid*mult))-(sqWid*(std::max(0, int(i-1)))), 
                y, sqWid, sqHi, true, tape.cellColors.at(std::max((int)(squarePos - i), 0)));

                graphics::drawShapeWithText(window, tape.readStr(i), 
                    x+((int)(sqWid*mult))+(sqWid*(std::max(0, int(i-1)))), 
                y, sqWid, sqHi, true,tape.cellColors.at(std::min(squarePos + i, tape.getSize()-1)));
                }
            else{
                // side messages
                graphics::drawShapeWithText(window, rs.str(), 
                    sqWid,
                y, sqWid*2, sqHi, true, tape.cellColors.at(std::max((int)(squarePos - i), 0)));

                graphics::drawShapeWithText(window, ls.str(), 
                    window.getWidth() - sqWid,
                y, sqWid*2, sqWid, true, tape.cellColors.at(std::min(squarePos + i, tape.getSize()-1)));
            }
        }
    }
};