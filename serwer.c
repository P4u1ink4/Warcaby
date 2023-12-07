#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "board.c"

void init_games();
void send_board(int socket, int board);
int check_j(char x);
int check_i(char x);
void change_board(char actual_sign, int i, int j, int i2, int j2, int board);
void get_possible_moves(int board, char player, int i, int j);
int check_move_from_list(char *client_message, int board, int player);
int check_if_have_beat(int board,int player,char *client_message, char actual_sign);
int check_move_validity(char *client_message,int board, int player);
int check_for_winner(int board);
void send_result(int num_player,int game_state,const int YOU_WON,const int YOU_LOST,int newSocket,int board);
int whos_turn(int turn,int player, const int YOUR_MOVE, const int ENEMY_MOVE, int newSocket);
int is_connect(int newSocket);
void *socketThread(void *arg);
int main();

// Struktura przechowująca informacje o grze
typedef struct {
    int num_clients; // ilosc graczy do dwóch
    int turn; // 1 - ruch białych O 2 - ruch czarnych X
    int game_state; // 0 - oczekiwanie 1 - rozgrywka 2 - gra zakończona 3 - wygrały czarne 4 - wygrały białe
    int white_pawns;
    int black_pawns;
    int can_play;
} GameInfo;

GameInfo games[MAX_GAMES];
int possibleGames[MAX_GAMES];
char possible_moves[100][6]; // Lista stringów, gdzie przechowywane będą ruchy
int moves_count = 0; // Licznik dostępnych ruchów

void init_games(){
    for(int i=0;i<MAX_GAMES;i++){
        games[i].num_clients=0;
        games[i].black_pawns=12;
        games[i].white_pawns=12;
        games[i].can_play=1;
    }
}

void send_board(int socket, int board){
    if (send(socket, boards[board], sizeof(boards[board]), 0) < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }
}

int check_j(char x){
    return (x >= 'A' && x <= 'H') ? (x - 'A') : 8;
}

int check_i(char x){
    return (x >= '1' && x <= '8') ? (x - '1') : 8;
}

void change_board(char actual_sign, int i, int j, int i2, int j2, int board){
    boards[board][i][j].is_empty = 1;
    boards[board][i][j].piece = ' ';
    boards[board][i2][j2].is_empty = 0;
    boards[board][i2][j2].piece = actual_sign;
}

// Funkcja do sprawdzania możliwych bić
void get_possible_moves(int board, char player, int i, int j){
    char enemy,enemyQ;
    
    if(player=='X' || player=='B'){
        enemy='O';
        enemyQ='W';
    }
    else if(player=='O' || player=='W'){
        enemy='X';
        enemyQ='B';
    }
    
    // Sprawdzenie dla pionków
    if (player == 'O' || player == 'X') {
        
        // Możliwe kierunki ruchu dla pionków
        int directions[4][2] = {{1, -1}, {1, 1}, {-1, -1}, {-1, 1}};
        
        for (int d = 0; d < 4; d++) {
            int x = directions[d][0];
            int y = directions[d][1];
            
            int new_i = i + x;
            int new_j = j + y;
            int jump_i = new_i + x;
            int jump_j = new_j + y;
            
            if (new_i >= 0 && new_i < BOARD_SIZE && new_j >= 0 && new_j < BOARD_SIZE &&
                jump_i >= 0 && jump_i < BOARD_SIZE && jump_j >= 0 && jump_j < BOARD_SIZE &&
                (boards[board][new_i][new_j].piece == enemy || boards[board][new_i][new_j].piece == enemyQ) && boards[board][jump_i][jump_j].is_empty==1) {
                sprintf(possible_moves[moves_count++], "%c%d-%c%d", j + 'A', i+1 , jump_j + 'A', jump_i+1);
            }
        }
    }
    // Sprawdzenie dla damek
    else if (player == 'W' || player == 'B') {
        
        int directions[4][2] = {{1, -1}, {1, 1}, {-1, -1}, {-1, 1}};
        int is_enemy = 0;
        for (int d = 0; d < 4; d++) {
            int x = directions[d][0];
            int y = directions[d][1];
            
            int new_i = i + x;
            int new_j = j + y;

            while (new_i >= 0 && new_i < BOARD_SIZE && new_j >= 0 && new_j < BOARD_SIZE) {
                if (boards[board][new_i][new_j].is_empty == 0) {
                    if((boards[board][new_i][new_j].piece == enemy || boards[board][new_i][new_j].piece == enemyQ)){
                        is_enemy = 1;
                        new_i += x;
                        new_j += y;
                    }
                    break;
                }
                new_i += x;
                new_j += y;
            }
            if(is_enemy==1){
                while(new_i >= 0 && new_i < BOARD_SIZE && new_j >= 0 && new_j < BOARD_SIZE){
                    if(boards[board][new_i][new_j].is_empty == 1){
                        sprintf(possible_moves[moves_count++], "%c%d-%c%d", j + 'A', i+1 , new_j + 'A', new_i+1);
                    }
                    else{
                        break;
                    }
                    new_i += x;
                    new_j += y;
                }
            }
        }
    }
}

// jesli wystepuje wielokrotne bicie - to sprawdzamy czy ruszamy dobrym pionkiem
int check_move_from_list(char *client_message, int board, int player){
    for (int i = 0; i < moves_count; i++) {
        if (strcmp(possible_moves[i], client_message) == 0) {
            for (int i = 0; i < moves_count; i++) {
                    memset(possible_moves[i], '\0', sizeof(possible_moves[i]));
            }
            moves_count=0;
            
            int i1 = check_i(client_message[1]);
            int i2 = check_i(client_message[4]);
            int j1 = check_j(client_message[0]);
            int j2 = check_j(client_message[3]);
            int temp_i=0;
            int temp_j=0;
            
            char player_sign = boards[board][i1][j1].piece;
            
            int delta_i = (i2 - i1 > 0) ? 1 : -1;
            int delta_j = (j2 - j1 > 0) ? 1 : -1;
            
            for (int i = i1 + delta_i, j = j1 + delta_j; i != i2 || j != j2; i += delta_i, j += delta_j) {
                if (boards[board][i][j].is_empty == 0) {
                    temp_i = i;
                    temp_j = j;
                    break;
                }
            }
            
            boards[board][temp_i][temp_j].is_empty = 1;
            boards[board][temp_i][temp_j].piece = ' ';
            
            change_board(player_sign, i1, j1, i2, j2, board);
            
            if(player_sign=='X' || player_sign=='B'){
                games[board].white_pawns-=1;
            }
            else{
                games[board].black_pawns-=1;
            }
            
            if(i2==7 && player_sign=='X'){
                boards[board][i2][j2].piece = 'B';
            }
            if(i2==0 && player_sign=='O'){
                boards[board][i2][j2].piece = 'W';
            }
            
            get_possible_moves(board, player_sign, check_i(client_message[4]), check_j(client_message[3]));
            
            if(moves_count>0){
                return 2;
            }
            else{
                return 1;
            }
        }
    }
    return 0;
}

// sprawdzamy czy jest przymusowe bicie
int check_if_have_beat(int board,int player,char *client_message, char actual_sign){
    char sign_player = ' ';
    char player_queen = ' ';
    
    switch(player){
        case 1:
            sign_player = 'O';
            player_queen = 'W';
            break;
        case 2:
            sign_player = 'X';
            player_queen = 'B';
            break;
        default:
            break;
    }
    
    int all_count = 0;
    for(int i=0;i<BOARD_SIZE;i++){
        for(int j=0;j<BOARD_SIZE;j++){
            if(boards[board][i][j].piece==sign_player || boards[board][i][j].piece==player_queen){
                for (int i = 0; i < moves_count; i++) {
                        memset(possible_moves[i], '\0', sizeof(possible_moves[i]));
                }
                moves_count=0;
                get_possible_moves(board, boards[board][i][j].piece, i, j);
                if(moves_count>0){
                    for(int x=0;x<moves_count;x++){
                        printf("%d %s %s\n",strcmp(possible_moves[x], client_message), possible_moves[x],client_message);
                        if (strcmp(possible_moves[x], client_message) == 0){
                            for (int i = 0; i < moves_count; i++) {
                                    memset(possible_moves[i], '\0', sizeof(possible_moves[i]));
                            }
                            moves_count=0;
                            return 1;
                        }
                    }
                }
                all_count += moves_count;
            }
        }
    }
    for (int i = 0; i < moves_count; i++) {
            memset(possible_moves[i], '\0', sizeof(possible_moves[i]));
    }
    moves_count=0;
    if(all_count==0){
        return 1;
    }
    else{ return 0; }
}

// sprawdzamy czy ruch przekazany przez gracza jest poprawny
int check_move_validity(char *client_message,int board, int player){
    int i,j,i2,j2;
    
    if(strlen(client_message)!=5){
        return 0;
    }
    
    j = check_j(client_message[0]);
    i = check_i(client_message[1]);
    j2 = check_j(client_message[3]);
    i2 = check_i(client_message[4]);
    
    if(i==8 || j==8 || client_message[2]!='-' || i2==8 || j2==8){
        return 0;
    }
    
    char sign_player = ' ';
    char sign_enemy = ' ';
    char player_queen = ' ';
    char enemy_queen = ' ';
    
    switch(player){
        case 1:
            sign_player = 'O';
            sign_enemy = 'X';
            player_queen = 'W';
            enemy_queen = 'B';
            break;
        case 2:
            sign_player = 'X';
            sign_enemy = 'O';
            player_queen = 'B';
            enemy_queen = 'W';
            break;
        default:
            break;
    }
    
    char actual_sign = boards[board][i][j].piece;
    
    if((actual_sign!=sign_player && actual_sign!=player_queen) || boards[board][i2][j2].is_empty!=1){
        return 0;
    }
    
    if(abs(i2-i)!=abs(j2-j) || abs(i2-i)==0){
        return 0;
    }
    
    // sprawdzamy czy istnieje przymusowe bicie, jeśli tak i gracz chciał ruszym innym pionkiem jest niepopranwy ruch
    if( check_if_have_beat(board,player,client_message, actual_sign)==0){
        return 0;
    }
    
    // bicie pionków
    if(actual_sign=='X' || actual_sign=='O'){
        if(abs(i2 - i)==2){
            if(abs(j2-j)==2){
                int tempI = (i2-i > 0) ? i + 1 : i - 1;
                if( boards[board][tempI][j+1].piece==enemy_queen || boards[board][tempI][j+1].piece==sign_enemy || boards[board][tempI][j-1].piece==enemy_queen || boards[board][tempI][j-1].piece==sign_enemy){
                    
                    if(j2>j){
                        if(boards[board][tempI][j+1].is_empty==0){
                            boards[board][tempI][j+1].is_empty = 1;
                            boards[board][tempI][j+1].piece = ' ';
                        }
                    }
                    else{
                        if(boards[board][tempI][j-1].is_empty==0){
                            boards[board][tempI][j-1].is_empty = 1;
                            boards[board][tempI][j-1].piece = ' ';
                        }
                    }
                    
                    change_board(actual_sign, i, j, i2, j2, board);
                    
                    if(actual_sign=='X'){
                        games[board].white_pawns-=1;
                        if(i2==7){
                            boards[board][i2][j2].piece = 'B';
                        }
                    }
                    else{
                        games[board].black_pawns-=1;
                        if(i2==0){
                            boards[board][i2][j2].piece = 'W';
                        }
                    }
                    
                    get_possible_moves(board, actual_sign, i2, j2);
                    if(moves_count>0){
                        return 2;
                    }
                    else{
                        return 1;
                    }
                }
                else{
                    return 0;
                }
            }
            else{
                return 0;
            }
        }
    }
    
    //ruch pionków
    if(actual_sign=='X'){
        if(i2==i+1){
            if(j2==j+1 || j2==j-1){
                change_board(actual_sign, i, j, i2, j2, board);
            }
            else{
                return 0;
            }
        }
        else{
            return 0;
        }
        
        if(i2==7){
            boards[board][i2][j2].piece = 'B';
        }
        
    }
    else if(actual_sign=='O'){
        if(i2==i-1){
            if(j2==j+1 || j2==j-1){
                change_board(actual_sign, i, j, i2, j2, board);
            }
            else{
                return 0;
            }
        }
        else{
            return 0;
        }
        if(i2==0){
            boards[board][i2][j2].piece = 'W';
        }
    }
    
    //ruch damek i bicie damek
    int is_pawn = 0;
    int temp_i,temp_j;
    if(actual_sign=='W' || actual_sign=='B'){
        int delta_i = (i2 > i) ? 1 : -1;
        int delta_j = (j2 > j) ? 1 : -1;

        for (int x = 1; (i + x * delta_i != i2) && (j + x * delta_j != j2); x++) {
            if (boards[board][i + x * delta_i][j + x * delta_j].is_empty == 0) {
                is_pawn++;
                temp_i = i + x * delta_i;
                temp_j = j + x * delta_j;
            }
        }
        
        if(is_pawn==0){
            change_board(actual_sign, i, j, i2, j2, board);
        }
        else if (is_pawn==1){
            if(actual_sign=='W' && (boards[board][temp_i][temp_j].piece=='X' || boards[board][temp_i][temp_j].piece=='B')){
                
                boards[board][temp_i][temp_j].is_empty = 1;
                boards[board][temp_i][temp_j].piece = ' ';
                
                change_board(actual_sign, i, j, i2, j2, board);
                
                games[board].black_pawns-=1;
                get_possible_moves(board, actual_sign, i2, j2);
                if(moves_count>0){
                    return 2;
                }
                else{
                    return 1;
                }
            }
            else if(actual_sign=='B' && (boards[board][temp_i][temp_j].piece=='O' || boards[board][temp_i][temp_j].piece=='W')){
                
                boards[board][temp_i][temp_j].is_empty = 1;
                boards[board][temp_i][temp_j].piece = ' ';
                
                change_board(actual_sign, i, j, i2, j2, board);
                
                games[board].white_pawns-=1;
                get_possible_moves(board, actual_sign, i2, j2);
                if(moves_count>0){
                    return 2;
                }
                else{
                    return 1;
                }
            }
            else{
                return 0;
            }
        }
        else{
            return 0;
        }
    }
    
    return 1;
}

// sprawdzanie ile pionków w grze - sprawdzanie wygranej
int check_for_winner(int board){
    // sprawdzanie szybkie, po dwóch zbitych pionkach
    
    // if(games[board].black_pawns<=10){
    //     return 4;
    // }
    // else if(games[board].white_pawns<=10){
    //     return 3;
    // }
    // else{
    //     return 1;
    // }

    // aktualne sprawdzenie czy gracz wygrał
   if(games[board].black_pawns>0 && games[board].white_pawns>0){
       return 1;
   }
   else if(games[board].black_pawns>0 && games[board].white_pawns==0){
       return 3;
   }
   else if(games[board].black_pawns==0 && games[board].white_pawns>0){
       return 4;
   }
   return 0;
}

// wysłanie wiadomości odnośnie wygranej lub przegranej
void send_result(int num_player,int game_state,const int YOU_WON,const int YOU_LOST,int newSocket,int board){
    if ((game_state == 3 && num_player==2) || (game_state==4 && num_player==1)) {
        if ( send(newSocket, &YOU_WON, sizeof(int), 0) < 0 ){
            perror("send error");
            exit(EXIT_FAILURE);
        }
        send_board(newSocket, board);
    }
    else
    {
        if ( send(newSocket, &YOU_LOST, sizeof(int), 0) < 0 ){
            perror("send error");
            exit(EXIT_FAILURE);
        }
        send_board(newSocket, board);
    }
}

// sprawdzanie którego z graczy jest ruch
int whos_turn(int turn,int player, const int YOUR_MOVE, const int ENEMY_MOVE, int newSocket){
    if ((turn == 1 && player == 1) || (turn == 2 && player == 2)) {
        if ( send(newSocket, &YOUR_MOVE, sizeof(int), 0) < 0 ){
            perror("send error");
            exit(EXIT_FAILURE);
        }
        return 1;
    } else {
        if ( send(newSocket, &ENEMY_MOVE, sizeof(int), 0) < 0 ){
            perror("send error");
            exit(EXIT_FAILURE);
        }
        return 2;
    }
}

// sprawdzamy czy gracz na pewno jest aktywny
int is_connect(int newSocket){
    int active;
    if (recv(newSocket, &active, sizeof(int), 0) < 1) {
        printf("Client disconnected\n");
        close(newSocket);
        return 0;
    }
    return 1;
}

void *socketThread(void *arg){
    int newSocket = *((int *)arg);
    printf("New thread\n");
    
    int board = -1;
    int num_player = 0;
    long n;
    char client_message[1000];
    
    // definicija const wiadomości game_statów
    const int MISTAKE = -2;
    const int EXIT = -1;
    const int WAIT = 0;
    const int YOUR_MOVE = 1;
    const int ENEMY_MOVE = 2;
    const int GOOD_MOVE = 3;
    const int YOU_WON = 4;
    const int YOU_LOST = 5;
    
    // Flagi
    int turn = 0;
    
    // Sprawdzenie czy jakiś gracz gdzieś nie czeka na przeciwnika, jeśli tak to dołączenie do niego
    for(int i=0; i < MAX_GAMES; i++){
        if (games[i].num_clients == 1 && games[i].can_play==1) {
            games[i].num_clients++;
            num_player = games[i].num_clients;
            board = i;
            games[i].game_state = 1;
            games[i].can_play = 0;
            break;
        }
    }

    // jeśli nikt nie czeka to tworzymy nową grę jeśli jest wolny slot
    if(board==-1){
        for (int i = 0; i < MAX_GAMES; i++) {
            if (games[i].num_clients == 0 && games[i].can_play==1) {
                games[i].num_clients++;
                num_player = games[i].num_clients;
                board = i;
                games[i].game_state = 0;
                
                clean_board(board);
                games[i].turn = 1;
                games[board].black_pawns = 12;
                games[board].white_pawns = 12;
                break;
            } else if (games[i].num_clients == 1 && games[i].can_play==1) {
                games[i].can_play = 0;
                games[i].num_clients++;
                num_player = games[i].num_clients;
                board = i;
                games[i].game_state = 1;
                break;
            }
        }
    }
    
    // Jeśli nie było wolnego slota do gry to informujemy o tym klienta
    if (board == -1) {
        if ( send(newSocket, &EXIT, sizeof(int), 0) < 0){
            perror("send error");
            exit(EXIT_FAILURE);
        }
        pthread_exit(NULL);
    }
    
    // Wysyłamy graczowi potrzebne informacje
    if ( send(newSocket, &num_player, sizeof(int), 0) < 0 ){
        perror("send error");
        exit(EXIT_FAILURE);
    }
    if ( send(newSocket, &board, sizeof(int), 0) < 0 ){
        perror("send error");
        exit(EXIT_FAILURE);
    }
    
    // Logika gry, obsluga game_statów
    while (1) {

        // oczekiwanie na gracza
        if (games[board].game_state == 0) {
            if(is_connect(newSocket)==0){
                break;
            }
            if ( send(newSocket, &WAIT, sizeof(int), 0) < 0 ){
                perror("send error wait");
                exit(EXIT_FAILURE);
            }
        }

        // koniec gry
        if (games[board].game_state == 2) {
            if(is_connect(newSocket)==0){
                break;
            }
            
            if ( send(newSocket, &EXIT, sizeof(int), 0) < 0 ){
                perror("send error");
                exit(EXIT_FAILURE);
            }
            break;
        }

        // sprawdzenie wygranej jeśli gra jest w toku
        if (games[board].game_state == 1) {
            games[board].game_state = check_for_winner(board);
        }

        // jeśli wygrana to wysłanie informacji
        if(games[board].game_state==3 || games[board].game_state==4){
            if(is_connect(newSocket)==0){
                break;
            }
            send_result(num_player,games[board].game_state,YOU_WON,YOU_LOST,newSocket, board);
            break;
        }

        // sprawdzenie kogo tura jeśli gra jest w toku
        if (games[board].game_state == 1) {
            if(is_connect(newSocket)==0){
                break;
            }
            turn = whos_turn(games[board].turn,num_player, YOUR_MOVE,ENEMY_MOVE, newSocket);
        }
        
        // jeśli gra jest w toku i jest ruch gracza 
        if (turn == 1 && games[board].game_state==1) {
            send_board(newSocket, board);
            
            n = recv(newSocket, client_message, 1000, 0);
            if (n < 1) {
                break;
            }
            printf("%s\n", client_message);
            
            // sprawdzamy czy jest wielokrotne bicie i czy ruch jest poprawny
            int checking;
            if (moves_count == 0) {
                checking = check_move_validity(client_message, board, num_player);
            } else {
                checking = check_move_from_list(client_message, board, num_player);
            }
            
            // 1 - dobry ruch i koniec ruchu
            // 2 - dobry ruch i dalsze bicie
            // .. - zły ruch
            switch(checking){
                case 1:
                    if ( send(newSocket, &GOOD_MOVE, sizeof(int), 0) < 0 ){
                        perror("send error");
                        exit(EXIT_FAILURE);
                    }
                    send_board(newSocket, board);
                    break;
                case 2:
                    if ( send(newSocket, &GOOD_MOVE, sizeof(int), 0) < 0 ){
                        perror("send error");
                        exit(EXIT_FAILURE);
                    }
                    send_board(newSocket, board);
                    break;
                default:
                    if ( send(newSocket, &MISTAKE, sizeof(int), 0) < 0 ){
                        perror("send error");
                        exit(EXIT_FAILURE);
                    }
                    printf("Niepoprawny ruch\n");
                    break;
            }
            
            if(checking==1 && check_for_winner(board) == 1){
                if (games[board].turn == 1) {
                    games[board].turn = 2;
                } else {
                    games[board].turn = 1;
                }
            }

            memset(client_message, 0, sizeof(client_message));
        }
    }
    
    printf("Exit socketThread\n");

    // jeśli wyłączenie zostało przerwaniem nagłym, to zmiana game_stat na koniec gry
    if(games[board].game_state==1){
        games[board].game_state = 2;
    }
    
    games[board].num_clients -= 1;
    
    if(games[board].num_clients==0){
        games[board].can_play=1;
    }
    pthread_exit(NULL);
}


int main() {
    initialize_boards();
    init_games();
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    // Create the socket.
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure settings of the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    // Bind the address struct to the socket
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("Binding failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Listen on the socket
    if (listen(serverSocket, 50) == -1) {
        perror("Listening failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    } else {
        printf("Listening\n");
    }

    pthread_t thread_id;
    while (1) {
        // Accept call creates a new socket for the incoming connection
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
        if (newSocket < 0) {
            perror("Accepting connection failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0) {
            perror("Failed to create thread");
            close(newSocket);
            continue; // Continue accepting connections
        }

        pthread_detach(thread_id);
    }

    close(serverSocket);
    return 0;
}
