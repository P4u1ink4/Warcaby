#include <stdio.h>

#define BOARD_SIZE 8
#define MAX_GAMES 5

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define YEL   "\x1B[33m"
#define WHT   "\x1B[37m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"

typedef struct {
    int is_empty;
    char piece;
} Square;

Square boards[MAX_GAMES][BOARD_SIZE][BOARD_SIZE]; // Globalna tablica plansz

void initialize_boards() {
    for (int k = 0; k < MAX_GAMES; k++) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                boards[k][i][j].is_empty = 1;
                boards[k][i][j].piece = ' ';
                if ((i + j) % 2 != 0 && i < 3) {
                    boards[k][i][j].is_empty = 0;
                    boards[k][i][j].piece = 'X';
                }
                if ((i + j) % 2 != 0 && i > 4) {
                    boards[k][i][j].is_empty = 0;
                    boards[k][i][j].piece = 'O';
                }
            }
        }
    }
}

void clean_board(int k) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            boards[k][i][j].is_empty = 1;
            boards[k][i][j].piece = ' ';
            if ((i + j) % 2 != 0 && i < 3) {
                boards[k][i][j].is_empty = 0;
                boards[k][i][j].piece = 'X';
            }
            if ((i + j) % 2 != 0 && i > 4) {
                boards[k][i][j].is_empty = 0;
                boards[k][i][j].piece = 'O';
            }
        }
    }
}

void display_board(Square board[BOARD_SIZE][BOARD_SIZE]) {
    printf("   A B C D E F G H\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("|");
            if(board[i][j].piece=='X' || board[i][j].piece=='B'){
                printf(BLU "%c" RESET, board[i][j].piece);
            }
            else{
                printf(YEL "%c" RESET, board[i][j].piece);
            }
        }
        printf("|\n");
    }
}

void display_inverse_board(Square board[BOARD_SIZE][BOARD_SIZE]) {
    printf("   H G F E D C B A\n");
    for (int i = BOARD_SIZE - 1; i >= 0; i--) {
        printf("%d ", i + 1);
        for (int j = BOARD_SIZE - 1; j >= 0; j--) {
            printf("|");
            if(board[i][j].piece=='X' || board[i][j].piece=='B'){
                printf(BLU "%c" RESET, board[i][j].piece);
            }
            else{
                printf(YEL "%c" RESET, board[i][j].piece);
            }
        }
        printf("|\n");
    }
}

