
// return codes
#define FAILURE		-1
#define SUCCESS		0
//using: https://stackoverflow.com/a/72073868 
#define cursorXY(x,y) printf("\033[%d;%dH",(y),(x))
enum gamecodes
{
	O_CHANCE = 0,
	X_CHANCE,
	TURN_DONE,
	NO_TURN,
	END_GCODES
};

enum wincodes
{
	DRAW = END_GCODES,
	WIN_O,
	WIN_X,
	END_WINCODES
};

// controls
enum keys
{
	UP_ARROW = 100,
	DOWN_ARROW,
	LEFT_ARROW,
	RIGHT_ARROW,
	ENTER_KEY,
};

struct termSize {
    int width;
    int height;
};

// a width * width table
struct game_table
{
	int **val_table;
	int width;
	int cur_x;
	int cur_y;
	int filled;
	int firstChar_xPos;
	int firstChar_yPos;
	struct termSize t_sz;
};

// public functions
int display_initGtable( int, struct game_table *);
void display_deinitGtable(struct game_table *);
int display_dispTable(struct game_table *g, int key_input, int turn, int cursor_state, int *redraw_table);
struct termSize get_terminal_width();