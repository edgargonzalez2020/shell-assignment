/*
 * Name:Edgar Gonzalez
 * ID: 1001336686
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports ten arguments + command itself

//#define DEBUG

struct command_struct 
{
    char * full_command;
    char * arg1,arg2,arg3,arg4,arg5,
        arg7,arg8,arg9,arg10;
};
struct command_struct  history[15];
int     history_index = 0;
pid_t pid_history[15] = {0};
int pid_index = 0;
int last_pid = -1;

// helper function to achieve the history command (!num)
// it will return -1 on fail and the command on success
// quite possible an easier way to do this
// but this is how I did this
// all it does is add all the digits 
// in the string
int check_for_exclamation(char * str)
{
    if( str[0] == '!' )
    {
        int i = 1;
        int res = 0;
        while( str[i] != '\0' )
        {
            res = res * 10 + ( str[i] - '0' );
            i++;
        }
        return res;
    }
    return -1;
}
void sig_handler( int sig )
{

    if( sig == 2 )
    {
        //SIGINT(ctrl-c)
        kill( pid_history[last_pid], SIGINT );
    }
    else if( sig == 20 )
    {
        //SIGTSTP(ctrl-z)
        //kill( pid_history[last_pid], SIGTSTP );
    }
    else if( sig == 18 )
    {
        //SIGCONT
    }
}
int main(int argc, char * argv[])
{
    struct sigaction act;
    memset( &act, '\0', sizeof(act) );
    act.sa_handler = &sig_handler;
    if( sigaction( SIGINT, &act, NULL) < 0 )
    {
        perror("Error");
        return 1;
    }
    if( sigaction( SIGTSTP, &act, NULL) < 0 )
    {
        perror("Error");
        return 1;
    }
    char *  cmd_str = (char *)malloc( MAX_COMMAND_SIZE );
    while( 1 )
    {
        printf("msh> ");
        while( !fgets(cmd_str, MAX_COMMAND_SIZE, stdin) ){}
        char *  token[MAX_NUM_ARGUMENTS];
        int     token_count = 0;
        char *  arg_ptr;
        char *  working_str = strdup( cmd_str );
        char *  working_root = working_str;

        while( (arg_ptr = strsep( &working_str, WHITESPACE ) ) != NULL
                && ( token_count < MAX_NUM_ARGUMENTS ) )
        {
            token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
            if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            token_count++;
        }
        // keep track of the command history
        struct command_struct comm;
        memset( &comm, '\0', sizeof(comm) );
        comm.full_command = strndup( cmd_str, MAX_COMMAND_SIZE );
        comm.arg1 = token[1];
        comm.arg1 = token[2];
        comm.arg1 = token[3];
        comm.arg1 = token[4];
        comm.arg1 = token[5];
        comm.arg1 = token[6];
        comm.arg1 = token[7];
        comm.arg1 = token[8];
        comm.arg1 = token[9];
        comm.arg1 = token[10];
        history[ history_index++ % 15] = comm;
#ifdef DEBUG
        int     token_index;
        for( token_index = 0; token_index < token_count; token_index++ )
        {
            printf("token[%d] = %s\n", token_index, token[token_index] );
        }

        int test;
        for( test = 0; test < 15; test++)
        {
            if( history[test].full_command != NULL)
                printf("%d: %s", test, history[test].full_command);
        }
#endif
        pid_t   child_pid = fork();
        // fork a proccess, run command when we are running in the child proccess. This happens when pid == 0.
        if ( child_pid == 0 )
        {
            /*
            //search for executable in the following order
            // './'
            // '/usr/local/bin'
            // '/usr/bin'
            // '/bin'
            */
            if( pid_index > 15 )
            {
                pid_index = 0;
            }
            pid_history[pid_index++] = getpid(); 
            last_pid++;
            last_pid %= 15; 
            char *  paths[4] = { "./", "/usr/local/bin/", "/usr/bin/", "/bin/" };
            int     i = 0;
            char    curr_working_string[MAX_COMMAND_SIZE]; 
            strcpy( curr_working_string, paths[i] );
            strcat( curr_working_string, token[0] );
            // if the command is cd we can only run in the parent proccess
            // so why bother trying to execute it
            if( token[0] != NULL && strcmp( token[0], "cd" ) == 0) 
                exit(0);
            else if( token[0]!= NULL && strcmp( token[0], "history" ) == 0 )
            {
                int i;
                for( i = 0; i < 15; i++ )
                {
                    if( history[i].full_command != NULL )
                        printf("%d: %s", i, history[i].full_command);
                }
            }
            else if( token[0] != NULL && strcmp( token[0], "quit" ) == 0 || strcmp( token[0], "exit" ) == 0 )
            {
                printf("Quiiting in pid: %d\n", getpid());
                exit( 0 );
            }
            else if( strcmp( token[0], "bg" ) == 0 )
            {
                printf("Calling SIGTSTP from bg in pid: %d\n", getpid());
                kill(pid_history[pid_index], SIGTSTP);

            }
            else if ( check_for_exclamation( token[0] ) != -1 )
            {
                int command_number = check_for_exclamation( token[0] );
                if( command_number <= 14 && command_number >= 0 )
                {

                }
            }
            else if( strcmp( token[0], "listpids") == 0 )
            {
                int i;
                for ( i = 0; i < 15; i++ )
                {
                    if( pid_history[i] != 0 )
                    {
                        printf("%d: %d\n", i, pid_history[i]); 
                    }
                }
            }
            while( ( execl( curr_working_string, token[0], token[1], token[2], token[3],
                    token[4], token[5],token[6], token[7], token[8], token[9], token[10],NULL) == -1 )&&
                    i < 4 )
            {
                strcpy( curr_working_string, paths[++i] );
                strcat( curr_working_string, token[0] );
            }

            // if the execl returns -1 then we did not find it in any of the above dirs
            // The only exception, is these we do not want this next line to execute when 
            // command is 'history', 'bg', 'listpids' the result will be incorrect
            
            int is_valid = strcmp(token[0], "cd") != 0 && strcmp( token[0], "history" ) != 0 
                && strcmp( token[0], "bg") != 0 
                && strcmp( token[0], "listpids") != 0 
                && check_for_exclamation( token[0] ) == -1 ? 1 : 0;
            
            if( execl(curr_working_string, token[0], token[1], token[2], token[3],
                    token[4], token[5],token[6], token[7], token[8], 
                    token[9], token[10],NULL) == -1 && is_valid) 
                printf("%s: Command not found.\n", token[0]);
        }
        else 
        {
            int status;
            waitpid( child_pid, &status, 0 );
            if( pid_index > 15 )
            {
                pid_index = 0;
            }
            pid_history[pid_index++] = getpid(); 
            last_pid++;
            last_pid %= 15; 
#ifdef DEBUG
            if( WIFSIGNALED( status ) )
            {
                // Print the signal that the child terminated with 
                printf("Child returned with status %d\n", WTERMSIG( status ) );
            }
#endif
            if( token[0] != NULL && strcmp( token[0], "cd" ) == 0)
            {
                // this command can only be ran as the parent
                chdir(token[1]);
            }
            else if( token[0] != NULL && 
                    ( strcmp( token[0], "quit" ) == 0 || strcmp( token[0], "exit" ) == 0 ) )
            {
                printf("Quiiting from pid: %d\n",getpid());
                exit( 0 );
            }

        }

        free( working_root );
    }
    exit( 0 );
}
