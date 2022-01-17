#include <errno.h>
#include <assert.h>

/* generalized errors */

enum GEN_ERR {
    E_OK = 0,
    E_ARG = 1,
    E_IO = 2,
    E_INIT = 3,
    E_SAVE = 4,
    E_UKW = 255
};

/*
enum RND_ERR {
    RND_EOK = -0,
    RND_EARG = -1,
    RND_EIO = -2
};

enum GAME_ERR {
    GAME_EOK = 0,
    GAME_EINIT = -1,
    GAME_EIO = -2,
    GAME_EARG = -3,
    GAME_ERAND = -4,
    GAME_ESAV = -5,
    GAME_EUKW = -255
};
*/
