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
#include <vector>
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

// ---- batched drawing ----
// Every draw used to be its own EM_ASM call. At a few hundred shapes a frame
// that is thousands of wasm->JS crossings per second, each marshalling a
// string for the colour, which made the page crawl. Instead we append ops to
// a flat buffer and hand the whole frame to JS in a single call.
namespace {

enum Op : int {
    OP_CLEAR = 0, OP_COLOR = 1, OP_FILLRECT = 2, OP_STROKERECT = 3,
    OP_FILLELLIPSE = 4, OP_STROKEELLIPSE = 5, OP_FILLCIRCLE = 6,
    OP_STROKECIRCLE = 7, OP_LINE = 8, OP_TEXT = 9, OP_FONT = 10,
};

std::vector<float>       g_ops;
std::vector<std::string> g_strs;
std::unordered_map<std::string, int> g_strIds;

int intern(const std::string &s) {
    const auto it = g_strIds.find(s);
    if (it != g_strIds.end()) return it->second;
    const int id = static_cast<int>(g_strs.size());
    g_strs.push_back(s);
    g_strIds.emplace(s, id);
    return id;
}

template<typename... A>
void push(Op op, A... args) {
    g_ops.push_back(static_cast<float>(op));
    (g_ops.push_back(static_cast<float>(args)), ...);
}

// measureText is the one call that must be answered synchronously. Memoise it:
// the same handful of labels and sizes recur every frame.
std::unordered_map<std::string, int> g_measure;

int measureCached(const std::string &text, int px) {
    const std::string key = std::to_string(px) + ":" + text;
    const auto it = g_measure.find(key);
    if (it != g_measure.end()) return it->second;
    const int w = EM_ASM_INT({
        const t = window.__tvctx;
        const prev = t.font;
        t.font = $1 + 'px Helvetica, Arial, sans-serif';
        const w = Math.round(t.measureText(UTF8ToString($0)).width);
        t.font = prev;
        return w;
    }, text.c_str(), px);
    g_measure.emplace(key, w);
    return w;
}

} // namespace

// Hand one frame's worth of ops to JS in a single crossing.
void flushFrame() {
    if (g_ops.empty()) return;
    std::string joined;
    for (size_t i = 0; i < g_strs.size(); ++i) {
        if (i) joined += '\n';
        joined += g_strs[i];
    }
    EM_ASM({
        const ops = new Float32Array(HEAPF32.buffer, $0, $1);
        const strs = UTF8ToString($2).split('\n');
        const t = window.__tvctx;
        let i = 0;
        while (i < ops.length) {
            switch (ops[i++]) {
                case 0: t.clearRect(0, 0, $3, $4); break;
                case 1: { const c = strs[ops[i++]]; t.fillStyle = c; t.strokeStyle = c; break; }
                case 2: t.fillRect(ops[i], ops[i+1], ops[i+2], ops[i+3]); i += 4; break;
                case 3: t.strokeRect(ops[i], ops[i+1], ops[i+2], ops[i+3]); i += 4; break;
                case 4: t.beginPath(); t.ellipse(ops[i]+ops[i+2]/2, ops[i+1]+ops[i+3]/2, Math.abs(ops[i+2])/2, Math.abs(ops[i+3])/2, 0, 0, 6.283185307); t.fill(); i += 4; break;
                case 5: t.beginPath(); t.ellipse(ops[i]+ops[i+2]/2, ops[i+1]+ops[i+3]/2, Math.abs(ops[i+2])/2, Math.abs(ops[i+3])/2, 0, 0, 6.283185307); t.stroke(); i += 4; break;
                case 6: t.beginPath(); t.arc(ops[i], ops[i+1], Math.abs(ops[i+2]), 0, 6.283185307); t.fill(); i += 3; break;
                case 7: t.beginPath(); t.arc(ops[i], ops[i+1], Math.abs(ops[i+2]), 0, 6.283185307); t.stroke(); i += 3; break;
                case 8: t.beginPath(); t.moveTo(ops[i], ops[i+1]); t.lineTo(ops[i+2], ops[i+3]); t.stroke(); i += 4; break;
                case 9: t.fillText(strs[ops[i]], ops[i+1], ops[i+2]); i += 3; break;
                case 10: t.font = ops[i++] + 'px Helvetica, Arial, sans-serif'; break;
                default: i = ops.length; break;
            }
        }
    }, g_ops.data(), (int)g_ops.size(), joined.c_str(), g_width, g_height);

    g_ops.clear();
    g_strs.clear();
    g_strIds.clear();
}

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

void Window::clear()                                   { push(OP_CLEAR); }
void Window::setColor(const std::string &color)        { g_color = resolveColor(color); push(OP_COLOR, intern(g_color)); }
std::string Window::getColor() const                   { return g_color; }
void Window::fillRect(int x, int y, int w, int h)      { push(OP_FILLRECT, x, y, w, h); }
void Window::drawRect(int x, int y, int w, int h)      { push(OP_STROKERECT, x, y, w, h); }
void Window::fillOval(int x, int y, int w, int h)      { push(OP_FILLELLIPSE, x, y, w, h); }
void Window::drawOval(int x, int y, int w, int h)      { push(OP_STROKEELLIPSE, x, y, w, h); }
void Window::fillCircle(int cx, int cy, int r)         { push(OP_FILLCIRCLE, cx, cy, r); }
void Window::drawCircle(int cx, int cy, int r)         { push(OP_STROKECIRCLE, cx, cy, r); }
void Window::drawLine(int x0, int y0, int x1, int y1)  { push(OP_LINE, x0, y0, x1, y1); }
void Window::drawLabel(const std::string &t, int x, int y) { push(OP_TEXT, intern(t), x, y); }

int  Window::getWidth()  const { return g_width; }
int  Window::getHeight() const { return g_height; }
bool Window::isOpen()    const { return true; }
void Window::update()          {}

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

int g_fontPx = 14;
void setFontSize(int px) { g_fontPx = px; push(OP_FONT, px); }
int measureText(const std::string &text) { return measureCached(text, g_fontPx); }

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
