#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define INF 2000000
#define KING 2000000
#define QUEEN 975
#define ROOK 500
#define BISHOP 330
#define KNIGHT 320
#define PAWN 100

#define ADEPTH 6  // if too slow change it to 5 or 4

int DEPTH = ADEPTH;

struct node {
	int board[64];
	int tomove; // 0 = white, 1 = black
	int fpoint; // cached evaluation
	int en_passant; // -1 or square index
	bool any_king_dead;
};
struct node* root;

struct node* init_node(void) {
	return calloc(1, sizeof(struct node));
}

void init_board(void) {
	static const int initial[64] = {
		-ROOK, -KNIGHT, -BISHOP, -QUEEN, -KING, -BISHOP, -KNIGHT, -ROOK,
		-PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
		ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK
	};
	root = init_node();
	memcpy(root->board, initial, sizeof(initial));
	root->tomove = 0;
	root->fpoint = 0;
	root->en_passant = -1;
	root->any_king_dead = false;
}

// ==─ Evaluation ============================================─
static const char mod_eval[7][64] = {
	{0},
	// pawn/rook/queen 1
	{17, 29, 44, 55, 55, 44, 29, 17,
	13, 22, 32, 40, 40, 32, 22, 13,
	13, 22, 32, 40, 40, 32, 22, 13,
	17, 29, 44, 55, 55, 44, 29, 17,
	17, 29, 44, 55, 55, 44, 29, 17,
	13, 22, 32, 40, 40, 32, 22, 13,
	5, 8, 12, 16, 16, 12, 8, 5,
	0, 2, 3, 4, 4, 3, 2, 0},
	// knight 2
	{0, 2, 3, 4, 4, 3, 2, 0,
	5, 8, 12, 16, 16, 12, 8, 5,
	13, 22, 32, 40, 40, 32, 22, 13,
	17, 29, 44, 55, 55, 44, 29, 17,
	17, 29, 44, 55, 55, 44, 29, 17,
	13, 22, 32, 40, 40, 32, 22, 13,
	5, 8, 12, 16, 16, 12, 8, 5,
	0, 2, 3, 4, 4, 3, 2, 0},
	// bishop 3
	{15, 10, 8, 4, 4, 8, 2, 15,
	13, 44, 16, 12, 16, 12, 44, 13,
	10, 22, 50, 40, 40, 50, 22, 10,
	10, 29, 44, 55, 55, 44, 29, 10,
	10, 29, 44, 55, 55, 44, 29, 10,
	10, 22, 50, 40, 40, 50, 22, 10,
	13, 44, 16, 12, 16, 12, 44, 13,
	15, 10, 8, 4, 4, 8, 2, 15},
};

int evaluate(struct node* n) {
	int score = 0;
	for (int i = 0; i < 64; i++) {
		int p_val = n->board[i];
		if (p_val == 0) continue;
		int pice = 0;
		switch (abs(p_val)) {
			case KING :   pice = 0; break;
			case QUEEN :
			case ROOK :
			case PAWN :	  pice = 1; break;
			case BISHOP : pice = 3; break;
			case KNIGHT : pice = 2; break;
		}
		if (p_val > 0) {
			score += mod_eval[pice][i] + p_val;
		} else {
			score += -mod_eval[pice][i^56] + p_val;
		}
	}
	n->fpoint = score;
	return score;
}

// == Debug / Display ========================================
void print_pice(int pice, int sq) {
	sq += ((sq/8)%2)? 0 : 1; 
	if (sq % 2) { //black
		printf("\033[107m\033[30m");
		switch(pice) {
			case -KING  : printf(" ♚  "); break;
			case -QUEEN : printf(" ♛  "); break;
			case -ROOK  : printf(" ♜  "); break;
			case -BISHOP: printf(" ♝  "); break;
			case -KNIGHT: printf(" ♞  "); break;
			case -PAWN  : printf(" ♟  "); break;
			case KING  : printf(" ♔  "); break;
			case QUEEN : printf(" ♕  "); break;
			case ROOK  : printf(" ♖  "); break;
			case BISHOP: printf(" ♗  "); break;
			case KNIGHT: printf(" ♘  "); break;
			case PAWN  : printf(" ♙  "); break;
			default    : printf("    ");
		}
	}
	else {
		printf("\033[97m\033[40m");
		switch(pice) {
			case KING  : printf(" ♚  "); break;
			case QUEEN : printf(" ♛  "); break;
			case ROOK  : printf(" ♜  "); break;
			case BISHOP: printf(" ♝  "); break;
			case KNIGHT: printf(" ♞  "); break;
			case PAWN  : printf(" ♟  "); break;
			case -KING  : printf(" ♔  "); break;
			case -QUEEN : printf(" ♕  "); break;
			case -ROOK  : printf(" ♖  "); break;
			case -BISHOP: printf(" ♗  "); break;
			case -KNIGHT: printf(" ♘  "); break;
			case -PAWN  : printf(" ♙  "); break;
			default    : printf("    ");
		}
	}
}
void print_board(struct node* n) {
	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0) printf("\n");
		print_pice(n->board[i], i);
	}
	printf("\033[0m\n tomove: %d eval: %f\n", n->tomove, (float)n->fpoint / 100);
}

int square_to_index(const char* s) {
	if (strlen(s) < 2) return -1;
	int file = s[0] - 'a';
	int rank = s[1] - '1';
	if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
	return (7 - rank) * 8 + file;
}

void index_to_square(int idx, char* buf) {
	int r = idx / 8;
	int f = idx % 8;
	sprintf(buf, "%c%hd", 'a' + f, 8 - r);
}

int pos(int r, int c) {
	return r * 8 + c;
}

// == Move Making ============================================

void make_move(struct node* n, int from, int to) {
	if (abs(n->board[to]) == KING) n->any_king_dead = true;
	int piece = n->board[from];
	n->board[to] = piece;
	n->board[from] = 0;
// Promotion (queen only)
	int promo_rank = (n->tomove == 0) ? 0 : 7;
	if (abs(piece) == PAWN && (to / 8) == promo_rank) {
		n->board[to] = (n->tomove == 0) ? QUEEN : -QUEEN;
	}
// En passant capture
	if (abs(piece) == PAWN && to == n->en_passant) {
		int dir = (n->tomove == 0) ? 1 : -1;
		n->board[pos((to / 8) + dir, to % 8)] = 0;
	}
// Set new en passant target
	n->en_passant = -1;
	if (abs(piece) == PAWN) {
		int dist = abs((from / 8) - (to / 8));
		if (dist == 2) {
			int ep_r = (from / 8 + to / 8) / 2;
			n->en_passant = pos(ep_r, from % 8);
		}
	}
// Castling – also move rook
	if (abs(piece) == KING && abs(to % 8 - from % 8) == 2) {
		bool kingside = (to % 8 > from % 8);
		int rank = (1 - n->tomove == 0) ? 0 : 7;
		if (kingside) {
			n->board[pos(rank, 5)] = n->board[pos(rank, 7)];
			n->board[pos(rank, 7)] = 0;
		} else {
			n->board[pos(rank, 3)] = n->board[pos(rank, 0)];
			n->board[pos(rank, 0)] = 0;
		}
	}
	n->tomove = 1 - n->tomove;
}

// == Move Generation Helpers ================================
bool on_board(int r, int c) {
	return r >= 0 && r < 8 && c >= 0 && c < 8;
}
bool is_white(int p) { return p > 0; }
bool is_black(int p) { return p < 0; }
bool is_enemy(int mover, int p) {
	return (mover == 0) ? is_black(p) : is_white(p);
}

// == Pseudo-legal moves ====================================─
void add_move(int* moves, int* cnt, int to) {
	if (*cnt < 128) moves[(*cnt)++] = to;
}

void gen_pawn_moves(struct node* n, int from, int* moves, int* cnt) {
	int r = from / 8, c = from % 8;
	int dir = (n->tomove == 0) ? -1 : 1;
	int start = (n->tomove == 0) ? 6 : 1;
	int nr = r + dir;
	if (on_board(nr, c) && n->board[pos(nr, c)] == 0)
		add_move(moves, cnt, pos(nr, c));
	if (r == start) {
		int nr2 = r + 2 * dir;
		if (on_board(nr2, c) &&
		        n->board[pos(nr, c)] == 0 &&
		        n->board[pos(nr2, c)] == 0)
			add_move(moves, cnt, pos(nr2, c));
	}
	for (int d = -1; d <= 1; d += 2) {
		int nc = c + d;
		if (on_board(nr, nc)) {
			int t = n->board[pos(nr, nc)];
			if ((t != 0 && is_enemy(n->tomove, t)) ||
			        pos(nr, nc) == n->en_passant)
				add_move(moves, cnt, pos(nr, nc));
		}
	}
}

void gen_sliding(struct node* n, int from, const int dirs[][2], int ndirs,
                 int* moves, int* cnt) {
	int r = from / 8, c = from % 8;
	int friendly = (n->tomove == 0) ? 1 : -1;
	for (int d = 0; d < ndirs; d++) {
		int dr = dirs[d][0], dc = dirs[d][1];
		int nr = r + dr, nc = c + dc;
		while (on_board(nr, nc)) {
			int sq = pos(nr, nc);
			int t = n->board[sq];
			if (t == 0) {
				add_move(moves, cnt, sq);
			} else {
				if (t * friendly < 0) add_move(moves, cnt, sq);
				break;
			}
			nr += dr;
			nc += dc;
		}
	}
}

void generate_moves(struct node* n, int from, int moves[], int* cnt) {
	*cnt = 0;
	int p = n->board[from];
	if (!p) return;
	int r = from / 8, c = from % 8;
	switch (abs(p)) {
	case KING: {
		static const int dr[] = { -1, -1, -1, 0, 0, 1, 1, 1};
		static const int dc[] = { -1, 0, 1, -1, 1, -1, 0, 1};
		for (int d = 0; d < 8; d++) {
			int nr = r + dr[d], nc = c + dc[d];
			if (on_board(nr, nc)) {
				int t = n->board[pos(nr, nc)];
				if (t == 0 || is_enemy(n->tomove, t))
					add_move(moves, cnt, pos(nr, nc));
			}
		}
		break;
	}
	case QUEEN: {
		static const int dirs[8][2] = {{ -1, -1}, { -1, 0}, { -1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
		gen_sliding(n, from, dirs, 8, moves, cnt);
		break;
	}
	case ROOK: {
		static const int dirs[4][2] = {{ -1, 0}, {1, 0}, {0, -1}, {0, 1}};
		gen_sliding(n, from, dirs, 4, moves, cnt);
		break;
	}
	case BISHOP: {
		static const int dirs[4][2] = {{ -1, -1}, { -1, 1}, {1, -1}, {1, 1}};
		gen_sliding(n, from, dirs, 4, moves, cnt);
		break;
	}
	case KNIGHT: {
		static const int dr[] = { -2, -2, -1, -1, 1, 1, 2, 2};
		static const int dc[] = { -1, 1, -2, 2, -2, 2, -1, 1};
		for (int d = 0; d < 8; d++) {
			int nr = r + dr[d], nc = c + dc[d];
			if (on_board(nr, nc)) {
				int t = n->board[pos(nr, nc)];
				if (t == 0 || is_enemy(n->tomove, t))
					add_move(moves, cnt, pos(nr, nc));
			}
		}
		break;
	}
	case PAWN:
		gen_pawn_moves(n, from, moves, cnt);
		break;
	}
}

// == Check detection ========================================
bool any_king_isDead(struct node* n) {
	bool bd = true, wd = true;
	for (int i = 0; i < 64; i++) {
		if (n->board[i] ==  KING) wd = false;
		if (n->board[i] == -KING) bd = false;
	}
	return (wd | bd);
}

// == Search ================================================─
typedef struct {
	int score;
	int from;
	int to;
} SearchResult;

int nodecount;

SearchResult minimax(struct node* n, int depth, int alpha, int beta) {
	SearchResult res = {0, -1, -1};

	if (depth == 0 || n->any_king_dead) {
		res.score = evaluate(n);
		nodecount++;
		return res;
	}

	bool maximizing = (n->tomove == 0);
	int best_val  = maximizing ? -INF : INF;
	int best_from = -1;
	int best_to   = -1;

	for (int from = 0; from < 64; from++) {
		int p = n->board[from];
		if (p == 0) continue;
		if ( maximizing && !is_white(p)) continue;
		if (!maximizing && !is_black(p)) continue;

		int moves[128];
		int cnt = 0;
		generate_moves(n, from, moves, &cnt);

		for (int i = 0; i < cnt; i++) {
			int to = moves[i];

			// == Save full state needed for undo ==================
			int saved_en_passant  = n->en_passant;
			int saved_tomove      = n->tomove;
			int saved_king_dead   = n->any_king_dead;   // BUG 3 FIX: save, don't blindly clear
			int moving_piece      = n->board[from];
			int captured          = n->board[to];

			// BUG 1 FIX: save rook squares before castling scrambles them
			int castle_rook_from = -1, castle_rook_to = -1, castle_rook_piece = 0;
			if (abs(moving_piece) == KING && abs(to % 8 - from % 8) == 2) {
				int rank = (n->tomove == 0) ? 7 : 0;
				bool kingside = (to % 8 > from % 8);
				if (kingside) {
					castle_rook_from  = pos(rank, 7);
					castle_rook_to    = pos(rank, 5);
				} else {
					castle_rook_from  = pos(rank, 0);
					castle_rook_to    = pos(rank, 3);
				}
				castle_rook_piece = n->board[castle_rook_from];
			}

			// BUG 2 FIX: save the en-passant captured pawn before it is erased
			int ep_captured_sq    = -1;
			int ep_captured_piece = 0;
			if (abs(moving_piece) == PAWN && to == n->en_passant) {
				int dir = (n->tomove == 0) ? 1 : -1;
				ep_captured_sq    = pos((to / 8) + dir, to % 8);
				ep_captured_piece = n->board[ep_captured_sq];
			}

			make_move(n, from, to);

			SearchResult child = minimax(n, depth - 1, alpha, beta);

			// == Undo ============================================─
			n->board[from]      = moving_piece;
			n->board[to]        = captured;
			n->en_passant       = saved_en_passant;
			n->tomove           = saved_tomove;
			n->any_king_dead    = saved_king_dead;      // BUG 3 FIX

			// BUG 1 FIX: restore rook to its original square
			if (castle_rook_from >= 0) {
				n->board[castle_rook_from] = castle_rook_piece;
				n->board[castle_rook_to]   = 0;
			}

			// BUG 2 FIX: restore en-passant captured pawn
			if (ep_captured_sq >= 0) {
				n->board[ep_captured_sq] = ep_captured_piece;
			}

			int val = child.score;
			if (maximizing) {
				if (val > best_val) {
					best_val  = val;
					best_from = from;
					best_to   = to;
				}
				if (val > alpha) alpha = val;
			} else {
				if (val < best_val) {
					best_val  = val;
					best_from = from;
					best_to   = to;
				}
				if (val < beta) beta = val;
			}
			if (alpha >= beta) goto cutoff;
		}
	}

cutoff:
	res.score = best_val;
	res.from  = best_from;
	res.to    = best_to;
	return res;
}

void print_banner(void) {
	printf(
" _           _     ___           _\n"
"| |__  __ _ (_)___| __|_ _  __ _(_)_ _\n"
"| '_ \\/ _` || / -_) _|| ' \\/ _` | | ' \\\n"
"|_.__/\\__,_|/ \\___|___|_||_\\__, |_|_||_|\n"
"          |__/             |___/\n");
}

// == Console play mode ==========================
void console_play(void) {
		#ifdef _WIN32
			system("cls");
		#else
			system("clear");
		#endif
	print_banner();
	init_board();
	print_board(root);
	char line[256];
	printf("You play WHITE by default.\n");
	printf("Enter move as e2e4, e7e8q, 0-0, 0-0-0  |  'b' = play as Black  |  'q' = quit\n");
	while (fgets(line, sizeof(line), stdin)) {
		line[strcspn(line, "\n")] = 0;
		if (strcmp(line, "q") == 0 || strcmp(line, "quit") == 0) break;
 
		if (strcmp(line, "b") == 0) {
			// Switch sides: engine plays white's first move immediately
			root->tomove = 0;
			printf("You are Black. Engine thinking...\n");
			goto engine_turn;
		}
		// Castling: translate to king two-square move; make_move handles the rook internally
		else if (strcmp(line, "0-0") == 0 || strcmp(line, "O-O") == 0) {
			int from = (root->tomove == 0) ? 60 : 4;
			int to   = (root->tomove == 0) ? 62 : 6;
			if (root->board[from] == 0) { printf("Illegal: no king on expected square\n"); continue; }
			make_move(root, from, to);
		}
		else if (strcmp(line, "0-0-0") == 0 || strcmp(line, "O-O-O") == 0) {
			int from = (root->tomove == 0) ? 60 : 4;
			int to   = (root->tomove == 0) ? 58 : 2;
			if (root->board[from] == 0) { printf("Illegal: no king on expected square\n"); continue; }
			make_move(root, from, to);
		}
		else if (strlen(line) >= 4) {
			char from_str[3] = {line[0], line[1], 0};
			char to_str[3]   = {line[2], line[3], 0};
			int from = square_to_index(from_str);
			int to   = square_to_index(to_str);
			if (from < 0 || to < 0 || root->board[from] == 0) {
				printf("Illegal move format\n");
				continue;
			}
			make_move(root, from, to);
		}
		else {
			printf("Unknown input\n");
			continue;
		}
 
		#ifdef _WIN32
			system("cls");
		#else
			system("clear");
		#endif
		print_board(root);
 
		if (any_king_isDead(root)) {
			printf("Game over.\n");
			break;
		}
 
engine_turn:
		printf("Thinking...\n");
		nodecount = 0;
		SearchResult r = minimax(root, DEPTH, -INF, INF);
		if (r.from >= 0 && r.to >= 0) {
			char buf_from[8], buf_to[8];
			index_to_square(r.from, buf_from);
			index_to_square(r.to,   buf_to);
			printf("Engine plays: %s%s  (eval %+.2f,  nodes %d)\n",
			       buf_from, buf_to, (float)r.score / 100, 2 * nodecount - 1);
			make_move(root, r.from, r.to);
			system("clear");
			print_board(root);
			if (any_king_isDead(root)) {
				printf("Game over.\n");
				break;
			}
			printf("Your move: ");
			fflush(stdout);
		} else {
			printf("Engine has no legal moves (checkmate / stalemate).\n");
			break;
		}
	}
	free(root);
	root = NULL;
}
 
// == UCI mode ================================
int main(int argc, char** argv) {
	srand(time(NULL));
	if (argc > 1 && strcmp(argv[1], "play") == 0) {
		console_play();
		return 0;
	}
	srand(time(NULL));
	init_board();
	char line[4096];
	setbuf(stdout, NULL);
	while (fgets(line, sizeof(line), stdin)) {
		line[strcspn(line, "\n")] = 0;
		if (!strlen(line)) continue;
		char cmd[64];
		if (sscanf(line, "%63s", cmd) != 1) continue;
		if (strcmp(cmd, "uci") == 0) {
			print_banner();
			puts("id name bajeEngin.0.07 - Chandan Das");
			puts("id author Chandan Das");
			puts("uciok");
		}
		else if (strcmp(cmd, "isready") == 0) {
			puts("readyok");
		}
		else if (strcmp(cmd, "ucinewgame") == 0) {
			init_board();
		}
		else if (strcmp(cmd, "position") == 0) {
			if (strstr(line, "startpos")) {
				init_board();
			} else if (strstr(line, "fen")) {
				init_board();
			}
			char* moves = strstr(line, "moves");
			if (moves) {
				moves += 5;
				char m[8];
				while (sscanf(moves, "%7s", m) == 1) {
					int from = square_to_index(m);
					int to   = square_to_index(m + 2);
					if (from >= 0 && to >= 0) make_move(root, from, to);
					moves += strlen(m) + 1;
				}
			}
		}
		else if (strcmp(cmd, "go") == 0) {
			printf("info depth %d\n", DEPTH);
			nodecount = 0;
			SearchResult r = minimax(root, DEPTH, -INF, INF);
			if (r.from >= 0 && r.to >= 0) {
				char buf[16];
				index_to_square(r.from, buf);
				int len = strlen(buf);
				index_to_square(r.to, buf + len);
				printf("bestmove %s\n", buf);
				make_move(root, r.from, r.to);
				fflush(stdout);
				printf("info string nodes %d depth %d score %.3f\n", 2 * nodecount - 1, DEPTH,
				       (float)r.score / 100);
			} else {
				if (any_king_isDead(root)) {
					printf("info string Checkmate - resigning\n");
				} else {
					printf("info string Stalemate / no moves - resigning\n");
				}
				puts("resign");
			}
		}
		else if (strcmp(cmd, "d") == 0 || strcmp(cmd, "board") == 0) {
			print_board(root);
		}
		else if (strcmp(cmd, "quit") == 0) {
			break;
		}
	}
	free(root);
	return 0;
}
