#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define CMD_MAX_LEN 1024
#define CMD_MAX_ARGS 128
#define MAX_BG_PROCESSES 100

char shell_prompt[] = "$ ";
char token_delimiters[] = " \t\r\n";
int arg_count = 0;
extern char **environ;
pid_t bg_processes[MAX_BG_PROCESSES];
int bg_process_count = 0;
int is_cmd_running = 0;
pid_t fg_process = 0;

void split_input(char input[], char *args[]) {
    char *token = strtok(input, token_delimiters);
    while (token != NULL) {
        char *env_var = strchr(token, '$');
        if (env_var) {
            char *env_name = env_var + 1;
            char *env_val = getenv(env_name);
            args[arg_count] = env_val;
        } else {
            args[arg_count] = token;
        }
        arg_count++;
        token = strtok(NULL, token_delimiters);
    }
    args[arg_count] = NULL;
}

void fg_process_handler(int sig) {
    if (fg_process > 0) {
        kill(fg_process, SIGTERM);
        printf("\nForeground process terminated due to timeout (10 seconds).\n");
    }
}

void execute_command(char *args[]) {
    is_cmd_running = 1;
    char cwd[512];
    if (strcmp(args[0], "cd") == 0) {
        if (arg_count < 2) {
            if (chdir(getenv("HOME")) != 0) {
                perror("chdir failed");
            }
        } else if (arg_count > 2) {
            fprintf(stderr, "Error: Too many arguments.\n");
        } else if (chdir(args[1]) != 0) {
            perror("chdir failed");
        }
    } else if (strcmp(args[0], "pwd") == 0) {
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; i < arg_count; i++) {
            if (args[i] != NULL) {
                printf("%s ", args[i]);
            }
        }
        printf("\n");
    } else if (strcmp(args[0], "env") == 0) {
        for (char **env = environ; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    } else if (strcmp(args[0], "setenv") == 0) {
        for (int i = 1; i < arg_count; i++) {
            char *var = strtok(args[i], "=");
            if (var != NULL) {
                char *val = strtok(NULL, "=");
                if (val != NULL) {
                    if (setenv(var, val, 1) != 0) {
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
        int run_in_bg = 0;
        if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
            run_in_bg = 1;
            args[arg_count - 1] = NULL;
        }
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        } else {
            if (!run_in_bg) {
                signal(SIGALRM, fg_process_handler);
                alarm(10);
                fg_process = pid;
                int status;
                waitpid(pid, &status, 0);
                alarm(0);
                fg_process = 0;
            } else {
                if (bg_process_count < MAX_BG_PROCESSES) {
                    bg_processes[bg_process_count++] = pid;
                } else {
                    fprintf(stderr, "Maximum background process limit reached.\n");
                }
            }
        }
    }
}

void sigint_handler(int sig) {
    if (is_cmd_running) {
        printf("\n(Use Ctrl-C to interrupt)\n");
    } else {
        printf("\n");
    }
    fflush(stdout);
}

void handle_bg_processes(int sig) {
    for (int i = 0; i < bg_process_count; i++) {
        pid_t bg_pid = waitpid(bg_processes[i], NULL, WNOHANG);
        if (bg_pid > 0) {
            printf("Background process %d completed.\n", bg_pid);
            for (int j = i; j < bg_process_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_process_count--;
            i--;
        }
    }
}

int main() {
    char input_line[CMD_MAX_LEN];
    char cwd[256];
    char *args[CMD_MAX_ARGS];

    signal(SIGCHLD, handle_bg_processes);
    signal(SIGINT, sigint_handler);

    while (true) {
        arg_count = 0;
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Error getting current directory");
        } else {
            printf("%s%s", cwd, shell_prompt);
            fflush(stdout);
        }

        if (fgets(input_line, CMD_MAX_LEN, stdin) == NULL && ferror(stdin)) {
            perror("fgets error");
            exit(1);
        }

        if (feof(stdin)) {
            printf("\n");
            break;
        }

        split_input(input_line, args);
        if (arg_count > 0) {
            execute_command(args);
        }
    }

    return 0;
}
