#include "tape.h"

using helper::toStr;
using helper::S_;
using helper::toSym;
using helper::symInd;

unsigned Tape::getHead(){return head;}

Tape::Tape() : head(0), size(54), values(new char[54]), tapeFill("S_") {
    for (unsigned i = 0; i < size; i++) {
        values[i] = toStr.at(S_);
        cellColors[i] = graphics::WHITE;
    }
}

Tape::~Tape() {
    delete[] values;
}

Tape::Tape(const Tape& other) : head(other.head), size(other.size), tapeFill(other.tapeFill) {
    values = new char[size]; 
    for (unsigned i = 0; i < size; i++) {
        values[i] = other.values[i]; 
        cellColors[i] = graphics::WHITE;
    }
}

Tape& Tape::operator=(const Tape& other) {
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

Symbol Tape::read(){
    char vl = values[head];
    for (const auto& pair : toStr){
        if (pair.second == vl){return pair.first;}
    }
    return toSym.at("S_");
}

Symbol Tape::readAt(unsigned i){
    char vl = values[i];
    for (const auto& pair : toStr){
        if (pair.second == vl){return pair.first;}
    }
    return toSym.at("S_");
}

string Tape::readStr(int displacement){
    return string(1, values[std::min(std::max(((int)head + displacement), 0), (const int) size)]);
}

void Tape::write(Symbol s){
    values[head] = toStr.at(s);
}

void Tape::right(){
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

void Tape::left(){
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

unsigned Tape::getSize(){
    return size;
}

ostream& operator<<(ostream& ss, const Tape& tp){
    ss << tp.toString(tp.size, 1);
    return ss;
}

string Tape::toString(unsigned len, unsigned step) const {
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

void Tape::draw(graphics::Window* window, Configuration* config){
    window->setColor(graphics::BLACK);
    window->fillRect(200, 200, 100, 100);    

    // unsigned x = window.getWidth()/2;
    // unsigned y = window.getHeight()/2;
    // // square size
    // unsigned dim = window.getHeight()/10;
    // // multiplier for scanned square
    // float mult = 1.25;

    // // squares on either side
    // int flank = 1 + ((window.getWidth() - (dim*(mult + 4)))/2.0)/dim;

    // // current square
    // graphics::drawShapeWithText(window, readStr(), x, y, dim*mult, dim*mult, true, cellColors.at(getHead()));

    // // head      
    // int sigWid = graphics::widthOfTextBox(config->sdSig, 3);
    // graphics::drawShapeAroundText(window, config->sdSig, 
    //     // y: - scann sq height - half my own height
    //     x, y - (mult*dim*0.5) - window.getHeight() * 0.0175, window.getHeight() * 0.035, config->color, 3);
    
    // // index-based signature       
    // string indexBasedSig = "Q" + std::to_string(config->index) + "{'S" + std::to_string(symInd.at(config->readSymbol)) + "'} ";            
    // int ibSigWid = graphics::widthOfTextBox(indexBasedSig, 0);                                                    
    // graphics::drawShapeWithText(window, indexBasedSig, x, y - (mult*dim*0.5) - window.getHeight() * 0.05, std::max(ibSigWid, sigWid), window.getHeight() * 0.035, true, config->color);
    
    // // human signature                                                                                    
    // string humanSig = config->signature + " ";            
    // int humSigWid = graphics::widthOfTextBox(humanSig, 0);                                                    
    // graphics::drawShapeWithText(window, humanSig, x, y - (mult*dim*0.5) - window.getHeight() * 0.085, std::max(humSigWid, sigWid), window.getHeight() * 0.035, true, config->color);

    // // edges
    // stringstream rs;
    // stringstream ls;
    // rs << "   ... << [" << getHead() - 1 << "]...";
    // ls << "   ...[" << getSize() - getHead() << "] >> ...";

    // for (unsigned i = 0; i <= flank; i++){
    //     if (i!= flank){
    //         // actual squares, i to the right and left
    //         graphics::drawShapeWithText(window, readStr(-i), 
    //             x-((int)(dim*mult))-(dim*(std::max(0, int(i-1)))), 
    //         y, dim, dim, true, cellColors.at(std::max((int)(getHead() - i), 0)));

    //         graphics::drawShapeWithText(window, readStr(i), 
    //             x+((int)(dim*mult))+(dim*(std::max(0, int(i-1)))), 
    //         y, dim, dim, true, cellColors.at(std::min(getHead() + i, getSize()-1)));
    //         }
    //     else{
    //         // side messages
    //         graphics::drawShapeWithText(window, rs.str(), 
    //             dim,
    //         y, dim*2, dim, true, cellColors.at(std::max((int)(getHead() - i), 0)));

    //         graphics::drawShapeWithText(window, ls.str(), 
    //             window.getWidth() - dim,
    //         y, dim*2, dim, true, cellColors.at(std::min(getHead() + i, getSize()-1)));
    //     }
    // }
}
