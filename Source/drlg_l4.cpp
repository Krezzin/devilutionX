/**
 * @file drlg_l4.cpp
 *
 * Implementation of the hell level generation algorithms.
 */
#include "drlg_l4.h"

#include "drlg_l1.h"
#include "engine/load_file.hpp"
#include "engine/random.hpp"
#include "monster.h"
#include "multi.h"
#include "objdat.h"

namespace devilution {

int diabquad1x;
int diabquad1y;
int diabquad2x;
int diabquad2y;
int diabquad3x;
int diabquad3y;
int diabquad4x;
int diabquad4y;

namespace {

bool hallok[20];
int l4holdx;
int l4holdy;
int SP4x1;
int SP4y1;
int SP4x2;
int SP4y2;
BYTE L4dungeon[80][80];
BYTE dung[20][20];
//int dword_52A4DC;

/**
 * A lookup table for the 16 possible patterns of a 2x2 area,
 * where each cell either contains a SW wall or it doesn't.
 */
const BYTE L4ConvTbl[16] = { 30, 6, 1, 6, 2, 6, 6, 6, 9, 6, 1, 6, 2, 6, 3, 6 };

/** Miniset: Stairs up. */
const BYTE L4USTAIRS[] = {
	// clang-format off
	4, 5, // width, height

	 6, 6, 6, 6, // search
	 6, 6, 6, 6,
	 6, 6, 6, 6,
	 6, 6, 6, 6,
	 6, 6, 6, 6,

	 0,  0,  0,  0, // replace
	36, 38, 35,  0,
	37, 34, 33, 32,
	 0,  0, 31,  0,
	 0,  0,  0,  0,
	// clang-format on
};
/** Miniset: Stairs up to town. */
const BYTE L4TWARP[] = {
	// clang-format off
	4, 5, // width, height

	6, 6, 6, 6, // search
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,
	6, 6, 6, 6,

	  0,   0,   0,   0, // replace
	134, 136, 133,   0,
	135, 132, 131, 130,
	  0,   0, 129,   0,
	  0,   0,   0,   0,
	// clang-format on
};
/** Miniset: Stairs down. */
const BYTE L4DSTAIRS[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,  0,  0,  0, 0, // replace
	0,  0, 45, 41, 0,
	0, 44, 43, 40, 0,
	0, 46, 42, 39, 0,
	0,  0,  0,  0, 0,
	// clang-format on
};
/** Miniset: Pentagram. */
const BYTE L4PENTA[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0,  98, 100, 103, 0,
	0,  99, 102, 105, 0,
	0, 101, 104, 106, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};
/** Miniset: Pentagram portal. */
const BYTE L4PENTA2[] = {
	// clang-format off
	5, 5, // width, height

	6, 6, 6, 6, 6, // search
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,
	6, 6, 6, 6, 6,

	0,   0,   0,   0, 0, // replace
	0, 107, 109, 112, 0,
	0, 108, 111, 114, 0,
	0, 110, 113, 115, 0,
	0,   0,   0,   0, 0,
	// clang-format on
};

/** Maps tile IDs to their corresponding undecorated tile ID. */
const BYTE L4BTYPES[140] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 6,
	6, 6, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 1, 2, 1, 2, 1, 1, 2,
	2, 0, 0, 0, 0, 0, 0, 15, 16, 9,
	12, 4, 5, 7, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

} // namespace

static void DrlgL4Shadows()
{
	for (int y = 1; y < DMAXY; y++) {
		for (int x = 1; x < DMAXY; x++) {
			if (IsNoneOf(dungeon[x][y], 3, 4, 8, 15)) {
				continue;
			}
			if (dungeon[x - 1][y] == 6) {
				dungeon[x - 1][y] = 47;
			}
			if (dungeon[x - 1][y - 1] == 6) {
				dungeon[x - 1][y - 1] = 48;
			}
		}
	}
}

static void InitL4Dungeon()
{
	memset(dung, 0, sizeof(dung));
	memset(L4dungeon, 0, sizeof(L4dungeon));

	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			dungeon[i][j] = 30;
			dflags[i][j] = 0;
		}
	}
}

void DRLG_LoadL4SP()
{
	setloadflag = false;
	if (QuestStatus(Q_WARLORD)) {
		pSetPiece = LoadFileInMem<uint16_t>("Levels\\L4Data\\Warlord.DUN");
		setloadflag = true;
	}
	if (currlevel == 15 && gbIsMultiplayer) {
		pSetPiece = LoadFileInMem<uint16_t>("Levels\\L4Data\\Vile1.DUN");
		setloadflag = true;
	}
}

void DRLG_FreeL4SP()
{
	pSetPiece = nullptr;
}

void DRLG_L4SetRoom(const uint16_t *dunData, int rx1, int ry1)
{
	int width = SDL_SwapLE16(dunData[0]);
	int height = SDL_SwapLE16(dunData[1]);

	const uint16_t *tileLayer = &dunData[2];

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			uint8_t tileId = SDL_SwapLE16(tileLayer[j * width + i]);
			if (tileId != 0) {
				dungeon[i + rx1][j + ry1] = tileId;
				dflags[i + rx1][j + ry1] |= DLRG_PROTECTED;
			} else {
				dungeon[i + rx1][j + ry1] = 6;
			}
		}
	}
}

void DRLG_L4SetSPRoom(int rx1, int ry1)
{
	setpc_x = rx1;
	setpc_y = ry1;
	setpc_w = SDL_SwapLE16(pSetPiece[0]);
	setpc_h = SDL_SwapLE16(pSetPiece[1]);

	DRLG_L4SetRoom(pSetPiece.get(), rx1, ry1);
}

static void L4makeDmt()
{
	int dmty = 1;
	for (int j = 0; dmty <= 77; j++, dmty += 2) {
		int dmtx = 1;
		for (int i = 0; dmtx <= 77; i++, dmtx += 2) {
			int val = 8 * L4dungeon[dmtx + 1][dmty + 1]
			    + 4 * L4dungeon[dmtx][dmty + 1]
			    + 2 * L4dungeon[dmtx + 1][dmty]
			    + L4dungeon[dmtx][dmty];
			dungeon[i][j] = L4ConvTbl[val];
		}
	}
}

static int L4HWallOk(int i, int j)
{
	int x;
	for (x = 1; dungeon[i + x][j] == 6; x++) {
		if (dflags[i + x][j] != 0) {
			break;
		}
		if (dungeon[i + x][j - 1] != 6) {
			break;
		}
		if (dungeon[i + x][j + 1] != 6) {
			break;
		}
	}

	if (IsAnyOf(dungeon[i + x][j], 10, 12, 13, 15, 16, 21, 22) && x > 3)
		return x;

	return -1;
}

static int L4VWallOk(int i, int j)
{
	int y;
	for (y = 1; dungeon[i][j + y] == 6; y++) {
		if (dflags[i][j + y] != 0) {
			break;
		}
		if (dungeon[i - 1][j + y] != 6) {
			break;
		}
		if (dungeon[i + 1][j + y] != 6) {
			break;
		}
	}

	if (IsAnyOf(dungeon[i][j + y], 9, 11, 14, 15, 16, 21, 23) && y > 3)
		return y;

	return -1;
}

static void L4HorizWall(int i, int j, int dx)
{
	if (dungeon[i][j] == 13) {
		dungeon[i][j] = 17;
	}
	if (dungeon[i][j] == 16) {
		dungeon[i][j] = 11;
	}
	if (dungeon[i][j] == 12) {
		dungeon[i][j] = 14;
	}

	for (int xx = 1; xx < dx; xx++) {
		dungeon[i + xx][j] = 2;
	}

	if (dungeon[i + dx][j] == 15) {
		dungeon[i + dx][j] = 14;
	}
	if (dungeon[i + dx][j] == 10) {
		dungeon[i + dx][j] = 17;
	}
	if (dungeon[i + dx][j] == 21) {
		dungeon[i + dx][j] = 23;
	}
	if (dungeon[i + dx][j] == 22) {
		dungeon[i + dx][j] = 29;
	}

	int xx = GenerateRnd(dx - 3) + 1;
	dungeon[i + xx][j] = 57;
	dungeon[i + xx + 2][j] = 56;
	dungeon[i + xx + 1][j] = 60;

	if (dungeon[i + xx][j - 1] == 6) {
		dungeon[i + xx][j - 1] = 58;
	}
	if (dungeon[i + xx + 1][j - 1] == 6) {
		dungeon[i + xx + 1][j - 1] = 59;
	}
}

static void L4VertWall(int i, int j, int dy)
{
	if (dungeon[i][j] == 14) {
		dungeon[i][j] = 17;
	}
	if (dungeon[i][j] == 8) {
		dungeon[i][j] = 9;
	}
	if (dungeon[i][j] == 15) {
		dungeon[i][j] = 10;
	}

	for (int yy = 1; yy < dy; yy++) {
		dungeon[i][j + yy] = 1;
	}

	if (dungeon[i][j + dy] == 11) {
		dungeon[i][j + dy] = 17;
	}
	if (dungeon[i][j + dy] == 9) {
		dungeon[i][j + dy] = 10;
	}
	if (dungeon[i][j + dy] == 16) {
		dungeon[i][j + dy] = 13;
	}
	if (dungeon[i][j + dy] == 21) {
		dungeon[i][j + dy] = 22;
	}
	if (dungeon[i][j + dy] == 23) {
		dungeon[i][j + dy] = 29;
	}

	int yy = GenerateRnd(dy - 3) + 1;
	dungeon[i][j + yy] = 53;
	dungeon[i][j + yy + 2] = 52;
	dungeon[i][j + yy + 1] = 6;

	if (dungeon[i - 1][j + yy] == 6) {
		dungeon[i - 1][j + yy] = 54;
	}
	if (dungeon[i - 1][j + yy - 1] == 6) {
		dungeon[i - 1][j + yy - 1] = 55;
	}
}

static void L4AddWall()
{
	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			if (dflags[i][j] != 0) {
				continue;
			}
			if (IsAnyOf(dungeon[i][j], 10, 12, 13, 15, 16, 21, 22) && GenerateRnd(100) < WALL_CHANCE) {
				int x = L4HWallOk(i, j);
				if (x != -1) {
					L4HorizWall(i, j, x);
				}
			}
			if (IsAnyOf(dungeon[i][j], 8, 9, 11, 14, 15, 16, 21, 23) && GenerateRnd(100) < WALL_CHANCE) {
				int y = L4VWallOk(i, j);
				if (y != -1) {
					L4VertWall(i, j, y);
				}
			}
		}
	}
}

static void L4tileFix()
{
	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 6)
				dungeon[i + 1][j] = 5;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 13;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 14;
		}
	}
	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 6)
				dungeon[i + 1][j] = 2;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 9)
				dungeon[i + 1][j] = 11;
			if (dungeon[i][j] == 9 && dungeon[i + 1][j] == 6)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 13;
			if (dungeon[i][j] == 6 && dungeon[i + 1][j] == 14)
				dungeon[i + 1][j] = 15;
			if (dungeon[i][j] == 6 && dungeon[i][j + 1] == 13)
				dungeon[i][j + 1] = 16;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 10;
			if (dungeon[i][j] == 6 && dungeon[i][j - 1] == 1)
				dungeon[i][j - 1] = 1;
		}
	}
	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 13 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 27;
			if (dungeon[i][j] == 27 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 27;
			if (dungeon[i][j] == 27 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 27)
				dungeon[i + 1][j] = 26;
			if (dungeon[i][j] == 27 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 15)
				dungeon[i + 1][j] = 14;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 15)
				dungeon[i + 1][j] = 14;
			if (dungeon[i][j] == 22 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 27 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 6 && dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] != 0) /* check */
				dungeon[i + 1][j] = 22;
			if (dungeon[i][j] == 22 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 1 && dungeon[i + 1][j - 1] == 1)
				dungeon[i + 1][j] = 13;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 30 && dungeon[i][j + 1] == 6)
				dungeon[i + 1][j] = 28;
			if (dungeon[i][j] == 16 && dungeon[i + 1][j] == 6 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 27;
			if (dungeon[i][j] == 16 && dungeon[i][j + 1] == 30 && dungeon[i + 1][j + 1] == 30)
				dungeon[i][j + 1] = 27;
			if (dungeon[i][j] == 6 && dungeon[i + 1][j] == 30 && dungeon[i + 1][j - 1] == 6)
				dungeon[i + 1][j] = 21;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 9)
				dungeon[i + 1][j] = 29;
			if (dungeon[i][j] == 9 && dungeon[i + 1][j] == 15)
				dungeon[i + 1][j] = 14;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 2)
				dungeon[i + 1][j] = 29;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 18)
				dungeon[i + 1][j] = 24;
			if (dungeon[i][j] == 9 && dungeon[i + 1][j] == 15)
				dungeon[i + 1][j] = 14;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 19 && dungeon[i + 1][j - 1] == 30)
				dungeon[i + 1][j] = 24;
			if (dungeon[i][j] == 24 && dungeon[i][j - 1] == 30 && dungeon[i][j - 2] == 6)
				dungeon[i][j - 1] = 21;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 28;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 28;
			if (dungeon[i][j] == 28 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 28 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 19 && dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 18 && dungeon[i + 1][j + 1] == 1)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 19 && dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 22 && dungeon[i + 1][j + 1] == 1)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 19 && dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 18 && dungeon[i + 1][j + 1] == 13)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 21 && dungeon[i + 2][j] == 2 && dungeon[i + 1][j - 1] == 18 && dungeon[i + 1][j + 1] == 1)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j + 1] == 1 && dungeon[i + 1][j - 1] == 22 && dungeon[i + 2][j] == 3)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 30 && dungeon[i + 1][j - 1] == 6)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 1)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 30)
				dungeon[i + 1][j] = 29;
			if (dungeon[i][j] == 28 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j - 1] == 21)
				dungeon[i + 1][j] = 24;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 30)
				dungeon[i + 1][j] = 29;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 18)
				dungeon[i + 1][j] = 25;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 9 && dungeon[i + 2][j] == 2)
				dungeon[i + 1][j] = 11;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 10)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 15 && dungeon[i][j + 1] == 3)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 22 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 18 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 24 && dungeon[i - 1][j] == 30)
				dungeon[i - 1][j] = 19;
			if (dungeon[i][j] == 21 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 21 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 10;
			if (dungeon[i][j] == 22 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 21 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 16 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 13 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 22 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 18 && dungeon[i + 2][j] == 30)
				dungeon[i + 1][j] = 24;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 9 && dungeon[i + 1][j + 1] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 27 && dungeon[i + 1][j + 1] == 2)
				dungeon[i + 1][j] = 29;
			if (dungeon[i][j] == 23 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 23 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 25 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 22 && dungeon[i + 1][j] == 9)
				dungeon[i + 1][j] = 11;
			if (dungeon[i][j] == 23 && dungeon[i + 1][j] == 9)
				dungeon[i + 1][j] = 11;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 11 && dungeon[i + 1][j] == 15)
				dungeon[i + 1][j] = 14;
			if (dungeon[i][j] == 23 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 27)
				dungeon[i + 1][j] = 26;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 18)
				dungeon[i + 1][j] = 24;
			if (dungeon[i][j] == 26 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 29 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 29 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 1 && dungeon[i][j - 1] == 15)
				dungeon[i][j - 1] = 10;
			if (dungeon[i][j] == 18 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 23 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 18 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 10;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 30 && dungeon[i + 1][j + 1] == 30)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 28 && dungeon[i + 1][j - 1] == 6)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 23 && dungeon[i + 1][j] == 18 && dungeon[i][j - 1] == 6)
				dungeon[i + 1][j] = 24;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 23 && dungeon[i + 2][j] == 30)
				dungeon[i + 1][j] = 28;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 30 && dungeon[i + 1][j - 1] == 6)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 23 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 29 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 29 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 26 && dungeon[i + 1][j] == 30)
				dungeon[i + 1][j] = 19;
			if (dungeon[i][j] == 16 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 13 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 10;
			if (dungeon[i][j] == 25 && dungeon[i][j + 1] == 30)
				dungeon[i][j + 1] = 18;
			if (dungeon[i][j] == 18 && dungeon[i][j + 1] == 2)
				dungeon[i][j + 1] = 15;
			if (dungeon[i][j] == 11 && dungeon[i + 1][j] == 3)
				dungeon[i + 1][j] = 5;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 9)
				dungeon[i + 1][j] = 11;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 13;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 13 && dungeon[i + 1][j - 1] == 6)
				dungeon[i + 1][j] = 16;
		}
	}
	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 21 && dungeon[i][j + 1] == 24 && dungeon[i][j + 2] == 1)
				dungeon[i][j + 1] = 17;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j + 1] == 9 && dungeon[i + 1][j - 1] == 1 && dungeon[i + 2][j] == 16)
				dungeon[i + 1][j] = 29;
			if (dungeon[i][j] == 2 && dungeon[i - 1][j] == 6)
				dungeon[i - 1][j] = 8;
			if (dungeon[i][j] == 1 && dungeon[i][j - 1] == 6)
				dungeon[i][j - 1] = 7;
			if (dungeon[i][j] == 6 && dungeon[i + 1][j] == 15 && dungeon[i + 1][j + 1] == 4)
				dungeon[i + 1][j] = 10;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 3)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 6)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 9 && dungeon[i][j + 1] == 3)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 10 && dungeon[i][j + 1] == 3)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 13 && dungeon[i][j + 1] == 3)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 5)
				dungeon[i][j + 1] = 12;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 16)
				dungeon[i][j + 1] = 13;
			if (dungeon[i][j] == 6 && dungeon[i][j + 1] == 13)
				dungeon[i][j + 1] = 16;
			if (dungeon[i][j] == 25 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 10;
			if (dungeon[i][j] == 13 && dungeon[i][j + 1] == 5)
				dungeon[i][j + 1] = 12;
			if (dungeon[i][j] == 28 && dungeon[i][j - 1] == 6 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 10)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 9)
				dungeon[i + 1][j] = 11;
			if (dungeon[i][j] == 11 && dungeon[i + 1][j] == 3)
				dungeon[i + 1][j] = 5;
			if (dungeon[i][j] == 10 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 27 && dungeon[i + 1][j] == 9)
				dungeon[i + 1][j] = 11;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 1)
				dungeon[i + 1][j] = 16;
			if (dungeon[i][j] == 11 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 3)
				dungeon[i + 1][j] = 5;
			if (dungeon[i][j] == 9 && dungeon[i + 1][j] == 3)
				dungeon[i + 1][j] = 5;
			if (dungeon[i][j] == 14 && dungeon[i + 1][j] == 3)
				dungeon[i + 1][j] = 5;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 3)
				dungeon[i + 1][j] = 5;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 5 && dungeon[i + 1][j - 1] == 16)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 2 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 9 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 1 && dungeon[i][j - 1] == 8)
				dungeon[i][j - 1] = 9;
			if (dungeon[i][j] == 28 && dungeon[i + 1][j] == 23 && dungeon[i + 1][j + 1] == 3)
				dungeon[i + 1][j] = 16;
		}
	}
	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 10)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 17 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 10 && dungeon[i + 1][j] == 4)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 17 && dungeon[i][j + 1] == 5)
				dungeon[i][j + 1] = 12;
			if (dungeon[i][j] == 29 && dungeon[i][j + 1] == 9)
				dungeon[i][j + 1] = 10;
			if (dungeon[i][j] == 13 && dungeon[i][j + 1] == 5)
				dungeon[i][j + 1] = 12;
			if (dungeon[i][j] == 9 && dungeon[i][j + 1] == 16)
				dungeon[i][j + 1] = 13;
			if (dungeon[i][j] == 10 && dungeon[i][j + 1] == 16)
				dungeon[i][j + 1] = 13;
			if (dungeon[i][j] == 16 && dungeon[i][j + 1] == 3)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 11 && dungeon[i][j + 1] == 5)
				dungeon[i][j + 1] = 12;
			if (dungeon[i][j] == 10 && dungeon[i + 1][j] == 3 && dungeon[i + 1][j - 1] == 16)
				dungeon[i + 1][j] = 12;
			if (dungeon[i][j] == 16 && dungeon[i][j + 1] == 5)
				dungeon[i][j + 1] = 12;
			if (dungeon[i][j] == 1 && dungeon[i][j + 1] == 6)
				dungeon[i][j + 1] = 4;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j] == 13 && dungeon[i][j + 1] == 10)
				dungeon[i + 1][j + 1] = 12;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 10)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 22 && dungeon[i][j + 1] == 11)
				dungeon[i][j + 1] = 17;
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 16)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 28 && dungeon[i + 1][j] == 23 && dungeon[i + 1][j + 1] == 1 && dungeon[i + 2][j] == 6)
				dungeon[i + 1][j] = 16;
		}
	}
	for (int j = 0; j < DMAXY; j++) {
		for (int i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 15 && dungeon[i + 1][j] == 28 && dungeon[i + 2][j] == 16)
				dungeon[i + 1][j] = 23;
			if (dungeon[i][j] == 21 && dungeon[i + 1][j - 1] == 21 && dungeon[i + 1][j + 1] == 13 && dungeon[i + 2][j] == 2)
				dungeon[i + 1][j] = 17;
			if (dungeon[i][j] == 19 && dungeon[i + 1][j] == 15 && dungeon[i + 1][j + 1] == 12)
				dungeon[i + 1][j] = 17;
		}
	}
}

static void DrlgL4Subs()
{
	for (int y = 0; y < DMAXY; y++) {
		for (int x = 0; x < DMAXX; x++) {
			int rv = GenerateRnd(3);
			if (rv == 0) {
				uint8_t c = dungeon[x][y];
				c = L4BTYPES[c];
				if (c != 0 && dflags[x][y] == 0) {
					rv = GenerateRnd(16);
					int i = -1;
					while (rv >= 0) {
						i++;
						if (i == sizeof(L4BTYPES)) {
							i = 0;
						}
						if (c == L4BTYPES[i]) {
							rv--;
						}
					}
					dungeon[x][y] = i;
				}
			}
		}
	}
	for (int y = 0; y < DMAXY; y++) {
		for (int x = 0; x < DMAXX; x++) {
			int rv = GenerateRnd(10);
			if (rv == 0) {
				uint8_t c = dungeon[x][y];
				if (L4BTYPES[c] == 6 && dflags[x][y] == 0) {
					dungeon[x][y] = GenerateRnd(3) + 95;
				}
			}
		}
	}
}

static void L4makeDungeon()
{
	for (int j = 0; j < 20; j++) {
		for (int i = 0; i < 20; i++) {
			int k = i * 2;
			int l = j * 2;
			L4dungeon[k][l] = dung[i][j];
			L4dungeon[k][l + 1] = dung[i][j];
			L4dungeon[k + 1][l] = dung[i][j];
			L4dungeon[k + 1][l + 1] = dung[i][j];
		}
	}
	for (int j = 0; j < 20; j++) {
		for (int i = 0; i < 20; i++) {
			int k = i * 2;
			int l = j * 2;
			L4dungeon[k][l + 40] = dung[i][19 - j];
			L4dungeon[k][l + 41] = dung[i][19 - j];
			L4dungeon[k + 1][l + 40] = dung[i][19 - j];
			L4dungeon[k + 1][l + 41] = dung[i][19 - j];
		}
	}
	for (int j = 0; j < 20; j++) {
		for (int i = 0; i < 20; i++) {
			int k = i * 2;
			int l = j * 2;
			L4dungeon[k + 40][l] = dung[19 - i][j];
			L4dungeon[k + 40][l + 1] = dung[19 - i][j];
			L4dungeon[k + 41][l] = dung[19 - i][j];
			L4dungeon[k + 41][l + 1] = dung[19 - i][j];
		}
	}
	for (int j = 0; j < 20; j++) {
		for (int i = 0; i < 20; i++) {
			int k = i * 2;
			int l = j * 2;
			L4dungeon[k + 40][l + 40] = dung[19 - i][19 - j];
			L4dungeon[k + 40][l + 41] = dung[19 - i][19 - j];
			L4dungeon[k + 41][l + 40] = dung[19 - i][19 - j];
			L4dungeon[k + 41][l + 41] = dung[19 - i][19 - j];
		}
	}
}

static void UShape()
{
	for (int j = 19; j >= 0; j--) {
		for (int i = 19; i >= 0; i--) {
			if (dung[i][j] != 1) {
				hallok[j] = false;
			}
			if (dung[i][j] == 1) {
				// BUGFIX: check that i + 1 < 20 and j + 1 < 20 (fixed)
				if (i + 1 < 20 && j + 1 < 20
				    && dung[i][j + 1] == 1 && dung[i + 1][j + 1] == 0) {
					hallok[j] = true;
				} else {
					hallok[j] = false;
				}
				i = 0;
			}
		}
	}

	int rv = GenerateRnd(19) + 1;
	do {
		if (hallok[rv]) {
			for (int i = 19; i >= 0; i--) {
				if (dung[i][rv] == 1) {
					i = -1;
					rv = 0;
				} else {
					dung[i][rv] = 1;
					dung[i][rv + 1] = 1;
				}
			}
		} else {
			rv++;
			if (rv == 20) {
				rv = 1;
			}
		}
	} while (rv != 0);

	for (int i = 19; i >= 0; i--) {
		for (int j = 19; j >= 0; j--) {
			if (dung[i][j] != 1) {
				hallok[i] = false;
			}
			if (dung[i][j] == 1) {
				// BUGFIX: check that i + 1 < 20 and j + 1 < 20 (fixed)
				if (i + 1 < 20 && j + 1 < 20
				    && dung[i + 1][j] == 1 && dung[i + 1][j + 1] == 0) {
					hallok[i] = true;
				} else {
					hallok[i] = false;
				}
				j = 0;
			}
		}
	}

	rv = GenerateRnd(19) + 1;
	do {
		if (hallok[rv]) {
			for (int j = 19; j >= 0; j--) {
				if (dung[rv][j] == 1) {
					j = -1;
					rv = 0;
				} else {
					dung[rv][j] = 1;
					dung[rv + 1][j] = 1;
				}
			}
		} else {
			rv++;
			if (rv == 20) {
				rv = 1;
			}
		}
	} while (rv != 0);
}

static int GetArea()
{
	int rv = 0;

	for (int j = 0; j < 20; j++) {
		for (int i = 0; i < 20; i++) { // NOLINT(modernize-loop-convert)
			if (dung[i][j] == 1) {
				rv++;
			}
		}
	}

	return rv;
}

static void L4drawRoom(int x, int y, int width, int height)
{
	for (int j = 0; j < height && j + y < 20; j++) {
		for (int i = 0; i < width && i + x < 20; i++) {
			dung[i + x][j + y] = 1;
		}
	}
}

static bool L4checkRoom(int x, int y, int width, int height)
{
	if (x <= 0 || y <= 0) {
		return false;
	}

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			if (i + x < 0 || i + x >= 20 || j + y < 0 || j + y >= 20) {
				return false;
			}
			if (dung[i + x][j + y] != 0) {
				return false;
			}
		}
	}

	return true;
}

static void L4roomGen(int x, int y, int w, int h, int dir)
{
	bool ran;
	int dirProb = GenerateRnd(4);
	int num = 0;

	if ((dir == 1 && dirProb == 0) || (dir != 1 && dirProb != 0)) {
		int cw;
		int ch;
		int cx1;
		int cy1;
		do {
			cw = (GenerateRnd(5) + 2) & ~1;
			ch = (GenerateRnd(5) + 2) & ~1;
			cy1 = h / 2 + y - ch / 2;
			cx1 = x - cw;
			ran = L4checkRoom(cx1 - 1, cy1 - 1, ch + 2, cw + 1); /// BUGFIX: swap args 3 and 4 ("ch+2" and "cw+1")
			num++;
		} while (!ran && num < 20);

		if (ran)
			L4drawRoom(cx1, cy1, cw, ch);
		int cx2 = x + w;
		bool ran2 = L4checkRoom(cx2, cy1 - 1, cw + 1, ch + 2);
		if (ran2)
			L4drawRoom(cx2, cy1, cw, ch);
		if (ran)
			L4roomGen(cx1, cy1, cw, ch, 1);
		if (ran2)
			L4roomGen(cx2, cy1, cw, ch, 1);
		return;
	}

	int width;
	int height;
	int rx;
	int ry;
	do {
		width = (GenerateRnd(5) + 2) & ~1;
		height = (GenerateRnd(5) + 2) & ~1;
		rx = w / 2 + x - width / 2;
		ry = y - height;
		ran = L4checkRoom(rx - 1, ry - 1, width + 2, height + 1);
		num++;
	} while (!ran && num < 20);

	if (ran)
		L4drawRoom(rx, ry, width, height);
	int ry2 = y + h;
	bool ran2 = L4checkRoom(rx - 1, ry2, width + 2, height + 1);
	if (ran2)
		L4drawRoom(rx, ry2, width, height);
	if (ran)
		L4roomGen(rx, ry, width, height, 0);
	if (ran2)
		L4roomGen(rx, ry2, width, height, 0);
}

static void L4firstRoom()
{
	int w = 14;
	int h = 14;
	if (currlevel != 16) {
		if (currlevel == quests[Q_WARLORD]._qlevel && quests[Q_WARLORD]._qactive != QUEST_NOTAVAIL) {
			assert(!gbIsMultiplayer);
			w = 11;
			h = 11;
		} else if (currlevel == quests[Q_BETRAYER]._qlevel && gbIsMultiplayer) {
			w = 11;
			h = 11;
		} else {
			w = GenerateRnd(5) + 2;
			h = GenerateRnd(5) + 2;
		}
	}

	int xmin = (20 - w) / 2;
	int xmax = 19 - w;
	int x = GenerateRnd(xmax - xmin + 1) + xmin;

	int ymin = (20 - h) / 2;
	int ymax = 19 - h;
	int y = GenerateRnd(ymax - ymin + 1) + ymin;

	if (currlevel == 16) {
		l4holdx = x;
		l4holdy = y;
	}
	if (QuestStatus(Q_WARLORD) || (currlevel == quests[Q_BETRAYER]._qlevel && gbIsMultiplayer)) {
		SP4x1 = x + 1;
		SP4y1 = y + 1;
		SP4x2 = SP4x1 + w;
		SP4y2 = SP4y1 + h;
	} else {
		SP4x1 = 0;
		SP4y1 = 0;
		SP4x2 = 0;
		SP4y2 = 0;
	}

	L4drawRoom(x, y, w, h);
	L4roomGen(x, y, w, h, GenerateRnd(2));
}

void L4SaveQuads()
{
	int x = l4holdx;
	int y = l4holdy;

	for (int j = 0; j < 14; j++) {
		for (int i = 0; i < 14; i++) {
			dflags[i + x][j + y] = 1;
			dflags[DMAXX - 1 - i - x][j + y] = 1;
			dflags[i + x][DMAXY - 1 - j - y] = 1;
			dflags[DMAXX - 1 - i - x][DMAXY - 1 - j - y] = 1;
		}
	}
}

void DRLG_LoadDiabQuads(bool preflag)
{
	{
		auto dunData = LoadFileInMem<uint16_t>("Levels\\L4Data\\diab1.DUN");
		diabquad1x = 4 + l4holdx;
		diabquad1y = 4 + l4holdy;
		DRLG_L4SetRoom(dunData.get(), diabquad1x, diabquad1y);
	}
	{
		auto dunData = LoadFileInMem<uint16_t>(preflag ? "Levels\\L4Data\\diab2b.DUN" : "Levels\\L4Data\\diab2a.DUN");
		diabquad2x = 27 - l4holdx;
		diabquad2y = 1 + l4holdy;
		DRLG_L4SetRoom(dunData.get(), diabquad2x, diabquad2y);
	}
	{
		auto dunData = LoadFileInMem<uint16_t>(preflag ? "Levels\\L4Data\\diab3b.DUN" : "Levels\\L4Data\\diab3a.DUN");
		diabquad3x = 1 + l4holdx;
		diabquad3y = 27 - l4holdy;
		DRLG_L4SetRoom(dunData.get(), diabquad3x, diabquad3y);
	}
	{
		auto dunData = LoadFileInMem<uint16_t>(preflag ? "Levels\\L4Data\\diab4b.DUN" : "Levels\\L4Data\\diab4a.DUN");
		diabquad4x = 28 - l4holdx;
		diabquad4y = 28 - l4holdy;
		DRLG_L4SetRoom(dunData.get(), diabquad4x, diabquad4y);
	}
}

static bool DrlgL4PlaceMiniSet(const BYTE *miniset, int tmin, int tmax, int cx, int cy, bool setview, int ldir)
{
	int sx;
	int sy;

	int sw = miniset[0];
	int sh = miniset[1];

	int numt = 1;
	if (tmax - tmin != 0) {
		numt = GenerateRnd(tmax - tmin) + tmin;
	}

	for (int i = 0; i < numt; i++) {
		sx = GenerateRnd(DMAXX - sw);
		sy = GenerateRnd(DMAXY - sh);
		bool found = false;
		int bailcnt;
		for (bailcnt = 0; !found && bailcnt < 200; bailcnt++) {
			found = true;
			if (sx >= SP4x1 && sx <= SP4x2 && sy >= SP4y1 && sy <= SP4y2) {
				found = false;
			}
			if (cx != -1 && sx >= cx - sw && sx <= cx + 12) {
				sx = GenerateRnd(DMAXX - sw);
				sy = GenerateRnd(DMAXY - sh);
				found = false;
			}
			if (cy != -1 && sy >= cy - sh && sy <= cy + 12) {
				sx = GenerateRnd(DMAXX - sw);
				sy = GenerateRnd(DMAXY - sh);
				found = false;
			}
			int ii = 2;
			for (int yy = 0; yy < sh && found; yy++) {
				for (int xx = 0; xx < sw && found; xx++) {
					if (miniset[ii] != 0 && dungeon[xx + sx][yy + sy] != miniset[ii]) {
						found = false;
					}
					if (dflags[xx + sx][yy + sy] != 0) {
						found = false;
					}
					ii++;
				}
			}
			if (!found) {
				sx++;
				if (sx == DMAXX - sw) {
					sx = 0;
					sy++;
					if (sy == DMAXY - sh) {
						sy = 0;
					}
				}
			}
		}
		if (bailcnt >= 200) {
			return false;
		}
		int ii = sw * sh + 2;
		for (int yy = 0; yy < sh; yy++) {
			for (int xx = 0; xx < sw; xx++) {
				if (miniset[ii] != 0) {
					dungeon[xx + sx][yy + sy] = miniset[ii];
					dflags[xx + sx][yy + sy] |= 8;
				}
				ii++;
			}
		}
	}

	if (currlevel == 15 && quests[Q_BETRAYER]._qactive >= QUEST_ACTIVE) { /// Lazarus staff skip bug fixed
		quests[Q_BETRAYER].position = { sx + 1, sy + 1 };
	}
	if (setview) {
		ViewX = 2 * sx + 21;
		ViewY = 2 * sy + 22;
	}

	return true;
}

#if defined(__3DS__)
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif

static void DrlgL4FTransparencyValueR(int i, int j, int x, int y, int d)
{
	if (dTransVal[x][y] != 0 || dungeon[i][j] != 6) {
		if (d == 1) {
			dTransVal[x][y] = TransVal;
			dTransVal[x][y + 1] = TransVal;
		}
		if (d == 2) {
			dTransVal[x + 1][y] = TransVal;
			dTransVal[x + 1][y + 1] = TransVal;
		}
		if (d == 3) {
			dTransVal[x][y] = TransVal;
			dTransVal[x + 1][y] = TransVal;
		}
		if (d == 4) {
			dTransVal[x][y + 1] = TransVal;
			dTransVal[x + 1][y + 1] = TransVal;
		}
		if (d == 5) {
			dTransVal[x + 1][y + 1] = TransVal;
		}
		if (d == 6) {
			dTransVal[x][y + 1] = TransVal;
		}
		if (d == 7) {
			dTransVal[x + 1][y] = TransVal;
		}
		if (d == 8) {
			dTransVal[x][y] = TransVal;
		}
	} else {
		dTransVal[x][y] = TransVal;
		dTransVal[x + 1][y] = TransVal;
		dTransVal[x][y + 1] = TransVal;
		dTransVal[x + 1][y + 1] = TransVal;
		DrlgL4FTransparencyValueR(i + 1, j, x + 2, y, 1);
		DrlgL4FTransparencyValueR(i - 1, j, x - 2, y, 2);
		DrlgL4FTransparencyValueR(i, j + 1, x, y + 2, 3);
		DrlgL4FTransparencyValueR(i, j - 1, x, y - 2, 4);
		DrlgL4FTransparencyValueR(i - 1, j - 1, x - 2, y - 2, 5);
		DrlgL4FTransparencyValueR(i + 1, j - 1, x + 2, y - 2, 6);
		DrlgL4FTransparencyValueR(i - 1, j + 1, x - 2, y + 2, 7);
		DrlgL4FTransparencyValueR(i + 1, j + 1, x + 2, y + 2, 8);
	}
}

static void DrlgL4FloodTVal()
{
	int yy = 16;
	for (int j = 0; j < DMAXY; j++) {
		int xx = 16;
		for (int i = 0; i < DMAXX; i++) {
			if (dungeon[i][j] == 6 && dTransVal[xx][yy] == 0) {
				DrlgL4FTransparencyValueR(i, j, xx, yy, 0);
				TransVal++;
			}
			xx += 2;
		}
		yy += 2;
	}
}

#if defined(__3DS__)
#pragma GCC pop_options
#endif

bool IsDURWall(char d)
{
	if (d == 25) {
		return true;
	}
	if (d == 28) {
		return true;
	}
	if (d == 23) {
		return true;
	}

	return false;
}

bool IsDLLWall(char dd)
{
	if (dd == 27) {
		return true;
	}
	if (dd == 26) {
		return true;
	}
	if (dd == 22) {
		return true;
	}

	return false;
}

static void DrlgL4TransFix()
{
	int yy = 16;
	for (int j = 0; j < DMAXY; j++) {
		int xx = 16;
		for (int i = 0; i < DMAXX; i++) {
			if (IsDURWall(dungeon[i][j]) && dungeon[i][j - 1] == 18) { // BUGFIX: check if j >= 1
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (IsDLLWall(dungeon[i][j]) && dungeon[i + 1][j] == 19) {
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (dungeon[i][j] == 18) {
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (dungeon[i][j] == 19) {
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (dungeon[i][j] == 24) {
				dTransVal[xx + 1][yy] = dTransVal[xx][yy];
				dTransVal[xx][yy + 1] = dTransVal[xx][yy];
				dTransVal[xx + 1][yy + 1] = dTransVal[xx][yy];
			}
			if (dungeon[i][j] == 57) {
				dTransVal[xx - 1][yy] = dTransVal[xx][yy + 1];
				dTransVal[xx][yy] = dTransVal[xx][yy + 1];
			}
			if (dungeon[i][j] == 53) {
				dTransVal[xx][yy - 1] = dTransVal[xx + 1][yy];
				dTransVal[xx][yy] = dTransVal[xx + 1][yy];
			}
			xx += 2;
		}
		yy += 2;
	}
}

static void DrlgL4Corners()
{
	for (int j = 1; j < DMAXY - 1; j++) {
		for (int i = 1; i < DMAXX - 1; i++) {
			if (dungeon[i][j] >= 18 && dungeon[i][j] <= 30) {
				if (dungeon[i + 1][j] < 18 || dungeon[i][j + 1] < 18) {
					dungeon[i][j] += 98;
				}
			}
		}
	}
}

void L4FixRim()
{
	for (int i = 0; i < 20; i++) { // NOLINT(modernize-loop-convert)
		dung[i][0] = 0;
	}
	for (int j = 0; j < 20; j++) {
		dung[0][j] = 0;
	}
}

void DRLG_L4GeneralFix()
{
	for (int j = 0; j < DMAXY - 1; j++) {
		for (int i = 0; i < DMAXX - 1; i++) {
			if ((dungeon[i][j] == 24 || dungeon[i][j] == 122) && dungeon[i + 1][j] == 2 && dungeon[i][j + 1] == 5) {
				dungeon[i][j] = 17;
			}
		}
	}
}

static void DrlgL4(lvl_entry entry)
{
	int ar;
	bool doneflag;

	do {
		DRLG_InitTrans();
		do {
			InitL4Dungeon();
			L4firstRoom();
			L4FixRim();
			ar = GetArea();
			if (ar >= 173) {
				UShape();
			}
		} while (ar < 173);
		L4makeDungeon();
		L4makeDmt();
		L4tileFix();
		if (currlevel == 16) {
			L4SaveQuads();
		}
		if (QuestStatus(Q_WARLORD) || (currlevel == quests[Q_BETRAYER]._qlevel && gbIsMultiplayer)) {
			for (int spi = SP4x1; spi < SP4x2; spi++) {
				for (int spj = SP4y1; spj < SP4y2; spj++) {
					dflags[spi][spj] = 1;
				}
			}
		}
		L4AddWall();
		DrlgL4FloodTVal();
		DrlgL4TransFix();
		if (setloadflag) {
			DRLG_L4SetSPRoom(SP4x1, SP4y1);
		}
		if (currlevel == 16) {
			DRLG_LoadDiabQuads(true);
		}
		if (QuestStatus(Q_WARLORD)) {
			if (entry == ENTRY_MAIN) {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, true, 0);
				if (doneflag && currlevel == 13) {
					doneflag = DrlgL4PlaceMiniSet(L4TWARP, 1, 1, -1, -1, false, 6);
				}
				ViewX++;
			} else if (entry == ENTRY_PREV) {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, false, 0);
				if (doneflag && currlevel == 13) {
					doneflag = DrlgL4PlaceMiniSet(L4TWARP, 1, 1, -1, -1, false, 6);
				}
				ViewX = 2 * setpc_x + 22;
				ViewY = 2 * setpc_y + 22;
			} else {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, false, 0);
				if (doneflag && currlevel == 13) {
					doneflag = DrlgL4PlaceMiniSet(L4TWARP, 1, 1, -1, -1, true, 6);
				}
				ViewX++;
			}
		} else if (currlevel != 15) {
			if (entry == ENTRY_MAIN) {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, true, 0);
				if (doneflag && currlevel != 16) {
					doneflag = DrlgL4PlaceMiniSet(L4DSTAIRS, 1, 1, -1, -1, false, 1);
				}
				if (doneflag && currlevel == 13) {
					doneflag = DrlgL4PlaceMiniSet(L4TWARP, 1, 1, -1, -1, false, 6);
				}
				ViewX++;
			} else if (entry == ENTRY_PREV) {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, false, 0);
				if (doneflag && currlevel != 16) {
					doneflag = DrlgL4PlaceMiniSet(L4DSTAIRS, 1, 1, -1, -1, true, 1);
				}
				if (doneflag && currlevel == 13) {
					doneflag = DrlgL4PlaceMiniSet(L4TWARP, 1, 1, -1, -1, false, 6);
				}
				ViewY++;
			} else {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, false, 0);
				if (doneflag && currlevel != 16) {
					doneflag = DrlgL4PlaceMiniSet(L4DSTAIRS, 1, 1, -1, -1, false, 1);
				}
				if (doneflag && currlevel == 13) {
					doneflag = DrlgL4PlaceMiniSet(L4TWARP, 1, 1, -1, -1, true, 6);
				}
				ViewX++;
			}
		} else {
			if (entry == ENTRY_MAIN) {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, true, 0);
				if (doneflag) {
					if (!gbIsMultiplayer && quests[Q_DIABLO]._qactive != QUEST_ACTIVE) {
						doneflag = DrlgL4PlaceMiniSet(L4PENTA, 1, 1, -1, -1, false, 1);
					} else {
						doneflag = DrlgL4PlaceMiniSet(L4PENTA2, 1, 1, -1, -1, false, 1);
					}
				}
				ViewX++;
			} else {
				doneflag = DrlgL4PlaceMiniSet(L4USTAIRS, 1, 1, -1, -1, false, 0);
				if (doneflag) {
					if (!gbIsMultiplayer && quests[Q_DIABLO]._qactive != QUEST_ACTIVE) {
						doneflag = DrlgL4PlaceMiniSet(L4PENTA, 1, 1, -1, -1, true, 1);
					} else {
						doneflag = DrlgL4PlaceMiniSet(L4PENTA2, 1, 1, -1, -1, true, 1);
					}
				}
				ViewY++;
			}
		}
	} while (!doneflag);

	DRLG_L4GeneralFix();

	if (currlevel != 16) {
		DRLG_PlaceThemeRooms(7, 10, 6, 8, true);
	}

	DrlgL4Shadows();
	DrlgL4Corners();
	DrlgL4Subs();
	DRLG_Init_Globals();

	if (QuestStatus(Q_WARLORD)) {
		for (int j = 0; j < DMAXY; j++) {
			for (int i = 0; i < DMAXX; i++) {
				pdungeon[i][j] = dungeon[i][j];
			}
		}
	}

	DRLG_CheckQuests(SP4x1, SP4y1);

	if (currlevel == 15) {
		for (int j = 0; j < DMAXY; j++) {
			for (int i = 0; i < DMAXX; i++) {
				if (dungeon[i][j] == 98) {
					Make_SetPC(i - 1, j - 1, 5, 5);
				}
				if (dungeon[i][j] == 107) {
					Make_SetPC(i - 1, j - 1, 5, 5);
				}
			}
		}
	}
	if (currlevel == 16) {
		for (int j = 0; j < DMAXY; j++) {
			for (int i = 0; i < DMAXX; i++) {
				pdungeon[i][j] = dungeon[i][j];
			}
		}
		DRLG_LoadDiabQuads(false);
	}
}

static void DrlgL4Pass3()
{
	DRLG_LPass3(30 - 1);
}

void CreateL4Dungeon(uint32_t rseed, lvl_entry entry)
{
	SetRndSeed(rseed);

	dminx = 16;
	dminy = 16;
	dmaxx = 96;
	dmaxy = 96;

	ViewX = 40;
	ViewY = 40;

	DRLG_InitSetPC();
	DRLG_LoadL4SP();
	DrlgL4(entry);
	DrlgL4Pass3();
	DRLG_FreeL4SP();
	DRLG_SetPC();
}

void LoadL4Dungeon(const char *path, int vx, int vy)
{
	dminx = 16;
	dminy = 16;
	dmaxx = 96;
	dmaxy = 96;

	DRLG_InitTrans();
	InitL4Dungeon();

	auto dunData = LoadFileInMem<uint16_t>(path);

	DRLG_L4SetRoom(dunData.get(), 0, 0);

	ViewX = vx;
	ViewY = vy;

	DrlgL4Pass3();
	DRLG_Init_Globals();

	SetMapMonsters(dunData.get(), { 0, 0 });
	SetMapObjects(dunData.get(), 0, 0);
}

void LoadPreL4Dungeon(const char *path)
{
	dminx = 16;
	dminy = 16;
	dmaxx = 96;
	dmaxy = 96;

	InitL4Dungeon();

	auto dunData = LoadFileInMem<uint16_t>(path);

	DRLG_L4SetRoom(dunData.get(), 0, 0);
}

} // namespace devilution
