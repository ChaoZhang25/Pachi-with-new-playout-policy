#define DEBUG
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "board.h"
#include "debug.h"
#include "tactics/selfatari.h"
#include "tactics/ladder.h"
#include "random.h"

static bool board_printed;

void
board_load(struct board *b, FILE *f, unsigned int size)
{
	board_printed = false;
	board_resize(b, size);
	board_clear(b);
	for (int y = size - 1; y >= 0; y--) {
		char line[256];
		if (!fgets(line, sizeof(line), f)) {
			fprintf(stderr, "Premature EOF.\n");
			exit(EXIT_FAILURE);
		}
		line[strlen(line) - 1] = 0; // chomp
		if (strlen(line) != size * 2 - 1) {
			fprintf(stderr, "Line not %d char long: %s\n", size * 2 - 1, line);
			exit(EXIT_FAILURE);
		}
		for (unsigned int i = 0; i < size * 2; i++) {
			enum stone s;
			switch (line[i]) {
				case '.': s = S_NONE; break;
				case 'X': s = S_BLACK; break;
				case 'O': s = S_WHITE; break;
				default: fprintf(stderr, "Invalid stone '%c'\n", line[i]);
					 exit(EXIT_FAILURE);
			}
			i++;
			if (line[i] != ' ' && i/2 < size - 1) {
				fprintf(stderr, "No space after stone %i: '%c'\n", i/2 + 1, line[i]);
				exit(EXIT_FAILURE);
			}
			if (s == S_NONE) continue;
			struct move m = { .color = s, .coord = coord_xy(b, i/2 + 1, y + 1) };
			if (board_play(b, &m) < 0) {
				fprintf(stderr, "Failed to play %s %s\n",
					stone2str(s), coord2sstr(m.coord, b));
				board_print(b, stderr);
				exit(EXIT_FAILURE);
			}
		}
	}
	if (DEBUGL(2))
		board_print(b, stderr);
	int suicides = b->captures[S_BLACK] || b->captures[S_WHITE];
	assert(!suicides);
}

bool
test_sar(struct board *b, char *arg)
{
	enum stone color = str2stone(arg);
	arg += 2;
	coord_t *cc = str2coord(arg, board_size(b));
	coord_t c = *cc; coord_done(cc);
	arg += strcspn(arg, " ") + 1;
	int eres = atoi(arg);
	if (DEBUGL(1))
		printf("sar %s %s %d...\t", stone2str(color), coord2sstr(c, b), eres);

	assert(board_at(b, c) == S_NONE);
	int rres = is_bad_selfatari(b, color, c);

	if (rres == eres) {
		if (DEBUGL(1))
			printf("OK\n");
	} else {
		if (debug_level <= 2) {
			if (DEBUGL(0) && !board_printed) {
				board_print(b, stderr);
				board_printed = true;
			}
			printf("sar %s %s %d...\t", stone2str(color), coord2sstr(c, b), eres);
		}
		printf("FAILED (%d)\n", rres);
	}
	return rres == eres;
}

bool
test_ladder(struct board *b, char *arg)
{
	enum stone color = str2stone(arg);
	arg += 2;
	coord_t *cc = str2coord(arg, board_size(b));
	coord_t c = *cc; coord_done(cc);
	arg += strcspn(arg, " ") + 1;
	int eres = atoi(arg);
	if (DEBUGL(1))
		printf("ladder %s %s %d...\t", stone2str(color), coord2sstr(c, b), eres);

	assert(board_at(b, c) == S_NONE);
	group_t atari_neighbor = board_get_atari_neighbor(b, c, color);
	assert(atari_neighbor);
	int rres = is_ladder(b, c, atari_neighbor, true);
	
	if (rres == eres) {
		if (DEBUGL(1))
			printf("OK\n");
	} else {
		if (debug_level <= 2) {
			if (DEBUGL(0) && !board_printed) {
				board_print(b, stderr);
				board_printed = true;
			}
			printf("ladder %s %s %d...\t", stone2str(color), coord2sstr(c, b), eres);
		}
		printf("FAILED (%d)\n", rres);
	}

	return (rres == eres);
}


void
unittest(char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	int total = 0;
	int passed = 0;
	int skipped = 0;
	
	struct board *b = board_init(NULL);
	char line[256];

	while (fgets(line, sizeof(line), f)) {
		line[strlen(line) - 1] = 0; // chomp
		switch (line[0]) {
			case '%': printf("\n%s\n", line); continue;
			case '!': printf("%s...\tSKIPPED\n", line); skipped++; continue;
			case 0: continue;
		}
		if (!strncmp(line, "boardsize ", 10)) {
			board_load(b, f, atoi(line + 10));
			continue;
		}
		
		total++;
		if (!strncmp(line, "sar ", 4))
			passed += test_sar(b, line + 4); 
		else if (!strncmp(line, "ladder ", 7))
			passed += test_ladder(b, line + 7);
		else {
			fprintf(stderr, "Syntax error: %s\n", line);
			exit(EXIT_FAILURE);
		}
	}

	fclose(f);
	
	printf("\n\n----------- [  %i/%i tests passed (%i%%)  ] -----------\n\n", passed, total, passed * 100 / total);
 	if (total == passed)
		printf("\nAll tests PASSED");
	else {
		printf("\nSome tests FAILED\n");
		exit(EXIT_FAILURE);
	}
	if (skipped > 0)
		printf(", %d test(s) SKIPPED", skipped);
	printf("\n");
}
