
// return codes
#define FAILURE		-1
#define SUCCESS		0

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

// a width * width table
struct game_table
{
	int **val_table;
	int width;
	int cur_x;
	int cur_y;
	int filled;
};

// public functions
int display_initGtable( int, struct game_table *);
void display_deinitGtable(struct game_table *);
int display_dispTable( struct game_table *, int, int, int);

