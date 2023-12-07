#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>

#include "board.c"

Square my_board[BOARD_SIZE][BOARD_SIZE]; // Plansza klienta

void hello(int player);
void recv_board(int sock);
void show_board(int player, int clientSocket);
void ending(int clientSocket);
void whats_next();
void winning(int clientSocket, int player);
void loosing(int clientSocket, int player);
void your_turn(int clientSocket, int board,int player);
void start_game(int player,int clientSocket);
int main();

int clientSocket;

void hello(int player){
    printf("\n");
    printf("WITAJ\n");
    printf("W trakcie swojego ruchu napisz sekwencje ruchu w składni:\n");
    printf("miejsce tymczasowe-miejsce docelowe np. F4-E3\n");
    printf("\n");
    printf("Jesteś graczem numer: %d\n", player);
}

void recv_board(int sock){
    memset(my_board, 0, sizeof(my_board));
    if (recv(sock, my_board, sizeof(my_board), 0) < 0) {
        perror("recv error");
        exit(EXIT_FAILURE);
    }
}

void show_board(int player, int clientSocket){
    printf("\n");
    recv_board(clientSocket);
    if(player==1){
        display_board(my_board);
    }
    else{
        display_inverse_board(my_board);
    }
}

void ending(int clientSocket){
    printf(RED "Gra została zakończona\n" RESET);
    close(clientSocket);
    exit(0);
}

void whats_next(){
    char message[100];
    printf(RED "\nGra została zakończona\n" RESET);
    printf("\njeśli chcesz zakończyć program wpisz ");
    printf(RED "exit\n" RESET);
    printf("jeśli chcesz zacząć nową grę wpisz ");
    printf(GRN "play\n" RESET);
    scanf("%s",message);
    while(strstr(message,"exit") == NULL && strstr(message,"play") == NULL)
    {
        printf("musisz wpisać 'exit' lub 'play'\n");
        scanf("%s",message);
    }
    if(strstr(message,"exit") != NULL){
        exit(0);
    }
    else{
        main();
    }
}

void winning(int clientSocket, int player){
    printf("\n     FINIAL BOARD     \n");
    show_board(player, clientSocket);
    printf(MAG "YOU WOOOOOON!\n" RESET);
    close(clientSocket);
    whats_next();
}

void loosing(int clientSocket, int player){
    printf("\n     FINIAL BOARD     \n");
    show_board(player, clientSocket);
    printf(CYN "YOU LOST:(\n" RESET);
    close(clientSocket);
    whats_next();
}

void your_turn(int clientSocket, int board,int player){
    char message[100];
    char player_sign;
    char player_queen;
    
    if(player==1){
        player_sign = 'O';
        player_queen = 'W';
    }
    else{
        player_sign = 'X';
        player_queen = 'B';
    }

    show_board(player, clientSocket);
    
    printf(GRN "\nIt's your turn!" RESET);
    
    printf(" Twoje pionki to: " );
    if(player_sign=='O'){
        printf(YEL "%c %c\n" RESET,player_sign,player_queen);
    }
    else{
        printf(BLU "%c %c\n",player_sign,player_queen);
    }
    
    printf(GRN "Podaj ruch albo wpisz 'exit'\n" RESET);
    
    scanf("%s",message);
    if(strstr(message,"exit") != NULL)
    {
        ending(clientSocket);
    }

    if (send(clientSocket , message , strlen(message) , 0) < 0) {
        perror("send error");
        exit(EXIT_FAILURE);
    }
    
    int message_stat;
    
    if (recv(clientSocket, &message_stat, sizeof(message_stat), 0) < 0) {
        perror("recv error");
        exit(EXIT_FAILURE);
    }
    
    if(message_stat==-2){
        printf(RED "Niepoprawny ruch\n" RESET);
    }
    else{
        show_board(player, clientSocket);
    }
    
    memset(&message, 0, sizeof (message));
}


void start_game(int player,int clientSocket){
    int board;
    int waitRoom = 0;
    int waitMove = 0;
    int game_stat;
    int active = 1;
    
    hello(player);
    
    if (recv(clientSocket, &board, sizeof(board), 0) < 0) {
        perror("recv error");
        exit(EXIT_FAILURE);
    }
    
    for(;;){
        
        if(send(clientSocket,&active,sizeof(int),0) < 0){
            perror("recv error");
            exit(EXIT_FAILURE);
        }
        
        if (recv(clientSocket, &game_stat, sizeof(game_stat), 0) < 0) {
            perror("recv error");
            exit(EXIT_FAILURE);
        }

        // -1 end game 0 wait for player 1 make move 2 wait for enemie's move 4 won 5 lost
        switch(game_stat){
            case -1:
                close(clientSocket);
                whats_next();
            case 0:
                waitMove = 0;
                if(waitRoom==0){
                    printf(RED "Waiting for other player..\n" RESET);
                    waitRoom=1;
                }
                break;
            case 1:
                waitMove = 0;
                waitRoom = 0;
                your_turn(clientSocket,board,player);
                break;
            case 2:
                waitRoom = 0;
                if(waitMove==0){
                    printf(RED "\nWait for your enemy move!\n" RESET);
                    waitMove=1;
                }
                break;
            case 4:
                winning(clientSocket, player);
            case 5:
                loosing(clientSocket,player);
            default:
                break;
        }
    }
}

int main(){
    int player;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    // Create the socket.
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    //Configure settings of the server address
    // Address family is Internet
    serverAddr.sin_family = AF_INET;

    //Set port number, using htons function
    serverAddr.sin_port = htons(1100);

    //Set IP address
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    //Connect the socket to the server using the address
    addr_size = sizeof serverAddr;
    if (connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) < 0) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }
    
    if (recv(clientSocket, &player, sizeof(player), 0) < 0) {
        perror("recv error");
        exit(EXIT_FAILURE);
    }
    
    if(player==-1){
        printf(RED "Za dużo graczy.. przepraszamy" RESET);
    }
    else{
        start_game(player, clientSocket);
    }
    
    close(clientSocket);
    return 0;
}
