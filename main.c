#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "fs/operations.h"
#include "timer.h"
#include <sys/time.h>


#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100


FILE *outputFile, *inputFile;
char* inputFileArg = NULL;
char* outputFileArg = NULL;
int numberThreads = 0;


char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];  
    }
    return NULL;
}

static void displayUsage (const char* appName){
    printf("Usage: %s input_filepath output_filepath threads_number\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }

    inputFile = fopen(argv[1], "r"); //opening input file
    outputFile = fopen(argv[2], "w"); //opening output file
    

    numberThreads = atoi(argv[3]);
    //synchstrategyArg = argv[4];

    if (outputFile == NULL) { 					//seing if outputfile is open correctly
        perror("Error opening output file");
        exit(EXIT_FAILURE);

       }

    else if(inputFile == NULL){
    	perror("Error opening input file");  //seing if inputfile is open correctly
    	exit(EXIT_FAILURE);
    }

    else if (!numberThreads) {
        fprintf(stderr, "Invalid number of threads\n");
        displayUsage(argv[0]);
    }
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput(){

        

    char line[MAX_INPUT_SIZE];

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
                if(numTokens != 3)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'l':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
    }
    
}



void applyCommands(){
    
    while (numberCommands > 0){

        const char* command = removeCommand();

        if (command == NULL){
            continue;
        }


        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c':
                switch (type) {
                    case 'f':
                        printf("Create file: %s\n", name);
                        create(name, T_FILE);
                        break;
                    case 'd':
                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l': 
                searchResult = lookup(name);
                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);
                break;
            case 'd':
                printf("Delete: %s\n", name);
                delete(name);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void runThreads(FILE* timeFp){
    TIMER_T startTime, stopTime;
    #if defined (RWLOCK) || defined (MUTEX)
        pthread_t* workers = (pthread_t*) malloc(numberThreads * sizeof(pthread_t));
    #endif

    TIMER_READ(startTime);
    #if defined (RWLOCK) || defined (MUTEX)
        for(int i = 0; i < numberThreads; i++){
            int err = pthread_create(&workers[i], NULL, applyCommands, NULL);
            if (err != 0){
                perror("Can't create thread");
                exit(EXIT_FAILURE);
            }
        }
        for(int i = 0; i < numberThreads; i++) {
            if(pthread_join(workers[i], NULL)) {
                perror("Can't join thread");
            }
        }
    #else
        applyCommands();
    #endif
    TIMER_READ(stopTime);

    printf( "TecnicoFS completed in %.4f seconds.\n", TIMER_DIFF_SECONDS(startTime, stopTime));
    #if defined (RWLOCK) || defined (MUTEX)
        free(workers);
    #endif
}



int main(int argc, char* argv[]) {

    /* init filesystem */
    init_fs();

    /* process input and print tree */
    parseArgs(argc, argv);
    processInput(inputFile);
    runThreads(outputFile);
    print_tecnicofs_tree(outputFile);
    
    fflush(outputFile);
    fclose(outputFile);
    fclose(inputFile);

    /* release allocated memory */
    destroy_fs();
    exit(EXIT_SUCCESS);
}
