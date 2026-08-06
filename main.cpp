#include "lib/terminal.cpp"
#include <cstdlib>
#include <ctime>

int main() {
    srand((unsigned)time(nullptr));
    Terminal terminal;
    terminal.run();
    return 0;
}
