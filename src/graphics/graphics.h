#pragma once

#include <string>
#include <memory>
#include <functional>
#include <exception>

namespace graphics {

// Type of events we support
enum class EventType {
    None,
    KeyDown,
    KeyUp,
    MouseBtnDown,
    MouseBtnUp
};

// Key codes for non a-z or 0-9 keys we support
enum class KeyCode {
    Backspace = 0xff08,
    Enter = 0xff0d,
    Escape = 0xff1b,
    Space = 32,
    Left = 0xff51,
    Up = 0xff52,
    Right = 0xff53,
    Down = 0xff54,
    F1 = 0xffbe,
    F2 = 0xffbf,
    F3 = 0xffc0,
    F4 = 0xffc1,
    F5 = 0xffc2,
    F6 = 0xffc3,
    F7 = 0xffc4,
    F8 = 0xffc5,
    F9 = 0xffc6,
    F10 = 0xffc7,
    F11 = 0xffc8,
    F12 = 0xffc9,
};

// Supported mouse buttons
enum class MouseButton {
    Left = 1,
    Middle = 2,
    Right = 3
};

// Simple struct for handling basic events
struct Event {
    EventType Type;
    union {
        struct {
            int Code;
        } Key;
        struct {
            int Button;
            int X;
            int Y;
        } Mouse;
    } Event;
};

// Operator overloads for convenient event handling
inline bool operator==(int i, KeyCode c) {
    return i == static_cast<int>(c);
}

inline bool operator==(KeyCode c, int i) {
    return i == c;
}

inline bool operator==(int i, MouseButton c) {
    return i == static_cast<int>(c);
}

inline bool operator==(MouseButton c, int i) {
    return i == c;
}


// ... (rest of your enums, structs, and class declarations as before) ...

// Exception class for error handling
class ErrorException : public std::exception {
public:
    ErrorException(const std::string& msg);
    virtual std::string getMessage() const;
    virtual const char* what() const noexcept override;
private:
    std::string mMsg;
};

void error(const std::string& msg);

// Color constants
inline const std::string BLACK = "#000000";
inline const std::string BLUE = "#0000FF";
inline const std::string CYAN = "#00FFFF";
inline const std::string DARK_GRAY = "#555555";
inline const std::string GRAY = "#888888";
inline const std::string GREEN = "#00FF00";
inline const std::string LIGHT_GRAY = "#CCCCCC";
inline const std::string MAGENTA = "#FF00FF";
inline const std::string ORANGE = "#FFA500";
inline const std::string PINK = "#FFC0CB";
inline const std::string RED = "#FF0000";
inline const std::string WHITE = "#FFFFFF";
inline const std::string YELLOW = "#FFFF00";

// Forward declarations
class WindowImpl;
class TerminalImpl;

class Window {
public:
    Window(int width, int height, const std::string& title = "Graphics Window");
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    void setTerminateOnClose(bool terminate);
    void clear();
    void setColor(const std::string& color);
    std::string getColor() const;
    void fillRect(int x, int y, int width, int height);
    void fillOval(int x, int y, int width, int height);
    void fillCircle(int centerX, int centerY, int radius);
    void drawRect(int x, int y, int width, int height);
    void drawOval(int x, int y, int width, int height);
    void drawCircle(int centerX, int centerY, int radius);
    void drawLine(int x0, int y0, int x1, int y1);
    void drawLabel(const std::string& text, int x, int y);
    int getWidth() const;
    int getHeight() const;
    bool hasEvents() const;
    struct Event getEvent();
    void update();
    bool isOpen() const;
private:
    std::unique_ptr<WindowImpl> mImpl;
};

class Terminal {
public:
    Terminal(int width, int height, const std::string& title = "Terminal Window");
    ~Terminal();
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
    void setTerminateOnClose(bool terminate);
    void clear();
    void setText(const std::string& text);
    void appendText(const std::string& text);
    void showCursor(bool show);
    bool hasEvents() const;
    struct Event getEvent();
    bool isOpen() const;
private:
    std::unique_ptr<TerminalImpl> mImpl;
};

// Utility functions
void pause(double milliseconds);
std::string colorToHex(int r, int g, int b);

void drawShapeWithText(Window& window, const std::string& text, 
    int centerX, int centerY, int width, int height, 
    bool isSquare = true, 
    const std::string& fillColor = graphics::WHITE,
    const std::string& borderColor = graphics::BLACK,
    const std::string& textColor = graphics::BLACK);

int widthOfTextBox(const std::string& text, int padding);

void drawShapeAroundText(Window& window, const std::string& text,
    int centerX, int centerY, int height, const std::string& fillColor,
    int padding = 0,
    bool isSquare = true,
    const std::string& borderColor = graphics::BLACK,
    const std::string& textColor = graphics::BLACK);

} // namespace graphics