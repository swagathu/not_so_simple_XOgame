
// return codes
#define FAILURE -1
#define SUCCESS 0

// XO
#define X_CHANCE 1
#define O_CHANCE 0

// result
#define DRAW 2
#define WIN_O 3
#define WIN_X 4

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
int display_dispTable( struct game_table *, int, int, int);

