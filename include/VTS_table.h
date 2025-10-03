#pragma once

#define ESC "\x1B"
#define BEL "\x07"
#define CSI ESC"["
#define OSC ESC"]"

#define BUF_MAIN_SCR 			CSI"?1049l"
#define BUF_ALTR_SCR 			CSI"?1049h"

#define CLEAR_DISPLAY			CSI"2J"
#define CLEAR_SCROLL_BACK		CSI"3J"

#define WINDOW_TITLE(title)		OSC"0;"#title BEL

#define DEC_MODE				ESC"(0"
#define ASCII_MODE				ESC"(B"

#define DEC_CORNER_BR			"j"
#define DEC_CORNER_UR			"k"
#define DEC_CORNER_UL			"l"
#define DEC_CORNER_BL			"m"
#define DEC_CROSS				"n"
#define DEC_LINE_HOR			"q"
#define DEC_CROSS_HL			"t"
#define DEC_CROSS_HR			"u"
#define DEC_CROSS_HB			"v"
#define DEC_CROSS_HU			"w"
#define DEC_LINE_VERT			"x"

#define DEC_LINE_HOR_(n) for(size_t i=0; i<n; i++){printf(DEC_LINE_HOR);}
#define DEC_LINE_UP(n) printf(DEC_CORNER_UL); DEC_LINE_HOR_(n) printf(DEC_CORNER_UR);
#define DEC_LINE_BOTTOM(n) printf(DEC_CORNER_BL); DEC_LINE_HOR_(n) printf(DEC_CORNER_BR);

#define CUR_SAVE 				ESC"7"
#define CUR_LOAD 				ESC"8"
#define CUR_SHOW 				CSI"?25h"
#define CUR_HIDE 				CSI"?25l"
#define CUR_BLINK_ON 			CSI"?12h"
#define CUR_BLINK_OFF 			CSI"?12l"

#define CUR_SHAPE_DEFAULT		CSI"0 q"
#define CUR_SHAPE_BLOCK_BLINK	CSI"1 q"
#define CUR_SHAPE_BLOCK_STEADY 	CSI"2 q"

#define CUR_TO_ORIGIN			CSI"1;1H"
#define CUR_MOVE_TO				CSI"%d;%dH"
#define CUR_MOVE_TO_(x,y)		CSI#x";"#y"H"
#define CUR_MOVE_UP				CSI"1A"
#define CUR_MOVE_UP_(n)			CSI#n"A"
#define CUR_MOVE_DOWN			CSI"1B"
#define CUR_MOVE_DOWN_(n)		CSI#n"B"
#define CUR_MOVE_FORWARD		CSI"1C"
#define CUR_MOVE_FORWARD_(n)	CSI#n"C"
#define CUR_MOVE_BACKWARD		CSI"1D"
#define CUR_MOVE_BACKWARD_(n)	CSI#n"D"
#define CUR_MOVE_NEXT_LINE		CSI"1E"
#define CUR_MOVE_NEXT_LINE_(n)	CSI#n"E"
#define CUR_MOVE_PREV_LINE		CSI"1F"
#define CUR_MOVE_PREV_LINE_(n)	CSI#n"F"
#define CUR_MOVE_HOR_ABS		CSI"%dG"
#define CUR_MOVE_HOR_ABS_(n)	CSI#n"G"
#define CUR_MOVE_VER_ABS		CSI"%dd"
#define CUR_MOVE_VER_ABS_(n)	CSI#n"d"


#define COL_DEFAULT				CSI"0m"
#define COL_BOLD				CSI"1m"
#define COL_NO_BOLD				CSI"22m"
#define COL_UNDERLINE			CSI"4m"
#define COL_NO_UNDERLINE		CSI"24m"
#define COL_NEGATIVE			CSI"7m"
#define COL_POSITIVE			CSI"27m"


// COL_F_* foreground
#define COL_F_BLACK				CSI"30m"
#define COL_F_RED				CSI"31m"
#define COL_F_GREEN				CSI"32m"
#define COL_F_YELLOW			CSI"33m"
#define COL_F_BLUE				CSI"34m"
#define COL_F_MAGENTA			CSI"35m"
#define COL_F_CYAN				CSI"36m"
#define COL_F_WHITE				CSI"37m"
#define COL_F_EXTENDED			CSI"38;2;%d;%d;%dm"
#define COL_F_EXTENDED_(r,g,b)	CSI"38;2;"#r";"#g";"#b"m"
#define COL_F_DEFAULT			CSI"39m"

// COL_B_* background
#define COL_B_BLACK				CSI"40m"
#define COL_B_RED				CSI"41m"
#define COL_B_GREEN				CSI"42m"
#define COL_B_YELLOW			CSI"43m"
#define COL_B_BLUE				CSI"44m"
#define COL_B_MAGENTA			CSI"45m"
#define COL_B_CYAN				CSI"46m"
#define COL_B_WHITE				CSI"47m"
#define COL_B_EXTENDED			CSI"48;2;%d;%d;%dm"
#define COL_B_EXTENDED_(r,g,b)	CSI"48;2;"#r";"#g";"#b"m"
#define COL_B_DEFAULT			CSI"49m"

// COL_BF_* bold/bright foreground
#define COL_BF_BLACK			CSI"90m"
#define COL_BF_RED				CSI"91m"
#define COL_BF_GREEN			CSI"92m"
#define COL_BF_YELLOW			CSI"93m"
#define COL_BF_BLUE				CSI"94m"
#define COL_BF_MAGENTA			CSI"95m"
#define COL_BF_CYAN				CSI"96m"
#define COL_BF_WHITE			CSI"97m"

// COL_BB_* bold/bright background
#define COL_BB_BLACK			CSI"100m"
#define COL_BB_RED				CSI"101m"
#define COL_BB_GREEN			CSI"102m"
#define COL_BB_YELLOW			CSI"103m"
#define COL_BB_BLUE				CSI"104m"
#define COL_BB_MAGENTA			CSI"105m"
#define COL_BB_CYAN				CSI"106m"
#define COL_BB_WHITE			CSI"107m"

// #DEPRECATED :all bright foreground
#define BLACK(t) "\x1B[90m" t "\x1B[0m"
#define RED(t) "\x1B[91m" t "\x1B[0m"
#define GREEN(t) "\x1B[92m" t "\x1B[0m"
#define YELLOW(t) "\x1B[93m" t "\x1B[0m"
#define BLUE(t) "\x1B[94m" t "\x1B[0m"
#define MAGENTA(t) "\x1B[95m" t "\x1B[0m"
#define CYAN(t) "\x1B[96m" t "\x1B[0m"
#define WHITE(t) "\x1B[97m" t "\x1B[0m"

#include <stdio.h>

static void VTSColorTable() {
	printf(COL_B_BLACK " " COL_B_RED " " COL_B_GREEN " " COL_B_YELLOW " " COL_B_BLUE " " COL_B_MAGENTA " " COL_B_CYAN " " COL_B_WHITE " " COL_B_DEFAULT);
	printf("\n");
	printf(COL_BB_BLACK " " COL_BB_RED " " COL_BB_GREEN " " COL_BB_YELLOW " " COL_BB_BLUE " " COL_BB_MAGENTA " " COL_BB_CYAN " " COL_BB_WHITE " " COL_B_DEFAULT);
	printf("\n");
	printf(COL_F_BLACK "A" COL_F_RED "B" COL_F_GREEN "C" COL_F_YELLOW "D" COL_F_BLUE "E" COL_F_MAGENTA "F" COL_F_CYAN "G" COL_F_WHITE "H" COL_F_DEFAULT);
	printf("\n");
	printf(COL_BF_BLACK "A" COL_BF_RED "B" COL_BF_GREEN "C" COL_BF_YELLOW "D" COL_BF_BLUE "E" COL_BF_MAGENTA "F" COL_BF_CYAN "G" COL_BF_WHITE "H" COL_F_DEFAULT);
	printf("\n");
}

static void VTSDecTable() {
	printf(DEC_MODE);
	printf(DEC_CORNER_UL " " DEC_LINE_HOR " " DEC_CORNER_UR "\n");
	printf("\n");
	printf(DEC_CROSS_HL " " DEC_CROSS_HU " " DEC_CROSS_HR "\n");
	printf("\n");
	printf(DEC_LINE_VERT " " DEC_CROSS " " DEC_LINE_VERT "\n");
	printf("\n");
	printf(DEC_CROSS_HL " " DEC_CROSS_HB " " DEC_CROSS_HR "\n");
	printf("\n");
	printf(DEC_CORNER_BL " " DEC_LINE_HOR " " DEC_CORNER_BR "\n");
	printf(ASCII_MODE);
}

