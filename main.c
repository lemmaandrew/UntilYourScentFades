#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#define clrscr() printf("\e[1;1H\e[2J");
#endif

// Wait for user to press enter
void wait_for_enter() {
#ifdef _WIN32
    getch();
#else
    getchar();
#endif
}

// The number of rows and columns of the terminal when measured.
// Does not account for rescaling terminal.
typedef struct {
    size_t rows;
    size_t cols;
} TerminalDims;

TerminalDims get_terminal_dims() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    TerminalDims tdims;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    tdims.rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    tdims.columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return tdims;
#else
    struct winsize termdims;
    ioctl(0, TIOCGWINSZ, &termdims);
    return (TerminalDims){.rows = termdims.ws_row, .cols = termdims.ws_col};
#endif
}

// The colors we need.
typedef enum {
    CLEAR,
    YELLOW,
    RED,
} CellColor;

// Print a yellow 'x' or a red 'o'.
void print_cell(CellColor c) {
    switch (c) {
        case YELLOW: {
            printf("\e[33mx\e[0m");
            break;
        }
        case RED: {
            printf("\e[31mo\e[0m");
            break;
        }
        default: {
            putchar(' ');
        }
    }
}

// Print all the colors onto the terminal.
void print_screen(size_t rows, size_t cols, CellColor screen[rows][cols]) {
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            print_cell(screen[i][j]);
        }
    }
}

// Fills the screen with colors.
void fill_screen(size_t rows, size_t cols, CellColor screen[rows][cols],
                 double density) {
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if ((double)rand() / (double)RAND_MAX < density) {
                if (rand() % 2) {
                    screen[i][j] = RED;
                } else {
                    screen[i][j] = YELLOW;
                }
            }
        }
    }
}

// Change the number of red 'o's to orange 'x's randomly.
// For any given red 'o', it has a given `chance_to_stay_red`.
void fade(size_t rows, size_t cols, CellColor screen[rows][cols],
          double chance_to_stay_red) {
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if (screen[i][j] == RED &&
                (double)rand() / (double)RAND_MAX < chance_to_stay_red) {
                screen[i][j] = YELLOW;
            }
        }
    }
}

// Whether any red 'o's remain.
bool any_red(size_t rows, size_t cols, CellColor screen[rows][cols]) {
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if (screen[i][j] == RED) {
                return true;
            }
        }
    }
    return false;
}

int main() {
    srand(time(NULL));
    TerminalDims tdims = get_terminal_dims();
    // Decrement row so we have room for user input
    --tdims.rows;
    double density = 0.25;
    double chance_to_stay_red = 0.8;
    CellColor screen[tdims.rows][tdims.cols];
    fill_screen(tdims.rows, tdims.cols, screen, density);
    while (any_red(tdims.rows, tdims.cols, screen)) {
        clrscr();
        print_screen(tdims.rows, tdims.cols, screen);
        printf("Press enter to fade");
        wait_for_enter();
        fade(tdims.rows, tdims.cols, screen, chance_to_stay_red);
    }
    clrscr();
    print_screen(tdims.rows, tdims.cols, screen);
}
