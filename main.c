#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "displaygame.h"

#define DEF_WIDTH 3
#define BLINKDELAY_MS 1000
#define LOOPDELAY_MS 100

struct termios terminal_bkp;
int winch_event = 0;
struct termSize * t_sz_global;

void handle_winch(int sig)
{
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
	{
		t_sz_global->width = w.ws_col;
		t_sz_global->height = w.ws_row;
		winch_event = 1;
	}
}

int getinputEvent(void)
{
	struct pollfd pfd;

	pfd.fd = STDIN_FILENO;
	pfd.events = POLLIN;

	int timeout = LOOPDELAY_MS; // in milliseconds;
	return poll(&pfd, 1, timeout);
}

int init_terminal(void)
{
	int ret = 0;
	struct termios term;
	ret = tcgetattr(STDERR_FILENO, &term);
	terminal_bkp = term;
	term.c_lflag = ~(ECHO);
	if (ret != 0)
		return FAILURE;
	ret = tcsetattr(STDIN_FILENO, TCSANOW, &term);
	if (ret != 0)
		return FAILURE;
	;
	printf("\e[?25l");
	return SUCCESS;
}

int reset_terminal(void)
{
	int ret = 0;

	ret = tcsetattr(STDIN_FILENO, TCSANOW, &terminal_bkp);
	if (ret != 0)
		return FAILURE;
	printf("\e[?25h");
	return SUCCESS;
}

int main(int argc, char *argv[])
{
	// Set initial terminal size
	int width = 0;
	char *useless;
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
	t_sz_global = &(gtable.t_sz);
	// call signal handler only after gtable initialization
	handle_winch(0);
	signal(SIGWINCH, handle_winch);
	char c;
	int key_input = 0;
	int turn = O_CHANCE;
	int iter = 0;
	int cursor_state = 0;
	int win = FAILURE;
	int cur_update_flag = 0;
	int show_value_flag = 0;
	int blink_fast_flag = 0;

	while (1)
	{
		cursorXY(0, 0);
		// printf("iter: %d, key_input: %d, cursor_state: %d, show_value_flag: %d, cur_update_flag: %d\n", iter, key_input, cursor_state, show_value_flag, cur_update_flag);
		if (getinputEvent())
		{
			if (!winch_event)
			{
				c = getchar();
			} else
			{
				c = 0;
			}
		}
		else
		{
			c = 0;
		}

		if (c == 'w')
		{
			cursor_state = 1;
			key_input = UP_ARROW;
			show_value_flag = 0;
		}
		else if (c == 's')
		{
			cursor_state = 1;
			key_input = DOWN_ARROW;
			show_value_flag = 0;
		}
		else if (c == 'a')
		{
			cursor_state = 1;
			key_input = LEFT_ARROW;
			show_value_flag = 0;
		}
		else if (c == 'd')
		{
			cursor_state = 1;
			key_input = RIGHT_ARROW;
			show_value_flag = 0;
		}
		else if (c == '\033')
		{
			// Escape sequence starts with '\033'
			char seq[2];
			seq[0] = getchar();
			seq[1] = getchar();
			// show cursor while moving around:
			cursor_state = 1;

			if (seq[0] == '[')
			{

				switch (seq[1])
				{
				case 'A':
					key_input = UP_ARROW;
					show_value_flag = 0;
					break;
				case 'B':
					key_input = DOWN_ARROW;
					show_value_flag = 0;
					break;
				case 'C':
					key_input = RIGHT_ARROW;
					show_value_flag = 0;
					break;
				case 'D':
					key_input = LEFT_ARROW;
					show_value_flag = 0;
					break;
				default:
					continue;
					break;
				}
			}
		}
		else if (c == 13 || c == ' ')
		{
			key_input = ENTER_KEY;
			show_value_flag = 1;
			cursor_state = 0;
			cur_update_flag = 1;
			iter = 0;
		}
		else if (c == 'q')
		{
			struct termSize t_sz = get_terminal_width();
			int term_height = t_sz.height;
			int side_len = width;
			int end_y = (term_height - (6 * side_len)) / 2 + (6 * side_len) % 2 + (side_len * 6) + 1;
			cursorXY(0, end_y);
			printf("q pressed. Exiting.\n");
			break;
		}
		else if (c == 0)
		{
			// for blinking cursor.
			iter = iter + LOOPDELAY_MS;
			if (show_value_flag != 1)
			{
				if (iter < BLINKDELAY_MS / 2 && cursor_state != 0)
				{
					cursor_state = 0;
					cur_update_flag = 1;
				}
				else if (iter >= BLINKDELAY_MS / 2 && cursor_state != 1)
				{
					cursor_state = 1;
					cur_update_flag = 1;
				}
				if (iter >= BLINKDELAY_MS)
				{
					iter = 0;
				}
			}
			else
			{
				if (blink_fast_flag == 0)
				{
					cursor_state = 0;
					blink_fast_flag = 1;
				}
				else
				{
					cursor_state = 1;
					blink_fast_flag = 0;
				}
				cur_update_flag = 1;
				if (iter >= BLINKDELAY_MS)
				{
					iter = 0;
					show_value_flag = 0;
				}
			}
			key_input = 0;
		}

		if (winch_event || cur_update_flag || key_input != 0)
		{
			win = display_dispTable(&gtable, key_input, turn, cursor_state, &winch_event);
			cur_update_flag = 0;
		}
		else
		{
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
			winch_event  = 1;
			display_dispTable(&gtable, key_input, turn, cursor_state, &winch_event);
			printf("Game Over..\n");
			break;
		}
	}
	display_deinitGtable(&gtable);
	reset_terminal();
	return SUCCESS;
}
