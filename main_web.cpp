// main_web.cpp -- browser entry point.
//
// The desktop main.cpp reads a JSON config that points at a .turing file and
// then blocks in run(). Neither works here: there are no files to point at,
// and a browser main loop cannot block. So this takes the program text
// directly and drives viz::tick() from emscripten's main loop instead.
#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>
#include "viz.h"
#include "machine.h"
#include "tape.h"
#include <algorithm>
#include <fstream>
#include <exception>
#include <chrono>
#include <string>

namespace {

// TuringMachine::fromStandardDescription(string, ...) expects text that has
// already been flattened; the fstream overload does that preprocessing before
// delegating to it. The browser has no fstream, so do the same work here.
// Keep in step with machine.cpp's fstream overload.
std::string preprocess(std::string text) {
    const size_t hash = text.find("#########");
    if (hash != std::string::npos) text = text.substr(hash + 9);
    std::replace(text.begin(), text.end(), '\t', ' ');
    std::replace(text.begin(), text.end(), '\n', ' ');
    std::replace(text.begin(), text.end(), '\r', ' ');
    return text;
}

TuringMachineVisualization *g_viz = nullptr;
std::chrono::high_resolution_clock::time_point g_last;
bool g_paused = false;

void frame() {
    if (!g_viz) return;
    const auto now = std::chrono::high_resolution_clock::now();
    const long long delta =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - g_last).count();
    g_last = now;
    if (g_paused) return;

    // Nothing thrown in here may escape into the browser: an uncaught
    // exception in the main loop leaves the tab wedged past even a reload.
    // TuringMachine::update() looks up the next configuration before it
    // reports the halt, so a program reaching HALT throws out_of_range.
    try {
        g_viz->tick(delta);
    } catch (const std::exception &e) {
        emscripten_cancel_main_loop();
        EM_ASM({ window.tvStopped && window.tvStopped(UTF8ToString($0)); }, e.what());
        return;
    } catch (...) {
        emscripten_cancel_main_loop();
        EM_ASM({ window.tvStopped && window.tvStopped("halted"); });
        return;
    }
    graphics::flushFrame();
}

} // namespace

extern "C" {

// Load a program and (re)start the loop. Safe to call repeatedly -- picking a
// different program from the dropdown just calls this again.
EMSCRIPTEN_KEEPALIVE void tv_load(const char *description, unsigned stateRate) {
    delete g_viz;
    g_viz = nullptr;
    const std::string prepped = preprocess(description);
    try {
        g_viz = new TuringMachineVisualization(prepped, stateRate);
    } catch (std::invalid_argument *e) {
        EM_ASM({ window.tvError && window.tvError("parse: " + UTF8ToString($0)); }, e->what());
        return;
    } catch (const std::exception &e) {
        EM_ASM({ window.tvError && window.tvError(UTF8ToString($0)); }, e.what());
        return;
    } catch (...) {
        EM_ASM({ window.tvError && window.tvError("failed to parse program"); });
        return;
    }
    g_paused = false;
    g_last = std::chrono::high_resolution_clock::now();
    emscripten_cancel_main_loop();
    emscripten_set_main_loop(frame, 0, 0);
    EM_ASM({ window.tvReady && window.tvReady(); });
}

// Diagnostic: identical program, but through the exact fstream code path the
// desktop build uses, via Emscripten's in-memory filesystem.
EMSCRIPTEN_KEEPALIVE void tv_load_file(const char *path, unsigned stateRate) {
    std::fstream f(path);
    EM_ASM({ console.log("[tv] fstream open=" + $0); }, (int)f.is_open());
    Tape *tape = new Tape();
    TuringMachine *m = TuringMachine::fromStandardDescription(f, tape, stateRate);
    EM_ASM({ console.log("[tv] fstream path built machine OK"); });
    (void)m;
}

EMSCRIPTEN_KEEPALIVE void tv_set_paused(int paused) { g_paused = paused != 0; }

}

int main() {
    // Nothing to do until the page hands us a program.
    EM_ASM({ window.tvBooted && window.tvBooted(); });
    return 0;
}

#endif
