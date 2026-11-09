#ifndef SCREEN_H
#define SCREEN_H

#include "bootinfo.h"

typedef enum {
    ERROR_SCREEN,
    CHIP_SCREEN,
    FILE_SCREEN
} activeScreen;

typedef struct screen {
	void (*start)(
		struct screen* _screen,
        void* input
	);
	void (*tick)(
        struct screen* _screen,
        void* input
    );
	void (*input)(
        struct screen* _screen,
        void* input
    );
	void (*render)(
        struct screen* _screen,
        void* input
    );
	void (*swap)(
        activeScreen screen
    );

	BootInfo* binfo;
} screen;


void initialize_screen(screen* _screen, BootInfo* binfo, void* input, void* startFunc, void* tickFunc, void* inputFunc, void * renderFunc, void* swapScreenFunc);
void swap_screen(activeScreen screen, void* func);

#endif