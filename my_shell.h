/* lib.h librería con las funciones equivalentes a las
de <string.h> y las funciones y estructuras para el
manejo de una pila */
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>      /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <fcntl.h>     /* Modos de apertura de función open()*/
#include <sys/stat.h>  /* Permisos función open() */
#include <sys/types.h> /* Definiciones de tipos de datos como size_t*/
#include <unistd.h>    /* Funciones read(), write(), close()*/
#include <limits.h>
#include <sys/wait.h>
#include <signal.h>

#define PROMPT "$"
#define BLUE  "\e[34m"
#define WHITE "\e[97m"
#define CYAN    "\e[96m"
#define RESET   "\e[0m"
#define BOLD "\e[1m"
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 64

struct info_process {
   pid_t pid;
   char status; // ‘N’, ’E’, ‘D’, ‘F’
   char cmd[COMMAND_LINE_SIZE]; // línea de comando
};
static struct info_process jobs_list[N_JOBS];


const char delimiter[6] = " \t\n\r";
const char del[6] = "\t\n\r";
char *cdCommand = "cd";
char *exportCommand = "export";
char *sourceCommand = "source";
char *jobsCommand = "jobs";
char *fgCommand = "fg";
char *bgCommand = "bg";
char *exitCommand = "exit";
int foreground_pid = 0;
char *command;
int n_pids = 1;

void imprimir_prompt();
char *read_line(char *line);
int execute_line(char *line);    
int parse_args(char **args, char *line);    
int check_internal(char **args);    
int internal_cd(char **args);    
int internal_export(char **args);    
int internal_source(char **args);    
int internal_jobs(char **args);    
int internal_fg(char **args);    
int internal_bg(char **args);   
void reaper(int signum);
void ctrlc(int signum); 
int jobs_list_add(pid_t pid,char status, char *cmd);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
void ctrlz(int signum);
int is_background(char **args); 
int is_output_redirection(char **args); 