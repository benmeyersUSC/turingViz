// graphics_web.cpp -- HTML canvas implementation of graphics.h.
//
// graphics.h is backend-agnostic (colours are hex strings, events are a plain
// struct), so porting to the browser meant writing a second implementation of
// it rather than touching anything else. The FLTK backend in graphics.cpp is
// untouched and still builds the desktop app; this file is only compiled when
// targeting Emscripten.
//
// Drawing goes straight to a 2D context via EM_ASM. Events arrive the other
// way: JS listeners in the shell page call the exported push functions below,
// which fill a queue that viz.cpp drains through hasEvents()/getEvent()
// exactly as it does under FLTK.
#ifdef __EMSCRIPTEN__

#include "graphics.h"
#include <emscripten/emscripten.h>
#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>

namespace graphics {

// Window is pimpl'd, so the unique_ptr member needs a complete type at the
// destructor. The canvas backend keeps no per-window state -- everything lives
// on the JS side -- so this is deliberately empty.
class WindowImpl {};

// ---- error handling (same contract as the FLTK backend) ----
ErrorException::ErrorException(const std::string &msg) : mMsg(msg) {}
std::string ErrorException::getMessage() const { return mMsg; }
const char *ErrorException::what() const noexcept { return mMsg.c_str(); }
void error(const std::string &msg) { throw ErrorException(msg); }

namespace {

std::deque<Event> g_events;
std::string       g_color = BLACK;
int               g_width = 0, g_height = 0;

// The FLTK backend accepts either a name ("RED") or a hex string; match that.
const std::string &resolveColor(const std::string &c) {
    static const std::unordered_map<std::string, std::string> named = {
        {"BLACK", BLACK}, {"BLUE", BLUE}, {"CYAN", CYAN}, {"DARK_GRAY", DARK_GRAY},
        {"GRAY", GRAY}, {"GREEN", GREEN}, {"LIGHT_GRAY", LIGHT_GRAY},
        {"MAGENTA", MAGENTA}, {"ORANGE", ORANGE}, {"PINK", PINK},
        {"RED", RED}, {"WHITE", WHITE}, {"YELLOW", YELLOW},
    };
    const auto it = named.find(c);
    return it == named.end() ? c : it->second;
}

} // namespace

// Called by the exported C entry points below; keeps the queue itself private.
void pushEvent(const Event &e) { g_events.push_back(e); }

// ---- Window ----
Window::Window(int width, int height, const std::string &title) {
    g_width = width;
    g_height = height;
    g_color = BLACK;
    EM_ASM({
        const c = document.getElementById('canvas');
        c.width = $0; c.height = $1;
        window.__tvctx = c.getContext('2d');
        window.__tvctx.font = '14px Helvetica, Arial, sans-serif';
        window.__tvctx.textBaseline = 'alphabetic';
        document.title = UTF8ToString($2);
    }, width, height, title.c_str());
}

Window::~Window() = default;
void Window::setTerminateOnClose(bool) {}

void Window::clear() {
    EM_ASM({ window.__tvctx.clearRect(0, 0, $0, $1); }, g_width, g_height);
}

void Window::setColor(const std::string &color) {
    g_color = resolveColor(color);
    EM_ASM({
        const s = UTF8ToString($0);
        window.__tvctx.fillStyle = s;
        window.__tvctx.strokeStyle = s;
    }, g_color.c_str());
}

std::string Window::getColor() const { return g_color; }

void Window::fillRect(int x, int y, int w, int h) {
    EM_ASM({ window.__tvctx.fillRect($0, $1, $2, $3); }, x, y, w, h);
}

void Window::drawRect(int x, int y, int w, int h) {
    EM_ASM({ window.__tvctx.strokeRect($0, $1, $2, $3); }, x, y, w, h);
}

// FLTK's oval takes a bounding box; canvas ellipse takes a centre and radii.
void Window::fillOval(int x, int y, int w, int h) {
    EM_ASM({
        const t = window.__tvctx;
        t.beginPath();
        t.ellipse($0 + $2 / 2, $1 + $3 / 2, $2 / 2, $3 / 2, 0, 0, Math.PI * 2);
        t.fill();
    }, x, y, w, h);
}

void Window::drawOval(int x, int y, int w, int h) {
    EM_ASM({
        const t = window.__tvctx;
        t.beginPath();
        t.ellipse($0 + $2 / 2, $1 + $3 / 2, $2 / 2, $3 / 2, 0, 0, Math.PI * 2);
        t.stroke();
    }, x, y, w, h);
}

void Window::fillCircle(int cx, int cy, int r) {
    EM_ASM({
        const t = window.__tvctx;
        t.beginPath(); t.arc($0, $1, $2, 0, Math.PI * 2); t.fill();
    }, cx, cy, r);
}

void Window::drawCircle(int cx, int cy, int r) {
    EM_ASM({
        const t = window.__tvctx;
        t.beginPath(); t.arc($0, $1, $2, 0, Math.PI * 2); t.stroke();
    }, cx, cy, r);
}

void Window::drawLine(int x0, int y0, int x1, int y1) {
    EM_ASM({
        const t = window.__tvctx;
        t.beginPath(); t.moveTo($0, $1); t.lineTo($2, $3); t.stroke();
    }, x0, y0, x1, y1);
}

void Window::drawLabel(const std::string &text, int x, int y) {
    EM_ASM({ window.__tvctx.fillText(UTF8ToString($0), $1, $2); }, text.c_str(), x, y);
}

int  Window::getWidth()  const { return g_width; }
int  Window::getHeight() const { return g_height; }
bool Window::isOpen()    const { return true; }   // a canvas never closes
void Window::update()          {}                 // canvas paints immediately

bool Window::hasEvents() const { return !g_events.empty(); }

Event Window::getEvent() {
    if (g_events.empty()) return Event{EventType::None, {}};
    Event e = g_events.front();
    g_events.pop_front();
    return e;
}


// ---- higher-level helpers declared in graphics.h ----
// The FLTK backend implements these with fl_font/fl_width; canvas has
// measureText, and the font is context state in both, so these are near
// line-for-line ports. Behaviour (including the baseline nudges and the
// reset to 14px at the end) is kept identical so drawings line up.

namespace {

void setFontSize(int px) {
    EM_ASM({ window.__tvctx.font = $0 + 'px Helvetica, Arial, sans-serif'; }, px);
}

int measureText(const std::string &text) {
    return EM_ASM_INT({
        return Math.round(window.__tvctx.measureText(UTF8ToString($0)).width);
    }, text.c_str());
}

} // namespace

int widthOfTextBox(const std::string &text, int padding, int txtSize) {
    setFontSize(txtSize);
    return measureText(text) + (padding * 2);
}

void drawShapeAroundText(Window &window, const std::string &text,
                         int centerX, int centerY, int height,
                         const std::string &fillColor,
                         int padding, int txtSize, bool isSquare,
                         const std::string &borderColor,
                         const std::string &textColor) {
    setFontSize(txtSize);
    const int textWidth = measureText(text);
    const int width = std::max(textWidth + (padding * 2), textWidth + 2);

    const int x = centerX - width / 2;
    const int y = centerY - height / 2;

    window.setColor(fillColor);
    if (isSquare) window.fillRect(x, y, width, height);
    else          window.fillOval(x, y, width, height);

    window.setColor(borderColor);
    if (isSquare) window.drawRect(x, y, width, height);
    else          window.drawOval(x, y, width, height);

    window.setColor(textColor);
    window.drawLabel(text, centerX - textWidth / 2, centerY + height / 9);
    setFontSize(14);
}

void drawShapeWithText(Window &window, const std::string &text,
                       int centerX, int centerY, int width, int height,
                       bool isSquare, const std::string &fillColor,
                       int txtSize, const std::string &borderColor,
                       const std::string &textColor) {
    setFontSize(txtSize);
    const int x = centerX - width / 2;
    const int y = centerY - height / 2;

    window.setColor(fillColor);
    if (isSquare) window.fillRect(x, y, width, height);
    else          window.fillOval(x, y, width, height);

    window.setColor(borderColor);
    if (isSquare) window.drawRect(x, y, width, height);
    else          window.drawOval(x, y, width, height);

    window.setColor(textColor);
    // Matches the FLTK backend, which estimates rather than measures here.
    int textWidth = static_cast<int>(text.length()) * 7;
    if (textWidth > width - 4) textWidth = width - 4;
    window.drawLabel(text, centerX - textWidth / 2, centerY + 14 / 4);
    setFontSize(14);
}

} // namespace graphics

// ---- entry points the shell page calls from JS ----
extern "C" {

EMSCRIPTEN_KEEPALIVE void tv_push_key(int code, int down) {
    graphics::Event e{};
    e.Type = down ? graphics::EventType::KeyDown : graphics::EventType::KeyUp;
    e.Event.Key.Code = code;
    graphics::pushEvent(e);
}

EMSCRIPTEN_KEEPALIVE void tv_push_mouse(int button, int x, int y, int down) {
    graphics::Event e{};
    e.Type = down ? graphics::EventType::MouseBtnDown : graphics::EventType::MouseBtnUp;
    e.Event.Mouse.Button = button;
    e.Event.Mouse.X = x;
    e.Event.Mouse.Y = y;
    graphics::pushEvent(e);
}

}

#endif // __EMSCRIPTEN__
