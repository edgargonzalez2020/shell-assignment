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

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports ten arguments + command itself

//#define DEBUG
int main(int argc, char * argv[])
{
    char *  cmd_str = (char *)malloc( MAX_COMMAND_SIZE );
    char *  history[15] = {'\0'};
    int     history_index = 0;
    while( 1 )
    {
        printf("msh> ");
        while( !fgets(cmd_str, MAX_COMMAND_SIZE, stdin) )
        {

        }
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
        history[ history_index++ % 15] = strndup( cmd_str, MAX_COMMAND_SIZE );


#ifdef DEBUG
        int     token_index;
        for( token_index = 0; token_index < token_count; token_index++ )
        {
            printf("token[%d] = %s\n", token_index, token[token_index] );
        }

        int test;
        for( test = 0; test < 15; test++)
        {
            if( history[test] != NULL)
                printf("%d: %s", test, history[test]);
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
            char *  paths[4] = { "./", "/usr/local/bin/", "/usr/bin/", "/bin/" };
            int     i = 0;
            char    curr_working_string[MAX_COMMAND_SIZE]; 
            strcpy( curr_working_string, paths[i] );
            strcat( curr_working_string, token[0] );
            // if the command is cd we can only run in the parent proccess
            // so why bother trying to execute it
            if( strcmp( token[0], "cd" ) == 0) 
                exit(0);
            else if( strcmp( token[0], "history" ) == 0 )
            {
                int i;
                for( i = 0; i < 15; i++ )
                {
                    if( history[i] != NULL )
                        printf("%d: %s", i, history[i]);
                }
            }
            while( ( execl(curr_working_string, token[0], token[1], token[2], token[3],
                    token[4], token[5],token[6], token[7], token[8], token[9], token[10],NULL) == -1 )&&
                    i < 4 )
            {
                strcpy( curr_working_string, paths[++i] );
                strcat( curr_working_string, token[0] );
            }

            // if the execl returns -1 then we did not find it in any of the above dirs
            // The only exception, is history we do not want this next line to execute when 
            // command is 'history', the result will be incorrect
            if( execl(curr_working_string, token[0], token[1], token[2], token[3],
                    token[4], token[5],token[6], token[7], token[8], token[9], token[10],NULL) == -1 
                    && strcmp(token[0], "cd") != 0 && strcmp( token[0], "history" ) != 0 ) 
                printf("%s: Command not found.\n", token[0]);
        }
        else 
        {
            // cd needs to be ran in the parent proccess
            int status;
            waitpid( child_pid, &status, 0 );
#ifdef DEBUG
            if( WIFSIGNALED( status ) )
            {
                // Print the signal that the child terminated with 
                printf("Child returned with status %d\n", WTERMSIG( status ) );
            }
#endif
            if( strcmp( token[0], "cd" ) == 0)
            {
                chdir(token[1]);
            }
        }

        free( working_root );
    }
    return 0;
}
