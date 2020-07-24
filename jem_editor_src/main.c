/*
     _____ ________ __       __
   |     |        |  \     /  \
    \$$$$| $$$$$$$| $$\   /  $$
      | $| $$__   | $$$\ /  $$$
 __   | $| $$  \  | $$$$\  $$$$
|  \  | $| $$$$$  | $$\$$ $$ $$
| $$__| $| $$_____| $$ \$$$| $$
 \$$    $| $$     | $$  \$ | $$
  \$$$$$$ \$$$$$$$$\$$      \$$

 */
//AUTHOR: Jose Emmanuel Maldonado
//GITHUB LINK: https://github.com/JoseMaldonado12
/* This editor heavily borrows (EI. goes nearly word for word )
 * from this guide: https://viewsourcecode.org/snaptoken/kilo/index.html
 * As such I have no plans to claim complete ownership or monetize this
 * in any way; anyone could do what I did. I simply tweaked many different
 * variables to what I deemed appropriate, renamed and restructured some code,
 * and even added/left out a bit of code myself that I thought was more efficient
 * and/or necessary. Anyone reading this is encouraged to do the same! Either use
 * my code as a baseline or follow the guide from scratch and perhaps learn a thing or 2
 * about C text editing. Or you can use my text editor as is for your own purposes.
 * Perhaps tab sizes are a huge issue for you and having a tab size of 3 is just perfect,
 * or perhaps its just a decent mix of vim difficulty and nano simplicity. Whatever
 * you decide to do with this, it'd be very cool if you shot me a message at my github
 * to see what you people are doing with this; I love hearing about other peoples projects,
 * or perhaps vast improvements to my own. Happy coding!*/
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
/**** defines ****/
//lets me re-map control keys to whatever i want
#define CTRL_KEY(x) ((x) & 0x1f)
#define INIT_BUF {NULL,0}
#define ROW_MARKER_S "~"
#define ROW_MARKER_C '~'
#define VERSION_NUMBER "0.3"
#define TAB_SIZE 3
#define WELCOME_MESSAGE "COMMANDS: Ctrl-Q = quit *** Ctrl-S = save. *** Ctrl-F = search the file. "
#define WELCOME_MESSAGE_LENGTH 150
#define PRESS_KEY_THIS_MANY_TIMES_TO_QUIT 1 //(plus add one)
#define ESCAPE '\x1b'
//used for portability purposes
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE


/**** global vars ****/


typedef struct edirow{
    int size;
    int render_size;
    char *render;
    char *c;

} edirow;

struct config {
    struct termios og;
    int current_rows;
    int current_cols;
    int cursorx, cursory;
    int tab_offset;
    int rownum;
    int row_offset;
    int col_offset;
    char *filename;
    int dirty_file;
    char status_message[WELCOME_MESSAGE_LENGTH];
    time_t status_message_time;
    edirow *row;
};
struct config cf;

struct wbuf {
    char *c;
    int size;
};

enum keys{
    BACKSPACE=127, //equal to the DEL ascii value
    LEFT=1200,
    RIGHT,
    UP,
    DOWN,
    DEL,
    HOME,
    END,
    PG_UP,
    PG_DOWN
};

/**** functions ****/
void echo_off();
void echo_on();
void destroy( const char *e);
int catch_throw_key();
void catch_and_process();
void screen_wipe();
void row_draws(struct wbuf *buf);
void init_edit();
int window_size(int *row, int *col);
int cursor_position(int *row, int *col);
void wappend(struct wbuf *buf, const char *s, int size);
void destructor(struct wbuf *buf);
void move_cursor(int move);
void openfile(char *filename);
void insertrow(int index,char *s, size_t size);
void scrolling();
void updaterow(edirow *row);
int calc_tab_offset(edirow *row, int current_cursor_x);
int reverse_calc_tab_offset(edirow *row, int tabbed_cursor);
void statusbar(struct wbuf *buffer);
void set_status_message(const char *fmt, ...);
void message_bar(struct wbuf *buffer);
void row_insert_char(edirow *row, int index, int c);
void insert_char(int c);
char *edirow_to_string(int *buffer_size);
void save_file();
void row_del_char(edirow *row, int index);
void del_char();
void del_row(int index);
void free_row(edirow *row);
void row_append_s(edirow *row, char *s, size_t len);
void insert_nwln();
char *edi_prompt(char *prompt, void (*callback)(char *, int));
void search();
void search_callback(char *query, int key);



/**** appending to write()****/
void wappend(struct wbuf *buf, const char *s, int size) {
    char *new = realloc(buf->c, buf->size + size);
    if (new == NULL) return;
    memcpy(&new[buf->size], s, size);
    buf->c = new;
    buf->size += size;
}

void destructor(struct wbuf *buf) {
    free(buf->c);
}


/**** editor set-up ****/
void echo_off(){

    //once this is finished turn echo back on
    if (tcgetattr(STDIN_FILENO, &cf.og) == -1)
        destroy("tcgetattr");//just in case

    atexit(echo_on);
    //make a copy of the original termios
    struct termios raw=cf.og;

    //v below is how you turn off important flags in c.

    //IXON= control s and control q
    //ICRNL=control m
    //the rest idk someone online told me to do it.
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    //ICANON= canonical mode to read input byte-by-byte
    //ISIG= control z and control c
    //ECHO= blocks user input to be shown
    //IEXTEN= control v control o
    raw.c_lflag &= ~(ECHO|ICANON| ISIG|IEXTEN);

    //fixes /n translations
    raw.c_oflag &= ~(OPOST);

    //sets character size to 8 bits per byte. Done to standardize the rest of the program (just in case)
    raw.c_cflag |= (CS8);

    raw.c_cc[VTIME] = 1;
    raw.c_cc[VMIN] = 0;



    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)== -1)
        destroy("tcsetattr"); //just in case
}

void echo_on(){
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cf.og)==-1)
            destroy("tcsetattr");//just in case
}

int window_size(int *row, int *col){
    struct winsize ws;

    if ( ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        //if ioctl doesnt work get the size some other way
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;

        return cursor_position(row,col);
    } else{
        *col= ws.ws_col;
        *row = ws.ws_row;
        return 0;
    }
} //returns -1 in failure, 0 successful

void destroy(const char *e){
    //v below is used so as to not dump garbage on screen upon faulty exit

    //output an escape sequence to screen to clean the screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    //move writing cursor back to the top right
    write(STDOUT_FILENO, "\x1b[H", 3);


    perror(e);
    exit(1);
}

//backup in case we cant get col and row normally
int cursor_position(int *row, int *col){

    char buffer[32];

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    int i;
    //This for loop skips the first character in the buffer, \x1b
    for ( i=0;i < sizeof(buffer)-1; i++){
        if (read(STDIN_FILENO, &buffer[i], 1) != 1)
            break;
        if (buffer[i] == 'R')
            break;
    }

    buffer[i]='\0';
    if (buffer[0] != ESCAPE || buffer[1] != '[')
        return -1;

    if (sscanf(&buffer[2], "%d;%d", row, col) != 2)
        return -1;
    return 0;
}
/**** takes care of input ****/

void move_cursor(int move){
    edirow *row= (cf.cursory >= cf.rownum) ? NULL : &cf.row[cf.cursory];
    switch(move){
        case LEFT:
            if(cf.cursorx != 0) {
                cf.cursorx--;
            }else if(cf.cursory>0){
                cf.cursory--;
                cf.cursorx=cf.row[cf.cursory].size;
            }
            break;
        case RIGHT:
            if(row && cf.cursorx < row->size) {
                cf.cursorx++;
            }else if(row && cf.cursorx == row->size){
                cf.cursory++;
                cf.cursorx=0;
            }
            break;
        case UP:
            if(cf.cursory !=0)
                cf.cursory--;
            break;
        case DOWN:
            if(cf.cursory < cf.rownum-1)
                cf.cursory++;
            break;
    }
    //corrects cursor back to its place
    row = (cf.cursory >= cf.rownum) ? NULL : &cf.row[cf.cursory];
    int row_size=row? row->size: 0;
    if(cf.cursorx>row_size)
        cf.cursorx = row_size;
}
int catch_throw_key() {
    char c;
    int reading;
    while ((reading = read(STDIN_FILENO, &c, 1)) != 1) {
        if (reading == -1 && errno != EAGAIN)
            destroy("read");
    }

    //if its an arrowkey treat it appropriately
    if (c == ESCAPE) {

        char strt[3];
        if (read(STDIN_FILENO, &strt[0], 1) != 1) return ESCAPE;
        if (read(STDIN_FILENO, &strt[1], 1) != 1) return ESCAPE;

        if (strt[0] == '[') {
            if (strt[1] >= '0' && strt[1] <= '9') {
                if (read(STDIN_FILENO, &strt[2], 1) != 1)
                    return ESCAPE;
                if (strt[2] == ROW_MARKER_C) {
                    switch (strt[1]) {
                        //home and end can be several different numbers
                        case '1':
                            return HOME;
                        case '3':
                            return DEL;
                        case '4':
                            return END;
                        case '5':
                            return PG_UP;
                        case '6':
                            return PG_DOWN;
                        case '7':
                            return HOME;
                        case '8':
                            return END;
                    }
                }

                } else {
                    switch (strt[1]) {
                        case 'A':
                            return UP;
                        case 'B':
                            return DOWN;
                        case 'C':
                            return RIGHT;
                        case 'D':
                            return LEFT;
                        case 'H':
                            return HOME;
                        case 'F':
                            return END;
                    }
                }
            } else if (strt[0] == 'O'){
            switch (strt[1]){
                case 'H':
                    return HOME;
                case 'F':
                    return END;
            }
        }
            return ESCAPE;
        } else { return c; }

    }
void catch_and_process(){
    static int quit_num=PRESS_KEY_THIS_MANY_TIMES_TO_QUIT;
    int c= catch_throw_key();

    if(c == CTRL_KEY('q')) {
        if(cf.dirty_file && quit_num >0){
            set_status_message("THE CURRENT FILE HAS UNSAVED CHANGES. "
                                   "Press Ctrl-Q again to quit or save first.");
            --quit_num;
            return;
        }
        //v below is used so as to not dump garbage on screen

        //output an escape sequence to screen to clean the screen
        write(STDOUT_FILENO, "\x1b[2J", 4);
        //move writing cursor back to the top right
        write(STDOUT_FILENO, "\x1b[H", 3);

        exit(0);
        }

    switch(c){
        case '\r':
            insert_nwln();
            break;

        case HOME:
            cf.cursorx=0;
            break;

        case END:
            if(cf.cursory < cf.rownum)
                cf.cursorx=cf.row[cf.cursory].size;
            break;

        case PG_DOWN:
        case PG_UP:
        {
            if (c == PG_UP) {
                cf.cursory = cf.row_offset;
            }else{
                cf.cursory = cf.row_offset + cf.current_rows - 1;
                if (cf.cursory > cf.rownum)
                    cf.cursory = cf.rownum;
            }
        }
        break;

        case UP:
        case DOWN:
        case LEFT:
        case RIGHT:
            move_cursor(c);
            break;

        case BACKSPACE:
        case CTRL_KEY('h'):
        case DEL:
            //del deletes char, goes left, and then goes right (to its OG position)
            if(c == DEL)
                move_cursor(RIGHT);
            del_char();
            break;

        case CTRL_KEY('l'):
        case ESCAPE:
            break;
        case CTRL_KEY('s'):
            save_file();
            break;
        case CTRL_KEY('f'):
            search();
            break;
        //if it isnt a special char, insert it's raw data directly into buffer
        default:
            insert_char(c);
            break;
    }
    quit_num=PRESS_KEY_THIS_MANY_TIMES_TO_QUIT;
}
/**** takes care of output ****/

void row_draws(struct wbuf *buf){
int i;
for(i=0; i<cf.current_rows; i++){
    int filerow= i + cf.row_offset;
    if(filerow >= cf.rownum) {
        //adds info about editor 1/2 of the the way down the screen.
        if (cf.rownum==0 && i == cf.current_rows  / 2) {
            char msg[50];
            int size = snprintf(msg, sizeof(msg), "JEM txt editor-----version %s", VERSION_NUMBER);
            if (size > cf.current_cols)
                size = cf.current_cols;

            //Below section is to ensure the message is centered.
            int center = (cf.current_cols - size) / 2;
            if (center) {
                wappend(buf, ROW_MARKER_S, 1);
                --center;
            }
            while (center--) {
                wappend(buf, " ", 1);
            }

            wappend(buf, msg, size);
        } else {
            wappend(buf, ROW_MARKER_S, 1);
        }
    } else{
        int size = cf.row[filerow].render_size - cf.col_offset;
        if (size < 0)
            size=0;
        if (size > cf.current_cols)
            size = cf.current_rows;
        wappend(buf, &cf.row[filerow].render[cf.col_offset], size);
    }
    //puts sequence after every line we draw to make it look nicer
    wappend(buf,"\x1b[K", 3);

    wappend(buf, "\r\n", 2);

}
}
void screen_wipe(){
    scrolling();
    struct wbuf buf=INIT_BUF;


    wappend(&buf, "\x1b[?25l", 6);
    //move writing cursor back to the top right
    wappend(&buf, "\x1b[H", 3);

    row_draws(&buf);
    statusbar(&buf);
    message_bar(&buf);

    //moves the cursor to the correct position
    char to_add[32];
    snprintf(to_add, sizeof(to_add), "\x1b[%d;%dH", (cf.cursory- cf.row_offset) + 1, (cf.tab_offset-cf.col_offset) + 1);
    wappend(&buf, to_add, strlen(to_add));

    /*v this below, alongside the first wappend, hides cursor for a split second
     * so as to not induce seizures on ppl (marker flickering)*/
    wappend(&buf, "\x1b[?25h", 6);

    write(STDOUT_FILENO, buf.c, buf.size);
    destructor(&buf);
}
void scrolling(){

    cf.tab_offset=0;
    //tab offset gets calculated; tab_offset=cursorx if no tabs, calculates tabs in otherwise
    if(cf.cursory<cf.rownum)
        cf.tab_offset=calc_tab_offset(&cf.row[cf.cursory], cf.cursorx);

    //fixes scrolling cursor going offscreen, follows cursor wherever it is.
    //vertical scrolling
    if (cf.cursory < cf.row_offset)
        cf.row_offset = cf.cursory;
    if (cf.cursory >= cf.row_offset + cf.current_rows)
        cf.row_offset = cf.cursory - cf.current_rows + 1;

    //horizontal scrolling
    if (cf.tab_offset< cf.col_offset)
        cf.col_offset = cf.tab_offset;
    if (cf.tab_offset >= cf.col_offset + cf.current_cols)
        cf.col_offset = cf.tab_offset - cf.col_offset + 1;

}
void statusbar(struct wbuf *buffer){
    //turns on inverted colors then turns off to make status bar pop out
    wappend(buffer, "\x1b[7m", 4);
    char status[120];
    char line_num[120];

    //get info to display line numbers
    int size = snprintf(status, sizeof(status), "%.20s - %d lines %s",
            cf.filename ? cf.filename : "{Name Missing}", cf.rownum,
            cf.dirty_file ? "(modified)" :  "");
    int line_num_size = snprintf(line_num, sizeof(line_num), "%d/%d",cf.cursory + 1, cf.rownum);
    if (size > cf.current_cols)
        size = cf.current_cols;
    wappend(buffer, status, size);

    //add line number info to the right edge of the screen
    for(; size<cf.current_cols; size++) {
        if((cf.current_cols - size) == line_num_size){
            wappend(buffer, line_num, line_num_size);
            break;
        } else
            wappend(buffer, " ", 1);

    }
    wappend(buffer, "\x1b[m", 3);
    wappend(buffer, "\r\n", 2);
}
void set_status_message(const char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(cf.status_message, sizeof(cf.status_message), fmt, ap);
    va_end(ap);
    cf.status_message_time = time(NULL);
}
void message_bar(struct wbuf *buffer){
    wappend(buffer, "\x1b[K", 3);
    int msgsize = strlen(cf.status_message);
    if (msgsize > cf.current_cols)
        msgsize = cf.current_cols;
    if (msgsize && time(NULL) - cf.status_message_time < 5)
        wappend(buffer, cf.status_message, msgsize);
}
char *edi_prompt(char *prompt,void (*callback)(char *, int)){

    size_t buffer_size = 128;
    char *buf = malloc(buffer_size);
    size_t buffer_len = 0;
    buf[0] = '\0';
    //will go on until button is pressed
    while (1) {
        set_status_message(prompt, buf);
        screen_wipe();
        int c = catch_throw_key();
        //press escape key to leave
        if(c == DEL || c == CTRL_KEY('h') || c == BACKSPACE){
            if(buffer_len != 0)
                buf[--buffer_len] = '\0';

        }else if(c == ESCAPE) {
            set_status_message("");
            if(callback)
                callback(buf,c);
            free(buf);
            return NULL;
        } else if (c == '\r') {
            if (buffer_len != 0) {
                // if there is user input and they have pressed enter, delete status message
                set_status_message("");
                if(callback)
                    callback(buf,c);
                return buf;
            }
        } else if (!iscntrl(c) && c < 128) {
            if (buffer_len == buffer_size - 1) {
                buffer_size *= 2;
                buf = realloc(buf, buffer_size);
            }
            buf[buffer_len++] = c;
            buf[buffer_len] = '\0';
        }
        if(callback)
            callback(buf,c);
    }
}


/**** row ops ****/
void insertrow(int index,char *s, size_t size) {
    //dont do shit out of bounds
    if (index < 0 || index > cf.rownum)
        return;

    cf.row = realloc(cf.row, sizeof(edirow) * (cf.rownum + 1));
    //make room at the specified index
    memmove(&cf.row[index + 1], &cf.row[index], sizeof(edirow) * (cf.rownum - index));



    cf.row[index].size=size;
    cf.row[index].c = malloc(size + 1);
    memcpy(cf.row[index].c, s, size);
    cf.row[index].c[size] = '\0';

    cf.row[index].render_size=0;
    cf.row[index].render=NULL;
    updaterow(&cf.row[index]);
    ++cf.rownum;
    ++cf.dirty_file;
}
void updaterow(edirow *row){

    //count the number of tabs there are
    int tabs=0;
    int i;
    for (i = 0; i < row->size; i++){
        if(row->c[i] == '\t')
            tabs++;
    }

    //release it of any previous alterations and allocate appropriate space
    free(row->render);
    row->render = malloc(row->size + 1 + tabs*(TAB_SIZE-1) + 1);

    //count how many characters there are in render
    int index = 0;
    for (i = 0; i < row->size; i++){
        //treats tabs as 8 spaces rather than chars.
        if(row->c[i] == '\t'){
            row->render[index++] = ' ';
            while(index %TAB_SIZE !=0)
                row->render[index++] = ' ';
        } else
            row->render[index++] = row->c[i];
    }
    row->render[index] = '\0';
    row->render_size = index;
}
int calc_tab_offset(edirow *row, int current_cursor_x){
    int current_tab_offset=0;
    int i;
    //iterates throught row, checks for tabs and spaces em appropriately
    for(i=0;i<current_cursor_x; i++){
        if(row->c[i] == '\t')
            current_tab_offset +=(TAB_SIZE-1) - (current_tab_offset%TAB_SIZE);
        ++current_tab_offset;
    }
    return current_tab_offset;
}
int reverse_calc_tab_offset(edirow *row, int tabbed_cursor){
    int current_tab=0;
    int i;
    //iterates throught row, checks for tabs and returns early if there is one.
    for(i=0;i<row->size; i++){
        if(row->c[i] == '\t')
            current_tab +=(TAB_SIZE-1) - (current_tab%TAB_SIZE);
        ++current_tab;
        if(current_tab > tabbed_cursor)
            return i;
    }
    return i;
}
void row_insert_char(edirow *row, int index, int c){
    if (index < 0 || index > row->size)
        index = row->size;

    row->c = realloc(row->c, row->size + 2);
    memmove(&row->c[index + 1], &row->c[index], row->size - index + 1);
    row->size++;
    row->c[index] = c;
    updaterow(row);
    ++cf.dirty_file; //can show how dirty a file is.
}
void row_del_char(edirow *row, int index){
    //nothing to delete? dont delete
    if (index < 0 || index >= row->size)
        return;

    memmove(&row->c[index], &row->c[index + 1], row->size - index);
    row->size--;
    updaterow(row);
    cf.dirty_file++;
}
void del_row(int index){
    //if row doesnt exist dont do anything
    if (index < 0 || index >= cf.rownum)
        return;
    //free any text thats on the current row to delete
    free_row(&cf.row[index]);
    //overwrites the current, deleted line with the row after that.
    memmove(&cf.row[index], &cf.row[index + 1], sizeof(edirow) * (cf.rownum- index - 1));
    --cf.rownum;
    ++cf.dirty_file;
}
void free_row(edirow *row){
    free(row->render);
    free(row->c);
}
void row_append_s(edirow *row, char *s, size_t len){

    //append the size of the previous row to the current row
    row->c = realloc(row->c, row->size + len + 1);

    memcpy(&row->c[row->size], s, len);
    row->size += len;
    row->c[row->size] = '\0';
    updaterow(row);
    cf.dirty_file++;
}

/****  text editing ops ****/
void insert_char(int c){
    //if this is true then cursor is on tilde; create new line and erase tilde.
    if (cf.cursory == cf.rownum)
        insertrow(cf.rownum,"",0);
    //inserts char to the current buffer
    row_insert_char(&cf.row[cf.cursory], cf.cursorx, c);
    ++cf.cursorx;
}
void del_char(){
    // if cursor is past EOF, theres nothing to delete
    if(cf.cursory == cf.rownum)
        return;
    if(cf.cursorx == 0 && cf.cursory == 0)
        return;
    edirow *row = &cf.row[cf.cursory];
    //
    if (cf.cursorx > 0) {
        row_del_char(row, cf.cursorx - 1);
        --cf.cursorx;
    } else{
        cf.cursorx= cf.row[cf.cursory - 1].size;
        row_append_s(&cf.row[cf.cursory - 1], row->c, row->size);
        del_row(cf.cursory);
        cf.cursory--;
    }
}
void insert_nwln(){
    if (cf.cursorx == 0)
        insertrow(cf.cursory, "", 0);
    else{
        edirow *row = &cf.row[cf.cursory];
        //add row right below current row, with contents that are to the right of the cursor
        insertrow(cf.cursory + 1, &row->c[cf.cursorx], row->size - cf.cursorx);

        //insertrow may have moved memory with realloc, so re-align row
        row = &cf.row[cf.cursory];

        row->size = cf.cursorx;
        row->c[row->size] = '\0';
        updaterow(row);
    }
    ++cf.cursory;
    cf.cursorx=0;

}

/**** file handling ****/
void openfile(char *filename){
    free(cf.filename);
    cf.filename = strdup(filename);
    FILE *fp = fopen(filename, "r");

    //failsafe
    if (!fp)
        destroy("fopen");

    char *line = NULL;
    size_t size = 0;
    ssize_t linesize;

    //grabs doc and iterates through all text line by line
    while((linesize = getline(&line, &size, fp)) != -1) {
        while (linesize > 0 && (line[linesize - 1] == '\n' || line[linesize - 1] == '\r'))
            linesize--;
        insertrow(cf.rownum,line,linesize);
    }
    free(line);
    fclose(fp);
    cf.dirty_file=0; //done so the message doesn't immeaditly pop out
}
char *edirow_to_string(int *buffer_size){
    int total_size = 0;
    int i;
    //get the length of all rows combined to malloc the necessary memory
    for (i = 0; i < cf.rownum; i++)
        total_size += cf.row[i].size + 1;

    *buffer_size = total_size;
    char *buffer = malloc(total_size);
    char *s = buffer;
    //iterate through all the written rows and copy them to buffer
    for (i = 0; i < cf.rownum; i++) {
        memcpy(s, cf.row[i].c, cf.row[i].size);
        s += cf.row[i].size;
        *s = '\n';
        s++;
    }

    return buffer;
}
void save_file(){

    if(cf.filename == NULL) {
        cf.filename = edi_prompt("Save as: %s ---Press ESC to cancel",NULL);
        if(cf.filename == NULL){
            set_status_message("Save cancelled. File not saved.");
            return;
        }
    }

    int size=0;
    char *buffer=edirow_to_string(&size);

    //DONT CHANGE THIS PLEASE
    //0644 is standard permission files
    // o_creat creates file if it doesnt exist
    int fid=open(cf.filename, O_RDWR | O_CREAT, 0644);


    //checks for correct file opening among other things
    if(fid != -1){
        if(ftruncate(fid,size) != -1){
            if(write(fid,buffer,size) == size){
                close(fid);
                cf.dirty_file=0; //done so the message doesn't immeaditly pop out
                free(buffer);
                set_status_message("File %s was saved with %d bytes! ",cf.filename, size);
                return;
            }

        }
        close(fid);
    }

    free(buffer);
    set_status_message("Unable to save file. File error: %s", strerror(errno));
}


/**** search ****/
void search(){
    int current_cursorx=cf.cursorx;
    int current_cursory=cf.cursory;
    int current_col_offset=cf.col_offset;
    int current_row_offset=cf.row_offset;
    char *query = edi_prompt("Search: %s (ESC to cancel, Arrow keys to cycle through searches)", search_callback);

    if(query)
        free(query);
    else{ //they pressed null, lets restore the original values
        cf.cursorx =current_cursorx;
        cf.cursory=current_cursory;
        cf.col_offset=current_col_offset;
        cf.row_offset=current_row_offset;
    }

}
void search_callback(char *query, int key){
    static int last_match_index = -1;
    static int direction = 1; //1=searching forwards, -1=searching backwards

    //check if they wanna leave search mode
    if (key == '\r' || key == ESCAPE){
        last_match_index=-1;
        direction=1;
        return;
        //move from search results according to the buttons they press.
    } else if(key==RIGHT || key==DOWN) {
        direction = 1;
    }else if(key==LEFT || key==UP) {
        direction = -1;
    }else{
        last_match_index=-1;
        direction=1;
    }

    if(last_match_index == -1)
        direction=1;

    int current_row_index=last_match_index;
    int i;
    //iterates through each row, then iterates through that row to find the matching string(s).
    //if cycling through the same query, it will continue where it left off, otherwise its starts anew.
    for(i=0; i<cf.rownum; i++){
        current_row_index +=direction;
        if(current_row_index == -1)
            current_row_index=cf.rownum-1;
        else if(current_row_index == cf.rownum)
            current_row_index=0;

        edirow *row= &cf.row[current_row_index];
        char *found=strstr(row->render, query);
        if(found){
            last_match_index=current_row_index;
            cf.cursory=current_row_index;
            cf.cursorx=reverse_calc_tab_offset(row,found-row->render);
            cf.row_offset= cf.rownum; //scrolls to bottom of file, forcing matching line to be top of screen
            break;
        }
    }
}
/**** main ****/
void init_edit(){
    //will initalize all values of the global struct
    cf.cursorx=0;
    cf.cursory=0;
    cf.rownum=0;
    cf.row=NULL;
    cf.row_offset=0;
    cf.col_offset=0;
    cf.tab_offset=0;
    cf.filename=NULL;
    cf.status_message[0] = '\0';
    cf.status_message_time = 0;
    cf.dirty_file =0;
    if (window_size(&cf.current_rows, &cf.current_cols) == -1)
        destroy("window_size");

    //makes room on the final line for the status bar
    cf.current_rows-=2;
}
int main(int argc, char *argv[]){
    //turns off echo
    echo_off();
    init_edit();
    //if theres an argument for it open that particular file
    if(argc >= 2)
        openfile(argv[1]);

    set_status_message(WELCOME_MESSAGE);

    while(1) {
        screen_wipe();
        catch_and_process(); //dw it exits in function
        }
    return 0;
    }