// Author: Saharsha Tiwari & Sameer Acharya
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128
#define MAX_BACKGROUND_PROCESSES 100

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
int argumentsCounts = 0;
extern char **environ;
pid_t background_processes[MAX_BACKGROUND_PROCESSES];
int num_background_processes = 0;
int is_process_running = 0;
pid_t foreground_process = 0;

void tokenizer(char command_line[], char *arguments[] ) {
    char *token = strtok(command_line, delimiters);
    while(token != NULL) {
        char *dollarSign = strchr(token, '$');
        if (dollarSign) {
            char *varName = dollarSign + 1;
            char *env_value = getenv(varName);
            arguments[argumentsCounts] = env_value;
        } else {
            arguments[argumentsCounts] = token;
            argumentsCounts++;
            token = strtok(NULL, delimiters);
        }
    }
}


void foreground_process_handler(int signum) {
    if (foreground_process > 0) {
        // Terminate the foreground process
        kill(foreground_process, SIGTERM);
        printf("\nTerminated .. Time limit exceeded 10 seconds  .\n");
    }
}

void executeCommands(char *arguments[]) {
    is_process_running = 1;
    char buff[512];
    if (strcmp(arguments[0], "cd") == 0) {
        if (argumentsCounts < 2) {
            if (chdir(getenv("HOME")) != 0) {
                perror("chdir failed");
            }
        } else if (argumentsCounts > 2) {
            fprintf(stderr, "Error: Too many arguments.\n");
        }else if (chdir(arguments[1]) != 0 ) {
            perror("chdir failed");
            if (errno == ENOENT) {
                fprintf(stderr, "Directory '%s' does not exist.\n", arguments[1]);
            } else if (errno == EACCES) {
                fprintf(stderr, "Permission denied for directory '%s'.\n", arguments[1]);
            } else if (errno == ENOTDIR) {
                fprintf(stderr, "'%s' is not a directory.\n", arguments[1]);
            } else {
                fprintf(stderr, "Failed to change directory: %s\n", strerror(errno));
            }
        }
    } else if (strcmp(arguments[0],"pwd") == 0) {
        getcwd(buff, sizeof(buff));
        printf("%s\n", buff);
    } else if (strcmp(arguments[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(arguments[0], "echo") == 0) {
        int i = 1;
        for (;i < argumentsCounts;i++) {
            if (arguments[i] != NULL) {
                printf("%s ", arguments[i]);
            }
        }
        printf("\n");

    } else if (strcmp(arguments[0], "env") ==0 ) {
        char **env = environ;
        while (*env) {
            printf("%s\n", *env);
            env++;
        }
    } else if (strcmp(arguments[0] , "setenv") ==0 ) {
        int i = 1;
        for (;i < argumentsCounts; i++ ) {
            char *token = strtok(arguments[i], "=");
            if (token != NULL) {
                char *var_name = token;
                token = strtok(NULL, "=");
                if (token != NULL) {
                    char *var_value = token;
                    if (setenv(var_name, var_value, 1) != 0) {
                        perror("setenv failed");
                    }
                } else {
                    printf("No value provided for the environment variable.\n");
                }
            } else {
                printf("Invalid input format.\n");
            }

        }
    } else {
        int run_in_background = 0;
        if (argumentsCounts > 0 && strcmp(arguments[argumentsCounts - 1], "&") == 0) {
            run_in_background = 1;
            // Remove the "&" from the arguments
            arguments[argumentsCounts - 1] = NULL;
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            arguments[argumentsCounts] = NULL;
            execvp(arguments[0], arguments);
            perror("execvp failed");
            fprintf(stderr, "An error occurred.\n");
            exit(1);
        } else {
            if (!run_in_background) {
                signal(SIGALRM, foreground_process_handler);
                alarm(10);
                foreground_process = pid;
                int status;
                waitpid(pid, &status, 0);
                alarm(0);
                foreground_process = 0;
            } else {
                // Add the background process to the list
                if (num_background_processes < MAX_BACKGROUND_PROCESSES) {
                    background_processes[num_background_processes] = pid;
                    num_background_processes++;
                } else {
                    fprintf(stderr, "Maximum number of background processes reached.\n");
                }
            }
        }
    }
}

void sigint_handler(int signum) {
    if (is_process_running) {
        printf("\n\t\t\t(type ctrl-C)\n");
    } else {
        printf("\n");
    }
    fflush(stdout);
}


// Signal handler for background process termination
void background_processes_handler(int signum) {
    int i = 0;
    for (; i < num_background_processes; i++) {
        pid_t bg_pid = waitpid(background_processes[i], NULL, WNOHANG);
        if (bg_pid > 0) {
            printf("Background process with PID %d has completed.\n", bg_pid);
            // Remove the completed process from the list
            int j = i;
            for (; j < num_background_processes - 1; j++) {
                background_processes[j] = background_processes[j + 1];
            }
            num_background_processes--;
            i--; // Recheck the same index
        }
    }
}

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];

    char buff[256];
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];

    signal(SIGCHLD, background_processes_handler);
    signal(SIGINT, sigint_handler);

    while (true) {

        do{
            argumentsCounts = 0;
            // Print the shell prompt.
            if (getcwd(buff, sizeof(buff)) == NULL) {
                perror("Error : Could not read the current directory");
            }
            else {
                printf("%s%s", buff, prompt);
                fflush(stdout);
            }

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")

            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }else {
                tokenizer(command_line, arguments);
                executeCommands(arguments);
                memset(*arguments, 0, sizeof(arguments));
                is_process_running = 0;
            }
        }while(command_line[0] == 0x0A);  // while just ENTER pressed


        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }
        // TODO:
        //

        // 0. Modify the prompt to print the current working directory


        // 1. Tokenize the command line input (split it on whitespace)


        // 2. Implement Built-In Commands


        // 3. Create a child process which will execute the command line input


        // 4. The parent process should wait for the child to complete unless its a background process


        // Hints (put these into Google):
        // man fork
        // man execvp
        // man wait
        // man strtok
        // man environ
        // man signals

        // Extra Credit
        // man dup2
        // man open
        // man pipes
    }
    // This should never be reached.
    return -1;
}
