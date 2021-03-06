/*
======================================================================
PROGRAM C Editor - An editor with top-down menus.
@author : Velorek
@version : 1.0
Last modified : 09/02/2019 - Down Scroll                                         
======================================================================*/

/*====================================================================*/
/* COMPILER DIRECTIVES AND INCLUDES                                   */
/*====================================================================*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "c_cool.h"
#include "list_choice.h"
#include "screen_buffer.h"
#include "opfile_dialog.h"
#include "user_inter.h"
#include "keyboard.h"

/*====================================================================*/
/* CONSTANT VALUES                                                    */
/*====================================================================*/

//GOODBYE MSG - ASCII ART
#define cedit_ascii_1 "  _____      ______    _ _ _   \n"
#define cedit_ascii_2 " / ____|    |  ____|  | (_) |  \n"
#define cedit_ascii_3 "| |   ______| |__   __| |_| |_ \n"
#define cedit_ascii_4 "| |  |______|  __| / _` | | __|\n"
#define cedit_ascii_5 "| |____     | |___| (_| | | |_ \n"
#define cedit_ascii_6 " \\_____|    |______\\__,_|_|\\__|\n"

//ABOUT - ASCII ART
#define ABOUT_ASC_1 "        _   __       \n"
#define ABOUT_ASC_2 "       /  _|__ _|.___\n"
#define ABOUT_ASC_3 "       \\_  |__(_]| |  v0.1 \n       Coded   by  V3l0rek\n "
#define ANIMATION "||//--\\\\"

//USER-DEFINED MESSAGES
#define UNKNOWN "UNTITLED"
#define STATUS_BAR_MSG1  " [C-Edit] | F2,CTRL+L: MENU | F1,ALT+H: HELP"
#define STATUS_BAR_MSG2 " [C-Edit] Press ESC to exit menu.             "
#define WLEAVE_MSG "\n       Are you sure\n    you want to quit?"
#define WSAVE_MSG ":\nFile saved successfully!"
#define WSAVELABEL_MSG "[-] File:"
#define WSAVETITLE_MSG "[C-EDIT SAVE AS]"
#define WNEWTITLE_MSG "[C-EDIT NEW FILE]"
#define WTITLEABOUT_MSG "[ABOUT]"
#define WABOUT_MSG "Coded by v3l0r3k.\n - 2019 - \n Spukhafte Fernwirkungen!"
#define WINFO_NOPEN "Error:\nThere isn't any file open!"
#define WINFO_SIZE "-File size: "
#define WINFO_SIZE2 "\n-No. of lines: "
#define WINFO_SIZE3 "\n-File name: "
#define WCHECKFILE_MSG " This file isn't a  \n text file. Program \n may crash. Open anyway?"
#define WINFONOTYET_MSG "Not implemented yet!"
#define WCHECKLINES_MSG " File longer than 32700 \n lines. You'll view those \n lines as read Mode! "
#define WMODIFIED_MSG " File has been modified\n Save current buffer?"
#define WFILEEXISTS_MSG " File exists. \n Overwrite?"

//MISC. CONSTANTS
#define EXIT_FLAG -1
#define FAILSAFE '$'		//Failsafe char to activate menus.
#define TAB_DISTANCE 8		//How many spaces TAB key will send.
#define START_CURSOR_X 2
#define START_CURSOR_Y 3
#define TIME_MS 3000
#define LIMIT_TIMER 10
#define ROWS_FAILSAFE 25
#define COLUMNS_FAILSAFE 80
#define K_LEFTMENU -1		//Left arrow key pressed while in menu
#define K_RIGHTMENU -2		//Right arrow key pressed while in menu
#define MAX_TEXT 150
#define MAX_PATH 1024

//MENU CONSTANTS
#define HOR_MENU -1
#define FILE_MENU 0
#define OPT_MENU 1
#define HELP_MENU 2
#define YESNO_MENU 3
#define OK_MENU 4
#define MAX_FILENAME 100

//DROP-DOWN MENUS
#define OPTION_1 0
#define OPTION_2 1
#define OPTION_3 2
#define OPTION_4 3
#define OPTION_5 4
#define OPTION_NIL -1		//Reset option
#define CONFIRMATION 1

// DISPLAY CONSTANTS
#define FILL_CHAR 32

//EDIT AREA CONSTANTS 
//Edit buffer is 32700 lines x 1022 chars

#define MAX_CHARS 1022		// 1022 chars per line
#define MAX_LINES 32700		// 32700 lines per page (max mem 32750)
#define CHAR_NIL '\0'
#define END_LINE_CHAR 0x0A	// $0A
#define VSCROLL_ON 1
#define VSCROLL_OFF 0
#define DIRECTION_DOWN 1
#define DIRECTION_UP 0
//FILE CONSTANTS
#define TMP_FILE "tmp.txt"
#define CONFIGFILE "cedit.cfg"
#define FILE_MODIFIED 1
#define FILE_UNMODIFIED 0
#define FILE_READMODE 2

/*====================================================================*/
/* TYPEDEF DEFINITIONS                                                */
/*====================================================================*/

typedef struct _charint {
  char    ch;
  char     specialChar;
} CHARINT;

typedef struct _editbuffer {
  CHARINT charBuf[MAX_LINES];
} EDITBUFFER;

typedef struct _editScroll {
  long totalLines; //Maximum no. of lines in File or Buffer
  long displayLength; //number of current display Lines
  long scrollLimit; //limit of scroll
  long scrollPointer; //last pointed/shown line
  long pagePointer; //last page displayed
  char scrollActive; //whether there are lines to scroll or not.
  long bufferX; //coordinates within the buffer
  long bufferY; //coordinates within the buffer
} EDITSCROLL;


/*====================================================================*/
/* GLOBAL VARIABLES */
/*====================================================================*/

LISTCHOICE *mylist, data;
SCREENCELL *my_screen;
SCROLLDATA openFileData;
EDITBUFFER editBuffer[MAX_LINES];
EDITSCROLL editScroll;

FILE   *filePtr;

int     rows = 0, columns = 0, old_rows = 0, old_columns = 0;	// Terminal dimensions
long    cursorX = START_CURSOR_X, cursorY = START_CURSOR_Y; //Cursor position
int     timerCursor = 0;	// Timer
char    kglobal = 0;		// Global variable for menu animation
char    currentFile[MAX_TEXT];
long    limitRow = 0, limitCol = 0;	// These variables account for the current limits of the buffer.
int     c_animation = 0;	//Counter for animation 
long    maxLineFile = 1;        //Last line of file
char    currentPath[MAX_PATH];
//FLAGS
int     exitp = 0;		// Exit flag for main loop
int     fileModified = FILE_UNMODIFIED;	//Have we modified the buffer?
char    timerOnOFF = 1;

/*====================================================================*/
/* PROTOTYPES OF FUNCTIONS                                            */
/*====================================================================*/

//Menu and display prototypes
void    credits();
int     main_screen();
int     refresh_editarea();
void    update_position();
void    drop_down(char *kglobal);
char    horizontal_menu();
void    filemenu();
void    optionsmenu();
void    helpmenu();
int     confirmation();
int     about_info();
int     help_info();
int     refresh_screen(int force_refresh);
int     refresh_line(long line);
void    cleanStatusBar();
void    smoothScroll(char direction);

//Key handling prototypes
int     special_keys(long *whereX, long *whereY, char ch);
//int special_keys(int *whereX, int *whereY,char ch, char chartrail[5], int *counter);
void    draw_cursor(long *whereX, long *whereY, int *timer);
int     timer_1(int *timer1, char onOff);
//Edit prototypes
void    cleanBuffer(EDITBUFFER editBuffer[MAX_LINES]);
int     writetoBuffer(EDITBUFFER editBuffer[MAX_LINES], long whereX,
		      long whereY, char ch);
int     process_input(EDITBUFFER editBuffer[MAX_LINES], long *whereX,
		      long *whereY, char ch);
int     findEndline(EDITBUFFER editBuffer[MAX_LINES], long line);
short   checkScrollValues(long lines);

//File-handling prototypes
int     openFile(FILE ** filePtr, char fileName[], char *mode);
int     closeFile(FILE * filePtr);
int     writeBuffertoFile(FILE * filePtr,
			  EDITBUFFER editBuffer[MAX_LINES]);
int     newDialog(char fileName[MAX_TEXT]);
int     saveDialog(char fileName[MAX_TEXT]);
int     saveasDialog(char fileName[MAX_TEXT]);
int     openFileHandler();
int     fileInfoDialog();
int     writeBuffertoDisplay(EDITBUFFER editBuffer[MAX_LINES]);
int     filetoBuffer(FILE * filePtr, EDITBUFFER editBuffer[MAX_LINES]);
long    getfileSize(FILE * filePtr);
long    countLinesFile(FILE * filePtr);
long    checkFile(FILE * filePtr);
int     file_exists(char *fileName);
int     handleopenFile(FILE ** filePtr, char *fileName, char *oldFileName);
int     createnewFile(FILE ** filePtr, char *fileName, int checkFile);
void    checkConfigFile(int setValue);

/*====================================================================*/
/* MAIN PROGRAM - CODE                                                */
/*====================================================================*/

int main(int argc, char *argv[]) {
  char    ch = 0, oldchar = 0;
  int     esc_key = 0;		//To control key input and scan for keycodes.
  int     keypressed = 0;
  int     timer1 = 0;		// Timer to display animation

  /*------------------------INITIAL VALUES----------------------------*/
  hidecursor();
  pushTerm();			//Save current terminal settings in failsafe
  create_screen();		//Create screen buffer to control display
  cleanBuffer(editBuffer);
  clearString(currentFile, MAX_TEXT);
  strcpy(currentFile, UNKNOWN);
  checkConfigFile(-1);		//Check config file for colorScheme. -1 -> first time
  getcwd(currentPath, sizeof(currentPath));	//Save current path
  editScroll.scrollActive = 0; //Scroll is inactive.
  editScroll.totalLines = 1; //There is just one line active in buffer
  editScroll.bufferX = 1;
  editScroll.bufferY = 1;
  main_screen();		//Draw screen
  resetch();			//Clear keyboard and sets ENTER = 13
  /*------------------------------------------------------------------*/
  
  
  /*------------------------CHECK ARGUMENTS----------------------------*/
  if(argc > 1) {
    //Does the file exist? Open or create?
    if(file_exists(argv[1]) == 1) {
      //open
      clearString(currentFile, MAX_TEXT);
      strcpy(currentFile, argv[1]);
      handleopenFile(&filePtr, currentFile, UNKNOWN);
    } else {
      //create
      clearString(currentFile, MAX_TEXT);
      strcpy(currentFile, argv[1]);
      createnewFile(&filePtr, currentFile, 0);
    }
  }
  /*------------------------------------------------------------------*/
  
  /*------------------------MAIN PROGRAM LOOP-------------------------*/
  do {
    /* CURSOR */
    draw_cursor(&cursorX, &cursorY, &timerCursor);
    /* Query if screen dimensions have changed and resize screen */
    refresh_screen(0);
    /* Wait for key_pressed to read key */
    keypressed = kbhit();
    /* Timer for animation to show system time and clean cursor */
    timer_1(&timer1, timerOnOFF);

    //update_screen();
    if(keypressed == 1) {

      /* Process SPECIAL KEYS and other ESC-related issues */
      esc_key = special_keys(&cursorX, &cursorY, ch);

      //If arrow keys are used repeatedly. This avoids printing unwanted chars.   
      if(oldchar == K_ESCAPE)
	esc_key = 1;

      //if (ch != 27 && oldchar == 7) esc_key = 0;

      oldchar = ch;

      ch = readch();

      /* EDIT */
      if(esc_key == 0) {
	//Process input and get rid of extra characters
	process_input(editBuffer, &cursorX, &cursorY, ch);	//Edit
	keypressed = 0;
      }
    } else {
      //Keypressed = 0 - Reset values
      esc_key = 0;
      ch = 0;
      oldchar = 0;
    }
  } while(exitp != EXIT_FLAG);	//exit flag for the whole program
  /*------------------------------------------------------------------*/
  
  //CREDITS
  if(filePtr != NULL) {
    closeFile(filePtr);
  }
  credits();
  return 0;
}

/*====================================================================*/
/* FUNCTION DEFINITIONS                                               */
/*====================================================================*/

/*----------------------------------------- */
/* Update cursor position display on screen */
/*----------------------------------------- */

void update_position() {
//Update cursor position display on screen
  write_str(columns - 20, rows, "| L:        C:     ", STATUSBAR, STATUSMSG);
  write_num(columns - 6, rows, editScroll.bufferX, 3, STATUSBAR, STATUSMSG);
  write_num(columns - 16, rows, editScroll.bufferY, 4, STATUSBAR, STATUSMSG);
}

/*--------------*/
/* EDIT Section */
/*------------- */

int findEndline(EDITBUFFER editBuffer[MAX_LINES], long line) {
  char    ch = 0;
  long     i = 0;
  int     result = 0;

  do {
    ch = editBuffer[line].charBuf[i].ch;
    i++;
    if(ch == CHAR_NIL || ch == END_LINE_CHAR)
      break;
  } while(i < MAX_CHARS);
  result = i;
  ch = 0;
  return result;
}
int process_input(EDITBUFFER editBuffer[MAX_LINES], long *whereX,
		  long *whereY, char ch) {
/* EDIT FUNCTIONS */
  char    accentchar[2];
  long     counter = 0;
  long     newPosition = 0;
  long     oldPosition = 0;
  long     positionX = 0;
  long     positionY = 0;
  int     i = 0;
  if(ch != K_ESCAPE) {

    //Calculate position values 
    limitCol = findEndline(editBuffer, editScroll.bufferY-1);
    //positionX = *whereX - START_CURSOR_X;	//Buffer position (x,y)
    
    //if (editScroll.scrollActive == 0)
     //positionY = *whereY - START_CURSOR_Y;
    //else
     positionX = editScroll.bufferX-1;
     positionY = editScroll.bufferY-1; 
    accentchar[0] = 0;
    accentchar[1] = 0;
    /* ---------------------------------------- */
    /* 
       READ CHARS WITH AND WITHOUT ACCENTS.
       Accent value is stored in the
       specialChar field of every item in the 
       buffer.
       Example:
       á : specialChar  = -61
       Char = -95
       a : specialChar = 0
       Char = 97                
     */

    /* ---------------------------------------- */
    //Check whether we are on Readmode
   if (fileModified != FILE_READMODE) {
    if((ch > 31 && ch < 127) || ch < 0) {
      //if a char has been read.
      read_accent(&ch, accentchar);

      //ADD SPACES IF CURSOR IS NOT AT THE END OF THE LINE
      if(positionX > limitCol) {
	for(i = limitCol - 1; i <= positionX; i++) {
	  writetoBuffer(editBuffer, i, positionY, ' ');
	  write_ch(i + START_CURSOR_X, *whereY, ' ', EDITAREACOL,
		   EDIT_FORECOLOR);
	}
      }
      //INSERT CHARS AT THE END OF LINE
      if(*whereX < MAX_CHARS && positionX >= limitCol - 1) {
	if(accentchar[0] != 0) {
	  //Special char ? print the two values to screen buffer.
	  write_ch(*whereX, *whereY, accentchar[0], EDITAREACOL,
		   EDIT_FORECOLOR);
	  write_ch(*whereX, *whereY, accentchar[1], EDITAREACOL,
		   EDIT_FORECOLOR);
	} else {
	  write_ch(*whereX, *whereY, accentchar[1], EDITAREACOL,
		   EDIT_FORECOLOR);
	}
	writetoBuffer(editBuffer, positionX, positionY, accentchar[0]);
	writetoBuffer(editBuffer, positionX, positionY, accentchar[1]);
	*whereX = *whereX + 1;
        editScroll.bufferX++;
      }
      //INSERT CHAR IN THE MIDDLE OF THE LINE
      if(*whereX < MAX_CHARS && positionX < limitCol - 1) {
	//move all chars one space to the side
	counter = 0;
	//move characters to the side
	while(counter <= (limitCol - positionX)) {
	  newPosition = limitCol - counter + 1;
	  oldPosition = limitCol - counter;
	  editBuffer[positionY].charBuf[newPosition].ch =
	      editBuffer[positionY].charBuf[oldPosition].ch;
	  editBuffer[positionY].charBuf[newPosition].specialChar =
	      editBuffer[positionY].charBuf[oldPosition].specialChar;
	  //Don't print null characters to string or 0x0A
	  if(editBuffer[positionY].charBuf[oldPosition].ch != CHAR_NIL ||
	     editBuffer[positionY].charBuf[oldPosition].ch !=
	     END_LINE_CHAR) {
	    if(accentchar[0] != 0
	       || editBuffer[positionY].charBuf[oldPosition].specialChar !=
	       0) {
	      //Special char ? print the two values to screen buffer.
	      write_ch(newPosition + START_CURSOR_X, *whereY,
		       editBuffer[positionY].charBuf[newPosition].
		       specialChar, EDITAREACOL, EDIT_FORECOLOR);
	      write_ch(newPosition + START_CURSOR_X, *whereY,
		       editBuffer[positionY].charBuf[newPosition].ch,
		       EDITAREACOL, EDIT_FORECOLOR);
	    } else {
	      write_ch(newPosition + START_CURSOR_X, *whereY,
		       editBuffer[positionY].charBuf[newPosition].ch,
		       EDITAREACOL, EDIT_FORECOLOR);
	    }
	  }
	  counter++;
	}
	//insert new SPECIAL char
	if(accentchar[0] != 0) {
	  //Special char ? print the two values to screen buffer.
	  write_ch(*whereX, *whereY, accentchar[0], EDITAREACOL,
		   EDIT_FORECOLOR);
	  write_ch(*whereX, *whereY, accentchar[1], EDITAREACOL,
		   EDIT_FORECOLOR);
	} else {
	  write_ch(*whereX, *whereY, accentchar[1], EDITAREACOL,
		   EDIT_FORECOLOR);
	}
	writetoBuffer(editBuffer, positionX, positionY, accentchar[0]);
	writetoBuffer(editBuffer, positionX, positionY, accentchar[1]);

	*whereX = *whereX + 1;
        editScroll.bufferX++;
      }
      update_screen();
      fileModified = FILE_MODIFIED;
    }

    if(ch == K_ENTER) {
      //RETURN - ENTER
      if(*whereY < rows - 2) {
	writetoBuffer(editBuffer, positionX, positionY, END_LINE_CHAR);
	*whereY = *whereY + 1;
	*whereX = START_CURSOR_X;
        editScroll.bufferX = 1;
        editScroll.bufferY++;
        editScroll.totalLines = editScroll.totalLines + 1;
	fileModified = FILE_MODIFIED;
      }
    }

    if(ch == K_BACKSPACE) {
      //BACKSPACE key
      write_ch(*whereX, *whereY, ' ', EDITAREACOL, EDITAREACOL);
      editBuffer[positionY].charBuf[positionX - 1].ch = CHAR_NIL;
      editBuffer[positionY].charBuf[positionX - 1].specialChar = CHAR_NIL;
      //Remove line if we continue pressing backspace
      if(*whereX == START_CURSOR_X && *whereY > START_CURSOR_Y) {
	*whereY = *whereY - 1;
	*whereX =
	    findEndline(editBuffer,
			*whereY - START_CURSOR_Y) + START_CURSOR_X;
        editScroll.bufferY--;
        editScroll.bufferX = findEndline(editBuffer,editScroll.bufferY);
      }
      if(*whereX > START_CURSOR_X) *whereX = *whereX - 1;
      editScroll.bufferX--;
    }

    if(ch == K_TAB) {
      //TAB key
      if(*whereX < columns - TAB_DISTANCE) *whereX = *whereX + TAB_DISTANCE;
      editScroll.bufferX = editScroll.bufferX + TAB_DISTANCE;
    }
    //*ch=0;
  } 
    if(ch == K_CTRL_L) {
      //Akin to F2
      //refresh_screen(-1); //update screen
      if(horizontal_menu() == K_ESCAPE) {
	//Exit horizontal menu with ESC 3x
	kglobal = K_ESCAPE;
	main_screen();
      }
      /*  Drop-down menu loop */
      drop_down(&kglobal);	//animation
    }
    if(ch == K_CTRL_C) {
      //Ask for confirmations CTRL-C -> Exit
      exitp = confirmation();	//Shall we exit? Global variable! 
    }
    if(ch == K_CTRL_H) {
      //Change color Scheme
      setColorScheme(colorsWindow(mylist));
      refresh_screen(1);
    }
  }
  return 0;
}

/*-----------------------------------------*/
/* Timer 1 Animation. Clock and cursor     */
/*-----------------------------------------*/

int timer_1(int *timer1, char onOff) {
/* Timer for animations - Display time and clean cursor */
  time_t  mytime = time(NULL);
  char   *time_str = ctime(&mytime);
  char    temp[4];

  temp[0] = '[';
  temp[1] = ANIMATION[c_animation];
  temp[2] = ']';
  temp[3] = '\0';
  if (onOff == 1){
  if(*timer1 == TIME_MS) {
    *timer1 = 0;
    time_str[strlen(time_str) - 1] = '\0';
    //display system time
    write_str(columns - strlen(time_str), 1, time_str, MENU_PANEL,
	      MENU_FOREGROUND0);
    write_str(columns - strlen(time_str) - 5, 1, temp, MENU_PANEL,
	      MENU_FOREGROUND0);
    update_position();		//update position display
    update_screen();		//update screen - main routine in timer
    c_animation++;
    if(c_animation > 6)
      c_animation = 0;
    return 0;
  } else {
    *timer1 = *timer1 + 1;
  }
  }
  return 1;
}

/*-----------------------------------------*/
/* Manage keys that send a ESC sequence    */
/*-----------------------------------------*/

int special_keys(long *whereX, long *whereY, char ch) {
/* MANAGE SPECIAL KEYS */
/* 
   New implementation: Trail of chars found in keyboard.c
   If K_ESCAPE is captured read a trail up to 5 characters from the console.
   This is to control the fact that some keys may change
   according to the terminal and expand the editor's possibilities.
   Eg: F2 can be either 27 79 81 or 27 91 91 82.  
*/

  int     esc_key = 0;
  char    chartrail[5];
  long    oldX;
  if(ch == K_ESCAPE) {
    read_keytrail(chartrail);	//Read trail after ESC key

    //Check key trails for special keys.

    //FUNCTION KEYS : F1 - F4
    if(strcmp(chartrail, K_F2_TRAIL) == 0 ||
       strcmp(chartrail, K_F2_TRAIL2) == 0) {
      //update screen
      //refresh_screen(-1);
      if(horizontal_menu() == K_ESCAPE) {
	//Exit horizontal menu with ESC 3x
	kglobal = K_ESCAPE;
	main_screen();
      }
      //  Drop-down menu loop */       
      drop_down(&kglobal);	//animation  
    } else if(strcmp(chartrail, K_F3_TRAIL) == 0 ||
	      strcmp(chartrail, K_F3_TRAIL2) == 0) {
      refresh_screen(1);
    } else if(strcmp(chartrail, K_F1_TRAIL) == 0 ||
	      strcmp(chartrail, K_F1_TRAIL2) == 0) {
      help_info();
    } else if(strcmp(chartrail, K_F4_TRAIL) == 0 ||
	      strcmp(chartrail, K_F4_TRAIL2) == 0) {
      about_info();
      // ARROW KEYS  
    } else if(strcmp(chartrail, K_LEFT_TRAIL) == 0) {
      //Left-arrow key  
      if(*whereX > 2){
	*whereX = *whereX - 1;
        editScroll.bufferX--;
      }
    } else if(strcmp(chartrail, K_RIGHT_TRAIL) == 0) {
      //Right-arrow key  
      if(*whereX < columns - 1){
	*whereX = *whereX + 1;
        editScroll.bufferX++;
      }
    } else if(strcmp(chartrail, K_UP_TRAIL) == 0) {
      //Up-arrow key  
      if(*whereY > 3) {
	*whereY = *whereY - 1;
        editScroll.bufferY--;
      }
      if (editScroll.scrollActive == 1 && *whereY == START_CURSOR_Y ) {
         if (editScroll.scrollPointer > 0){
            //editScroll.pagePointer = editScroll.scrollPointer + j;
            editScroll.scrollPointer = editScroll.scrollPointer - 1;
            editScroll.bufferY--;
            oldX = *whereX;
            smoothScroll(DIRECTION_UP);
            *whereY = START_CURSOR_Y;
            *whereX = oldX;
          }
      }
     
    } else if(strcmp(chartrail, K_DOWN_TRAIL) == 0) {
      //Down-arrow key  
      if(*whereY < rows - 2 && editScroll.totalLines > *whereY - START_CURSOR_Y+1) {
	*whereY = *whereY + 1;
        editScroll.bufferY++;
      }
      if (editScroll.scrollActive == 1 && *whereY == rows-2 ) {
          if (editScroll.scrollPointer < editScroll.scrollLimit){
            //editScroll.pagePointer = editScroll.scrollPointer + j;
            editScroll.scrollPointer = editScroll.scrollPointer + 1;
            editScroll.bufferY++;
            oldX = *whereX;
            smoothScroll(DIRECTION_DOWN);
            //refresh_editarea();
            *whereY = rows-2;
            *whereX = oldX;
          }
      }
    } else if(strcmp(chartrail, K_DELETE) == 0) {
      //delete button;
    } else if(strcmp(chartrail, K_ALT_H) == 0) {
      help_info();
    } else if(strcmp(chartrail, K_ALT_O) == 0) {
      openFileHandler();	//Open file Dialog
    } else if(strcmp(chartrail, K_ALT_N) == 0) {
      newDialog(currentFile);	// New file
      refresh_screen(-1);
    } else if(strcmp(chartrail, K_ALT_A) == 0) {
      saveasDialog(currentFile);	//Save as.. file
      refresh_screen(-1);
    } else if(strcmp(chartrail, K_ALT_D) == 0) {
      fileInfoDialog();		//Info file
    } else if(strcmp(chartrail, K_ALT_W) == 0) {
      if(strcmp(currentFile, UNKNOWN) == 0)
	saveasDialog(currentFile);	//Write to file
      else {
	saveDialog(currentFile);
      }
      refresh_screen(-1);
    } else if(strcmp(chartrail, K_ALT_X) == 0) {
      if(fileModified == 1)
	exitp = confirmation();	//Shall we exit? Global variable!
      else
	exitp = EXIT_FLAG;
    }
    esc_key = 1;
  } else {
    //Reset esc_key
    esc_key = 0;
  }
  return esc_key;
}

/*-----------------------------------------*/
/* Draw cursor on screen and animate it    */
/*-----------------------------------------*/

void draw_cursor(long *whereX, long *whereY, int *timer) {
/* CURSOR is drawn directly to screen and not to buffer */
  long     positionX = 0, limitCol = 0, positionY = 0;
  char    currentChar = FILL_CHAR;
  char    specialChar;

  *timer = *timer + 1;		//increase timer counter for animation

  //Calculate position
  limitCol = findEndline(editBuffer, *whereY - START_CURSOR_Y);
  //positionX = *whereX - START_CURSOR_X;	//Buffer position (x,y)
  //positionY = *whereY - START_CURSOR_Y;
  positionX = editScroll.bufferX-1;
  positionY = editScroll.bufferY-1;

  if(*timer < LIMIT_TIMER) {
    gotoxy(*whereX, *whereY);
    outputcolor(EDIT_FORECOLOR, EDITAREACOL);
    printf("|");
  } else {
    *timer = 0;			//reset timer
    //Is the cursor at the end or in the middle of text?
    if(positionX < limitCol) {
      currentChar = editBuffer[positionY].charBuf[positionX].ch;
    } else {
      currentChar = FILL_CHAR;
    }

    //Display character in yellow.
    gotoxy(*whereX, *whereY);
    //outputcolor(FH_YELLOW, 1);
    outputcolor(F_YELLOW, EDITAREACOL);
    //Is it an accent or special char?
    if(currentChar < 0) {
      specialChar = editBuffer[positionY].charBuf[positionX].specialChar;
      printf("%c%c", specialChar,
	     editBuffer[positionY].charBuf[positionX].ch);
    } else {
      printf("%c", currentChar);
    }
    resetAnsi(0);
  }
}

/*-----------------*/
/* Refresh screen  */
/*-----------------*/

int refresh_screen(int force) {
/* Query terminal dimensions again and check if resize 
   has been produced */
  get_terminal_dimensions(&rows, &columns);
  editScroll.displayLength = rows - 4; //update Scroll 
  if(rows != old_rows || columns != old_columns || force == 1
     || force == -1) {
    if(force != -1) {
      free_buffer();		//delete structure from memory for resize
      create_screen();		//create new structure 
      main_screen();		//Refresh screen in case of resize
    } else
      refresh_editarea();	//only update edit area to avoid flickering effect
    return 1;
  } else {
    return 0;
  }
}

/*-------------------*/
/* Draw main screen  */
/*-------------------*/

int main_screen() {
  int     i;

  //strcpy(msg, currentFile);
  //Save previous values  
  get_terminal_dimensions(&rows, &columns);
  old_rows = rows;
  old_columns = columns;
  cursorX = START_CURSOR_X;
  cursorY = START_CURSOR_Y;
  draw_cursor(&cursorX, &cursorY, &timerCursor);
  //Failsafe just in case it can't find the terminal dimensions
  if(rows == 0)
    rows = ROWS_FAILSAFE;
  if(columns == 0)
    columns = COLUMNS_FAILSAFE;
  screen_color(EDITAREACOL);
  //Draw upper and lower bars
  for(i = 1; i <= columns; i++) {
    write_ch(i, 1, FILL_CHAR, MENU_PANEL, MENU_PANEL);
  }

  for(i = 1; i < columns; i++) {
    write_ch(i, rows, FILL_CHAR, STATUSBAR, STATUSMSG);
  }
  // Text messages
  write_str(1, 1, "File  Options  Help", MENU_PANEL, MENU_FOREGROUND0);
  write_str(1, rows, STATUS_BAR_MSG1, STATUSBAR, STATUSMSG);

  /* Frames */
  //window appearance and scroll bar
  for(i = 2; i < rows; i++) {
    write_ch(columns, i, ' ', SCROLLBAR_BACK, SCROLLBAR_FORE);	//Scroll bar
    write_ch(1, i, NVER_LINE, EDITWINDOW_BACK, EDITWINDOW_FORE);	//upper vertical line box-like char 
  }
  for(i = 2; i < columns; i++) {
    write_ch(i, 2, NHOR_LINE, EDITWINDOW_BACK, EDITWINDOW_FORE);	//horizontal line box-like char
    write_ch(i, rows - 1, ' ', EDITWINDOW_BACK, EDITWINDOW_FORE);
  }
  write_ch(1, 2, NUPPER_LEFT_CORNER, EDITWINDOW_BACK, EDITWINDOW_FORE);	//upper-left box-like char
  //horizontal scroll bar
  for(i = 2; i < columns; i++) {
    write_ch(i, rows - 1, FILL_CHAR, SCROLLBAR_BACK, SCROLLBAR_FORE);
  }
  //Window-appearance
  write_ch(columns, 2, NUPPER_RIGHT_CORNER, EDITWINDOW_BACK, EDITWINDOW_FORE);	//right window corner
  write_ch(columns, rows - 1, NLOWER_RIGHT_CORNER, EDITWINDOW_BACK,
	   EDITWINDOW_FORE);
  write_ch(1, rows - 1, NLOWER_LEFT_CORNER, EDITWINDOW_BACK,
	   EDITWINDOW_FORE);

  //Center and diplay file name
  write_str((columns / 2) - (strlen(currentFile) / 2), 2, currentFile,
	    MENU_PANEL, MENU_FOREGROUND0);
  //Scroll symbols
  write_ch(columns, 3, '^', SCROLLBAR_ARR, SCROLLBAR_FORE);
  write_ch(columns, rows - 2, 'v', SCROLLBAR_ARR, SCROLLBAR_FORE);
  write_ch(columns, 4, '*', SCROLLBAR_SEL, SCROLLBAR_FORE);
  write_ch(3, rows - 1, '*', SCROLLBAR_SEL, SCROLLBAR_FORE);
  write_ch(2, rows - 1, '<', SCROLLBAR_ARR, SCROLLBAR_FORE);
  write_ch(columns - 1, rows - 1, '>', SCROLLBAR_ARR, SCROLLBAR_FORE);

//  update_screen();
  checkScrollValues(editScroll.totalLines); //make calculations for scroll.
 //Write editBuffer
  writeBuffertoDisplay(editBuffer);

  return 0;
}

/*----------------------------*/
/* Only refresh current line  */
/*----------------------------*/

int refresh_line(long line) {
  long     i;
  get_terminal_dimensions(&rows, &columns);
  old_rows = rows;
  old_columns = columns;
  cursorX = START_CURSOR_X;
  cursorY = START_CURSOR_Y;
  draw_cursor(&cursorX, &cursorY, &timerCursor);

  //Failsafe just in case it can't find the terminal dimensions
  if(rows == 0)
    rows = ROWS_FAILSAFE;
  if(columns == 0)
    columns = COLUMNS_FAILSAFE;

  //Paint blue line
  for(i = START_CURSOR_X; i < columns - 1; i++)
    write_ch(i, line, FILL_CHAR, EDITAREACOL, EDITAREACOL);

  update_screen();
  return 0;
}

/*-------------------------*/
/* Only refresh edit Area  */
/*-------------------------*/

int refresh_editarea() {
  int     i, j;
  get_terminal_dimensions(&rows, &columns);
  old_rows = rows;
  old_columns = columns;
  cursorX = START_CURSOR_X;
  cursorY = START_CURSOR_Y;
  draw_cursor(&cursorX, &cursorY, &timerCursor);
  //Failsafe just in case it can't find the terminal dimensions
  //timerOnOFF=0;
  if(rows == 0)
    rows = ROWS_FAILSAFE;
  if(columns == 0)
    columns = COLUMNS_FAILSAFE;

  //Paint blue edit area
  for(j = START_CURSOR_Y; j < rows - 1; j++)
    for(i = START_CURSOR_X; i < columns - 1; i++)
    {
      gotoxy(i,j);
      outputcolor(EDITAREACOL,EDITAREACOL);
      printf("%c",FILL_CHAR);
      flush_cell(i,j);
      //write_ch(i, j, FILL_CHAR, EDITAREACOL, EDITAREACOL);
     }
  for(i = 2; i < columns; i++) {
    write_ch(i, 2, NHOR_LINE, EDITWINDOW_BACK, EDITWINDOW_FORE);	//horizontal line box-like char
  }
  write_ch(1, 2, NUPPER_LEFT_CORNER, EDITWINDOW_BACK, EDITWINDOW_FORE);	//upper-left box-like char

  //Center and diplay file name
  write_str((columns / 2) - (strlen(currentFile) / 2), 2, currentFile,
	    MENU_PANEL, MENU_FOREGROUND0);

  update_position();
  //Write editBuffer
  writeBuffertoDisplay(editBuffer);
  return 0;
}
 
/*-------------------*/
/* Scroll Animation  */
/*-------------------*/

void smoothScroll(char direction) {
  int     i, j;
  long    lastLine,lastLine2;
 //Failsafe just in case it can't find the terminal dimensions
  //timerOnOFF=0;

//Paint blue edit area
  for(j = START_CURSOR_Y; j < rows - 1; j++){
    if (direction==DIRECTION_DOWN){
      lastLine = findEndline(editBuffer, editScroll.scrollPointer+j-1-START_CURSOR_Y);
      lastLine2 = findEndline(editBuffer, editScroll.scrollPointer+j-START_CURSOR_Y);    
    } 
    else{
      lastLine = findEndline(editBuffer, editScroll.scrollPointer+j+1-START_CURSOR_Y);
      lastLine2 = findEndline(editBuffer, editScroll.scrollPointer+j-START_CURSOR_Y);    
    }
    if (lastLine2<lastLine){
    for(i = START_CURSOR_X; i < lastLine+1; i++)
    {
       if (i<columns-2){
       flush_cell(i,j);
       //write_ch(i, j, FILL_CHAR, EDITAREACOL, EDITAREACOL);
      }
     }
    }
   }
  for(i = 2; i < columns; i++) {
    write_ch(i, 2, NHOR_LINE, EDITWINDOW_BACK, EDITWINDOW_FORE);	//horizontal line box-like char
  }
  write_ch(1, 2, NUPPER_LEFT_CORNER, EDITWINDOW_BACK, EDITWINDOW_FORE);	//upper-left box-like char

  //Center and diplay file name
  write_str((columns / 2) - (strlen(currentFile) / 2), 2, currentFile,
	    MENU_PANEL, MENU_FOREGROUND0);

  update_position();
  //Write editBuffer
  writeBuffertoDisplay(editBuffer);
}
  
/*-------------------*/
/* Clean Status Bar  */
/*-------------------*/

 
//Clean Status bar
void cleanStatusBar(){
  int i;
  for(i = 1; i < columns; i++) {
    write_ch(i, rows, FILL_CHAR, STATUSBAR, STATUSMSG);
  }
}
 
/*--------------------------*/
/* Display horizontal menu  */
/*--------------------------*/

char horizontal_menu() {
  char    temp_char;
  cleanStatusBar();
  write_str(1, rows, STATUS_BAR_MSG2, STATUSBAR, STATUSMSG);
  update_screen();
  loadmenus(mylist, HOR_MENU);
  temp_char = start_hmenu(&data);
  free_list(mylist);
  return temp_char;
}

/*-------------------------*/
/* Display File menu       */
/*-------------------------*/

void filemenu() {  
  cleanStatusBar();
  data.index = OPTION_NIL;
  write_str(1, rows, STATUS_BAR_MSG2, STATUSBAR, STATUSMSG);
  loadmenus(mylist, FILE_MENU);
  write_str(1, 1, "File", MENU_SELECTOR, MENU_FOREGROUND1);
  draw_window(1, 2, 13, 8, MENU_PANEL, MENU_FOREGROUND0, 1);
  kglobal = start_vmenu(&data);
  close_window();
  write_str(1, 1, "File  Options  Help", MENU_PANEL, MENU_FOREGROUND0);
  update_screen();
  free_list(mylist);

  if(data.index == OPTION_1) {
    //New file option
    newDialog(currentFile);
    //Update new global file name
    refresh_screen(-1);
  }
  if(data.index == OPTION_2) {
    //External Module - Open file dialog.
    openFileHandler();
  }
  if(data.index == OPTION_3) {
    //Save option
    //if (fileModified != FILE_READMODE) 
    if(strcmp(currentFile, UNKNOWN) == 0)
      saveasDialog(currentFile);
    else {
      saveDialog(currentFile);
    }
    //Update new global file name
    refresh_screen(-1);
  }
  if(data.index == OPTION_4) {
    //Save as option  
    saveasDialog(currentFile);
    refresh_screen(-1);
  }

  if(data.index == OPTION_5) {
    //Exit option
    if(fileModified == 1)
      exitp = confirmation();	//Shall we exit? Global variable!
    else
      exitp = EXIT_FLAG;
  }
  data.index = OPTION_NIL;
  cleanStatusBar();
   //Restore message in status bar
   write_str(1, rows, STATUS_BAR_MSG1, STATUSBAR, STATUSMSG);
}

/*--------------------------*/
/* Display Options menu     */
/*--------------------------*/

void optionsmenu() {
  int  setColor;
  data.index = OPTION_NIL;
  cleanStatusBar();
  write_str(1, rows, STATUS_BAR_MSG2, STATUSBAR, STATUSMSG);
  loadmenus(mylist, OPT_MENU);
  write_str(7, 1, "Options", MENU_SELECTOR, MENU_FOREGROUND1);
  draw_window(7, 2, 20, 6, MENU_PANEL, MENU_FOREGROUND0, 1);
  kglobal = start_vmenu(&data);
  close_window();
  write_str(1, 1, "File  Options  Help", MENU_PANEL, MENU_FOREGROUND0);
  update_screen();

  free_list(mylist);
  if(data.index == OPTION_1) {
    //File Info
    fileInfoDialog();
  }
  if(data.index == OPTION_3) {
    //Set Colors
    setColor = colorsWindow(mylist);
    setColorScheme(setColor);
    checkConfigFile(setColor);	//save new configuration in config file
    refresh_screen(1);
  }
  data.index = OPTION_NIL;
  //Restore message in status bar
  cleanStatusBar();
  write_str(1, rows, STATUS_BAR_MSG1, STATUSBAR, STATUSMSG);

}

/*--------------------------*/
/* Display Help menu        */
/*--------------------------*/

void helpmenu() { 
  cleanStatusBar();
  data.index = OPTION_NIL;
  write_str(1, rows, STATUS_BAR_MSG2, STATUSBAR, STATUSMSG);
  loadmenus(mylist, HELP_MENU);
  write_str(16, 1, "Help", MENU_SELECTOR, MENU_FOREGROUND1);
  draw_window(16, 2, 26, 5, MENU_PANEL, MENU_FOREGROUND0, 1);
  kglobal = start_vmenu(&data);
  close_window();
  write_str(1, 1, "File  Options  Help", MENU_PANEL, MENU_FOREGROUND0);
  update_screen();
  free_list(mylist);
  if(data.index == OPTION_1) {
    //About info
    help_info();
  }
  if(data.index == OPTION_2) {
    //About info
    about_info();
  }
  data.index = -1;
  //Restore message in status bar
  cleanStatusBar();
  write_str(1, rows, STATUS_BAR_MSG1, STATUSBAR, STATUSMSG);
}

/*-------------------------------*/
/* Display Confirmation Dialog   */
/*-------------------------------*/

/* Displays a window to asks user for confirmation */
int confirmation() {
  int     ok = 0;
  if(fileModified == 1) {
    ok = yesnoWindow(mylist, WMODIFIED_MSG);
    data.index = OPTION_NIL;
    //save file if modified?
    if(ok == 1) {
      if(strcmp(currentFile, UNKNOWN) == 0)
	saveasDialog(currentFile);
      else {
	saveDialog(currentFile);
      }
      ok = EXIT_FLAG;
    } else {
      ok = EXIT_FLAG;
    }
  } else {
    ok = -1;			//Exit without asking.
  }
  return ok;
}

/*--------------------------*/
/* Display About Dialog     */
/*--------------------------*/

int about_info() {
  int     ok = 0;
  char    msg[100];
  msg[0] = '\0';
  strcat(msg, ABOUT_ASC_1);
  strcat(msg, ABOUT_ASC_2);
  strcat(msg, ABOUT_ASC_3);
  strcat(msg, "\0");
  alertWindow(mylist, WTITLEABOUT_MSG, msg);
  return ok;
}

/*--------------------------*/
/* Display About Dialog     */
/*--------------------------*/

int help_info() {
  int     ok = 0;
  char    msg[500];
  msg[0] = '\0';
  strcat(msg, HELP0);		//located in user_inter.h
  strcat(msg, HELP1);		//located in user_inter.h
  strcat(msg, HELP2);		//located in user_inter.h
  strcat(msg, HELP3);		//located in user_inter.h
  strcat(msg, HELP4);		//located in user_inter.h
  strcat(msg, HELP5);		//located in user_inter.h
  strcat(msg, HELP6);		//located in user_inter.h
  strcat(msg, HELP7);		//located in user_inter.h
  strcat(msg, HELP8);		//located in user_inter.h
  strcat(msg, HELP9);		//located in user_inter.h
  strcat(msg, HELP10);		//located in user_inter.h
  strcat(msg, "\0");
  helpWindow(mylist, msg);
  return ok;
}

/*----------------------*/
/* Drop_Down Menu Loop  */
/*----------------------*/

void drop_down(char *kglobal) {
/* 
   Drop_down loop animation. 
   K_LEFTMENU/K_RIGHTMENU -1 is used when right/left arrow keys are used
   so as to break vertical menu and start the adjacent menu
   kglobal is changed by the menu functions.
*/

  do {
    if(*kglobal == K_ESCAPE) {
      //Exit drop-down menu with ESC           
      *kglobal = 0;
      break;
    }
    if(data.index == FILE_MENU) {
      filemenu();
      if(*kglobal == K_LEFTMENU) {
	data.index = OPT_MENU;
      }
      if(*kglobal == K_RIGHTMENU) {
	data.index = HELP_MENU;
      }
    }
    if(data.index == OPT_MENU) {
      optionsmenu();
      if(*kglobal == K_LEFTMENU) {
	data.index = HELP_MENU;
      }
      if(*kglobal == K_RIGHTMENU) {
	data.index = FILE_MENU;
      }
    }
    if(data.index == HELP_MENU) {
      helpmenu();
      if(*kglobal == K_LEFTMENU) {
	data.index = FILE_MENU;
      }
      if(*kglobal == K_RIGHTMENU) {
	data.index = OPT_MENU;
      }
    }
  } while(*kglobal != K_ENTER);
}

/*---------*/
/* Credits */
/*---------*/

void credits() {
/* Frees memory and displays goodbye message */
  free_buffer();
  resetTerm();			//restore terminal settings from failsafe
  showcursor();
  resetAnsi(0);
  screencol(0);
  outputcolor(7, 0);
  printf(cedit_ascii_1);
  printf(cedit_ascii_2);
  printf(cedit_ascii_3);
  printf(cedit_ascii_4);
  printf(cedit_ascii_5);
  printf(cedit_ascii_6);

  printf("\nCoded by v3l0r3k 2019.\n");

}

/*=================*/
/*BUFFER functions */
/*=================*/

/*----------------------------------*/
/* Initialize and clean edit buffer */
/*----------------------------------*/

void cleanBuffer(EDITBUFFER editBuffer[MAX_LINES]) {
//Clean and initialize the edit buffer
  long     i, j;

  for(j = 0; j < MAX_LINES; j++)
    for(i = 0; i < MAX_CHARS; i++) {
      editBuffer[j].charBuf[i].specialChar = 0;
      editBuffer[j].charBuf[i].ch = CHAR_NIL;
    }
}

int writetoBuffer(EDITBUFFER editBuffer[MAX_LINES], long whereX, long whereY,
		  char ch) {
  int     oldValue;
  oldValue = editBuffer[whereY].charBuf[whereX].specialChar;
  if(ch == SPECIAL_CHARS_SET1) {
    editBuffer[whereY].charBuf[whereX].specialChar = SPECIAL_CHARS_SET1;
    editBuffer[whereY].charBuf[whereX].ch = ch;
  } else if(ch == SPECIAL_CHARS_SET2) {
    editBuffer[whereY].charBuf[whereX].specialChar = SPECIAL_CHARS_SET2;
    editBuffer[whereY].charBuf[whereX].ch = ch;
  } else {
    //Special char example -61 -95 : á
    if(oldValue == SPECIAL_CHARS_SET1 || oldValue == SPECIAL_CHARS_SET2)
      editBuffer[whereY].charBuf[whereX].specialChar = oldValue;
    else
      editBuffer[whereY].charBuf[whereX].specialChar = 0;

    editBuffer[whereY].charBuf[whereX].ch = ch;
  }
  return 1;
}

/*========================*/
/*FILE handling functions */
/*========================*/

/*-----------*/
/* Open file */
/*-----------*/

int openFile(FILE ** filePtr, char fileName[], char *mode)
/* 
Open file.
@return ok = 1 ? 0 Success! 
*/
{
  int     ok = 0;
  *filePtr = fopen(fileName, mode);

  if(*filePtr != NULL) {
    //File opened correctly.
    ok = 1;
  } else {
    //Error
    ok = 0;
  }
  //devolvemos el resultado
  return ok;
}

/*------------*/
/* Close file */
/*------------*/

int closeFile(FILE * filePtr) {
/* 
   Close file
@return ok: 
*/
  int     ok;

  if(filePtr != NULL) {
    ok = fclose(filePtr);
  }

  return ok;
}

/*--------------------------*/
/* Write EditBuffer to File */
/*--------------------------*/

int writeBuffertoFile(FILE * filePtr, EDITBUFFER editBuffer[MAX_LINES]) {
  char    tempChar, oldChar;
  long     lineCounter = 0;
  long     inlineChar = 0;
  int     specialChar = 0;

  //Check if pointer is valid
  if(filePtr != NULL) {
    rewind(filePtr);		//Make sure we are at the beginning
    lineCounter = 0;
    inlineChar = 0;
    do {
      oldChar = tempChar;
      tempChar = editBuffer[lineCounter].charBuf[inlineChar].ch;
      specialChar =
	  editBuffer[lineCounter].charBuf[inlineChar].specialChar;
      inlineChar++;
      if(tempChar == END_LINE_CHAR) {
	inlineChar = 0;
	lineCounter++;
      }
      if(tempChar > 0) {
	fprintf(filePtr, "%c", tempChar);
      } else if(tempChar >= -128 && tempChar <= -65) {
	//Special accents
	fprintf(filePtr, "%c%c", specialChar, tempChar);
      }
    } while(tempChar != CHAR_NIL);
    //Check if last line finishes in 0x0A - oldChar = 0x0A
    if(oldChar != END_LINE_CHAR)
      fprintf(filePtr, "%c", END_LINE_CHAR);
  }
  return 1;
}

/*------------------*/
/* Save File Dialog */
/*------------------*/

int saveDialog(char fileName[MAX_TEXT]) {
  int     ok = 0;
  char    tempMsg[MAX_TEXT];
  data.index = OPTION_NIL;
  openFile(&filePtr, fileName, "w");
  writeBuffertoFile(filePtr, editBuffer);
  strcpy(tempMsg, fileName);
  strcat(tempMsg, WSAVE_MSG);
  ok = infoWindow(mylist, tempMsg);
  fileModified = 0;		//reset modified flag

  return ok;
}

int saveasDialog(char fileName[MAX_TEXT]) {
  int     ok, count;
  char    tempFile[MAX_TEXT], tempMsg[MAX_TEXT];
  data.index = OPTION_NIL;

  clearString(tempFile, MAX_TEXT);

  count = inputWindow(WSAVETITLE_MSG, WSAVELABEL_MSG, tempFile);

  if(count > 0) {
    //Save file
    data.index = OPTION_NIL;
    clearString(fileName, MAX_TEXT);
    strcpy(fileName, tempFile);

    //Check whether file exists.
    if(file_exists(fileName)) {
      ok = yesnoWindow(mylist, WFILEEXISTS_MSG);
      if(ok == CONFIRMATION) {
	//Save anyway.
	openFile(&filePtr, fileName, "w");
	writeBuffertoFile(filePtr, editBuffer);
	strcpy(tempMsg, fileName);
	strcat(tempMsg, WSAVE_MSG);
	ok = infoWindow(mylist, tempMsg);
	fileModified = 0;
      } else {
	//Do nothing.
      }
    } else {
      openFile(&filePtr, fileName, "w");
      writeBuffertoFile(filePtr, editBuffer);
      strcpy(tempMsg, fileName);
      strcat(tempMsg, WSAVE_MSG);
      ok = infoWindow(mylist, tempMsg);
      fileModified = 0;
    }
  }
  return ok;
}

/*-----------------*/
/* New File Dialog */
/*-----------------*/

int newDialog(char fileName[MAX_TEXT]) {
  char    tempFile[MAX_TEXT];
  int     ok, count;

  clearString(tempFile, MAX_TEXT);
  //clearString(fileName, MAX_TEXT);

  data.index = OPTION_NIL;

  count = inputWindow(WNEWTITLE_MSG, WSAVELABEL_MSG, tempFile);
  if(count > 0) {
    //Check whether file exists and create file.
    ok = createnewFile(&filePtr, tempFile, 1);
    if(ok == 1) {
      cleanBuffer(editBuffer);
      strcpy(fileName, tempFile);
    }
    ok = 1;
  }
  return ok;
}

/*--------------------------*/
/* OPEN File Dialog HANDLER */
/*--------------------------*/

int openFileHandler() {
  char    oldFile[MAX_TEXT];
  chdir(currentPath);		//Go back to original path
  openFileDialog(&openFileData);
  //Update new global file name
  if(openFileData.itemIndex != 0) {
    //Change current File Name 
    //if the index is different than CLOSE_WINDOW
    clearString(oldFile, MAX_TEXT);
    strcpy(oldFile, currentFile);	//save previous file to go back
    clearString(currentFile, MAX_TEXT);
    strcpy(currentFile, openFileData.path);
    cleanBuffer(editBuffer);
    //Open file and dump first page to buffer - temporary
    if(filePtr != NULL)
      closeFile(filePtr);
    handleopenFile(&filePtr, currentFile, oldFile);
  }
  return 1;
}

/*------------------*/
/* File Info Dialog */
/*------------------*/

int fileInfoDialog() {
  long    size = 0, lines = 0;
  char    sizeStr[12];
  char    linesStr[12];
  char    tempMsg[50];
  if(filePtr != NULL) {
    closeFile(filePtr);
    openFile(&filePtr, currentFile, "r");
    size = getfileSize(filePtr);
    lines = countLinesFile(filePtr);
    if(size <= 0)
      size = 0;
    if(lines <= 0)
      lines = 0;
    sprintf(sizeStr, "%ld", size);
    sprintf(linesStr, "%ld", lines);
    strcpy(tempMsg, WINFO_SIZE);
    strcat(tempMsg, sizeStr);
    strcat(tempMsg, " bytes.");
    strcat(tempMsg, WINFO_SIZE2);
    strcat(tempMsg, linesStr);
    strcat(tempMsg, " lines.\n");
    //strcat(WINFO_SIZE3, currentFile);
    //strcat(tempMsg, currentFile);
    alertWindow(mylist, currentFile, tempMsg);
  } else {
    infoWindow(mylist, WINFO_NOPEN);
  }
  return 0;
}


/*-----------------------------*/
/* Calculate Scroll Values     */
/*-----------------------------*/

short checkScrollValues(long lines){
 editScroll.displayLength= rows - 5;
 editScroll.totalLines = lines;
 editScroll.scrollLimit=editScroll.totalLines - editScroll.displayLength;
 if (editScroll.totalLines > editScroll.displayLength)
   {
     editScroll.scrollActive = 1;
     return VSCROLL_ON;
     
 } else{
     editScroll.scrollActive = 0;
   return VSCROLL_OFF;
 }
}


/*-----------------------------*/
/* Write EditBuffer to Display */
/*-----------------------------*/

int writeBuffertoDisplay(EDITBUFFER editBuffer[MAX_LINES]) {
  long     i = 0, j = 0, z=0; 
  char    tempChar, specialChar;

  //Dump edit buffer to screen
  if (editScroll.scrollActive == 1)
    z=editScroll.scrollPointer;
  else
    z=0;
  do {
    tempChar = editBuffer[z+j].charBuf[i].ch;
    specialChar = editBuffer[z+j].charBuf[i].specialChar;
    if(tempChar != CHAR_NIL) {
      if(tempChar != END_LINE_CHAR) {
	write_ch(i + START_CURSOR_X, j + START_CURSOR_Y, specialChar,
		 EDITAREACOL, EDIT_FORECOLOR);
	write_ch(i + START_CURSOR_X, j + START_CURSOR_Y, tempChar,
		 EDITAREACOL, EDIT_FORECOLOR);
	i++;
	if(i == columns - 3) {
	  //temporary restriction until scroll is implemented
	  i = 0;
	  j++;
	}
      }
      if(tempChar == END_LINE_CHAR) {
	i = 0;
	j++;
        if (j>editScroll.displayLength-1) break;
      }
    }
  } while(tempChar != CHAR_NIL);
  update_screen();
  return 1;
}

/*------------------------------*/
/* Open file and dump to buffer */
/*------------------------------*/

int filetoBuffer(FILE * filePtr, EDITBUFFER editBuffer[MAX_LINES]) {
  long     inlineChar = 0, lineCounter = 0;
  char    ch;
  fileModified = FILE_UNMODIFIED;
  //Check if pointer is valid
  if(filePtr != NULL) {
    rewind(filePtr);		//Make sure we are at the beginning

    ch = getc(filePtr);		//Peek ahead in the file
    while(!feof(filePtr)) {
      if(ch != CHAR_NIL) {
	//Temporary restrictions until scroll is implemented.
	//if(lineCounter == rows - 4)
	//  break;
        
	if(ch == SPECIAL_CHARS_SET1 || ch == SPECIAL_CHARS_SET2) {
	  writetoBuffer(editBuffer, inlineChar, lineCounter, ch);
	  //Read accents
	  ch = getc(filePtr);
	  writetoBuffer(editBuffer, inlineChar, lineCounter, ch);
	} else {
	  if(ch > 0)
	    writetoBuffer(editBuffer, inlineChar, lineCounter, ch);
	}
	inlineChar++;
	if(ch == END_LINE_CHAR) {
	  inlineChar = 0;
	  ch = 0;
	  lineCounter++;
	}
       if (lineCounter == MAX_LINES) {
           //open as readMode
           fileModified = FILE_READMODE;
           break;
       }
       //If file is bigger than buffer
      //break loop at last allowed line.
      }
      ch = getc(filePtr);
    }
  }
  return 1;
}

/*---------------*/
/* Get file size */
/*---------------*/

long getfileSize(FILE * filePtr) {
  long    sz;
  if(filePtr != NULL) {
    fseek(filePtr, 0L, SEEK_END);
    sz = ftell(filePtr);
    rewind(filePtr);
  }

  return sz;
}

/*-----------------*/
/* Check file type */
/*-----------------*/

long countLinesFile(FILE * filePtr) {
  char    ch;
  long    counterA = 0;
  if(filePtr != NULL) {
    rewind(filePtr);		//Make sure we are at the beginning

    ch = getc(filePtr);		//Peek ahead in the file
    while(!feof(filePtr)) {
      if(ch == END_LINE_CHAR) {
	counterA++;
      }
      ch = getc(filePtr);
    }
  }
  return counterA;
}

long checkFile(FILE * filePtr) {
  char    ch;
  long    counterA = 0;
  if(filePtr != NULL) {
    rewind(filePtr);		//Make sure we are at the beginning

    ch = getc(filePtr);		//Peek ahead in the file
    while(!feof(filePtr)) {
      if(ch < 9) {
	//discard accents
	if(ch > -60)
	  counterA++;
      }
      ch = getc(filePtr);
    }
  }
  return counterA;
}

/*----------------------*/
/* Check if file exists */
/*----------------------*/

int file_exists(char *fileName) {
  int     ok;
  if(access(fileName, F_OK) != -1) {
    ok = 1;			//File exists
  } else {
    ok = 0;
  }
  return ok;
}

/*------------------*/
/* Open file checks */
/*------------------*/

int handleopenFile(FILE ** filePtr, char *fileName, char *oldFileName) {
  long    checkF = 0;
  long    linesinFile =0;
  int     ok = 0;

  openFile(filePtr, fileName, "r");
  //Check for binary characters to determine filetype
  checkF = checkFile(*filePtr);
  if(checkF > 5) {
    //File doesn't seem to be a text file. Open anyway?  
    ok = yesnoWindow(mylist, WCHECKFILE_MSG);
    if(ok == 1) {
     filetoBuffer(*filePtr, editBuffer);
      refresh_screen(-1);
      //fileModified = FILE_READMODE; // Open as readmode
    } else {
      //Go back to previous file
      ok = -1;
      clearString(fileName, MAX_TEXT);
      strcpy(fileName, oldFileName);
      //Open file again and dump first page to buffer - temporary
      openFile(filePtr, currentFile, "r");
      filetoBuffer(*filePtr, editBuffer);
    }
  } else {
    //Check wheter file is bigger than buffer. 32700 lines
    linesinFile = countLinesFile(*filePtr);
    editScroll.scrollPointer = 0; //Set pointer to 0
    editScroll.pagePointer =0; //set page Pointer to 0
    checkScrollValues(linesinFile); //make calculations for scroll.
    if (linesinFile > MAX_LINES) 
    ok=infoWindow(mylist, WCHECKLINES_MSG);
    //Dump file into edit buffer.
    filetoBuffer(*filePtr, editBuffer);
    refresh_screen(-1);
  }
  return ok;
}

/*-----------------*/
/* New file checks */
/*-----------------*/

int createnewFile(FILE ** filePtr, char *fileName, int checkFile) {
  int     ok;

//Check if file exists if indicated.
  if(checkFile == 1) {
    if(file_exists(fileName)) {
      ok = yesnoWindow(mylist, WFILEEXISTS_MSG);
      if(ok == CONFIRMATION) {
	//Overwrite anyway.
	openFile(filePtr, fileName, "w");
	ok = 1;
      } else {
	//Do nothing. Don't overwrite.
	ok = 0;
      }
    } else {
      //File does not exist.
      //clearString(fileName, MAX_TEXT);
      //strcpy(fileName,tempFile);
      openFile(filePtr, fileName, "w");
      ok = 1;
    }

  } else {
    //Don't check if file exists.
    openFile(filePtr, fileName, "w");
    refresh_screen(-1);
  }
  return ok;
}

/*-------------------*/
/* Checks config file*/
/*-------------------*/

void checkConfigFile(int setValue) {
  FILE   *fp;
  int     ok = 0;
  char    ch;

  if(setValue == -1) {
    if(file_exists(CONFIGFILE)) {
      //If we open the program for the first time.
      //Read config file
      ok = openFile(&fp, CONFIGFILE, "r");
      if(ok == 1) {
	ch = getc(fp);
	ch = ch - '0';
	setColorScheme(ch);
      } else {
	printf("Error opening configfile. Setting to defaults.\n");
	setColorScheme(0);
      }
      closeFile(fp);
    } else {
      //Create config file
      ok = openFile(&fp, CONFIGFILE, "w");
      if(ok == 1) {
	fprintf(fp, "%d", 0);
	setColorScheme(0);	//Create config file and set default color Scheme        
      } else {
	printf("Error creating config file. Setting to defaults.\n");
	setColorScheme(0);
      }
      closeFile(fp);
    }
  } else {
    //If we call the function from the menu.
    //Set new value in config file
    ok = openFile(&fp, CONFIGFILE, "w");
    if(ok == 1) {
      fprintf(fp, "%d", setValue);
      //setColorScheme(setValue); //Create config file and set default color Scheme        
    } else {
      printf("Error creating config file. Setting to defaults.\n");
      setColorScheme(0);
    }
    closeFile(fp);
  }
}
