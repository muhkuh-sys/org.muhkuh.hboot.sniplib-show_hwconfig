/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009 by Hilscher GmbH           *
 *                                                                         *
 *   Author: Christoph Thelen (cthelen@hilscher.com)                       *
 *                                                                         *
 *   Redistribution or unauthorized use without expressed written          *
 *   agreement from the Hilscher GmbH is forbidden.                        *
 ***************************************************************************/


#include "console_io.h"

#include <stddef.h>
#include <string.h>

#include "serial_vectors.h"
#include "uprintf.h"


#define CONSOLE_MAX_HISTORY_LINES 8
#define CONSOLE_HISTORY_BUFFER_SIZE 512


typedef enum CONSOLE_READ_STATE_ENUM
{
	CONSOLE_READ_STATE_ASCII                = 0,
	CONSOLE_READ_STATE_ESCAPE_RECEIVE       = 1,
	CONSOLE_READ_STATE_ESCAPE_PLAYBACK      = 2
} CONSOLE_READ_STATE_T;


typedef struct CONSOLE_ESC_SEQ_STRUCT
{
	unsigned long ulSeq;
	CONSOLE_KEYS_T tKey;
} CONSOLE_ESC_SEQ_T;


#define KEY(a,b,c,d) (a|(b<<8U)|(c<<16U)|(d<<24U))

static const CONSOLE_ESC_SEQ_T atEscSeqs[] =
{
	{ KEY(0x41U,0x5BU,0xFFU,0xFFU), CONSOLE_CURSOR_UP },
	{ KEY(0x42U,0x5BU,0xFFU,0xFFU), CONSOLE_CURSOR_DOWN },
	{ KEY(0x44U,0x5BU,0xFFU,0xFFU), CONSOLE_CURSOR_LEFT },
	{ KEY(0x43U,0x5BU,0xFFU,0xFFU), CONSOLE_CURSOR_RIGHT },
	{ KEY(0x48U,0x5BU,0xFFU,0xFFU), CONSOLE_HOME },
	{ KEY(0x46U,0x5BU,0xFFU,0xFFU), CONSOLE_END },
	{ KEY(0x7EU,0x33U,0x5BU,0xFFU), CONSOLE_DELETE },
	{ KEY(0x7EU,0x35U,0x5BU,0xFFU), CONSOLE_PAGE_UP },
	{ KEY(0x7EU,0x36U,0x5BU,0xFFU), CONSOLE_PAGE_DOWN },
	{ KEY(0x7EU,0x32U,0x5BU,0xFFU), CONSOLE_INSERT },
	{ KEY(0x50U,0x4FU,0xFFU,0xFFU), CONSOLE_F1 },
	{ KEY(0x51U,0x4FU,0xFFU,0xFFU), CONSOLE_F2 },
	{ KEY(0x52U,0x4FU,0xFFU,0xFFU), CONSOLE_F3 },
	{ KEY(0x53U,0x4FU,0xFFU,0xFFU), CONSOLE_F4 },
	{ KEY(0x7EU,0x35U,0x31U,0x5BU), CONSOLE_F5 },
	{ KEY(0x7EU,0x37U,0x31U,0x5BU), CONSOLE_F6 },
	{ KEY(0x7EU,0x38U,0x31U,0x5BU), CONSOLE_F7 },
	{ KEY(0x7EU,0x39U,0x31U,0x5BU), CONSOLE_F8 },
	{ KEY(0x7EU,0x30U,0x32U,0x5BU), CONSOLE_F9 },
	{ KEY(0x7EU,0x31U,0x32U,0x5BU), CONSOLE_F10 },
	{ KEY(0x7EU,0x33U,0x32U,0x5BU), CONSOLE_F11 },
	{ KEY(0x7EU,0x34U,0x32U,0x5BU), CONSOLE_F12 }
};


static CONSOLE_READ_STATE_T tConsoleReadState;
static unsigned long ulEscapeSequence;
static int fModeIsOverwrite;
static unsigned int s_uiLastChar;

static char acLine[CONSOLE_MAX_LINE_LENGTH];

static unsigned int uiCurrentHistoryLine;
static unsigned int auiHistory_LineSize[CONSOLE_MAX_HISTORY_LINES];
static char *apcHistory_Line[CONSOLE_MAX_HISTORY_LINES];
static char acHistory_Lines[CONSOLE_HISTORY_BUFFER_SIZE];

static unsigned int s_uiHistoryLines;

/*-------------------------------------*/

static const CONSOLE_IO_OPTIONS_T s_tConsoleIoOptions =
{
	.uc_console_line_size = 80U,
	.uc_console_linefeed_mode = CONSOLE_LINEFEED_CRLF
};



void console_io_init(void)
{
	unsigned int uiHistoryLines;
	char *pcHistoryLineCnt;
	char *pcHistoryLineEnd;
	unsigned int uiLineSize;


	/* No escape character received yet. */
	tConsoleReadState = CONSOLE_READ_STATE_ASCII;

	/* The mode is "insert" by default. */
	fModeIsOverwrite = 0;

	/* Initialize the last char to none. */
	s_uiLastChar = 0;

	/* Initialize the current history line number. */
	uiCurrentHistoryLine = 0;

	/* Clear all history lines. */
	memset(auiHistory_LineSize, 0, sizeof(auiHistory_LineSize));
	memset(acHistory_Lines, 0, sizeof(acHistory_Lines));

	/* Initialize the history lines. */
	uiHistoryLines = 0;
	pcHistoryLineCnt = acHistory_Lines;
	pcHistoryLineEnd = acHistory_Lines + sizeof(acHistory_Lines);
	uiLineSize = s_tConsoleIoOptions.uc_console_line_size;
	do
	{
		apcHistory_Line[uiHistoryLines] = pcHistoryLineCnt;
		++uiHistoryLines;
		pcHistoryLineCnt += uiLineSize;
		if( pcHistoryLineCnt>=pcHistoryLineEnd )
		{
			break;
		}
	} while( uiHistoryLines<CONSOLE_MAX_HISTORY_LINES );
	s_uiHistoryLines = uiHistoryLines;
}



/*-------------------------------------*/


static int console_get_key_unescaped(void)
{
	int iKey;
	unsigned int uiChar;
	const CONSOLE_ESC_SEQ_T *ptCnt;
	const CONSOLE_ESC_SEQ_T *ptEnd;


	/* the default is no key pressed */
	iKey = CONSOLE_NONE;

	/* is a character waiting? */
	if( tSerialVectors.fn.fnPeek()!=0 )
	{
		/* get a character */
		uiChar = tSerialVectors.fn.fnGet();

		switch( tConsoleReadState )
		{
		default:
		case CONSOLE_READ_STATE_ASCII:
			/* received escape? */
			if( uiChar==CONSOLE_ESC )
			{
				/* yes -> switch to escape receive mode */
				tConsoleReadState = CONSOLE_READ_STATE_ESCAPE_RECEIVE;
				ulEscapeSequence = 0xFFFFFFFFU;
			}
			else
			{
				if( s_uiLastChar==0x0dU && uiChar==0x0aU )
				{
					/* ignore any LF after CR */
					iKey = CONSOLE_NONE;
				}
				else if( uiChar==0x0aU )
				{
					/* treat all stray LF as enter */
					iKey = CONSOLE_ENTER;
				}
				else if( uiChar==0x0dU )
				{
					/* CR is enter */
					iKey = CONSOLE_ENTER;
				}
				else
				{
					/* take this as a normal ASCII key */
					iKey = (int)uiChar;
				}
				s_uiLastChar = uiChar;
			}
			break;


		case CONSOLE_READ_STATE_ESCAPE_RECEIVE:
			/* shift in the new char */
			ulEscapeSequence <<= 8U;
			ulEscapeSequence  |= uiChar;

			/* loop over all known sequences */
			ptCnt = atEscSeqs;
			ptEnd = ptCnt + (sizeof(atEscSeqs)/sizeof(atEscSeqs[0]));
			while( ptCnt<ptEnd )
			{
				/* compare the escape sequences */
				if( ulEscapeSequence==ptCnt->ulSeq )
				{
					/* found the sequence */

					/* replace the sequence with the special character */
					iKey = ptCnt->tKey;

					/* leave the escape receive state */
					tConsoleReadState = CONSOLE_READ_STATE_ASCII;
					break;
				}

				/* no match -> try next list element */
				++ptCnt;
			}

			/* if this point is reached, no sequence matched */

			/* is it possible to shift one more char into the sequence buffer? */
			if( (ulEscapeSequence>>24U)!=0xFFU )
			{
				/* no, the sequence is full, move on to the playback state */
				tConsoleReadState = CONSOLE_READ_STATE_ESCAPE_PLAYBACK;

				/* all sequences start with the escape char */
				iKey = CONSOLE_ESC;
			}
			break;


		case CONSOLE_READ_STATE_ESCAPE_PLAYBACK:
			/* shift out one more char from the buffer */
			iKey = (int)(ulEscapeSequence>>24U);

			/* move next char up in the escape buffer */
			ulEscapeSequence <<= 8U;
			ulEscapeSequence  |= 0xFFU;

			/* was this the last char in the buffer? */
			if( ulEscapeSequence==0xFFFFFFFFU )
			{
				/* yes, the buffer is empty now -> move back to ASCII mode */
				tConsoleReadState = CONSOLE_READ_STATE_ASCII;
			}
			break;
		}
	}

	return iKey;
}


static unsigned long console_update_line(unsigned int uiCursorPos, unsigned int uiOldLength, unsigned int uiNewLength, const char *pcNewLine)
{
	unsigned int uiPos;


	/* is the new line longer? */
	if( uiOldLength<=uiNewLength )
	{
		uiPos = uiCursorPos;

		/* print chars until new line end */
		while( uiPos<uiNewLength )
		{
			tSerialVectors.fn.fnPut(pcNewLine[uiPos]);
			++uiPos;
		}

		/* cursor to position 0 */
		while( uiPos>0 )
		{
			tSerialVectors.fn.fnPut('\b');
			--uiPos;
		}

		/* print chars until old cursor position */
		while( uiPos<uiCursorPos )
		{
			tSerialVectors.fn.fnPut(pcNewLine[uiPos]);
			++uiPos;
		}
	}
	else if( uiCursorPos<=uiNewLength )
	{
		uiPos = uiCursorPos;

		/* print chars until new line end */
		while( uiPos<uiNewLength )
		{
			tSerialVectors.fn.fnPut(pcNewLine[uiPos]);
			++uiPos;
		}

		/* clear the rest of the old line */
		while( uiPos<uiOldLength )
		{
			tSerialVectors.fn.fnPut(' ');
			++uiPos;
		}

		/* cursor to pos 0 */
		while( uiPos>0 )
		{
			tSerialVectors.fn.fnPut('\b');
			--uiPos;
		}

		/* print chars until old cursor position */
		while( uiPos<uiCursorPos )
		{
			tSerialVectors.fn.fnPut(pcNewLine[uiPos]);
			++uiPos;
		}
	}
	else
	{
		uiPos = uiCursorPos;

		/* cursor to end of new line */
		while( uiPos>uiNewLength )
		{
			tSerialVectors.fn.fnPut('\b');
			--uiPos;
		}

		/* clear the rest of the old line */
		while( uiPos<uiOldLength )
		{
			tSerialVectors.fn.fnPut(' ');
			++uiPos;
		}

		/* cursor to position 0 */
		while( uiPos>0 )
		{
			tSerialVectors.fn.fnPut('\b');
			--uiPos;
		}

		/* print chars until new line end */
		while( uiPos<uiNewLength )
		{
			tSerialVectors.fn.fnPut(pcNewLine[uiPos]);
			++uiPos;
		}

		uiCursorPos = uiPos;
	}
	tSerialVectors.fn.fnFlush();

	return uiCursorPos;
}


const char *console_io_read_line(unsigned int uiMaxLineSize)
{
	int iKey;
	int fLinefeedReceived;
	unsigned int uiCnt;
	char cZp;
	unsigned int uiLineSize;
	unsigned int uiOldLineSize;
	unsigned int uiCursorPos;
	unsigned int uiActiveHistoryLine;


	/* start a new line */
	uiLineSize = 0;
	uiCursorPos = 0;

	uiActiveHistoryLine = uiCurrentHistoryLine;

	fLinefeedReceived = 0;

	do
	{
		iKey = console_get_key_unescaped();
		switch( iKey )
		{
		case CONSOLE_ENTER:
			fLinefeedReceived = 1;
			break;

		case CONSOLE_NONE:
			/* nothing to do */
			break;

		case CONSOLE_CURSOR_UP:
			/* move one entry up in the history */
			uiCnt = uiActiveHistoryLine - 1;
			/* wrap to end of table */
			if( uiCnt>=s_uiHistoryLines )
			{
				uiCnt = s_uiHistoryLines-1;
			}
			/* is this not the starting line and does the line exist? */
			if( uiCnt!=uiCurrentHistoryLine && auiHistory_LineSize[uiCnt]!=0 )
			{
				/* yes, it exists -> display this one */

				/* write back the active history line */
				auiHistory_LineSize[uiActiveHistoryLine] = uiLineSize;
				memcpy(apcHistory_Line[uiActiveHistoryLine], acLine, uiLineSize);

				/* save the old line size for later */
				uiOldLineSize = uiLineSize;

				/* get the new history line */
				uiLineSize = auiHistory_LineSize[uiCnt];
				memcpy(acLine, apcHistory_Line[uiCnt], uiLineSize);

				uiCursorPos = console_update_line(uiCursorPos, uiOldLineSize, uiLineSize, acLine);

				/* accept the new line number */
				uiActiveHistoryLine = uiCnt;
			}
			break;

		case CONSOLE_CURSOR_DOWN:
			if( uiCurrentHistoryLine!=uiActiveHistoryLine )
			{
				/* move one entry down in the history */
				uiCnt = uiActiveHistoryLine + 1;
				/* wrap to start of table */
				if( uiCnt>=s_uiHistoryLines )
				{
					uiCnt = 0;
				}
				/* is this the current line or does the line exist? */
				if( uiCnt==uiCurrentHistoryLine || auiHistory_LineSize[uiCnt]!=0 )
				{
					/* yes, it exists -> display this one */

					/* write back the active history line */
					auiHistory_LineSize[uiActiveHistoryLine] = uiLineSize;
					memcpy(apcHistory_Line[uiActiveHistoryLine], acLine, uiLineSize);

					/* save the old line size for later */
					uiOldLineSize = uiLineSize;

					/* get the new history line */
					uiLineSize = auiHistory_LineSize[uiCnt];
					memcpy(acLine, apcHistory_Line[uiCnt], uiLineSize);

					uiCursorPos = console_update_line(uiCursorPos, uiOldLineSize, uiLineSize, acLine);

					/* accept the new line number */
					uiActiveHistoryLine = uiCnt;
				}
			}
			break;

		case CONSOLE_CURSOR_LEFT:
			/* move cursor left */
			if( uiCursorPos>0 )
			{
				tSerialVectors.fn.fnPut('\b');
				tSerialVectors.fn.fnFlush();
				--uiCursorPos;
			}
			break;

		case CONSOLE_CURSOR_RIGHT:
			/* move cursor left */
			if( uiCursorPos<uiLineSize )
			{
				tSerialVectors.fn.fnPut(acLine[uiCursorPos]);
				tSerialVectors.fn.fnFlush();
				++uiCursorPos;
			}
			break;

		case CONSOLE_HOME:
			while( uiCursorPos>0 )
			{
				tSerialVectors.fn.fnPut('\b');
				--uiCursorPos;
			}
			tSerialVectors.fn.fnFlush();
			break;

		case CONSOLE_END:
			while( uiCursorPos<uiLineSize )
			{
				tSerialVectors.fn.fnPut(acLine[uiCursorPos]);
				++uiCursorPos;
			}
			tSerialVectors.fn.fnFlush();
			break;

		case CONSOLE_DELETE:
			if( uiCursorPos<uiLineSize )
			{
				uiCnt = uiCursorPos;
				while( uiCnt<uiLineSize-1 )
				{
					cZp = acLine[uiCnt+1];
					acLine[uiCnt] = cZp;
					tSerialVectors.fn.fnPut(cZp);
					++uiCnt;
				}
				tSerialVectors.fn.fnPut(' ');

				/* move cursor back */
				uiCnt = uiLineSize;
				while( uiCnt>uiCursorPos )
				{
					tSerialVectors.fn.fnPut('\b');
					--uiCnt;
				}

				--uiLineSize;
				tSerialVectors.fn.fnFlush();
			}
			break;

		case CONSOLE_BACKSPACE:
			if( uiCursorPos>0 )
			{
				tSerialVectors.fn.fnPut('\b');
				--uiCursorPos;
				uiCnt = uiCursorPos;
				while( uiCnt<uiLineSize-1 )
				{
					cZp = acLine[uiCnt+1];
					acLine[uiCnt] = cZp;
					tSerialVectors.fn.fnPut(cZp);
					++uiCnt;
				}
				tSerialVectors.fn.fnPut(' ');

				/* move cursor back */
				uiCnt = uiLineSize;
				while( uiCnt>uiCursorPos )
				{
					tSerialVectors.fn.fnPut('\b');
					--uiCnt;
				}

				--uiLineSize;
				tSerialVectors.fn.fnFlush();
			}
			break;

		case CONSOLE_PAGE_UP:
		case CONSOLE_PAGE_DOWN:
			break;

		case CONSOLE_INSERT:
			fModeIsOverwrite ^= 1;
			break;

		case CONSOLE_F1:
		case CONSOLE_F2:
		case CONSOLE_F3:
		case CONSOLE_F4:
		case CONSOLE_F5:
		case CONSOLE_F6:
		case CONSOLE_F7:
		case CONSOLE_F8:
		case CONSOLE_F9:
		case CONSOLE_F10:
		case CONSOLE_F11:
		case CONSOLE_F12:
			break;


		default:
			if( fModeIsOverwrite!=0 )
			{
				if( uiCursorPos<uiMaxLineSize )
				{
					/* this seems to be a normal character, just add it to the buffer */
					acLine[uiCursorPos] = (unsigned char)iKey;

					/* increase line size if the cursor is at the end of the line */
					if( uiCursorPos==uiLineSize )
					{
						++uiLineSize;
					}

					/* move cursor right */
					++uiCursorPos;
					/* print the char */
					tSerialVectors.fn.fnPut((char)iKey);
					tSerialVectors.fn.fnFlush();
				}
			}
			else
			{
				if( uiLineSize<uiMaxLineSize )
				{
					/* are chars behind the cursor? */
					if( uiCursorPos<uiLineSize )
					{
						/* yes -> move them 1 position to the right */
						uiCnt = uiLineSize;
						while( uiCnt>uiCursorPos )
						{
							acLine[uiCnt] = acLine[uiCnt-1];
							--uiCnt;
						}
					}

					/* write the char to the buffer */
					acLine[uiCursorPos] = (unsigned char)iKey;
					++uiLineSize;

					/* redraw rest of the line */
					uiCnt = uiCursorPos;
					while( uiCnt<uiLineSize )
					{
						tSerialVectors.fn.fnPut(acLine[uiCnt]);
						++uiCnt;
					}

					/* move cursor one position to the right */
					++uiCursorPos;

					/* move cursor back */
					uiCnt = uiLineSize;
					while( uiCnt>uiCursorPos )
					{
						tSerialVectors.fn.fnPut('\b');
						--uiCnt;
					}
					tSerialVectors.fn.fnFlush();
				}
			}
			break;
		}
	} while( fLinefeedReceived==0 );

	/* only accept non-empty lines in the history buffer */
	if( uiLineSize>0 )
	{
		/* write this line to the current history entry */
		auiHistory_LineSize[uiCurrentHistoryLine] = uiLineSize;
		memcpy(apcHistory_Line[uiCurrentHistoryLine], acLine, uiLineSize);

		/* move to the next history line */
		++uiCurrentHistoryLine;
		if( uiCurrentHistoryLine>=s_uiHistoryLines )
		{
			uiCurrentHistoryLine = 0;
		}
	}

	/* NOTE: use uprintf instead of tSerialVectors.fn.fnPut here to get newline translation. */
	uprintf("\n");
	acLine[uiLineSize] = 0;

	return acLine;
}


const char *console_io_read_plain_line(unsigned int uiMaxLineSize)
{
	size_t sizPos;
	unsigned int uiChar;


	sizPos = 0;
	do
	{
		/* get a character */
		uiChar = tSerialVectors.fn.fnGet();
		/* NOTE: do not respect the linefeed settings here. This
		 * function has usecases where the input does not come from
		 * the plain terminal but a file (i.e. uuencoded data).
		 */
		if( uiChar!='\n' && uiChar!='\r' )
		{
			acLine[sizPos] = (char)uiChar;
			++sizPos;
		}
		else
		{
			break;
		}
	} while( sizPos<uiMaxLineSize );

	/* terminate the string */
	acLine[sizPos] = '\0';

	return acLine;
}
