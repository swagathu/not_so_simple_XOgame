#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "displaygame.h"

const int leftGap = 5;
static int cur_x = 0;
static int cur_y = 0;

int draw_value_single(int x, int y, int val, int cursor_state);

struct termSize get_terminal_width()
{
    struct winsize w;
    struct termSize t;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
    {
        t.width = -1;
        t.height = -1;
        return t; // Error handling
    }
    t.width = w.ws_col;
    t.height = w.ws_row;
    return t;
}

static inline void printXY(char *data, int x, int y, int len)
{
    cursorXY(x, y);
    cur_x = x + len;
    cur_y = y;
    printf("%s", data);
    return;
}

int display_initGtable(int width, struct game_table *g)
{
    int **val_table;
    int **temp2 = NULL;
    int *temp = NULL;

    g->width = width;
    temp2 = malloc(sizeof(int *) * width);
    if (temp2 == NULL)
    {
        printf("malloc error in val table, %s", strerror(errno));
        return FAILURE;
    }
    val_table = temp2;

    for (int i = 0; i < width; i++)
    {
        temp = malloc(sizeof(int) * width);
        if (temp == NULL)
        {
            printf("malloc error in val_table[%d] loc, %s", i, strerror(errno));
            free(val_table);
            return FAILURE;
        }
        val_table[i] = temp;

        for (int j = 0; j < width; j++)
        {
            val_table[i][j] = -1;
        }
    }

    g->val_table = val_table;
    g->cur_x = 0;
    g->cur_y = 0;
    g->filled = 0;
    return SUCCESS;
}

void display_deinitGtable(struct game_table *g)
{
    free(g->val_table);
}

void display_clearScreen(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

void display_SetCurLoc(int width, int key_input, int *cur_x_loc, int *cur_y_loc)
{
    switch (key_input)
    {
    case UP_ARROW:
        if (*cur_y_loc == 0)
        {
            *cur_y_loc = width - 1;
        }
        else
        {
            *cur_y_loc = *cur_y_loc - 1;
        }
        break;
    case DOWN_ARROW:
        if (*cur_y_loc == (width - 1))
        {
            *cur_y_loc = 0;
        }
        else
        {
            *cur_y_loc = *cur_y_loc + 1;
        }
        break;
    case LEFT_ARROW:
        if (*cur_x_loc == 0)
        {
            *cur_x_loc = width - 1;
        }
        else
        {
            *cur_x_loc = *cur_x_loc - 1;
        }
        break;
    case RIGHT_ARROW:
        if (*cur_x_loc == (width - 1))
        {
            *cur_x_loc = 0;
        }
        else
        {
            *cur_x_loc = *cur_x_loc + 1;
        }
        break;
    default:
        break;
    }
}

int display_setValue(struct game_table *g, int key_input, int turn)
{
    int **table = g->val_table;
    if (key_input != ENTER_KEY)
    {
        return NO_TURN;
    }
    int val = table[g->cur_x][g->cur_y];
    if (val != -1)
    {
        return NO_TURN;
    }
    if (turn == O_CHANCE)
    {
        table[g->cur_x][g->cur_y] = 0;
        g->filled++;
    }
    else
    {
        table[g->cur_x][g->cur_y] = 1;
        g->filled++;
    }
    return TURN_DONE;
}

int display_getWinner(int val)
{
    if (val == X_CHANCE)
    {
        return WIN_X;
    }
    else
    {
        return WIN_O;
    }
}

int display_validateTable(int width, struct game_table *g)
{
    int **table = g->val_table;

    // Check rows
    for (int i = 0; i < width; i++)
    {
        if (table[i][0] != -1)
        {
            int j;
            for (j = 1; j < width; j++)
            {
                if (table[i][j] != table[i][0])
                {
                    break;
                }
            }
            if (j == width)
            {
                return display_getWinner(table[i][0]); // Return the winning player (0 or 1)
            }
        }
    }

    // Check columns
    for (int j = 0; j < width; j++)
    {
        if (table[0][j] != -1)
        {
            int i;
            for (i = 1; i < width; i++)
            {
                if (table[i][j] != table[0][j])
                {
                    break;
                }
            }
            if (i == width)
            {
                return display_getWinner(table[0][j]); // Return the winning player (0 or 1)
            }
        }
    }

    // Check main diagonal
    if (table[0][0] != -1)
    {
        int i;
        for (i = 1; i < width; i++)
        {
            if (table[i][i] != table[0][0])
            {
                break;
            }
        }
        if (i == width)
        {
            return display_getWinner(table[0][0]); // Return the winning player (0 or 1)
        }
    }

    // Check anti-diagonal
    if (table[0][width - 1] != -1)
    {
        int i;
        for (i = 1; i < width; i++)
        {
            if (table[i][width - 1 - i] != table[0][width - 1])
            {
                break;
            }
        }
        if (i == width)
        {
            return display_getWinner(table[0][width - 1]); // Return the winning player (0 or 1)
        }
    }

    if (g->filled == (width * width))
    {
        return DRAW;
    }

    return FAILURE; // No winner yet
}

int display_printWinner(int width, int winner)
{
    struct termSize t_sz = get_terminal_width();
    int term_height = t_sz.height;
    int side_len = width;
    int end_y = (term_height - (6 * side_len)) / 2 + (6 * side_len) % 2 + (side_len * 6) + 1;
    cursorXY(0, end_y);
    if (winner != FAILURE)
    {
        if ((winner == WIN_O) || (winner == WIN_X))
        {
            printf("Player %c has won!\n", (winner == WIN_O) ? 'O' : 'X');
        }
        else if (winner == DRAW)
        {
            printf("Draw , No winner!\n");
        }
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}

int draw_Table(int side_len, struct game_table *game_table)
{
    struct termSize t_sz = get_terminal_width();
    char *line_data = malloc(t_sz.width * sizeof(char));
    int term_width = t_sz.width;
    int term_height = t_sz.height;
    if (t_sz.width == -1 || t_sz.height == -1)
    {
        printf("Error getting terminal size\n");
        return -1;
    }
    if (side_len < 1)
    {
        printf("Invalid side length\n");
        return -1;
    }
    // if (side_len > term_width) {
    //     printf("Side length is too large for terminal\n");
    //     return -1;
    // }

    printXY("+++++++++++", ((t_sz.width - 11) / 2) + (11 % 2), 1, 11);
    printXY("+ XO game +", ((t_sz.width - 11) / 2) + (11 % 2), 2, 11);
    printXY("+++++++++++", ((t_sz.width - 11) / 2) + (11 % 2), 3, 11);

    int gap = (term_width - (6 * side_len)) / 2 + (6 * side_len) % 2;
    cur_y = (term_height - (6 * side_len)) / 2 + (6 * side_len) % 2;
    int i = 0, j = 0;
    // printf("%d , %d", cur_x, cur_y);
    printXY("╔═════", gap, cur_y, 6);
    for (int i = 1; i < side_len; i++)
    {
        // printf("%d , %d", cur_x, cur_y);
        printXY("╦═════", cur_x, cur_y, 6);
    }
    printXY("╗", cur_x, cur_y, 6);
    // printf("\n%d , %d", cur_x, cur_y);

    for (int i = 0; i < side_len * 2 - 1; i++)
    {
        cur_y += 1;
        cur_x = gap;
        if (i % 2 == 0)
        {
            for (int j = 0; j < side_len; j++)
            {
                printXY("║     ", cur_x, cur_y, 6);
            }
            printf("║");
        }
        else
        {
            printXY("╠═════", cur_x, cur_y, 6);
            for (int j = 1; j < side_len; j++)
            {
                printXY("╬═════", cur_x, cur_y, 6);
            }
            printXY("╣", cur_x, cur_y, 6);
        }
    }
    cur_y += 1;
    cur_x = gap;
    printXY("╚═════", gap, cur_y, 6);
    for (int i = 1; i < side_len; i++)
    {
        // printf("%d , %d", cur_x, cur_y);
        printXY("╩═════", cur_x, cur_y, 6);
    }
    printXY("╝", cur_x, cur_y, 6);
    fflush(stdout);
    free(line_data);
    return 0;
}

int draw_value_single(int x, int y, int val, int cursor_state)
{
    struct termSize t_sz = get_terminal_width();
    int term_width = t_sz.width;
    int term_height = t_sz.height;
    int side_len = 3;
    int FirstChar_xpos = (term_width - (6 * side_len)) / 2 + (6 * side_len) % 2 + 3;
    const int yiter = 2;
    const int xiter = 6;
    int FirstChar_ypos = (term_height - (6 * side_len)) / 2 + (6 * side_len) % 2 + 1;
    if (cursor_state)
    {
        printXY("_", FirstChar_xpos + (x * xiter), FirstChar_ypos + (y * yiter), 1);
        return 0;
    }
    else if (val == 0)
    {
        printXY("◯", FirstChar_xpos + (x * xiter), FirstChar_ypos + (y * yiter), 1);
    }
    else if (val == 1)
    {
        printXY("᙭", FirstChar_xpos + (x * xiter), FirstChar_ypos + (y * yiter), 1);
    }
    else if (val == -1)
    {
        printXY(" ", FirstChar_xpos + (x * xiter), FirstChar_ypos + (y * yiter), 1);
    }
    else
    {
        printXY("?", FirstChar_xpos + (x * xiter), FirstChar_ypos + (y * yiter), 1);
    }
    return 0;
}

int draw_values(int width, int **table, int x, int y, int cursor_state)
{
    struct termSize t_sz = get_terminal_width();
    int term_width = t_sz.width;
    int term_height = t_sz.height;
    int side_len = width;
    int FirstChar_xpos = (term_width - (6 * side_len)) / 2 + (6 * side_len) % 2 + 3;
    const int yiter = 2;
    const int xiter = 6;
    int FirstChar_ypos = (term_height - (6 * side_len)) / 2 + (6 * side_len) % 2 + 1;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (i == x && j == y)
            {
                draw_value_single(i, j, table[i][j], cursor_state);
            }
            else
            {
                draw_value_single(i, j, table[i][j], 0);
            }
        }
    }

    return 0;
}
int init_flag = 0;
int display_dispTable(struct game_table *g, int key_input, int turn, int cursor_state, int redraw_table)
{
    // printf("Key Input: %d, Turn: %d, Cursor State: %d, Redraw Table: %d\n", key_input, turn, cursor_state, redraw_table);
    int width = g->width;
    int **table = g->val_table;
    int ret = FAILURE;
    int ret2 = NO_TURN;

    if (width < 1)
    {
        printf("Table width is not valid!\n");
        return FAILURE;
    }

    if (key_input != 0)
    {
        display_SetCurLoc(width, key_input, &(g->cur_x), &(g->cur_y));
        draw_values(width, table, g->cur_x, g->cur_y, cursor_state);
    }

    struct termSize t_sz = get_terminal_width();

    ret2 = display_setValue(g, key_input, turn);

    if (redraw_table || init_flag == 0)
    {
        display_clearScreen();
        draw_Table(width, g);
        draw_values(width, table, g->cur_x, g->cur_y, cursor_state);
        init_flag = 1;
    }
    draw_value_single(g->cur_x, g->cur_y, table[g->cur_x][g->cur_y], cursor_state);
    // Print Current Position
    char print_pos[100] = {0};
    sprintf(print_pos, "Xpos = %d, Ypos = %d, cur_turn: %c", g->cur_x + 1, g->cur_y + 1, (turn == O_CHANCE) ? 'O' : 'X');
    int print_pos_len = strlen(print_pos);
    int print_pos_x = ((t_sz.width - print_pos_len) / 2) + (print_pos_len % 2);
    int print_pos_y = ((t_sz.height - g->width) / 2) + (g->width % 2) - ((g->width * 6) / 2) - 2;
    printXY(print_pos, print_pos_x, print_pos_y, print_pos_len);
    fflush(stdout);
    int winner = FAILURE;
    if (key_input == ENTER_KEY && g->filled >= (width * 2 - 1))
    {
        winner = display_validateTable(width, g);
    }
    if (winner != FAILURE)
    {
        ret = display_printWinner(g->width, winner);
    }
    if (ret == SUCCESS)
    {
        return SUCCESS;
    }
    else
    {
        return ret2;
    }
    return FAILURE;
}

// int main() {
//     struct game_table gtable;

// 	display_initGtable(3, &gtable);
//     display_dispTable(&gtable, ENTER_KEY, X_CHANCE, 0, 1);
//     // while ( 1) {
//         display_dispTable(&gtable, ENTER_KEY, X_CHANCE, 0, 0);
//         // draw_value_single(0, 0, 0, 0);
//         sleep(1);
//         display_dispTable(&gtable, ENTER_KEY, X_CHANCE, 1, 0);
//         // sleep(1);
//     // }
//     printf("\n");
//     return 0;
// }