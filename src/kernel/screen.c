#include "screen.h"

void initialize_screen(screen* _screen, BootInfo* binfo, void* input, void* startFunc, void* tickFunc, void* inputFunc, void* renderFunc, void* swapFunc) {
    _screen->start = startFunc;
    _screen->tick = tickFunc;
    _screen->input = inputFunc;
    _screen->render = renderFunc;
    _screen->swap = swapFunc;
    _screen->binfo = binfo;

    _screen->start(_screen, input);
}