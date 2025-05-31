#include "graphics.h"

// Use standard FLTK includes (let Homebrew set the include path)
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>

#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <unordered_map>

namespace graphics {

// Error handling implementation
ErrorException::ErrorException(const std::string& msg) : mMsg(msg) {}

std::string ErrorException::getMessage() const {
    return mMsg;
}

const char* ErrorException::what() const noexcept {
    return mMsg.c_str();
}

void error(const std::string& msg) {
    throw ErrorException(msg);
}

// Color conversion utility
std::string colorToHex(int r, int g, int b) {
    std::stringstream ss;
    ss << "#" << std::hex << std::setfill('0') 
       << std::setw(2) << r
       << std::setw(2) << g
       << std::setw(2) << b;
    return ss.str();
}

// Convert hex color string to RGB values
void hexToRgb(const std::string& hex, unsigned char& r, unsigned char& g, unsigned char& b) {
    if (hex.size() != 7 || hex[0] != '#') {
        error("Invalid color format. Expected #RRGGBB");
    }
    
    try {
        r = std::stoi(hex.substr(1, 2), nullptr, 16);
        g = std::stoi(hex.substr(3, 2), nullptr, 16);
        b = std::stoi(hex.substr(5, 2), nullptr, 16);
    } catch (...) {
        error("Invalid color format. Expected #RRGGBB");
    }
}

// Custom drawing area class for FLTK
class DrawingArea : public Fl_Box {
public:
    DrawingArea(int x, int y, int w, int h) : Fl_Box(x, y, w, h) {
        box(FL_FLAT_BOX);
        color(FL_WHITE);
    }

    void draw() override {
        Fl_Box::draw();
        fl_push_clip(x(), y(), w(), h());
        
        // Draw all stored shapes
        std::lock_guard<std::mutex> lock(drawMutex);
        for (const auto& cmd : drawCommands) {
            cmd();
        }
        
        fl_pop_clip();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(drawMutex);
        drawCommands.clear();
        redraw();
    }

    void addDrawCommand(std::function<void()> command) {
        std::lock_guard<std::mutex> lock(drawMutex);
        drawCommands.push_back(command);
        redraw();
    }

private:
    std::vector<std::function<void()>> drawCommands;
    std::mutex drawMutex;
};

// Simple window implementation
class WindowImpl {
public:
    Fl_Double_Window* window;
    DrawingArea* drawArea;
    std::queue<Event> eventQueue;
    std::mutex eventMutex;
    std::string currentColor;
    bool shouldTerminateOnClose;
    
    WindowImpl(int width, int height, const std::string& title) {
        // Initialize FLTK
        Fl::visual(FL_DOUBLE | FL_RGB);
        
        window = new Fl_Double_Window(width, height, title.c_str());
        drawArea = new DrawingArea(0, 0, width, height);
        window->end();
        
        // Simple callback for closing
        window->callback([](Fl_Widget* w, void* data) {
            auto* impl = static_cast<WindowImpl*>(data);
            if (impl->shouldTerminateOnClose) {
                exit(0);
            }
        }, this);

        
        shouldTerminateOnClose = true;
        currentColor = BLACK;
        
        window->show();
        Fl::check();
    }
    
    ~WindowImpl() {
        if (window) {
            window->hide();
            delete window;
        }
    }


};

// Simple terminal implementation
class TerminalImpl {
public:
    Fl_Double_Window* window;
    Fl_Text_Display* display;
    Fl_Text_Buffer* buffer;
    std::queue<Event> eventQueue;
    std::mutex eventMutex;
    bool shouldTerminateOnClose;
    
    TerminalImpl(int width, int height, const std::string& title) {
        // Initialize FLTK
        Fl::visual(FL_DOUBLE | FL_RGB);
        
        window = new Fl_Double_Window(width, height, title.c_str());
        buffer = new Fl_Text_Buffer();
        display = new Fl_Text_Display(0, 0, width, height);
        display->buffer(buffer);
        window->end();
        
        // Simple callback for closing
        window->callback([](Fl_Widget* w, void* data) {
            auto* impl = static_cast<TerminalImpl*>(data);
            if (impl->shouldTerminateOnClose) {
                exit(0);
            }
        }, this);
        
        shouldTerminateOnClose = true;
        
        window->show();
        Fl::check();
    }
    
    ~TerminalImpl() {
        if (window) {
            window->hide();
            delete window;
        }
        delete buffer;
    }
};

// Window implementation
Window::Window(int width, int height, const std::string& title) : mImpl(new WindowImpl(width, height, title)) {
}

Window::~Window() = default;

void Window::setTerminateOnClose(bool terminate) {
    mImpl->shouldTerminateOnClose = terminate;
}

void Window::clear() {
    mImpl->drawArea->clear();
}

void Window::setColor(const std::string& color) {
    // Handle both predefined colors and hex format
    static const std::unordered_map<std::string, std::string> colorMap = {
        {"BLACK", BLACK},
        {"BLUE", BLUE},
        {"CYAN", CYAN},
        {"DARK_GRAY", DARK_GRAY},
        {"GRAY", GRAY},
        {"GREEN", GREEN},
        {"LIGHT_GRAY", LIGHT_GRAY},
        {"MAGENTA", MAGENTA},
        {"ORANGE", ORANGE},
        {"PINK", PINK},
        {"RED", RED},
        {"WHITE", WHITE},
        {"YELLOW", YELLOW}
    };
    
    auto it = colorMap.find(color);
    if (it != colorMap.end()) {
        mImpl->currentColor = it->second;
    } else if (color[0] == '#' && color.length() == 7) {
        mImpl->currentColor = color;
    } else {
        error("Invalid color: " + color);
    }
}

std::string Window::getColor() const {
    return mImpl->currentColor;
}

void Window::fillRect(int x, int y, int width, int height) {
    unsigned char r, g, b;
    hexToRgb(mImpl->currentColor, r, g, b);
    
    mImpl->drawArea->addDrawCommand([x, y, width, height, r, g, b]() {
        fl_color(r, g, b);
        fl_rectf(x, y, width, height);
    });
}

void Window::fillOval(int x, int y, int width, int height) {
    unsigned char r, g, b;
    hexToRgb(mImpl->currentColor, r, g, b);
    
    mImpl->drawArea->addDrawCommand([x, y, width, height, r, g, b]() {
        fl_color(r, g, b);
        fl_pie(x, y, width, height, 0, 360);
    });
}

void Window::fillCircle(int centerX, int centerY, int radius) {
    fillOval(centerX - radius, centerY - radius, radius * 2, radius * 2);
}

void Window::drawRect(int x, int y, int width, int height) {
    unsigned char r, g, b;
    hexToRgb(mImpl->currentColor, r, g, b);
    
    mImpl->drawArea->addDrawCommand([x, y, width, height, r, g, b]() {
        fl_color(r, g, b);
        fl_rect(x, y, width, height);
    });
}

void Window::drawOval(int x, int y, int width, int height) {
    unsigned char r, g, b;
    hexToRgb(mImpl->currentColor, r, g, b);
    
    mImpl->drawArea->addDrawCommand([x, y, width, height, r, g, b]() {
        fl_color(r, g, b);
        fl_arc(x, y, width, height, 0, 360);
    });
}

void Window::drawCircle(int centerX, int centerY, int radius) {
    drawOval(centerX - radius, centerY - radius, radius * 2, radius * 2);
}

void Window::drawLine(int x0, int y0, int x1, int y1) {
    unsigned char r, g, b;
    hexToRgb(mImpl->currentColor, r, g, b);
    
    mImpl->drawArea->addDrawCommand([x0, y0, x1, y1, r, g, b]() {
        fl_color(r, g, b);
        fl_line(x0, y0, x1, y1);
    });
}

void Window::drawLabel(const std::string& text, int x, int y) {
    unsigned char r, g, b;
    hexToRgb(mImpl->currentColor, r, g, b);
    
    mImpl->drawArea->addDrawCommand([text, x, y, r, g, b]() {
        fl_color(r, g, b);
        fl_draw(text.c_str(), x, y);
    });
}

int Window::getWidth() const {
    return mImpl->window->w();
}

int Window::getHeight() const {
    return mImpl->window->h();
}

bool Window::hasEvents() const {
    // Simplified version without real event handling for now
    return false;
}

Event Window::getEvent() {
    // Simplified version that just returns an empty event
    Event emptyEvent;
    emptyEvent.Type = EventType::None;
    return emptyEvent;
}

void Window::update() {
    mImpl->window->redraw();
    Fl::check();
}

bool Window::isOpen() const {
    return mImpl->window && mImpl->window->shown();
}



// Terminal implementation
Terminal::Terminal(int width, int height, const std::string& title) : mImpl(new TerminalImpl(width, height, title)) {
}

Terminal::~Terminal() = default;

void Terminal::setTerminateOnClose(bool terminate) {
    mImpl->shouldTerminateOnClose = terminate;
}

void Terminal::clear() {
    mImpl->buffer->text("");
    Fl::check();
}

void Terminal::setText(const std::string& text) {
    mImpl->buffer->text(text.c_str());
    Fl::check();
}

void Terminal::appendText(const std::string& text) {
    mImpl->buffer->append(text.c_str());
    Fl::check();
}

void Terminal::showCursor(bool show) {
    mImpl->display->show_cursor(show ? 1 : 0);
    Fl::check();
}

bool Terminal::hasEvents() const {
    // Simplified version without real event handling for now
    return false;
}

Event Terminal::getEvent() {
    // Simplified version that just returns an empty event
    Event emptyEvent;
    emptyEvent.Type = EventType::None;
    return emptyEvent;
}

bool Terminal::isOpen() const {
    return mImpl->window && mImpl->window->shown();
}

void pause(double milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>(milliseconds)));
}

void drawShapeWithText(Window& window, const std::string& text, 
    int centerX, int centerY, int width, int height, 
    bool isSquare, 
    const std::string& fillColor,
    const std::string& borderColor,
    const std::string& textColor) {
// Calculate top-left corner from center point
int x = centerX - width / 2;
int y = centerY - height / 2;

// Draw the shape with fill color
window.setColor(fillColor);
if (isSquare) {
window.fillRect(x, y, width, height);
} else {
window.fillOval(x, y, width, height);
}

// Draw the border
window.setColor(borderColor);
if (isSquare) {
window.drawRect(x, y, width, height);
} else {
window.drawOval(x, y, width, height);
}

// Draw centered text
window.setColor(textColor);

// Better text positioning
int charWidth = 7;  // More accurate for monospace
int textWidth = text.length() * charWidth;
int textHeight = 14;

// Ensure text fits within shape
if (textWidth > width - 4) {
// Text too wide, just center what we can
textWidth = width - 4;
}

int textX = centerX - textWidth / 2;
int textY = centerY + textHeight / 4;  // Slight adjustment for baseline

window.drawLabel(text, textX, textY);
}


// int widthOfTextBox(const std::string& text, int padding){
//     // More generous text width estimate
    
//     int charWidth = 12;  
//     int textWidth = text.length() * charWidth;

//     // Add minimum width to prevent too-thin boxes
//     return textWidth + (padding * 2);
// }


// void drawShapeAroundText(Window& window, const std::string& text,
//     int centerX, int centerY, int height, 
//     const std::string& fillColor,
//     int padding,
//     bool isSquare,
//     const std::string& borderColor,
//     const std::string& textColor) {
//         // More generous text width estimate
//         int charWidth = text.length() < 2 ? 5 : text.length() < 3 ? 7 : 12;  
//         int textWidth = text.length() * charWidth;

//         // Add minimum width to prevent too-thin boxes
//         int width = std::max(textWidth + (padding * 2), 50);

//         // Calculate top-left corner
//         int x = centerX - width / 2;
//         int y = centerY - height / 2;

//         // Draw shape
//         window.setColor(fillColor);
//         if (isSquare) {
//         window.fillRect(x, y, width, height);
//         } else {
//         window.fillOval(x, y, width, height);
//         }

//         // Draw border
//         window.setColor(borderColor);
//         if (isSquare) {
//         window.drawRect(x, y, width, height);
//         } else {
//         window.drawOval(x, y, width, height);
//         }

//         // Draw text - properly centered
//         window.setColor(textColor);
//         int textX = (centerX - (text.length() * charWidth) / 2) + (charWidth * (charWidth > 7));  // Center the text
//         int textY = centerY + 5;   // FLTK draws from baseline

//         window.drawLabel(text, textX, textY);
// }

int widthOfTextBox(const std::string& text, int padding) {
    fl_font(FL_HELVETICA, 14); // Set font and size
    int textWidth = fl_width(text.c_str());
    return textWidth + (padding * 2);
}

void drawShapeAroundText(Window& window, const std::string& text,
    int centerX, int centerY, int height,
    const std::string& fillColor,
    int padding,
    bool isSquare,
    const std::string& borderColor,
    const std::string& textColor) {

    fl_font(FL_HELVETICA, 14); // Set font and size
    int textWidth = fl_width(text.c_str());
    int width = std::max(textWidth + (padding * 2), 50);

    int x = centerX - width / 2;
    int y = centerY - height / 2;

    window.setColor(fillColor);
    if (isSquare) window.fillRect(x, y, width, height);
    else window.fillOval(x, y, width, height);

    window.setColor(borderColor);
    if (isSquare) window.drawRect(x, y, width, height);
    else window.drawOval(x, y, width, height);

    window.setColor(textColor);
    int textX = centerX - textWidth / 2;
    int textY = centerY + height / 4; // baseline adjustment
    window.drawLabel(text, textX, textY);
}

} // namespace graphics
