#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "displaygame.h"

int display_initGtable(int width, struct game_table *g)
{
	int **val_table;
	int ** temp2 = NULL;
	int * temp = NULL;

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

void display_deinitGtable(struct game_table * g) {
	free(g->val_table);
}

void display_clearScreen(void)
{
	printf("\033[2J\033[H");
	fflush(stdout);
}

int display_drawTop(int width)
{

	printf("\n");
	// draw the top line:
	printf("     ╔═════");
	for (int i = 1; i < width; i++)
	{
		printf("╦═════");
	}
	printf("╗\n");
	return SUCCESS;
}

int display_drawRow(int curRow, int width, int **table, int cur_x, int cur_y, int cursor_state)
{
	printf("     ");
	for (int i = 0; i < width; i++)
	{
		if ((cur_y == curRow) && (cur_x == i) && cursor_state)
		{
			printf("║  _  ");
		}
		else
		{
			if (table[i][curRow] == -1)
			{
				printf("║  *  ");
			}
			else
			{
				printf("║  %c  ", (table[i][curRow] == 0) ? 'O' : 'X');
			}
		}
	}
	printf("║\n");

	return SUCCESS;
}

int display_drawRowDiv(int curRow, int width)
{
	if (curRow == (width - 1))
	{
		return SUCCESS;
	}
	printf("     ");
	printf("╠═════");
	for (int i = 1; i < width; i++)
	{
		printf("╬═════");
	}
	printf("╣\n");
	return SUCCESS;
}

int display_drawBottom(int width)
{
	printf("     ");
	printf("╚═════");
	for (int i = 1; i < width; i++)
	{
		printf("╩═════");
	}
	printf("╝\n");
	return SUCCESS;
}

void display_SetCurLoc(int width, int key_input, int *cur_x, int *cur_y)
{
	switch (key_input)
	{
		case UP_ARROW:
			if (*cur_y == 0)
			{
				*cur_y = width - 1;
			}
			else
			{
				*cur_y = *cur_y - 1;
			}
			break;
		case DOWN_ARROW:
			if (*cur_y == (width - 1))
			{
				*cur_y = 0;
			}
			else
			{
				*cur_y = *cur_y + 1;
			}
			break;
		case LEFT_ARROW:
			if (*cur_x == 0)
			{
				*cur_x = width - 1;
			}
			else
			{
				*cur_x = *cur_x - 1;
			}
			break;
		case RIGHT_ARROW:
			if (*cur_x == (width - 1))
			{
				*cur_x = 0;
			}
			else
			{
				*cur_x = *cur_x + 1;
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

int display_printWinner(int winner)
{
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

int display_dispTable(struct game_table *g, int key_input, int turn, int cursor_state)
{
	int width = g->width;
	int **table = g->val_table;
	int ret = FAILURE;
	int ret2 = NO_TURN;

	if (width < 1)
	{
		printf("Table width is not valid!\n");
		return FAILURE;
	}
	display_clearScreen();
	if (key_input != 0) {
		display_SetCurLoc(width, key_input, &(g->cur_x), &(g->cur_y));
	}
	printf("\n\
+++++++++++\n\
+ XO game +\n\
+++++++++++\n\n");
	printf("Xpos = %d, Ypos = %d, cur_turn: %c\n", g->cur_x + 1, g->cur_y + 1, (turn == O_CHANCE) ? 'O' : 'X');

	ret2 = display_setValue(g, key_input, turn);

	display_drawTop(width);

	for (int i = 0; i < width; i++)
	{
		display_drawRow(i, width, table, g->cur_x, g->cur_y, cursor_state);
		display_drawRowDiv(i, width);
	}
	display_drawBottom(width);

	int winner = FAILURE;
	if (key_input == ENTER_KEY && g->filled >= (width * 2 - 1)) {
		winner = display_validateTable(width, g);
	}
	if (winner != FAILURE) { 
		ret = display_printWinner(winner);
	}
	if (ret == SUCCESS)
	{
		return SUCCESS;
	} else {
		return ret2;
	}
	return FAILURE;
}

