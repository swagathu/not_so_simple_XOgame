#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include "displaygame.h"

#define DEF_WIDTH 3
#define BLINKDELAY_MS 1000
#define LOOPDELAY_MS     100

int getinputEvent(void) {
	struct pollfd pfd;

	pfd.fd = STDIN_FILENO;
	pfd.events = POLLIN;

	int timeout = LOOPDELAY_MS;//in milliseconds

	return poll(&pfd, 1, timeout);
}

int init_terminal(void)
{
	int ret = 0;
	struct termios term;
	ret = tcgetattr(STDERR_FILENO, &term);
	term.c_lflag = ~(ECHO);
	if (ret != 0)
		return FAILURE;
	ret = tcsetattr(STDIN_FILENO, TCSANOW, &term);
	if (ret != 0)
		return FAILURE;;
	printf("\e[?25l");
	return SUCCESS;
}

int reset_terminal(void)
{
	int ret = 0;
	struct termios term;
	ret = tcgetattr(STDERR_FILENO, &term);
	term.c_lflag = (ECHO);
	if (ret != 0)
		return FAILURE;
	ret = tcsetattr(STDIN_FILENO, TCSANOW, &term);
	if (ret != 0)
		return FAILURE;
	printf("\e[?25h");
	return SUCCESS;
}

int main(int argc, char *argv[])
{
	int width = 0;
	char * useless;
	if (argc > 1)
	{
		width = strtoul(argv[1], &useless, 10);
	}

	if (width == 0)
	{
		width = DEF_WIDTH;
	}

	init_terminal();
	struct game_table gtable;
	display_initGtable(width, &gtable);

	char c;
	int key_input = 0;
	int turn = O_CHANCE;
	int iter = 0;
	int cursor_state = 0;
	int win = FAILURE;
	int cur_update_flag = 0;
	//display_dispTable(&gtable, key_input, turn, cursor_state);
	while (1)
	{
		if (getinputEvent()) {
			c = getchar();
		} else {
			c = 0;
		}
		if (c == '\033')
		{ // Escape sequence starts with '\033'
			char seq[2];
			seq[0] = getchar();
			seq[1] = getchar();
			//show cursor while moving around:
			cursor_state = 1;

			if (seq[0] == '[')
			{
				switch (seq[1])
				{
					case 'A':
						key_input = UP_ARROW;
						break;
					case 'B':
						key_input = DOWN_ARROW;
						break;
					case 'C':
						key_input = RIGHT_ARROW;
						break;
					case 'D':
						key_input = LEFT_ARROW;
						break;
				}
			}
		}
		else if (c == 13)
		{
			key_input = ENTER_KEY;
		}
		else if (c == 'q')
		{
			printf("q pressed. Exiting.\n");
			break;
		} else if (c == 0) {
			//for blinking cursor.
			iter = iter + LOOPDELAY_MS;

			if (iter < BLINKDELAY_MS/2 && cursor_state != 0) {
				cursor_state = 0;
				cur_update_flag = 1;
			} else if (iter >= BLINKDELAY_MS/2 && cursor_state != 1) {
				cursor_state = 1;
				cur_update_flag = 1;
			}
			if (iter >= BLINKDELAY_MS) {
				iter = 0;
			}
			key_input = 0;
		}
		if (cur_update_flag || key_input != 0) {
			win = display_dispTable(&gtable, key_input, turn, cursor_state);
			cur_update_flag = 0;
		} else {
			win = FAILURE;
		}
		if (key_input == ENTER_KEY && win == TURN_DONE)
		{
			if (turn == O_CHANCE)
			{
				turn = X_CHANCE;
			}
			else
			{
				turn = O_CHANCE;
			}
		}
		if (win == SUCCESS)
		{
			cursor_state = 0;
			display_dispTable(&gtable, key_input, turn, cursor_state);
			printf("Game Over..\n");
			break;
		}
	}
	display_deinitGtable(&gtable);
	reset_terminal();
	return SUCCESS;
}
