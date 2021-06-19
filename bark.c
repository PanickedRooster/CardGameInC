#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct Card* init_deck(char* file, int* deckCount);
struct Card** create_board(int width, int height);
int is_free(int pos, struct Card** board, int width, int height, int row, 
        int col);
void cal_score(struct Card** board, int width, int height);
int is_game_over(struct Card** board, int* deckCount, int* emptyCards, 
        int width, int height);

/*A struct named card made in order to store values from a given deckfile
 * as well as a value utilised when calculating the score*/
struct Card {
    char suit;
    int number;
    int score;
};

/*reads the line of a given file and returns it as a char* (string)*/
char* read_line(FILE* file) {
    char* result = malloc(sizeof(char) * 40);
    int position = 0;
    int next = 0;
    while (1) {
        next = fgetc(file);
        if (next == EOF || next == '\n') {
            result[position] = '\0';
            return result;
        } else {
            result[position++] = (char)next;
        }
    }
}

/*initalizes the deck from a given deckfile name and adds a deckCount
 * in order to check the deckfiles validity when stating amout of 
 * cards*/
struct Card* init_deck(char* file, int* deckCount) {
    FILE* gameFile;
    gameFile = fopen(file, "r");
    if (gameFile == NULL) {
        fprintf(stderr, "Unable to parse deckfile\n");
        exit(3);
    }
    int numberOfCards;
    numberOfCards = atoi(read_line(gameFile));
    if (numberOfCards < 11) {
        fprintf(stderr, "Short deck\n");
        exit(5);
    } 
    struct Card* card = NULL;
    struct Card* temp = realloc(card, sizeof(struct Card) * numberOfCards);
    if (temp != NULL) {
        card = temp; 
    }
    for (int i = 0; i < numberOfCards; i++) {
        char* temp = read_line(gameFile);
        int size = strlen(temp);
        if (size > 2) {
            fprintf(stderr, "Unable to parse file deckfile\n");
            exit(3);
        }
        char suit = temp[1];
        char val = temp[0];
        int number = val - '0';

        if (isalpha(suit) == 0) {
            fprintf(stderr, "Unable to parse deckfile");
            exit(3);
        } else if (number > 9 || number < 1) {
            fprintf(stderr, "Unable to parse deckfile");
            exit(3);
        }
        card[i].suit = suit;
        card[i].number = number;
        ++*deckCount;
    }
    if (*deckCount != numberOfCards) {
        fprintf(stderr, "Unable to parse deckfile\n");
        exit(3);
    }
    fclose(gameFile);
    return card;
}

/*checks given parameters are within the given constraints and exits 
 * using a specific number when they do not fall within*/
void code_check(char* p1, char* p2, int width, int height) {
    if (width < 2 || width > 101 || height < 2 || height > 101) {
        fprintf(stderr, "Incorrect arg types\n");
        exit(2);
    }
    if (strcmp(p1, "a") && strcmp(p1, "h")) {
        fprintf(stderr, "Incorrect arg types\n");
        exit(2);
    }
    if (strcmp(p2, "a") && strcmp(p2, "h")) {
        fprintf(stderr, "Incorrect arg types\n");
        exit(2);
    }
}

/*When called upon the function will draw a current version of the given
 * board including cards and lays the deck out in a width x height
 * format*/
void draw_board(struct Card** board, int width, int height) {   
    for (int x = 1; x < height + 1; x++) {
        for (int y = 1; y < width + 1; y++) {
            if (board[y][x].number == 0) {
                printf(".."); 
            } else {
                printf("%d%c", board[y][x].number, board[y][x].suit);
            }
        }
        printf("\n");
    }
}

/*Used when starting a new game or loading a saved game. It initializes the
 * board that will be used during that game and sets all spaces to 0 (..)*/
struct Card** create_board(int width, int height) {
    struct Card** board = malloc(width * (sizeof(struct Card*) * 2));
    for (int i = 1; i < width + 1; i++) {
        board[i] = (struct Card*)malloc(height * (sizeof(struct Card) * 2));
    }
    for (int x = 1; x < height + 1; x++) {
        for (int y = 1; y < width + 1; y++) {
            board[y][x].number = 0;
            board[y][x].suit = 0;
            board[y][x].score = 1;
        } 
    }
    return board;
}

/*This function is used when either initializing the two players first hand,
 * giving the given player a hand of 6 as the function is called before each
 * platers turn. If the players hand count (a way to track the amount of cards
 * each player has) = 5 it will add a card from the deck to the end of the 
 * hand.*/
struct Card* hand(struct Card* deck, int* deckCount, int* handCount, 
        struct Card* hand, int* emptyCards) {
    if (*emptyCards == *deckCount) {
        return hand;
    } else if (*handCount == 5) {
        hand[5] = deck[*emptyCards];
        deck[*emptyCards].number = 0;
        ++*emptyCards;
        ++*handCount;
        return hand;
    } else if (*handCount == 6) {
        return hand;
    } else {
        for (int x = 0; x < 5; x++) {
            hand[x] = deck[*emptyCards];
            ++*handCount;
            deck[*emptyCards].number = 0;
            ++*emptyCards;
        }
        return hand;
    }
    return hand;
}

/*Cheks if the given corner is able to be placed given a particular row and 
 * col on the given board. If the function returns 1 it is able to be placed 
 * given the surronding cards (^V<>). If returns 2 the co-ordinates passed 
 * through was not a corner.*/
int is_free(int pos, struct Card** board, int width, int height, int row, 
        int col) {
    if (pos == 1) {
        if (board[1][height].number != 0 || board[width][1].number != 0 || 
                board[col + 1][1].number != 0 || 
                board[col][row + 1].number != 0) {
            return 1;
        }
    } else if (pos == 2) {
        if (board[1][1].number != 0 || board[width][height].number != 0 || 
                board[1][height - 1].number != 0 || 
                board[col + 1][row].number != 0) {      
            return 1;
        }
    } else if (pos == 3) {
        if (board[width][1].number != 0 || board[1][height].number != 0 || 
                board[width - 1][height].number != 0 || 
                board[width][height - 1].number != 0) {           
            return 1;
        }
    } else if (pos == 4) {
        if (board[1][1].number != 0 || board[width][height].number != 0 || 
                board[width - 1][1].number != 0 ||
                board[width][row + 1].number != 0) { 
            return 1;
        }
    } else {
        return 2;
    }
    return 0;
}

/*Cheks if the given side is able to be placed given a particular row, col,
 * board. If the function returns 1 it is able to be placed given the
 * surronding cards (^V<>). If returns 2 the co-ordinates passed through was 
 * not a side.*/
int is_free_side(int pos, struct Card** board, int row, int col, int width, 
        int height) {
    if (pos == 1) {
        if (board[col][height].number != 0 || 
                board[col - 1][row].number != 0 ||
                board[col + 1][row].number != 0 || 
                board[col][row + 1].number != 0) {
            return 1;
        }
    } else if (pos == 2) {
        if (board[1][row].number != 0 || board[width][row - 1].number != 0 ||
                board[width][row + 1].number != 0 || 
                board[width - 1][row].number != 0) {
            return 1;
        }
    } else if (pos == 3) {
        if (board[col][1].number != 0 || board[col][row - 1].number != 0 || 
                board[col + 1][row].number != 0 || 
                board[col - 1][row].number != 0) {
            return 1;
        }
    } else if (pos == 4) {
        if (board[width][row].number != 0 || 
                board[col + 1][row].number != 0 || 
                board[col][row + 1].number != 0 || 
                board[col][row - 1].number != 0) { 
            return 1;
        }
    } else {
        return 2;
    }
    return 0;
}

/*Checks which corner the given row and col is in. 1 = topleft, 2 = bottomleft,
 * 3 = bottomright, 4 = topright. It then passes through is_free to check
 * if it can be placed. If a paramter is equalled to 1 however, the 
 * function will return the corner itself (Used for scoring).*/
int corner_check(struct Card** board, int row, int col, int width, int height, 
        int number) {
    int corner;
    if ((row == 1) && (col == 1)) {
        corner = 1;
    } else if ((col == 1) && (row == height)) { 
        corner = 2;
    } else if ((col == width) && (row == height)) {
        corner = 3;
    } else if ((col == width) && (row == 1)) {
        corner = 4;
    } else {
        return 5;    
    }
    if (number == 0) {
        return is_free(corner, board, width, height, row, col);
    } else {
        return corner;
    }
}


/*Checks what side the given row and col is on. 1 = top, 2 = right,
 * 3 = bottom, 4 = left. It then passes through is_free_side to check
 * if it can be placed. If a paramter is equalled to 1 however, the
 * function will return the side itself (Used for scoring).*/
int side_check(struct Card** board, int row, int col, int width, int height, 
        int corner, int number) {
    int side;
    if (corner != 0) {
        if (row == 1) {
            side = 1;
        } else if (col == width) {
            side = 2;
        } else if (row == height) {
            side = 3;
        } else if (col == 1) {
            side = 4;
        } else {
            return 5;
        }
    } else {
        return 0;
    }
    if (number == 0) {
        return is_free_side(side, board, row, col, width, height);
    } else {
        return side;
    }
}

/*Board check combines both corner and side check function and checks wheter:
 * 1. The board is empty, in which case it is ok to place a card where ever.
 * 2. If board !empty, whether the given row and col is a corner or side
 *       where if neither, it will just search around the card basically.
 * 3. If a corner or side = 1, returns 1 (can be placed)*/
int board_check(struct Card** board, int row, int col, int width, int height) {
    int emptyBoard = 0;
    int breaks = 0;
    for (int i = 1; i < height + 1 && breaks == 0; i++) {
        for (int j = 1; j < width + 1 && breaks == 0; j++) {
            if (board[j][i].number != 0) {
                emptyBoard++;
                breaks++;
            }
        }
    }  
    if (emptyBoard != 0) { 
        if (board[col][row].number != 0) {
            return 0;
        }
        int corner = corner_check(board, row, col, width, height, 0);
        int side = side_check(board, row, col, width, height, corner, 0);
        if (corner == 5 && side == 5) {
            if (board[col - 1][row].number == 0 && 
                    board[col + 1][row].number == 0 && 
                    board[col][row + 1].number == 0 && 
                    board[col][row - 1].number == 0) { 
                return 0;
            } else {
                return 1;
            }
        } else if (corner == 0 && side == 0) {
            return 0; 
        } else if ((corner != 0 && corner != 5) || (side != 5 && side != 0)) {
            return 1;
        }
    } else {
        return 1;
    }
    return 0;
}

/*Prints the hand based on the type of player. Always printing 6 cards*/
void print_hand(struct Card* theHand, int player, int type) {
    if (type) {
        printf("Hand:");
    } else {
        printf("Hand(%d):", player);
    }
    for (int x = 0; x < 6; x++) {
        printf(" ");
        printf("%d%c", theHand[x].number, theHand[x].suit);
    }
    printf("\n");
}

/*Place shuffle places a card on the board, and then shuffles the cards 
 * so that a function mention prior (hand) is able to add another card
 * when called upon. The function sets the choosen card to 0 and replaces
 * it with the card to its right, doing this process untill the 5th index
 * is 0.*/ 
void place_shuffle(struct Card* theHand, struct Card** board, int row, 
        int col, int card, int* handCount) { 
    int placementIndex = card;
    struct Card placementCard = theHand[placementIndex - 1];
    for (int i = placementIndex - 1; i < 6; i++) {
        if (i == 5) {
            theHand[i].number = 0;
        } else {
            theHand[i] = theHand[i + 1];
        }
    }
    --*handCount;
    board[col][row] = placementCard;
}

/*Checks if the save name meets the given constraints, return 1 if so*/
int check_save(char* saveFile) {
    int alpha = 0;
    char* legitName = malloc(sizeof(char) * 20);
    int size = strlen(saveFile);
    strncpy(legitName, saveFile + 4, size - 4);

    if (legitName == NULL) {
        fprintf(stdout, "Unable to save\n");
        return 0;
    }
    for (int i = 0; i < strlen(legitName); i++) {
        if (isalpha(legitName[i])) {
            alpha++;
        }
    }
    if (alpha == 0) {
        fprintf(stdout, "Unable to save\n");
        return 0;
    }
    return 1;
}

/*Save game saves the current state of the game if the humans's savefile 
 * passes the check_save function. If it does, then a file is created,
 * paramaters are printed onto it and the file is closed. The game then
 * continues as normal.*/
void save_game(char* saveFile, int width, int height, int* emptyCards, 
        int player, char* deckName, struct Card* phand1, 
        struct Card* phand2, struct Card** board) {
    char* legitName = malloc(sizeof(char) * 80);
    FILE* outputFile;
    struct Card* p1Hand;
    struct Card* p2Hand;

    if (player == 1) {
        p1Hand = phand1;
        p2Hand = phand2;
    } else {
        p1Hand = phand2;
        p2Hand = phand1;
    }
    strncpy(legitName, saveFile + 4, strlen(saveFile) - 4);
    outputFile = fopen(legitName, "w");
    fflush(stdout);
    fprintf(outputFile, "%d %d %d %d\n", width, height, *emptyCards, player);
    fprintf(outputFile, "%s\n", deckName);
    for (int i = 0; i < 6; i++) {
        if (p1Hand[i].number == 0) {
            break;
        }
        fprintf(outputFile, "%d%c", p1Hand[i].number, p1Hand[i].suit);
    }
    fprintf(outputFile, "\n");
    for (int i = 0; i < 6; i++) {
        if (p2Hand[i].number == 0) {
            break;
        }
        fprintf(outputFile, "%d%c", p2Hand[i].number, p2Hand[i].suit);
    }
    fprintf(outputFile, "\n");
    for (int i = 1; i < height + 1; i++) {
        for (int j = 1; j < width + 1; j++) {
            if (board[j][i].number == 0) {
                fprintf(outputFile, "**");
            } else {
                fprintf(outputFile, "%d%c", board[j][i].number, 
                        board[j][i].suit);
            }
        }
        fprintf(outputFile, "\n");
    }
    fflush(outputFile);
    fclose(outputFile);
}

/*Human turn is a void function which will first check if the game is over, 
 * acting accordningly if so. It then picks up a card form the deck
 * given the hand, and prints the hand. It then prompts the player to enter 
 * paramaters used to place a chosen card on the deck. Using a function before
 * it checks whether the move is valid, assuming the constraints are valid also
 * and then places it, shuffles the hand, and redraws the board. Here a player 
 * can decide whether they want to save the game or not through the prompt.*/
void human_turn(int player, struct Card* theHand, struct Card** board, 
        struct Card* deck, int* deckCount, int* handCount, 
        int* emptyCards, int width, int height, char* deckName, 
        struct Card* opHand) {
    if (is_game_over(board, deckCount, emptyCards, width, height)) {
        cal_score(board, width, height);
        exit(0);
    }
    hand(deck, deckCount, handCount, theHand, emptyCards);
    int card, row, col;
    int type = 0;
    print_hand(theHand, player, type);
    while (1) { 
        printf("Move? "); 
        char* input = read_line(stdin);
        if (input == '\0') {
            continue;
        } 
        if (feof(stdin) == 1) {
            fprintf(stderr, "End of input\n");
            exit(7);
        }
        if (strncmp(input, "SAVE", 4) == 0) {
            if (check_save(input)) {
                save_game(input, width, height, emptyCards, player, deckName, 
                        theHand, opHand, board);
                continue;
            }
            continue;
        }
        sscanf(input, "%d %d %d", &card, &col, &row);
        if (card > 6 || card <= 0) {
            continue; 
        } else if (row > height || row <= 0 || col > width || col <= 0) {
            continue;
        } else if (strlen(input) < 5) {
            continue;
        } else if (board_check(board, row, col, width, height) == 0) {
            continue; 
        } else {
            place_shuffle(theHand, board, row, col, card, handCount);
            draw_board(board, width, height);
            break;
        }   
    }
}

/*AI is a function created for the 'a' type or automated, essentially 
 * picking the first card in its deck to place down. The AI will search
 * by seeing if it can place the card given generated row and cols. If 
 * the AI's turn is 1, it will search the board from left to right, top to 
 * bottom. If the AI's turn is 2 it will search the board from right to left, 
 * bottom to top. If its the first play, they will place in the center
 * of the board. After every turn it will redraw the deck.*/
void ai(int player, struct Card* theHand, struct Card** board, struct Card* 
        deck, int* deckCount, int* handCount, int* emptyCards, 
        int width, int height) {
    int type = 1;
    if (is_game_over(board, deckCount, emptyCards, width, height)) {
        cal_score(board, width, height);
    }
    hand(deck, deckCount, handCount, theHand, emptyCards); 
    print_hand(theHand, player, type);
    int emptyBoard = 0;
    int check = 0;
    for (int i = 1; i < height + 1; i++) {
        for (int j = 1; j < width + 1; j++) {
            if (board[j][i].number != 0) {        
                emptyBoard++;
            }
        }
    } 
    if (player == 1 && emptyBoard != 0) { 
        for (int i = 1; i < height + 1 && check == 0; i++) { 
            for (int j = 1; j < width + 1 && check == 0; j++) { 
                if (board_check(board, i, j, width, height)) {
                    place_shuffle(theHand, board, i, j, 1, handCount);
                    fprintf(stdout, "Player %d plays %d%c in column %d row",
                            player, board[j][i].number, board[j][i].suit,
                            j);
                    fprintf(stdout, " %d\n", i);
                    check++;
                }
            }
        }
    } else if (player == 2 && emptyBoard != 0) { 
        for (int i = height; i > 0 && check == 0; i--) {         
            for (int j = width; j > 0 && check == 0; j--) {      
                if (board_check(board, i, j, width, height)) {
                    place_shuffle(theHand, board, i, j, 1, handCount);
                    fprintf(stdout, "Player %d plays %d%c in column %d row",
                            player, board[j][i].number, board[j][i].suit, j);
                    fprintf(stdout, " %d\n", i);
                    check++;
                }
            }
        }
    } else { 
        place_shuffle(theHand, board, ((height + 1) / 2), ((width + 1) / 2), 1,
                handCount);
        fprintf(stdout, "Player %d plays %d%c in column %d row %d\n", player, 
                board[(width + 1) / 2][(height + 1) / 2].number,
                board[(width + 1) / 2][(height + 1) / 2].suit, 
                (width + 1) / 2, (height + 1) / 2);
    }
    draw_board(board, width, height);
}

/*Checks if any cards have been placed, or are present on the given board
 * returning 1 if completely full*/
int is_board_full(struct Card** board, int width, int height) {
    for (int i = 1; i < height + 1; i++) {
        for (int j = 1; j < width + 1; j++) {
            if (board[j][i].number == 0) {
                return 0;
            }
        }
    }
    return 1;
}

/*Checks if the game is over by give constraints, the deck having no more
 * playable cards, or the board being full.*/ 
int is_game_over(struct Card** board, int* deckCount, int* emptyCards, 
        int width, int height) {
    if (*deckCount == *emptyCards) {
        return 1;
    } else {  
        return is_board_full(board, width, height);
    }
    return 0;
}


/*Play game essentially checks what types the players are, whoses turn is it 
 * (if new game turn = 1), and if the game is over. If the game isnt over, 
 * the function runs playing combonations of h and a types until the game is
 * over or specified otherwise.*/
void play_game(char* p1, char* p2, char* deckName, struct Card* p1hand, 
        struct Card* p2hand, struct Card** board, struct Card* deck, 
        int* dCount, int* p1HandCount, int* p2HandCount, int* eCards, 
        int w, int h, int turn) {
    while (is_game_over(board, dCount, eCards, w, h) == 0) {
        if (*p1 == 'h' && *p2 == 'h') {
            if (turn == 1) {
                human_turn(1, p1hand, board, deck, dCount, p1HandCount, 
                        eCards, w, h, deckName, p2hand);
                human_turn(2, p2hand, board, deck, dCount, p2HandCount, 
                        eCards, w, h, deckName, p1hand);
            } else {
                human_turn(2, p2hand, board, deck, dCount, p2HandCount, 
                        eCards, w, h, deckName, p1hand);
                human_turn(1, p1hand, board, deck, dCount, p1HandCount, 
                        eCards, w, h, deckName, p2hand);
            }
        } else if (*p1 == 'h' && *p2 == 'a') {
            if (turn == 1) {
                human_turn(1, p1hand, board, deck, dCount, p1HandCount,
                        eCards, w, h, deckName, p1hand);
                ai(2, p2hand, board, deck, dCount, p2HandCount, eCards, w, h);
            } else {
                ai(2, p2hand, board, deck, dCount, p2HandCount, eCards, w, h);
                human_turn(1, p1hand, board, deck, dCount, p1HandCount, eCards,
                        w, h, deckName, p1hand);
            }
        } else if (*p1 == 'a' && *p2 == 'h') {
            if (turn == 1) {
                ai(1, p1hand, board, deck, dCount, p1HandCount, eCards, w, h);
                human_turn(2, p2hand, board, deck, dCount, p2HandCount, 
                        eCards, w, h, deckName, p1hand);
            } else {
                human_turn(2, p2hand, board, deck, dCount, p2HandCount, 
                        eCards, w, h, deckName, p1hand);
                ai(1, p1hand, board, deck, dCount, p1HandCount, eCards, w, h);
            }
        } else if (*p1 == 'a' && *p2 == 'a') {
            if (turn == 1) {
                ai(1, p1hand, board, deck, dCount, p1HandCount, eCards, w, h);
                ai(2, p2hand, board, deck, dCount, p2HandCount, eCards, w, h);
            } else {
                ai(2, p2hand, board, deck, dCount, p2HandCount, eCards, w, h);
                ai(1, p1hand, board, deck, dCount, p1HandCount, eCards, w, h); 
            }
        }
    }
}

/*Used when loading a game, the function adds given cards to a given hand,
 * depending on the length of string inputed. (10 = 5 cards, 12 = 6 cards)*/
void add_cards(struct Card* hand, char* cards, int* handCount) {
    int size = strlen(cards);
    int counter = 0;
    for (int i = 0; i < size; i++) {
        char suit = cards[i + 1];
        int number = cards[i] - '0';
        if (isalpha(suit) == 0) {
            printf("Unable to load");
            exit(3);
        }
        hand[counter].number = number;
        hand[counter].suit = toupper(suit);
        i++;
        counter++;
        ++*handCount;
    }
}

/*Load board initializes a given board (in the form of strings),
 * adding cards to the board in the correct location or entering 0 cards
 * into the boards spaces. Checks if the board given is full and exits
 * accordingly. */
void load_board(FILE* load, int height, int width, struct Card** board) {
    char* card = malloc(sizeof(char) * 20);
    for (int i = 1; i < height + 1; i++) {
        int counter = 1;
        char* row = read_line(load);
        sscanf(row, "%s", card);
        if (*card == EOF) {
            fprintf(stdout, "Unable to parse load file");
        }
        for (int j = 0; j < width * 2; j++) {
            if (card[j] != '*' && card[j + 1] != '*') {
                board[counter][i].number = card[j] - '0';
                board[counter][i].suit = card[j + 1];
                counter++;
            } else {
                counter++;
            }
            j++;
        }
    }
    draw_board(board, width, height);
    if (is_board_full(board, width, height)) {
        fprintf(stderr, "Board full");
        exit(6);
    }
}

/*If Up checks all the possible spaces above a given row and col
 * on the board and returns the cards co-ordinates if it
 * exists. If Not returns 0.*/
int* if_up(int row, int col, int width, int height, struct Card** board, 
        int number, int corner, int side) {
    int* coup = (int*)calloc(2, sizeof(int));  
    if (corner != 5) {
        if (corner == 1 || corner == 4) {
            if (board[col][height].number > number) { 
                coup[0] = col;
                coup[1] = height;
            } else {
                coup[0] = 0;
            }
        } else if (corner == 2 || corner == 3) { 
            if (board[col][row - 1].number > number) {
                coup[0] = col;
                coup[1] = row - 1;
            } else { 
                coup[0] = 0;
            }
        }
    } else if (corner == 5 && side != 5) {
        if (side == 1) {
            if (board[col][height].number > number) {
                coup[0] = col;
                coup[1] = height;
            } else {
                coup[0] = 0;
            }
        } else if (side == 2 || side == 3 || side == 4) {
            if (board[col][row - 1].number > number) {
                coup[0] = col;
                coup[1] = row - 1;
            } else {
                coup[0] = 0;
            }
        }
    } else {
        if (board[col][row - 1].number > number) {
            coup[0] = col;
            coup[1] = row - 1;
        } else {
            coup[0] = 0;
        }
    } 
    return coup;
}

/*If Down checks all the possible spaces below given row and col 
 * on the board and returns the cards co-ordinates if it 
 * exists. If Not returns 0.*/
int* if_down(int row, int col, int width, int height, struct Card** board, 
        int number, int corner, int side) {
    int* codown = malloc(sizeof(int) * 2); 
    if (corner != 5) {
        if (corner == 1 || corner == 4) {
            if (board[col][row + 1].number > number) {
                codown[0] = col;
                codown[1] = row + 1;
            } else {
                codown[0] = 0;
            }
        } else if (corner == 2 || corner == 3) {
            if (board[col][1].number > number) {
                codown[0] = col;
                codown[1] = 1;
            } else {
                codown[0] = 0;
            }
        }
    } else if (corner == 5 && side != 5) {
        if (side == 1 || side == 2 || side == 4) {
            if (board[col][row + 1].number > number) {
                codown[0] = col;
                codown[1] = row + 1;
            } else {
                codown[0] = 0;
            }
        } else if (side == 3) {
            if (board[col][1].number > number) {
                codown[0] = col;
                codown[1] = 1;
            } else {
                codown[0] = 0;
            }
        }
    } else {
        if (board[col][row + 1].number > number) {
            codown[0] = col;
            codown[1] = row + 1;
        } else {
            codown[0] = 0;
        }
    }
    return codown;
}

/*If Right checks all the possible spaces to the right of a given row and col
 * on the board and returns the cards co-ordinates if it exists. 
 * If Not returns 0.*/
int* if_right(int row, int col, int width, int height, struct Card** board, 
        int number, int corner, int side) {
    int* coright = malloc(sizeof(int) * 2);
    if (corner != 5) {
        if (corner == 1 || corner == 2) {
            if (board[col + 1][row].number > number) {
                coright[0] = col + 1;
                coright[1] = row;
            } else {
                coright[0] = 0;
            }
        } else if (corner == 3 || corner == 4) {
            if (board[1][row].number > number) {
                coright[0] = 1;
                coright[1] = row;
            } else {
                coright[0] = 0;
            }
        }
    } else if (corner == 5 && side != 5) {
        if (side == 1 || side == 3 || side == 4) {
            if (board[col + 1][row].number > number) {
                coright[0] = col + 1;
                coright[1] = row;
            } else {
                coright[0] = 0;
            }
        } else if (side == 2) {
            if (board[1][row].number > number) {
                coright[0] = 1;
                coright[1] = row;
            } else {
                coright[0] = 0;
            }
        }
    } else {
        if (board[col + 1][row].number > number) {
            coright[0] = col + 1;
            coright[1] = row;
        } else {
            coright[0] = 0;
        }
    }
    return coright;
}

/*If Left checks all the possible spaces to the left of a given row and col
 * on the board and returns the cards co-ordinates if it exists. 
 * If Not returns 0.*/
int* if_left(int row, int col, int width, int height, struct Card** board, 
        int number, int corner, int side) {
    int* coleft = malloc(sizeof(int) * 2);    
    if (corner != 5) {
        if (corner == 1 || corner == 2) {
            if (board[width][row].number > number) {
                coleft[0] = width;
                coleft[1] = row;
            } else {
                coleft[0] = 0;
            }
        } else if (corner == 3 || corner == 4) {
            if (board[col - 1][row].number > number) {
                coleft[0] = col - 1;
                coleft[1] = row;
            } else {
                coleft[0] = 0;
            }
        }
    } else if (corner == 5 && side != 5) {
        if (side == 1 || side == 2 || side == 3) {
            if (board[col - 1][row].number > number) {
                coleft[0] = col - 1;
                coleft[1] = row;
            } else { 
                coleft[0] = 0;
            }
        } else if (side == 4) {
            if (board[width][row].number > number) {
                coleft[0] = width;
                coleft[1] = row;
            } else { 
                coleft[0] = 0;
            }
        }
    } else {
        if (board[col - 1][row].number > number) {
            coleft[0] = col - 1;
            coleft[1] = row;
        } else {
            coleft[0] = 0;
        }
    }
    return coleft;
}

/*The recursive function checks if there is any valid cards around a given card
 * and if so it will call itself on each valid card to check for higher cards.
 * If it cannot find any it will check if the cards suit matches the original 
 * suit and if so will return the amoubt of cards it has passed. If not it will
 * return a score of the most recent card with the same suit it passed prior.*/
int recursive(char suit, int steps, int col, int row, int w, int h, 
        int type, struct Card** board, int final) {
    int* scores = malloc(sizeof(int) * 4);
    for (int i = 0; i < 4; i++) {
        scores[i] = 0;
    }
    steps++;
    int c = corner_check(board, row, col, w, h, 1);
    int s = side_check(board, row, col, w, h, 1, 1); 
    int* up = if_up(row, col, w, h, board, board[col][row].number, c, s); 
    int* down = if_down(row, col, w, h, board, board[col][row].number, c, s);
    int* right = if_right(row, col, w, h, board, board[col][row].number, c, s);
    int* left = if_left(row, col, w, h, board, board[col][row].number, c, s);
    if (type == 0) {    
        down[0] = 0;
    } else if (type == 1) {
        up[0] = 0;
    } else if (type == 2) {
        left[0] = 0;
    } else if (type == 3) {
        right[0] = 0;
    }
    if (up[0] == 0 && down[0] == 0 && left[0] == 0 && right[0] == 0) { 
        if (board[col][row].suit == suit) {
            return steps;
        } 
    } else {
        final = (board[col][row].suit == suit) ? steps : final;
        if (up[0] != 0) {
            scores[0] = recursive(suit, steps, up[0], up[1], w, h, 0, board, 
                    final);
        } 
        if (down[0] != 0) {
            scores[1] = recursive(suit, steps, down[0], down[1], w, 
                    h, 1, board, final);
        }
        if (right[0] != 0) {
            scores[2] = recursive(suit, steps, right[0], right[1], w, 
                    h, 2, board, final);
        }
        if (left[0] != 0) {
            scores[3] = recursive(suit, steps, left[0], left[1], w, h,
                    3, board, final);
        }
        for (int i = 0; i < 4; i++) {
            if(scores[i] > final) {
                final = scores[i];
            }
        }
    }
    return final;
}

/*Prints the highest score from each persons valid hands.*/
void print_score(struct Card** board, int width, int height) {
    struct Card* p1Score = malloc(sizeof(struct Card) * (width * height));
    struct Card* p2Score = malloc(sizeof(struct Card) * (width * height));
    int p1Counter = 0;
    int p2Counter = 0;
    int p1 = 0;
    int p2 = 0;
    for (int i = 1; i < height + 1; i++) {
        for (int j = 1; j < width + 1; j++) {
            if (board[j][i].number != 0) {
                if (board[j][i].suit % 2 == 1) {
                    p1Score[p1Counter].score = board[j][i].score;
                    p1Counter++;
                } else {
                    p2Score[p2Counter].score = board[j][i].score;
                    p2Counter++;
                }
            }
        }
    }
    for (int i = 0; i < p1Counter; i++) {
        if (p1Score[i].score > p1) {
            p1 = p1Score[i].score;
        }
    }
    for (int i = 0; i < p2Counter; i++) {
        if (p2Score[i].score > p2) {
            p2 = p2Score[i].score;
        }
    }
    fprintf(stdout, "Player 1=%d Player 2=%d\n", p1, p2);
    exit(0);
}


/*Calculates the score of each card by checking all cards around itself,
 * then using a recusrive function checks their cards around them, etc.
 * they then return the highest path and from that 4 are returned. From
 * those 4 the highest will be set to the struct Card scord.*/
void cal_score(struct Card** board, int w, int h) {
    for (int i = 1; i < h + 1; i++) {
        for (int j = 1; j < w + 1; j++) {
            int c = corner_check(board, i, j, w, h, 1);
            int s = side_check(board, i, j, w, h, 1, 1);
            int* up = if_up(i, j, w, h, board, board[j][i].number, c, s);
            int* down = if_down(i, j, w, h, board, board[j][i].number, c, s);
            int* right = if_right(i, j, w, h, board, board[j][i].number, c, s);
            int* left = if_left(i, j, w, h, board, board[j][i].number, c, s);
            if (board[j][i].number != 0) {
                int score = 1;
                int steps = 1;
                int final = 1;
                char suit = board[j][i].suit;
                int* scores = malloc(sizeof(int) * 4);
                for (int i = 0; i < 4; i++) {
                    scores[i] = 0;
                }
                if (up[0] != 0) { 
                    int temp = recursive(suit, steps, up[0], up[1], w, 
                            h, 0, board, final);    
                    scores[0] = temp;
                }
                if (down[0] != 0) {
                    int temp = recursive(suit, steps, down[0], down[1], 
                            w, h, 1, board, final); 
                    scores[1] = temp; 
                }    
                if (right[0] != 0) {
                    int temp = recursive(suit, steps, right[0], right[1], 
                            w, h, 2, board, final);
                    scores[2] = temp;
                }
                if (left[0] != 0) {
                    int temp = recursive(suit, steps, left[0], left[1], 
                            w, h, 3, board, final);
                    scores[3] = temp;
                }
                for (int i = 0; i < 4; i++) {
                    score = (scores[i] > score) ? scores[i] : score;
                }
                board[j][i].score = score;
            }
        }
    }
    print_score(board, w, h);
}

/*Loads the game from a given file by reading each line and returning 
 * it as a string, and basis player types on an input.
 * Once the loaded game is over, it will call the cal_score function
 * and return the scores of the game.*/
void load_game(char* argv[]) { 
    struct Card* p1Hand = malloc(sizeof(struct Card) * 6);
    struct Card* p2Hand = malloc(sizeof(struct Card) * 6);
    int p1HandCount = 0;
    int p2HandCount = 0;
    int deckCount = 0;
    int width, height, emptyCards, turn;
    FILE* load = fopen(argv[1], "r");
    char* deckName = malloc(sizeof(char) * 20);
    int lineNo = 2;
    code_check(argv[2], argv[3], 3, 3);
    if (load == NULL) {
        fprintf(stderr, "Unable to parse savefile\n");
        exit(4);
    }
    char* firstLine = read_line(load);
    sscanf(firstLine, "%d %d %d %d", &width, &height, &emptyCards, &turn);
    struct Card** board = create_board(width, height);
    struct Card* fullDeck = malloc(sizeof(struct Card) * deckCount);
    code_check(argv[2], argv[3], width, height);
    while (1) {
        char* temp = read_line(load);
        if (lineNo == 2) {
            sscanf(temp, "%s", deckName);
            if (deckName[strlen(deckName) - 1] == '/') {
                fprintf(stderr, "Unable to parse deckfile\n");
                exit(3);
            }
            fullDeck = init_deck(deckName, &deckCount);
            lineNo++;
        } else if (lineNo == 3) {
            char* temps = malloc(sizeof(char) * 20);
            sscanf(temp, "%s", temps);
            add_cards(p1Hand, temps, &p1HandCount);    
            lineNo++;
        } else if (lineNo == 4) {
            char* temps = malloc(sizeof(char) * 20);
            sscanf(temp, "%s", temps);
            add_cards(p2Hand, temps, &p2HandCount);
            lineNo++;
            load_board(load, height, width, board);
            play_game(argv[2], argv[3], deckName, p1Hand, p2Hand, board, 
                    fullDeck, &deckCount, &p1HandCount, &p2HandCount, 
                    &emptyCards, width, height, turn);
        }

    }
    if(is_game_over(board, &deckCount, &emptyCards, width, height)) { 
        cal_score(board, width, height);
    }
}

/*Starts a new game given paramaters and player types. If the game ends 
 * cal_score is called and the scores are printed and the game ends.*/
void start_game(char* argv[]) {
    int deckCount = 0;
    int emptyCards = 0;
    struct Card** board;
    int p1HandCount = 0;
    int p2HandCount = 0;
    int width = atoi(argv[2]);
    int height = atoi(argv[3]);
    char* deckName = malloc(sizeof(char) * 20);
    code_check(argv[4], argv[5], width, height);
    deckName = argv[1];
    struct Card* fullDeck = init_deck(deckName, &deckCount);
    struct Card* p1Hand;
    p1Hand = malloc(sizeof(struct Card) * 6);
    struct Card* p2Hand;
    p2Hand = malloc(sizeof(struct Card) * 6);
    hand(fullDeck, &deckCount, &p1HandCount, p1Hand, &emptyCards);
    hand(fullDeck, &deckCount, &p2HandCount, p2Hand, &emptyCards);

    board = create_board(width, height);
    draw_board(board, width, height);
    play_game(argv[4], argv[5], deckName, p1Hand, p2Hand, board, fullDeck, 
            &deckCount, &p1HandCount, &p2HandCount, &emptyCards, width, 
            height, 1);
    if(is_game_over(board, &deckCount, &emptyCards, width, height)) { 
        free(p1Hand);
        free(p2Hand);
        free(fullDeck);
        cal_score(board, width, height);
        exit(0);
    }
}


/*int main(int argc, char** argv) {
    if (argc != 6 && argc != 4) {  
        fprintf(stderr, "Usage: bark savefile p1type p2type\nbark deck width");
        fprintf(stderr, " height p1type p2type\n");
        exit(1);
    } else if (argc == 6) {
        start_game(argv);
    } else if (argc == 4) {
        load_game(argv);
    }
    return 0;
}*/

int sarray(int n, int t, int* values) {
    

int main(int argc, char** argv) {
    

    return 0;
}
