/* Generated by re2c 0.5 on Mon Sep 20 15:47:10 1999 */
#line 1 "url_scanner.re"
/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999 The PHP Group                         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef MIN
#define MIN(a,b) (a)<(b)?(a):(b)

#define YYCTYPE char
#define YYCURSOR state->crs
#define YYLIMIT state->end
#define YYMARKER state->ptr
#define YYFILL(n)

typedef enum {
	INITIAL,
	REF
} state;

typedef struct {
	state state;
	const char *crs;
	const char *end;
	const char *ptr;
	const char *start;
	char *target;
	size_t targetsize;
	const char *data;
} lexdata;

#define FINISH { catchup(state); goto finish; }

#define BEGIN(x) 						\
		switch(state->state) { 			\
			case INITIAL: 				\
				catchup(state); 		\
				break; 					\
			case REF: 					\
				screw_url(state); 		\
				break; 					\
		} 								\
		state->state = x; 				\
		state->start = state->crs; 		\
		goto nextiter

#define ATTACH(s, n) 										\
{ 															\
	size_t _newlen = state->targetsize + n; 				\
	state->target = realloc(state->target, _newlen + 1); 	\
	memcpy(state->target + state->targetsize, s, n); 		\
	state->targetsize = _newlen; 							\
	state->target[_newlen] = '\0'; 							\
}
	
#define URLLEN 512
	
static void screw_url(lexdata *state)
{
	int len;
	char buf[URLLEN];
	char url[URLLEN];
	const char *p, *q;
	char c;

	/* search outer limits for URI */
	for(p = state->start; p < state->crs && (c = *p); p++)
		if(c != '"' && c != ' ') break;

	/*  
	 *  we look at q-1, because q points to the character behind the last
	 *  character we are going to copy and the decision is based on that last
	 *  character 
	 */

	for(q = state->crs; q > state->start && (c = *(q-1)); q--)
		if(c != '"' && c != ' ') break;

	/* attach beginning */
	
	ATTACH(state->start, p-state->start);
	
	/* copy old URI */
	len = MIN(q - p, sizeof(buf) - 1);
	memcpy(url, p, len);
	url[len] = '\0';
	
	/* construct new URI */
	len = snprintf(buf, sizeof(buf), "%s%c%s", url,
			memchr(state->start, '?', len) ? '&' : '?',
			state->data);

	/* attach new URI */
	ATTACH(buf, len);
	
	/* attach rest */
	ATTACH(q, state->crs - q);
}

static void catchup(lexdata *state) 
{
	ATTACH(state->start, (state->crs - state->start));
}

#line 135


static void url_scanner(lexdata *state)
{
	while(state->crs < state->end) {
	
	switch(state->state) {
		case INITIAL: 
{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	  0,  64,  64,  64,  64,  64,  64,  64, 
	 64, 192,  64, 192, 192,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	192,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,   0,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	 64,  64,  64,  64,  64,  64,  64,  64, 
	};
	goto yy0;
yy1:	++YYCURSOR;
yy0:
	if((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
	if(yybm[0+yych] & 64)	goto yy4;
	if(yych <= '\000')	goto yy7;
yy2:	yych = *++YYCURSOR;
	if(yych <= 'F'){
		if(yych == 'A')	goto yy9;
		if(yych >= 'F')	goto yy10;
	} else {
		if(yych <= 'a'){
			if(yych >= 'a')	goto yy9;
		} else {
			if(yych == 'f')	goto yy10;
		}
	}
yy3:yy4:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy5:	if(yybm[0+yych] & 64)	goto yy4;
yy6:
#line 146
	{ BEGIN(INITIAL); }
yy7:	yych = *++YYCURSOR;
yy8:
#line 147
	{ FINISH; }
yy9:	yych = *++YYCURSOR;
	if(yych == 'H')	goto yy3;
	if(yych == 'h')	goto yy3;
	goto yy25;
yy10:	yych = *++YYCURSOR;
	if(yych == 'R')	goto yy11;
	if(yych != 'r')	goto yy3;
yy11:	yych = *++YYCURSOR;
	if(yych == 'A')	goto yy12;
	if(yych != 'a')	goto yy3;
yy12:	yych = *++YYCURSOR;
	if(yych == 'M')	goto yy13;
	if(yych != 'm')	goto yy3;
yy13:	yych = *++YYCURSOR;
	if(yych == 'E')	goto yy14;
	if(yych != 'e')	goto yy3;
yy14:	yych = *++YYCURSOR;
	if(yych == 'S')	goto yy3;
	if(yych == 's')	goto yy3;
	goto yy16;
yy15:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy16:	if(yybm[0+yych] & 128)	goto yy15;
	if(yych == 'S')	goto yy17;
	if(yych != 's')	goto yy3;
yy17:	yych = *++YYCURSOR;
	if(yych == 'R')	goto yy18;
	if(yych != 'r')	goto yy3;
yy18:	yych = *++YYCURSOR;
	if(yych == 'C')	goto yy19;
	if(yych != 'c')	goto yy3;
yy19:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy20:	if(yych <= '\f'){
		if(yych == '\t')	goto yy19;
		if(yych <= '\n')	goto yy3;
		goto yy19;
	} else {
		if(yych <= ' '){
			if(yych <= '\037')	goto yy3;
			goto yy19;
		} else {
			if(yych != '=')	goto yy3;
		}
	}
yy21:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy22:	if(yych <= '\n'){
		if(yych == '\t')	goto yy21;
	} else {
		if(yych <= '\f')	goto yy21;
		if(yych == ' ')	goto yy21;
	}
yy23:
#line 144
	{ BEGIN(REF); }
yy24:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy25:	if(yych <= '\037'){
		if(yych <= '\t'){
			if(yych <= '\b')	goto yy3;
			goto yy24;
		} else {
			if(yych <= '\n')	goto yy3;
			if(yych <= '\f')	goto yy24;
			goto yy3;
		}
	} else {
		if(yych <= 'H'){
			if(yych <= ' ')	goto yy24;
			if(yych <= 'G')	goto yy3;
		} else {
			if(yych != 'h')	goto yy3;
		}
	}
yy26:	yych = *++YYCURSOR;
	if(yych == 'R')	goto yy27;
	if(yych != 'r')	goto yy3;
yy27:	yych = *++YYCURSOR;
	if(yych == 'E')	goto yy28;
	if(yych != 'e')	goto yy3;
yy28:	yych = *++YYCURSOR;
	if(yych == 'F')	goto yy29;
	if(yych != 'f')	goto yy3;
yy29:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy30:	if(yych <= '\f'){
		if(yych == '\t')	goto yy29;
		if(yych <= '\n')	goto yy3;
		goto yy29;
	} else {
		if(yych <= ' '){
			if(yych <= '\037')	goto yy3;
			goto yy29;
		} else {
			if(yych != '=')	goto yy3;
		}
	}
yy31:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy32:	if(yych <= '\n'){
		if(yych == '\t')	goto yy31;
	} else {
		if(yych <= '\f')	goto yy31;
		if(yych == ' ')	goto yy31;
	}
yy33:
#line 145
	{ BEGIN(REF); }
}
#line 148

			break;
		case REF: 
{
	YYCTYPE yych;
	unsigned int yyaccept;
	static unsigned char yybm[] = {
	  0, 192, 192, 192, 192, 192, 192, 192, 
	192,  32, 192,  32,  32, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	 32, 192,   0,   0, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 128, 192, 192, 192,   0, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	192, 192, 192, 192, 192, 192, 192, 192, 
	};
	goto yy34;
yy35:	++YYCURSOR;
yy34:
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yybm[0+yych] & 64)	goto yy39;
	if(yych <= '\000')	goto yy36;
	if(yych <= '"')	goto yy37;
	if(yych <= '#')	goto yy36;
	if(yych <= '=')	goto yy42;
yy36:yy37:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy38:	if(yybm[0+yych] & 64)	goto yy39;
	if(yych <= '\000')	goto yy36;
	if(yych <= '!')	goto yy37;
	if(yych == ':')	goto yy42;
	goto yy36;
yy39:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy40:	if(yybm[0+yych] & 64)	goto yy39;
	if(yych <= '"'){
		if(yych <= '\000')	goto yy41;
		if(yych <= '!')	goto yy48;
		goto yy50;
	} else {
		if(yych <= '#')	goto yy51;
		if(yych <= '=')	goto yy42;
	}
yy41:
#line 152
	{ BEGIN(INITIAL); }
yy42:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy43:	if(yybm[0+yych] & 128)	goto yy42;
	if(yych <= '\000')	goto yy44;
	if(yych <= '!')	goto yy45;
	if(yych <= '"')	goto yy47;
yy44:
#line 154
	{ 
			/* don't modify absolute links */
			state->state = INITIAL; BEGIN(INITIAL); 
	}
yy45:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy46:	if(yych <= '\f'){
		if(yych == '\t')	goto yy45;
		if(yych <= '\n')	goto yy44;
		goto yy45;
	} else {
		if(yych <= ' '){
			if(yych <= '\037')	goto yy44;
			goto yy45;
		} else {
			if(yych != '"')	goto yy44;
		}
	}
yy47:	yych = *++YYCURSOR;
	goto yy44;
yy48:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy49:	if(yych <= '\f'){
		if(yych == '\t')	goto yy48;
		if(yych <= '\n')	goto yy41;
		goto yy48;
	} else {
		if(yych <= ' '){
			if(yych <= '\037')	goto yy41;
			goto yy48;
		} else {
			if(yych != '"')	goto yy41;
		}
	}
yy50:	yych = *++YYCURSOR;
	goto yy41;
yy51:	yych = *++YYCURSOR;
yy52:	YYCURSOR -= 1;
#line 153
	{ BEGIN(INITIAL); }
}
#line 158

  			break;
	}
nextiter:
	;
	}
finish:
	;
}

char *url_adapt(const char *src, size_t srclen, const char *data, size_t *newlen)
{
	lexdata state;

	state.state = INITIAL;
	state.start = state.crs = src;
	state.end = src + srclen;
	state.ptr = NULL;
	state.target = NULL;
	state.targetsize = 0;
	state.data = data;

	url_scanner(&state);

	if(newlen) *newlen = state.targetsize;

	return state.target;
}
