#include "tape.h"

unsigned Tape::getHead(){return head;}

Tape::Tape() : head(0), size(54), values(new char[54]), tapeFill("S_") {
    for (unsigned i = 0; i < size; i++) {
        values[i] = toStr.at(S_);
        cellColors[i] = DEFAULT_COLOR;
    }
}

Tape::~Tape() {
    delete[] values;
}

Tape::Tape(const Tape& other) : head(other.head), size(other.size), tapeFill(other.tapeFill) {
    values = new char[size]; 
    for (unsigned i = 0; i < size; i++) {
        values[i] = other.values[i]; 
        cellColors[i] = DEFAULT_COLOR;
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
                cellColors[i] = DEFAULT_COLOR;
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
            cellColors[i] = DEFAULT_COLOR;
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

unsigned Tape::getCellsInUse(){
    return cellsInUse;
}

void Tape::incrCellsInUse(){
    cellsInUse++;
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

void Tape::draw(Window* window, Configuration* config, unsigned x, unsigned y, unsigned wWidth, unsigned wHeight, double mult, double iterPercent){  
    Direction lastMv = config->direction;
    
    iterPercent = iterPercent > 0.54 ? 1.0 : iterPercent/0.54;

    // square size
    unsigned squareSize = wHeight/10.0;

    // grayness
    drawShapeWithText(*window, "", x, y, wWidth, squareSize * mult * mult, true, DARK_GRAY);

    // squares on either side
    int flank = ((wWidth - (squareSize * mult)) / 2.0) / squareSize;

    int sigWid = widthOfTextBox(config->sdSig, 3);
    
    // index-based signature       
    string indexBasedSig = "Q" + to_string(config->stateIndex) + "{'S" + to_string(symInd.at(config->readSymbol)) + "'} ";            
    int ibSigWid = widthOfTextBox(indexBasedSig, 0);                                                    
    drawShapeWithText(*window, indexBasedSig, x, y - (mult*squareSize*0.5) - wHeight * 0.05, 
                std::max(ibSigWid, sigWid), wHeight * 0.035, true, config->color);
    
    // edges
    stringstream rs;
    stringstream ls;
    rs << "   ... << [" << getHead() - 1 << "]...";
    ls << "   ...[" << getSize() - getHead() << "] >> ...";

    x = lastMv == LEFT ? x - (squareSize * (1.0 - iterPercent)) : lastMv == RIGHT ? x + (squareSize * (1.0 - iterPercent)) : x;

    for (unsigned i = 1; i <= flank; i++){
        if (i!= flank){
            // actual squares, i to the right and left
            drawShapeWithText(*window, readStr(-i), 
                x-(squareSize*mult*0.5)-(squareSize*0.75)-(squareSize*(i-1)), 
            y, squareSize, squareSize, true, cellColors.at(std::max((int)(getHead() - i), 0)));

            drawShapeWithText(*window, readStr(i), 
                x+(squareSize*mult*0.5)+(squareSize*0.75)+(squareSize*(i-1)), 
            y, squareSize, squareSize, true, cellColors.at(std::min(getHead() + i, getSize()-1)));
            }
        else{
            // side messages
            drawShapeWithText(*window, rs.str(), 
                squareSize,
            y, squareSize*2, squareSize, true, cellColors.at(std::max((int)(getHead() - i), 0)));

            drawShapeWithText(*window, ls.str(), 
                window->getWidth() - squareSize,
            y, squareSize*2, squareSize, true, cellColors.at(std::min(getHead() + i, getSize()-1)));
        }
    }

    x = lastMv == LEFT ? x + (squareSize * (1.0 - iterPercent)) : lastMv == RIGHT ? x - (squareSize * (1.0 - iterPercent)) : x;

    drawShapeWithText(*window, "", x, y, squareSize*mult + squareSize/2.0, squareSize*mult*mult, true, config->color);

    // current square
    drawShapeWithText(*window, readStr(), x, y, squareSize*mult, squareSize*mult, true, cellColors.at(getHead()));
    
    // head      
    string headDraw = any_cast<bool>(window->params.at("protein_mode")) ? config->sdSig : config->signature;
    drawShapeAroundText(*window, headDraw, 
        // y: - scann sq height - half my own height
        x, y - (mult*squareSize*0.5) - wHeight * 0.0175, wHeight * 0.035, config->color, 3);
    
}

void Tape::drawWhole(Window* window, Configuration* config, unsigned wWidth, unsigned wHeight, double heightMult, double iterPercent){
    Direction lastMv = config->direction;
    iterPercent = iterPercent > 0.54 ? 1.0 : iterPercent/0.54;
    
    unsigned wid = wWidth/cellsInUse;

    // moving head:
    string headthing;
    int headX;
    if (getHead() < cellsInUse) {
        // Head is within the visible cells - show exact position
        headthing = "HEAD ";
        headX = (getHead() + 0.5) * wid;
    } else {
        // Head is beyond visible cells - show arrow at right edge
        headthing = "HEAD >> ";
        headX = wWidth - wid/2.0;
    }
    // min size
    int cappedWid = max(wid, (unsigned)(12 * headthing.size()));
    headX = lastMv == LEFT ? headX + (cappedWid * (1.0 - iterPercent)) : lastMv == RIGHT ? headX - (cappedWid * (1.0 - iterPercent)) : headX;
    drawShapeWithText(*window, headthing, headX, wHeight * (1.0 - 5 * heightMult), cappedWid, wHeight * heightMult * 0.5, true, config->color);

    for (unsigned i = 0; i < cellsInUse; i++){
        // config-stained view
        window->setColor(cellColors.at(i));
        window->fillRect(i * wid, wHeight * (1.0 - 4.0*heightMult), wid, wHeight * heightMult);
        
        window->setColor(BLACK);
        window->drawRect(i * wid, wHeight * (1.0 - 4.0*heightMult), wid, wHeight * heightMult);

        string stainViewTitle = "Tape Squares By Most Recent Configuration:";
        double widthStainText = widthOfTextBox(stainViewTitle, 3) / 2.0;
        drawShapeAroundText(*window, stainViewTitle, widthStainText, wHeight * (1.0 - 4.25*heightMult), wHeight*heightMult*0.5, DEFAULT_COLOR, 3, 14, true, DEFAULT_COLOR, BLACK);


        // binary view
        if (toStr.at(readAt(i)) == '0'){
            window->setColor(WHITE);
        }
        else if (toStr.at(readAt(i)) == '1'){
            window->setColor(BLACK);
        }
        else{
            window->setColor(LIGHT_GRAY);
        }
        if (i == getHead() && iterPercent >= 1.0){
            window->setColor(config->color);
        }
        window->fillRect(i * wid, wHeight * (1.0 - 2.5*heightMult), wid, wHeight * heightMult);
    
        window->setColor(BLACK);
        window->drawRect(i * wid, wHeight * (1.0 - 2.5*heightMult), wid, wHeight * heightMult);

       
        double width1 = widthOfTextBox("Computed Numbers:", 3) / 2.0;
        int wordsX = width1;
        drawShapeAroundText(*window, "Computed Numbers:", wordsX, wHeight * (1.0 - 2.75*heightMult), wHeight*heightMult*0.5, DEFAULT_COLOR, 0, 14, true, DEFAULT_COLOR);
    
        double width2 = widthOfTextBox("1s", 9) / 2.0;
        wordsX += width1 + width2;
        drawShapeAroundText(*window, "1", wordsX, wHeight * (1.0 - 2.75*heightMult), wHeight*heightMult*0.5, BLACK, 9, 14, true, DEFAULT_COLOR, WHITE);
    
        wordsX += 2 * width2;
        drawShapeAroundText(*window, "0", wordsX, wHeight * (1.0 - 2.75*heightMult), wHeight*heightMult*0.5, WHITE, 9, 14, true, DEFAULT_COLOR, BLACK);
    }
}
