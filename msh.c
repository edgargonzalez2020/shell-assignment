  //
/* 
 * Name:    Edgar Gonzalez
 * ID:      1001336686
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
//#define DEBUG
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11    // Mav shell only supports ten arguments + command itself

#define MAX_HISTORY 50

int is_custom_created_command( char * );

void bg_command( void );

void print_pids( void );

void print_history( void );

int check_for_exclamation( char * );

void free_all_mem( char **, char * );

void free_tokens( char ** );

void parse_command( char ** ,char * );

void execute_command( char ** );

void sig_handler( int );

//Globals
char * history[MAX_HISTORY];
pid_t pid_history[15] = {0};
int history_index = 0;
int pid_index = 0;
int last_pid = -1;
int last_history = -1;
int last_child = 0;
int main()
{
    // Install the signal handler 
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sig_handler;
    if( sigaction(SIGINT, &act, NULL ) < 0 )
    {
        perror("Error");
        return -1;
    }
    if( sigaction(SIGTSTP, &act, NULL) < 0 )
    {
        perror("Error");
        return -1;
    }

    char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
    // Store the parent pid once 
    pid_history[pid_index++] = getpid();
    
    while( 1 )
    {
        // Print out the msh prompt
        printf ("msh> ");
        while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

        /* Parse input */
        char * token[MAX_NUM_ARGUMENTS];
        int   token_count = 0;                                 
                                                               
        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;                                         
                                                               
        char *working_str  = strdup( cmd_str );                

        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;

        // Tokenize the input stringswith whitespace used as the delimiter
        while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
                  (token_count<MAX_NUM_ARGUMENTS))
        {
          token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
          if( strlen( token[token_count] ) == 0 )
          {
            token[token_count] = NULL;
          }
            token_count++;
        }
        if( cmd_str != NULL && strcmp( cmd_str, "" ) != 0 && strcmp( cmd_str, "\n" ) != 0 )
        {
            // Store the current history string and increment the global index
            history[history_index++ % MAX_HISTORY] = strndup( cmd_str, MAX_COMMAND_SIZE );
            // keep track of the last index that we used
            // accounts for wrap around
            last_history = ++last_history % MAX_HISTORY;
        }
        
        if( token[0] != NULL )
        {
            // if we encounter a '!' in our string we overide 
            // the current tokens, with the tokens of
            // our command in history
            // history_command holds the index in the history
            // array, if the index is invalid we set the valid 
            // flag to zero so as not to try
            // to execute an invalid command 
            int is_valid = 1;
            int history_command = check_for_exclamation( token[0] );
            if( history_command != -1 )
            {
                if( history_command >= 0 && history_command <= 49 )
                {
                    if( history[history_command] != NULL )
                        parse_command( token ,history[history_command] );
                }
                else 
                {
                    is_valid = 0;
                }
            }
            // we only want to fork for system commands 
            // this helper function helps me decide whether to
            // fork: see function for details
            int is_not_needed_to_fork = is_custom_created_command( token[0] );
            if( !is_not_needed_to_fork && is_valid)
            {
                pid_t   child_pid;
                child_pid = fork();
                if( child_pid == 0 )
                {
                    execute_command( token );            
                    _exit( 0 );
                }
                else 
                {
                    // update the pid array with the child pid
                    int status;
                    pid_history[pid_index++ % 15] = child_pid;
                    last_pid = ++last_pid % 15;
                    waitpid( child_pid, &status, 0 );
                    // we need this so that we can
                    // background the last proccess 
                    last_child = child_pid;
                }
            }
            else
            {
                // these are the commands that do not
                // need to be forked 
                if( is_valid )
                {
                    switch( is_not_needed_to_fork )
                    {
                        case 1: bg_command();
                                break;
                        case 2: print_pids();
                                break;
                        case 3: print_history();
                                break;
                        case 4:
                        case 5: free( working_root );
                                exit( 0 );
                                break;
                        case 7: if( token[1] != NULL )
                                    chdir(token[1]);
                                break;
                    }
                }
                else
                {
                    // This print is only executed when 
                    // history_command is out of range
                    printf("Command not in history.\n");
                }
            }
        }
        free( working_root );
    }
    return 0;
}

// This code is used enough to warrant creating a function
// it returns the command # if the command is: bg, history, listpids, quit, or exit
// else it will return false
// command numbers as follow:
// bg = 1
// listpids = 2
// history = 3
// quit = 4
// exit = 5
// contains '!' = 6
// cd = 7
// returns: int 
// args: a char * that contains a command
int is_custom_created_command( char * token)
{
    int exclamation = check_for_exclamation( token );
    if( strcmp( token, "bg" ) == 0 ) return 1;
    if( strcmp( token, "listpids" ) == 0 ) return 2;
    if( strcmp( token, "history" ) == 0 ) return 3;
    if( strcmp( token, "quit" ) == 0 ) return 4;
    if( strcmp( token, "exit" ) == 0 ) return 5;
    if( strcmp( token, "cd" ) == 0 ) return 7;
    if( exclamation != -1) return exclamation;
    return 0;
}

// Call SIGCONT to background last 
// child proccess 
void bg_command()
{
    kill(last_child, SIGCONT);
}

// utility function to print all the pids 
void print_pids()
{
    int i;
    for( i = 0; i < 15; i++ )
    {
        if( pid_history[i] != 0 )
            printf("%d: %d\n", i , pid_history[i]);
    }
}

// utility function to print all the history
// commands
void print_history() 
{
    int i;
    for( i = 0; i < MAX_HISTORY; i++ )
    {
        if( history[i] != NULL )
            printf("%d: %s", i ,history[i]); 
    }
}

// This function returns an integer given
// in the string with 
// the presence of a '!' beggining the token
// else if not detected returns -1
// e.g !33 -> 33 
// returns: int
// args: char * that conains the command 
int check_for_exclamation( char * token )
{
    if( token[0] == '!' )
    {
        if( token[1] >= '0' && token[1] <= '9' )
        {
            int i = 1;
            int res = 0;
            while( token[i] != '\0' )
            {
                res = res * 10 + ( token[i] - '0' );
                i++;
            }
            return res;
        }
    }
    return -1;
}

// utility command
// args: char ** that contains a list of tokens for a command
void execute_command( char ** token )
{
    // We want to check for our executable in this order 
    // so we will loop through this array with path_index
    int path_index = 0;
    char * paths[4] = { "./", "/usr/local/bin/", "/usr/bin/", "/bin/"  };
    // allocate memory for the current working string that conatains the path
    // to the executable
    char * curr_dir = malloc( MAX_COMMAND_SIZE * sizeof( char ) + 1);
    
    // copy our first path into the current working string and concatenate it
    // with our command we wish to execute
    strcpy( curr_dir, paths[0] );
    strcat( curr_dir, token[0] );

    // Try to execute the command and update the current working string
    // to iterate through all the paths 
    // if we reach '/bin/' and execl returns -1, then the executable was
    // never found
    while( path_index < 3 && execl( curr_dir, 
                token[0], token[1], token[2], token[3], 
                token[4], token[5], token[6], token[7],
                token[8], token[9], token[10], NULL ) == -1 )
    {
        strcpy( curr_dir, paths[++path_index] );
        strcat( curr_dir, token[0] );
    }
    // experiencd weird behavior if (path_index < 4) above
    // seperated the last directory and it fixed the problem? 
    int ret = execl( curr_dir, 
            token[0], token[1], token[2], token[3], 
            token[4], token[5], token[6], token[7],
            token[8], token[9], token[10], NULL );
    if( ret == -1 )
        // This statement will only every execute if the executable was never found 
        printf("%s: Command not found.\n", token[0]);

}

// parses user input and updates token array
// it doesnt return anyting, it modifies the array in place 
// cmd_str is the desired string to tokenize
// utility function for the history '!' command 
// args: char**(tokens), char *( cmd_str)
void parse_command(char ** token ,char * cmd_str ) 
{
    /* Parse input */

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
    free( working_root );
}

// defines signal behavior
// in this case we just want default behavior 
// but we dont want to kill our shell
void sig_handler( int signum ) {}
