//Paula Ferrer Salom 
#include "my_shell.h"

//Crea un prompt personalizado yuxtaponiendo variables de entorno
void imprimir_prompt(){
    char *ptr;
    long size;
    size = pathconf(".", _PC_PATH_MAX);

    if((ptr = (char *)malloc((size_t) size)) != NULL) {
        ptr = getcwd(ptr, (size_t) size);
    }
    if(ptr==NULL) perror("getcwd() error");

    printf(BLUE);
    printf(BOLD);
    printf("%s", getenv("USER"));
    printf(WHITE);
    printf("%s", ":");
    printf(CYAN);
    printf("%s","~");
    printf("%s", ptr);
    printf(WHITE);
    printf("%s",PROMPT);
    printf(RESET);

    free(ptr);
}

//Imprime el prompt y lee una linea de consola
//Devuelve un puntero a la línea leída
char *read_line(char *line){
    char *ptr;
    fflush(stdout);
    imprimir_prompt();

    ptr = fgets(line, COMMAND_LINE_SIZE, stdin);
    //En el caso CTRL+D sale con exit(0)
    if (!ptr && !feof(stdin)){ 
        ptr = line;
        ptr[0] = 0;
    }else if (feof(stdin)) { //feof(stdin)!=0 significa que se ha activado el indicador EOF
        printf("Bye bye \n");
        exit(0);
    }
 
    return ptr;
}

//Obtiene la línea fragmentada en tokens 
//y comprueba si es un comando interno
//Si se trata de un comando externo lo ejecutamos con una llamada al sistema
int execute_line(char *line){

    char mensaje[1200];
    pid_t pid;//PID para resultado del fork
    int bckgrnd = 0;
    char **args= (char**) malloc(ARGS_SIZE * (sizeof(char*)));
    for (int i = 0; i < ARGS_SIZE; i++) { 
		args[i] = (char*) malloc(COMMAND_LINE_SIZE * (sizeof(char)));
	}
        strcpy(mensaje, line);
        strtok(mensaje, del);
        //strcpy(jobs_list[0].cmd, mensaje);
        parse_args(args, line);
        if(check_internal(args)== 0 && line[0] != '#' && args[0]){//Se trata de un comando externo
            bckgrnd = is_background(args);
            pid=fork();//Se crea el proceso hijo
            if(pid < 0){
                perror("ERROR");
                exit(1);
            }else if(pid == 0){//Proceso hijo
                signal(SIGCHLD, SIG_DFL);
                signal(SIGINT, SIG_IGN); //CTRL+C
                signal(SIGTSTP, SIG_IGN); //CTRL+Z
                is_output_redirection(args);

                if(execvp(args[0], args) < 0){//Se llama al sistema para ejecutar el comando externo
                    perror("ERROR");
                    exit(1);
                }
                exit(0);
            }else{//Proceso padre
                if(bckgrnd){ //proceso en segundo plano
                    if(jobs_list_add(pid, 'E', mensaje)< 0){
                       perror("ERROR");
                        exit(1); 
                    }
                }else{ //proceso en primer plano
                    foreground_pid = pid; //guardamos el hijo dentro de foreground_pid
                    jobs_list[0].pid = pid;
                    strcpy(jobs_list[0].cmd, mensaje);
                    jobs_list[0].status = 'E';
                    while(jobs_list[0].pid > 0) { //si se está ejecutando un hijp
                        pause(); //esperamos a que acabe
                    }
                }
                
            }    
        } 

    
    free(args);

    return 0;
}   

//Obtiene el vector de los diferentes tokens
//Devuelve el nº de tokens
int parse_args(char **args, char *line){
    
    char* token;
    int i = 0;
    //Extraemos el primer token
    token = strtok(line, delimiter);
    //Extraemos el resto de tokens
    while(token != NULL && *token != '#' && *token != '\0'){
        strcpy(args[i], token);
        token = strtok(NULL, delimiter);
        i++;
    }
    args[i]=NULL;
    
    return i;
}   

//Averigua si args[0] es un comando interno y llama a la función para tratarlo
//Devuelve 1 si es un comando interno y 0 si no lo es
int check_internal(char **args){
    
    if(args[0] == NULL || args[0] == 0){
        return 0;
    }
    
    if(strcmp(args[0],cdCommand)==0){
        internal_cd(args);
        return 1;
    }else if(strcmp(args[0],exportCommand)==0){
        internal_export(args);
        return 1;
    }else if(strcmp(args[0],sourceCommand)==0){
        internal_source(args);
        return 1;
    }else if(strcmp(args[0],jobsCommand)==0){
        internal_jobs(args);
        return 1;
    }else if(strcmp(args[0],fgCommand)==0){
        internal_fg(args);
        return 1;
    }else if(strcmp(args[0],bgCommand)==0){
        internal_bg(args);
        return 1;
    }else if(strcmp(args[0],exitCommand)==0){
        exit(0);
    }


    return 0;
}

//Cambia de directorio al indicado a partir de args[1] 
//Es capaz de acceder a directorios con espacios
//Devuelve 0 si funciona correctamente y EXIT_FAILURE en caso contrario
int internal_cd(char **args){
    char *ptr;
    long size;
    int n = 0;
    size = pathconf(".", _PC_PATH_MAX);

    //Si no se indica nada el directorio es HOME
    if(args[1]== NULL){
        //Cambiamos de directorio
        if(chdir(getenv("HOME"))==-1){
            perror("ERROR");
            return EXIT_FAILURE;
        }
    }else{
        //Detectamos si se trata de un directorio espaciado indicado por comillas
        if(strchr(args[1], '"') || strchr(args[1], '\'')){
            //Borramos las comillas y unimos en un solo argumento args[1]
            n = strlen(args[1]);
            for(int i = 0; i<=n; i++){
                args[1][i] = args[1][i+1];
            }
            n = strlen(args[2]);
            args[2][n-1]='\0';
            strcat(args[1], " ");
            strcat(args[1], args[2]);
        }
        //Detectamos si se trata de un directorio espaciado indicado por un guión
        if(strchr(args[1], '\\')){
            //Borramos el guión y unimos en un solo argumento args[1]
            n = strlen(args[1]);
            args[1][n-1]='\0';
            strcat(args[1], " ");
            strcat(args[1], args[2]);
        }
        //Cambiamos de directorio
        if(chdir(args[1])==-1){
            perror("ERROR");
            return EXIT_FAILURE;
        }
    }

    //Comprobamos el directorio actual
    if((ptr = (char *)malloc((size_t) size)) != NULL) {
        ptr = getcwd(ptr, (size_t) size);
    }
    if(ptr==NULL) perror("getcwd() error");

    free(ptr);

    return 0;
} 

//Cambia la variable de entorno por un valor dados por args[1]
//Devuelve 0 si funciona correctamente y EXIT_FAILURE en caso contrario
int internal_export(char **args){
    int n=0;
    char* token;
    char **exportArgs= (char**) malloc(ARGS_SIZE * (sizeof(char*)));
    for (int i = 0; i < ARGS_SIZE; i++) { 
		exportArgs[i] = (char*) malloc(COMMAND_LINE_SIZE * (sizeof(char)));
	}
    //Descomponemos en tokens args[1] para obtener:
    //nombre->exportArgs[0] y valor->exportArgs[1]
    token = strtok(args[1], "=");
    while(token != NULL){
        strcpy(exportArgs[n], token);
        token = strtok(NULL, delimiter);
        n++;
    }
    exportArgs[n]=NULL;

    //Comprobamos que la sintaxix sea correcta
    if(exportArgs[0] == NULL){
        printf("Error de sintaxis. Uso: export Nombre=Valor\n");
        free(exportArgs);
        return EXIT_FAILURE;
    }
    if( exportArgs[1] == NULL){
        printf("Error de sintaxis. Uso: export Nombre=Valor\n");
        free(exportArgs);
        return EXIT_FAILURE;
    }

    //Cambiamos el valor de la variable de entorno
    if(setenv(exportArgs[0],exportArgs[1],1)<0){
        perror("ERROR");
        free(exportArgs);
        return EXIT_FAILURE;
    }
    
    free(exportArgs);
    return 0;
} 

//Lee un fichero de comandos y los ejecuta linea a linea
//Devuelve -1 si ha habido un error y 0 en caso contrario
int internal_source(char **args){
    
    FILE * fp;
    char *line = (char*) malloc(COMMAND_LINE_SIZE * (sizeof(char*)));

    if(args[1]== NULL){//Comprueba la sintaxis
        printf("Error de sintaxis. Uso: source <nombre_fichero>\n");
        return -1;   
    }
    fp = fopen(args[1], "r");
    if(fp == NULL){
        perror("ERROR");
        return -1;
    }
    //Lee y ejecuta los comandos del fichero
    while (fgets(line, 100, fp)!=NULL){
        fflush(fp);
        execute_line(line);
    }
    fclose(fp);
    free(line);
    
    return 0;
} 

//Imprime por pantalla los datos guardados en jobs_list 
//de todos los trabajos que contiene
int internal_jobs(char **args){
    for (int i = 1; i < n_pids; i++){
    printf("[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status, jobs_list[i].cmd);
  }
    return 0;
}  

//Envía un trabajo del background al foreground, 
//o reactiva la ejecución en foreground de un trabajo que había sido detenido
int internal_fg(char **args){
    int pos = 0;
    if(args[1]){
        pos = atoi(args[1]);
        if(pos >= n_pids || pos == 0){
            fprintf(stderr, "no existe ese trabajo\n");
            return -1;
        }
        if(jobs_list[pos].status == 'D' || jobs_list[pos].status == 'E'){ //si el status es 'E' no hay porqué enviarsela
            kill(jobs_list[pos].pid, SIGCONT);
            strtok(jobs_list[pos].cmd, "&");
            jobs_list[0].pid = jobs_list[pos].pid;
            jobs_list[0].status = 'E';
            strcpy(jobs_list[0].cmd, jobs_list[pos].cmd);
            jobs_list_remove(pos);
            printf("%s\n", jobs_list[0].cmd);
        }
        while(jobs_list[0].pid != 0){ //mientras haya un proceso en foreground
			pause(); //esperamos a que acabe
		}
    }
    return 0;
} 

//Reactiva un proceso detenido para que siga ejecutándose pero en segundo plano
int internal_bg(char **args){
    int pos = 0;
    if(args[1]){
        pos = atoi(args[1]);
        if(pos >= n_pids || pos == 0){
            fprintf(stderr, "no existe ese trabajo\n");
            return -1;
        }
        if(jobs_list[pos].status == 'E'){
            fprintf(stderr, "el trabajo ya está en 2º plano\n");
            return -1;
        }
        jobs_list[pos].status = 'E';
        strcat(jobs_list[pos].cmd, " &");
        kill(jobs_list[pos].pid, SIGCONT);
        printf("[%d] %d\t%c\t%s\n", pos, jobs_list[pos].pid, jobs_list[pos].status, jobs_list[pos].cmd);
    }
    return 0;
} 

//Manejador própio de la señal SIGCHLD
//Controla si el hijo que acaba es el que se ejecuta en primer plano,
//en ese caso muestra la señal por la que finaliza y actualiza los valores de jobs_list[0]
void reaper(int signum){
    signal(SIGCHLD, reaper);
    pid_t ended = 0;
    int status = 0;
    int Job = 0;

    while((ended=waitpid(-1, &status, WNOHANG))>0){
        if(ended == jobs_list[0].pid){ //proceso en primer plano
            if (WIFEXITED(status)){
            }else if (WIFSIGNALED(status)){
            }
                jobs_list[0].pid = 0;
                strcpy(jobs_list[0].cmd, "");
                jobs_list[0].status = 'F';
        }else{ //proceso en segundo plano
                Job = jobs_list_find(ended);
                if (WIFEXITED(status)){
                }else if (WIFSIGNALED(status)){
                }
                fprintf(stderr, "Terminado PID %d (%s) en jobs_list[%d] con status %d\n", jobs_list[Job].pid, jobs_list[Job].cmd, Job, status);
                jobs_list_remove(Job);
        }
    }
}

//Manejador própio de la señal SIGINT
//controla que la señal no afecte a procesos en segundo plano ni al minishell
void ctrlc(int signum){

  signal(SIGINT, ctrlc);
  printf("\n");
  fflush(stdout);
  if (jobs_list[0].pid > 0){
    if (strcmp(jobs_list[0].cmd, command)){
      kill(jobs_list[0].pid, SIGTERM);
    }else{
    }
  }else{
  }
}

//Añade un trabajo en la última posición del array de trabajos 
//si este no ha alcanzado su número máximo de trabajos,
//en tal caso devuelve -1 de lo contrario devuelve la posición en el array
int jobs_list_add(pid_t pid,char status, char *cmd){
    if(n_pids < N_JOBS){
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        strcpy(jobs_list[n_pids].cmd, cmd);
        printf("[%d]\t%d\t%c\t%s\n", n_pids, jobs_list[n_pids].pid, jobs_list[n_pids].status, jobs_list[n_pids].cmd);
        n_pids++;
        return n_pids;
    }else{
        printf("Se ha alcanzado el número máximo de trabajos permitidos\n");
        return -1;
    }
}

//Busca en el array de trabajos el PID que recibe como argumento,
//si lo encuentra devuelve la posición, en caso contrario 0
int jobs_list_find(pid_t pid){
    for(int i = 1; i < n_pids; i++){
        if(jobs_list[i].pid == pid){
            return i;
        }
    }
    return 0;
}

//Elimina el trabajo de la posicion dada
//mueve trabajo que se encuentra en última posición a ese
//en caso de error devuelve -1, de lo contrario 0
int jobs_list_remove(int pos){
    if(pos < n_pids){
        jobs_list[pos].pid = jobs_list[n_pids - 1].pid ;
        jobs_list[pos].status = jobs_list[n_pids-1].status ;
        strcpy(jobs_list[pos].cmd, jobs_list[n_pids-1].cmd);
        n_pids--;
    }else{
        return -1;
    }
    return 0;
}

//Se encarga de detener procesos
//comprueba que haya un proceso en foreground y que este no sea el minishell
void ctrlz(int signum){
    signal(SIGTSTP, ctrlz);
    if(jobs_list[0].pid > 0){ //proceso en primer plano
        if (strcmp(jobs_list[0].cmd, command)){//el proceso no es el minishell
            kill(jobs_list[0].pid, SIGSTOP);
            jobs_list[0].status = 'D';
            jobs_list_add(jobs_list[0].pid, jobs_list[0].status, jobs_list[0].cmd);
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            strcpy(jobs_list[0].cmd, "");
        }else{
        }
    }else{
    }
}

//Comprueba que el comando sea en segundo plano,
//mirando si contiene el caracter & y devuelve 1,
//en caso de que no lo sea, devuelve 0
int is_background(char **args){
    int i = 0;
    while(args[i] != NULL){
        i++;
    }
    if(strcmp(args[i-1], "&") == 0){
        args[i-1] = NULL;
        return 1;
    }
    return 0;
}

//Busca un > seguido del nombre de un fichero. 
//Si lo encuentra abre el fichero y cambia el valor de args[1] por null
int is_output_redirection(char **args){
    int i = 0;
    int fd = 0;
    while (args[i] != NULL){
        if(strcmp(args[i], ">")==0){
            fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(fd, 1);

            if(fd =='\0'){
                perror("Error abriendo el fichero");
                return -1;
            }

            close(fd);
            args[i] = NULL;
            return 1;
        }else{
            i++;
        }
    }
    return 0;
}


//__________________ BUCLE PRINCIPAL_____________________
int main(int argc, char *argv[]){

    signal(SIGCHLD, reaper);//llamada al enterrador de zombies cuando un hijo acaba (señal SIGCHLD)
    signal(SIGINT, ctrlc);//SIGINT es la señal de interrupción que produce Ctrl+C
    signal(SIGTSTP, ctrlz);//SIGTSTP es la señal de interrupción que produce Ctrl+Z
    command = argv[0];
    char line[COMMAND_LINE_SIZE]; 
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'F'; 
    strcpy(jobs_list[0].cmd, "");
    

    while(read_line(line)){
        execute_line(line);
    }
    return 0;
}