#include <nlohmann/json.hpp>
#include "viz.h"

using std::cout;
using std::rand;
using std::endl;
using std::min;
using std::max;
using std::vector;
using std::string;

using json = nlohmann::json;

int main(int argc, const char* argv[]) {

    if (argc < 2)
    {
        std::cerr << "require another argument" << std::endl;
        return 1;
    }
    std::ifstream ifs(argv[1]);
    json jf = json::parse(ifs);
    auto program_file = jf.at("turing program").get<std::string>();
    std::fstream file(program_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open file" << std::endl;
        return 1;
    }

    Tape* tp;
    TuringMachine* tm = TuringMachine::fromStandardDescription(file, tp);

    return 0;
}