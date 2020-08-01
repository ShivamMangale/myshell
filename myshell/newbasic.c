#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>


char initdir[PATH_MAX];
int bgflag = 0;
int co = 0;
int pflag = 0;
int lrflag = 0;
int rrflag = 0;
int dlrflag = 0;
int drrflag = 0;
int status;
int runningcount = 0;
int jobtopid[200];
int c_pid = 0;

int pidlist[100];
int pidtojob[40000];
char joblist[1000][200];
char dir[PATH_MAX];
char cwd[PATH_MAX];
char hostname[1000+1];
int num;
int tempstatus[200];
char sysname[1001];
int c1 = 0;
int jobcount=0;
int errr = 0;

void procExit()
{
    int i = 0;
    while(i <= jobcount - 1)
    {
        if (waitpid(pidlist[i], &status, WNOHANG) > 0)
        {
            if (WIFEXITED(status) > 0)
            {
                tempstatus[i]=0;
                printf("%s with pid = %d has exited normally\n", joblist[i], pidlist[i]);
            }
            else if (WIFSIGNALED(status) != 0)
            {
                tempstatus[i]=0;
                printf("%s with pid = %d has exited with signal\n",joblist[i], pidlist[i]);
            }
            else
            {
                errr++;
                printf("%s with pid = %d has exited abnormally\n", joblist[i], pidlist[i]);
            }
        }
        ++i;
    }
}

void sigHandler(int type)
{
    if(c_pid == 0)  return;
    else    kill(c_pid, type);
}

char **tokenize(char *str)
{
    char *inp=malloc(PATH_MAX);
    strcpy(inp, str);
    char *token;
    token = strtok(inp, " \n");
    int i=0;
    char **args = malloc(100000);
    while(token != NULL)
    {
        args[i] = realloc(args[i], strlen(token));
        strcpy(args[i], token);
        token = strtok(NULL, " \n");
        ++i;
    }
    return args;
}
int counttot = 0;
char **tokencmd(char *line)
{
    char *inp=malloc(PATH_MAX);
    strcpy(inp, line);

    char *token=strtok(inp, ";");
    c1 = 0;
    char **args = malloc(100000);
    counttot = 0;
    while(token!=NULL){
        args[c1]=malloc(strlen(token));
        counttot++;
        strcpy(args[c1], token);
        c1++;
        token=strtok(NULL, ";");
    }
    // printf("%d\n",counttot);
    counttot = 0;
    return args;
}

char *replaceWord(const char *s, const char *oldW, const char *newW) 
{ 
    char *result; 
    int i, cnt = 0; 
    int newWlen = strlen(newW); 
    int oldWlen = strlen(oldW); 
  
    
    for (i = 0; s[i] != '\0'; i++) 
    { 
        if (strstr(&s[i], oldW) == &s[i]) 
        { 
            cnt++; 
  
            i += oldWlen - 1; 
        } 
    } 
  
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1); 
  
    i = 0; 
    while (*s) 
    { 
        if (strstr(s, oldW) == s) 
        { 
            strcpy(&result[i], newW); 
            i += newWlen; 
            s += oldWlen; 
        } 
        else
            result[i++] = *s++; 
    } 
  
    result[i] = '\0'; 
    return result; 
} 

void pinfo(char **args)
{
	char path[PATH_MAX];
    char l1[100];
    char l2[100];
    char *ptr1 = malloc(10000);
    char *ptr2 = malloc(10000);
    int pid;
    
    char *str2 = malloc(PATH_MAX);
    if(args[1]){
        strcpy(str2, args[1]);
        printf("pid -- %s\n", args[1]);
        pid = atoi(args[1]);
    }
    else{
        pid = getpid();
        printf("pid -- %d\n", pid);
        sprintf(str2, "%d", pid);
    }
    char *str1 = malloc(10000);
    char prostr[] = "/proc/";
    strcpy(str1,prostr);

    char *str4 = malloc(10000);
    char stastr[] = "/status";
    strcpy(str4,stastr);
    strcat(str1, str2);
    strcat(str1, str4);
  	FILE *statfile1;
    statfile1 = fopen(str1, "r");
    if(statfile1 == 0)				
    {
        perror("open() error");
        return ;
    }
    int workdone = 0;
    while(fgets(l1, 100, statfile1) && workdone == 0)
    {
        if(strncmp(l1, "State:", 6) == 0);
        else					 continue;
        ptr1 = l1;
        ptr1 += 7;
        while(isspace(*ptr1)) ++ptr1;

        printf("Process Status -- %c\n", ptr1[0]);
        workdone = 1;
    }
     
    FILE *statfile2;
    statfile2 = fopen(str1, "r");
    if(statfile2<0)				
    {
        perror("open() error");
        return ;
    }
    workdone = 0;
    while(fgets(l2, 100, statfile2) && workdone == 0)
    {
        if(strncmp(l2, "VmSize:", 7) == 0);
        else					continue;
        ptr2 = l2;
        ptr2 += 8;
        while(isspace(*ptr2)) ++ptr2;

        printf("memory -- %s", ptr2);
        workdone = 1;
    }
    
    char buf1[1024];
     
    sprintf(path, "/proc/%d/exe", pid);
     
    ssize_t ret = readlink(path, buf1, 1024);
    char final[100];
    if (ret <= 0)  perror("Readlink");
    else
    {
        buf1[ret] = 0;
        if(strstr(buf1, initdir))
        {
        	char empt[] = "";
            replaceWord(buf1, initdir,empt);
            printf("Executable Path -- ~%s\n", buf1);
        }
        else printf("Executable Path -- %s\n", buf1);
    }
}   

void changedir(char *str)
{
	
	char pat[400];
	char delim[] = " ";
	int flag = 0;
		
	int c = 0;
	for(int i=0;i<strlen(str);++i)
	{
		if(flag == 1)
		{
			if(isspace(str[i]))
			{
				break;
			}
			pat[c++] = str[i];
		}
		else if(isspace(str[i]))	flag = 1;
	}
	pat[c] = 0;
	
	char home[] = "~";
	if(strlen(pat)==0 || strcmp(pat,home)==0)	strcpy(pat,cwd);
	if(strstr(pat,home)!=NULL)	strcpy(pat,replaceWord(pat,home,cwd));
	chdir(pat);
}

int clean_whitespace(const char *in, char *out)
{
    int len, count=0, i;
    int flag = 0;
    char delim[] = " ";
    int enc = 0;
    if((in) && (out))
    {
        len = strlen(in);
        for(i=0;i<len;i++)
        {
            if(!isspace(in[i]))
            {
                out[count++] = in[i];
                flag = 0;
                enc = 1;
            }
            else
            {
            	if(flag == 0 && enc == 1)
            	{
            		flag = 1;
            		out[count++] = delim[0];
            	}
            }
        }
        out[count]=0;
    }
    return count;
}

char* permissions(char *file)
{
    struct stat st;
    char *permissionbit = malloc(sizeof(char) * 9 + 1);
    if(stat(file, &st) == 0){
        mode_t perm = st.st_mode;
        permissionbit[0] = (perm & S_IRUSR) ? 'r' : '-';
        permissionbit[1] = (perm & S_IWUSR) ? 'w' : '-';
        permissionbit[2] = (perm & S_IXUSR) ? 'x' : '-';
        permissionbit[3] = (perm & S_IRGRP) ? 'r' : '-';
        permissionbit[4] = (perm & S_IWGRP) ? 'w' : '-';
        permissionbit[5] = (perm & S_IXGRP) ? 'x' : '-';
        permissionbit[6] = (perm & S_IROTH) ? 'r' : '-';
        permissionbit[7] = (perm & S_IWOTH) ? 'w' : '-';
        permissionbit[8] = (perm & S_IXOTH) ? 'x' : '-';
        permissionbit[9] = '\0';
        return permissionbit;     
    }
    else{
        return strerror(errno);
    }   
}


void pathpwd()
{
	char curr[1001];
	getcwd(curr, sizeof(curr));
	printf("%s\n",curr);
}

void doecho(char *str)
{
	printf("%s\n",str+5);
}

void getpinfo(char *str)
{
	
	char **args;
	args = tokenize(str);
	pinfo(args);
}

void dols(char *str)
{
	
	int aflag = 0;
	int lflag = 0;
	int dirflag = 0;

	if(strstr(str,"-a")	!= NULL)	aflag = 1;
	if(strstr(str,"-al") != NULL)	aflag=lflag=1;
	if(strstr(str,"-l")	!= NULL)	lflag = 1;
	if(strstr(str,"-la") != NULL)	aflag=lflag=1;

	char pat[400];
	DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;
    char old[400];
    getcwd(old,sizeof(old));
    if(aflag == 1 || lflag == 1 || strlen(str) < 4)	getcwd(pat,sizeof(pat));
    else
    {
    	char delim[] = " ";
		int flag = 0;
		
		int c = 0;
		for(int i=0;i<strlen(str);++i)
		{
			if(flag == 1)
			{
				if(isspace(str[i]))
				{
					break;
				}
				pat[c++] = str[i];
			}
			else if(isspace(str[i]))	flag = 1;
		}
		pat[c] = 0;
        char home[] = "~";
        dirflag = 1;
        char curr[400];
        if(strlen(pat)==0 || strcmp(pat,home)==0)   strcpy(pat,cwd);
	    else
        {
           strcpy(curr,cwd);
	       strcat(curr,"/");
	       strcat(curr,pat);
	       strcpy(pat,curr);
	    }
        chdir(pat);	
	}
    char buf[512];
    getcwd(pat,sizeof(pat));
    mydir = opendir(pat);
    int co = 0;
    char* ans = malloc(sizeof(char) * 9 + 1);

    while((myfile = readdir(mydir)) != NULL)
    {
    	struct stat stats;
    	struct tm dt;
    	if (stat(myfile->d_name, &stats) == 0)
	    {
	    	if((aflag == 0 && co<2))	;
	    	else if(lflag)
	    	{
				ans = permissions(myfile->d_name);
				
				printf("\n%s \t",ans);
				
				struct passwd *pw = getpwuid(stats.st_uid);
				struct group  *gr = getgrgid(stats.st_gid);
				printf("%d\t",stats.st_nlink);
				printf("%s\t",pw->pw_name);
				printf("%s\t",gr->gr_name);
		    	printf("%lld\t",mystat.st_size);
		        dt = *(gmtime(&stats.st_mtime));
		    	printf("%d-%d %d:%d\t\t", dt.tm_mday, dt.tm_mon, dt.tm_hour, dt.tm_min);
	        }
	        // else	printf("\n");
	        sprintf(buf, "%s/%s", pat, myfile->d_name);
	        stat(buf, &mystat);
	        if(aflag == 0 && co<2);
	        else	printf(" %s ", myfile->d_name);
	        co++;
	    }
	    else
	    {
	    	perror("File");
	        printf("Unable to get file properties.\n");
	        printf("Please check whether '%s' file exists.\n",myfile->d_name);
	    }
    	
    }
    printf("\n");
    closedir(mydir);
	chdir(old);	
}


void dosyscall(char *str)
{	
	execl("/bin/sh", "/bin/sh", "-c", str, 0);
}

void callpro(char *str)
{
    char **args;
    int flag=1;
    args = tokenize(str);
    int status;
    char *app=args[0];
    pid_t pid=fork();
    if(str[strlen(str)-1]!='&' && str[strlen(str) - 2] != '&')
    {
        flag=0;
    }
    int pos = strlen(str);
    pos--;
    pos--;
    str[pos]='\0';
    if(pid==0)
    {
        setpgid(0,0);
        if(execvp(app,args)>=0) return;
        else    perror("error in execution");
    }
    else
    {
        if(flag == 1)
        {
            strcpy(joblist[jobcount],app);
            pidlist[jobcount++]=(int)pid;
            return;
        }
        else{
            c_pid = (int)pid;
            waitpid(pid,&status,WUNTRACED);
            if(WIFSTOPPED(status))
            {
                strcpy(joblist[jobcount], app);
                pidlist[jobcount++]=(int)pid;
            }
        }
    }
    return;
}

void dopipe(char *str)
{
    printf("in do pipe\n");
    int savestdout=dup(1);
    int savestdin=dup(0);
    // char str[110];
    // scanf("%[^\n]%*c", str); 
    int pflag = 0;
    for(int i=0;i<strlen(str);++i)  if(str[i] == '|')   pflag++;

    char **args = malloc(1000000);
    char* token = strtok(str, "|");
    int co = -1;
    while(token != NULL)
    {
        co++;
        args[co] = realloc(args[co], strlen(token));
        strcpy(args[co],token);
        // printf("%s\n",args[co]);
        token = strtok(NULL,"|");
    }

    for(int i=0;i<=co;++i)
    {
        int len = strlen(args[i]);
        char out[len+1];
        int count = clean_whitespace(args[i], out);
        strcpy(args[i],out);
    }


    for(int i=0;i<co;++i)
    {
        int fd[2];
        pipe(fd);
        if (!fork()) {
            dup2(fd[1], 1); // remap output back to parent
            execl("/bin/sh", "/bin/sh", "-c", args[i], 0);
            perror("exec");
             }

        else{
            dup2(fd[0], 0);
            close(fd[1]);
        }

        close(fd[0]);
        close(fd[1]);
    }
    dup2(savestdout,1);
    pid_t pid;
    int status;

    if ((pid = fork()) < 0)
        perror("fork() error");
    else if (pid == 0) {
        dosyscall(args[co]);             
    }
    else
    {
        waitpid(pid,&status,0);
    }
    dup2(savestdin,0);

}

void doredir(char *str)
{
    printf("in do redir\n");
    int savedstdin = dup(0);
    int savedstdout = dup(1);

    char **args = malloc(1000000);

    int lflag = 0;
    int rflag = 0;
    int len = strlen(str);
    for(int i=0;str[i] != 0;++i)
    {
        if(str[i] == '<')   lflag = 1;
        else if(str[i] == '>')
        {
            if(str[i+1] == '>') rflag = 2;
            else                rflag = rflag > 1?rflag : 1;
        }
    }

    char *token;
    if(lflag == 1 && rflag == 0)    //cat < t1.txt
    {
        token = strtok(str,"<");
        if(token == NULL)
        {
            printf("Invalid input\n");
            return ;
        }
        args[0] = realloc(args[0], strlen(token));
        strcpy(args[0],token);
        token = strtok(NULL,"<");
        args[1] = realloc(args[1], strlen(token));
        strcpy(args[1],token);

        for(int i=0;i<=1;++i)
        {
            int len = strlen(args[i]);
            char out[len+1];
            int count = clean_whitespace(args[i], out);
            strcpy(args[i],out);
        }
        
        int in = open(args[1],O_RDONLY);
        dup2(in,0);
        pid_t pid;
        int status;

        if ((pid = fork()) < 0)
            perror("fork() error");
        else if (pid == 0) {
            execl("/bin/sh", "/bin/sh", "-c", args[0], 0); 
        }
        else
        {
            waitpid(pid,&status,0);
        }

        dup2(savedstdout,1);
        dup2(savedstdin,0);
    }
    if(rflag > 0 && lflag == 0)     // cat t1.txt > rand.txt
    {
        char com[3];
        if(rflag == 2)  strcpy(com,">>");
        else            strcpy(com,">");
        token = strtok(str,com);
        if(token == NULL)
        {
            printf("Invalid input\n");
            return ;
        }
        args[0] = realloc(args[0], strlen(token));
        strcpy(args[0],token);
        token = strtok(NULL,com);
        args[1] = realloc(args[1], strlen(token));
        strcpy(args[1],token);

        for(int i=0;i<=1;++i)
        {
            int len = strlen(args[i]);
            char out[len+1];
            int count = clean_whitespace(args[i], out);
            strcpy(args[i],out);
        }
        
        int out;
        if(rflag == 1)  out = open(args[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR,644);
        else            out = open(args[1], O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR,644);
        
        dup2(out,1);
        pid_t pid;
        int status;

        if ((pid = fork()) < 0)
            perror("fork() error");
        else if (pid == 0) {
            execl("/bin/sh", "/bin/sh", "-c", args[0], 0); 
        }
        else
        {
            waitpid(pid,&status,0);
        }

        dup2(savedstdout,1);
        dup2(savedstdin,0);
    }
    if(lflag == 1 && rflag > 0)     // cat < t1.txt > rand.txt
    {
        char com[3];
        if(rflag == 2)  strcpy(com,">>");
        else            strcpy(com,">");

        token = strtok(str,"<");
        if(token == NULL)
        {
            printf("Invalid input\n");
            return ;
        }
        args[0] = realloc(args[0], strlen(token));
        strcpy(args[0],token);
        token = strtok(NULL,com);
        args[1] = realloc(args[1], strlen(token));
        strcpy(args[1],token);
        token = strtok(NULL,com);
        args[2] = realloc(args[2], strlen(token));
        strcpy(args[2],token);
        for(int i=0;i<=2;++i)
        {
            int len = strlen(args[i]);
            char out[len+1];
            int count = clean_whitespace(args[i], out);
            strcpy(args[i],out);
        }

        args[1][strlen(args[1])-1] = 0;
        int in = open(args[1],O_RDONLY);
        int out;
        if(rflag == 1)        out = open(args[2], O_WRONLY | O_TRUNC | O_CREAT,0644);
        else if(rflag == 2)   out = open(args[2], O_APPEND | O_WRONLY | O_CREAT,0644);
        pid_t pid;
        int status;
        dup2(in,STDIN_FILENO);
        
        dup2(out,STDOUT_FILENO);
        
        pid = fork();
        if(pid == 0) {
            execl("/bin/sh", "/bin/sh", "-c", args[0], (char*)0); 
            abort();
        }
        else
        {
            waitpid(pid,&status,0);
            dup2(savedstdout,1);
            dup2(savedstdin,0); 
        }
    }
}

void jobs()
{
    num = 0;
    char filetoread[200], status[20];
    for(int k=0; k<jobcount; k++)
    {
        sprintf(filetoread, "/proc/%d/stat", pidlist[k]);
        char itr[100];
        int fd  = open(filetoread, O_RDONLY);
        if(fd != -1)    
        {
            read(fd, itr, 100);
            close(fd);
            int j = 0;
            for(int i = 0; i < 100 ; i++)
            {
                if(j == 2){
                    if(itr[i] == 'T')           strcpy(status, "Stopped");
                    else if(itr[i] == 'S')      
                        {
                            strcpy(status, "Running");
                            runningcount++;
                        }
                    else if(itr[i] == 'R')      
                        {
                            strcpy(status,"Running");
                            runningcount++;
                        }
                    else if(itr[i] == 'Z')      strcpy(status, "Killed");
                    else                        strcpy(status, "Unknown");
                    break;
                }
                if(itr[i]!=' ');
                else    j++;
            }
        }
        else        strcpy(status, "UNKNOWN");    
        if(strcmp(status, "UNKNOWN") && strcmp(status, "Killed"))
        {
            printf("[%d] %s %s [%d]\n",num, status, joblist[k], pidlist[k]);
            num++;
        }
    }
}

void kjob(char *str)
{
    char filetoread[200], status[10];
    char **args;
    args = tokenize(str);
    int index = atoi(args[1]);
    int pidx = 0;
    int num = 0;
    for(int k=0; k<jobcount; k++)
    {
        sprintf(filetoread, "/proc/%d/stat", pidlist[k]);
        char itr[100];
        int fd  = open(filetoread, O_RDONLY);
        if(fd != -1) 
        {
            read(fd, itr, 100);
            close(fd);
            int j = 0;
            for(int i = 0; i < 100 ; i++)
            {
                if(j == 2)
                { 
                    if(itr[i] == 'T')           strcpy(status, "Stopped");
                    else if(itr[i] == 'S')     
                    {
                        strcpy(status, "Running");
                        runningcount++;
                    }
                    else if(itr[i] == 'Z')      strcpy(status, "Killed");
                    else if(itr[i] == 'R')     
                    {
                        strcpy(status, "Running");
                        runningcount++;
                    }
                    else                        strcpy(status, "Unknown");
                    break;
                }

                if(itr[i]!=' ');
                else    j++;
            }
        }
        else               strcpy(status, "UNKNOWN");
        if(strcmp(status, "UNKNOWN") && strcmp(status, "Killed"))   num++;
        if(num!=index);
        else
        {
            pidx = pidlist[k];
            break;
        }
    }

    kill(pidx, atoi(args[2]));
}

void fg(char *str)
{
    char **args;
    args = tokenize(str);
    pid_t pidx = 0;
    int index = atoi(args[1]);
    num = 0;
    char filetoread[200], status[100];
    for(int k=0; k<jobcount; k++)
    {
        sprintf(filetoread, "/proc/%d/stat", pidlist[k]);
        char itr[100];
        int fd  = open(filetoread, O_RDONLY);
        if(fd != -1)           
        {
            read(fd, itr, 100);
            close(fd);
            int j = 0;
            for(int i = 0; i < 100 ; i++)
            {
                if(j == 2)
                {
                    if(itr[i] == 'T')           strcpy(status, "Stopped");
                    else if(itr[i] == 'S')      
                        {
                            strcpy(status, "Running");
                            runningcount++;
                        }
                    else if(itr[i] == 'Z')      strcpy(status, "Killed");
                    else if(itr[i] == 'R')      strcpy(status, "Running");
                    else                        strcpy(status, "Unknown");
                    break;
                } 
                if(itr[i]!=' ');
                else    j++;
            }
            if(strcmp(status, "Killed") && strcmp(status, "UNKNOWN"))           num++;
            if(num!=index);
            else
            {
                pidx = pidlist[k];
                break;
            }
        }
        else          strcpy(status, "UNKNOWN");   
    }
    c_pid=pidx;
    kill(pidx, SIGCONT);
    int stat,wpid;

    wpid=waitpid(pidx,&stat,WUNTRACED);
    c_pid=0;
    
}

void bg(char *str)
{
    char **args;
    args = tokenize(str);
    char filetoread[200], status[20];
    int index = atoi(args[1]);
    int pidx = 0;
    num = 0;
    for(int k=0; k<jobcount; k++)
    {
        sprintf(filetoread, "/proc/%d/stat", pidlist[k]);
        char itr[100];
        int fd  = open(filetoread, O_RDONLY);
        if(fd != -1)
        {
            read(fd, itr, 100);
            close(fd);
            int j = 0;
            for(int i = 0; i < 100 ; i++){
                if(j == 2)
                {
                    if(itr[i] == 'T')           strcpy(status, "Stopped");
                    else if(itr[i] == 'S')      strcpy(status, "Running");
                    else if(itr[i] == 'Z')      strcpy(status, "Killed");
                    else if(itr[i] == 'R')      
                        {
                            strcpy(status, "Running");
                            runningcount++;
                        }
                    else                        strcpy(status, "Unknown");
                    break;
                }
                if(itr[i]!=' ');
                else    j++;
            }
            if(strcmp(status, "Killed") && strcmp(status, "UNKNOWN"))       num++;
            if(num!=index);
            else
            {
                pidx = pidlist[k];
                break;
            }
        }
        else                 strcpy(status, "UNKNOWN");
    }

    kill(pidx, SIGCONT);
}

void overkill()
{
    int i = 0;
    int sign = 9;
    while(i <= jobcount - 1)
    {
        kill(pidlist[i], sign); 
        i++;       
    }
}

void sete(char *str)
{
    char **args = malloc(1000000);
    char* varname;
    char* value;
    value = "";
    varname = "";

    char* token = strtok(str, " "); 
    if(token == NULL)
    {
        printf("invalid input\n");
        return ;
    }
    
    token = strtok(NULL, "["); 
    if(token == NULL)
    {
        printf("Invalid input\n");
        return ;
    }
    args[0] = realloc(args[0], strlen(token));
    strcpy(args[0],token);
    token = strtok(NULL, "]");
    if(token == NULL)   args[1] = "";
    else
    {
        args[1] = realloc(args[1], strlen(token)); 
        strcpy(args[1],token);
    }

    int flag = setenv(args[0],args[1],1);
    if(flag < 0)    printf("Error\n");
}

void unsete(char *str)
{
    char **args = malloc(1000000);
    char* token = strtok(str, " "); 
    if(token == NULL)
    {
        printf("invalid input\n");
        return ;
    }

    token = strtok(NULL, " "); 
    if(token == NULL)
    {
        printf("Invalid input\n");
        return ;
    }
    args[0] = realloc(args[0], strlen(token));
    strcpy(args[0],token);
    int flag = unsetenv(args[0]);
    if(flag < 0)    printf("Error\n");
}

void doboth(char *str)
{
    printf("in do both\n");
    int savedstdout=dup(1);
    int savedstdin=dup(0);

    int pflag = 0;
    int lflag = 0;
    int pos = strlen(str);
    for(int i=0;i<strlen(str);++i)  
        {
            if(str[i] == '|')   pflag++;
            if(str[i] == '<')   lflag = 1;
        }

    printf("%d===lflag\n",lflag);
    char **args = malloc(100000);
    char *inp = malloc(1000);
    inp = realloc(inp, strlen(str));
    strcpy(inp,str);
    int rflag = 0;
    for(int i=0;i<strlen(inp);++i)
    {
        if(inp[i] == '>')
        {
            if(inp[i+1] == '>') rflag = 2;
            else                rflag = 1;
            pos = i;
            break;
        }
    }
    char *token;
    // printf("he\n");
    if(lflag == 0)
    {
        // printf("here\n");
        token = strtok(str,"|");
        args[0] = realloc(args[0], strlen(token));
        strcpy(args[0],token);
        // printf("lflag done\n");
    }
    else
    {
        token = strtok(str,"<");
        args[0] = realloc(args[0], strlen(token));
        strcpy(args[0],token);
        token = strtok(NULL,"|");
        int len = strlen(token);
        char out[len+1];
        int count = clean_whitespace(token, out);
        strcpy(token,out);
        token[strlen(token) - 1] = 0;
        int in = open(args[1],O_RDONLY);
        dup2(in,STDIN_FILENO);
    }
    // printf("after lflag\n");
    token = strtok(NULL,"|");
    while(token != NULL)
    {
        co++;
        args[co] = realloc(args[co], strlen(token));
        strcpy(args[co],token);
        // printf("%s\n",args[co]);
        token = strtok(NULL,"|");
    }
    if(rflag != 0)
    {
        args[co] = strtok(args[co],">");
        // printf("%s\n",args[co]);

    }
    
    char *repl = malloc(1000);
    if(rflag != 0)
    {
        char *trav = strtok(inp,">");
        trav = strtok(NULL,">");
        repl = realloc(repl,strlen(trav));
        strcpy(repl,trav);
        int len = strlen(repl);
        char out[len+1];
        int count = clean_whitespace(repl, out);
        strcpy(repl,out);
    }
    // for(int i=0;i<strlen(inp);++i)
    // {
    //     if(inp[i] == '>')
    //     {
    //         inp[i] = ' ';
    //         inp[i+1] = 0;
    //         break;
    //     }
    // }    
   
    for(int i=0;i<=co;++i)
    {
        int len = strlen(args[i]);
        char out[len+1];
        int count = clean_whitespace(args[i], out);
        strcpy(args[i],out);
        // args[i][strlen(args[i])-1] = 0;
    }
    // printf("co=%d\n",co);
    // for(int i=0;i<=co;++i)  printf("%s\n",args[i]);
    // printf("%s\n",repl);
    int out = 0;
    if(rflag == 1)      out = open(repl, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR,644);
    else if(rflag == 2) out = open(repl, O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR,644);    
    else                out = savedstdout;
    // printf("%d\n",out);
    for(int i=0;i<co;++i)
    {
        int fd[2];
        pipe(fd);
        if (!fork()) {
            dup2(fd[1], 1); // remap output back to parent
            execl("/bin/sh", "/bin/sh", "-c", args[i], 0);
            perror("exec");
             }

        else{
            dup2(fd[0], 0);
            close(fd[1]);
        }

        close(fd[0]);
        close(fd[1]);
    }
    dup2(savedstdout,1);
    pid_t pid;
    int status;

    if ((pid = fork()) < 0)
        perror("fork() error");
    else if (pid == 0) {
        dup2(out,1);
        dosyscall(args[co]);             
    }
    else
    {
        waitpid(pid,&status,0);
        dup2(savedstdout,1);
        dup2(savedstdin,0); 
    }

}

void shell()
{
	char *input;
    size_t bufsize = 64;
    size_t ch;

	
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
	   } else {
	       perror("getcwd() error");
	       return;
	}
	strcpy(initdir,cwd);
	struct utsname buffer;

	int errn = 0;
   	if (uname(&buffer) != 0) {
      perror("uname");
      return;  
   	}

   	
   	strcpy(sysname,buffer.sysname);
   	int noerror = 0;
   	
	if(gethostname(hostname, sizeof(hostname)) != 0)	
	{
		perror("Hostname");
		return ;
	}
	char home[] = "~";
    for(int i=0;i<40000;++i)
    {
        jobtopid[i%200] = -1;
        pidtojob[i] = -1;
    }
	while(!noerror)
	{
        signal(SIGINT, sigHandler);
        signal(SIGTSTP, sigHandler);
		getcwd(dir, sizeof(dir));
		if(strcmp(cwd,dir)==0)	strcpy(dir,"~");
		else if(strstr(dir,cwd)!=NULL)	strcpy(dir,replaceWord(dir, cwd, home)); 

		printf("<%s@%s:%s>",hostname,sysname,dir);
		
		char *input; 
        input = readline("");
        add_history(input);
    
   		int fl = 0;
   		int j = 0;
   		
   		char str[400];
   		int len = strlen(input);
    	char out[len+1];
    	int count = clean_whitespace(input, out);
   		strcpy(str,out);
   		
   		char **com=tokencmd(str);
        for(int i =0 ; i < c1; i++)
        {
            signal(SIGTSTP, sigHandler);
            signal(SIGINT, sigHandler);
            bgflag = 0;
            pflag = 0;
            lrflag = 0;
            rrflag = 0;
            dlrflag = 0;
            drrflag = 0;
            strcpy(str,com[i]);
            for(int i=0;i<strlen(str);++i)
                {
                    if(str[i] == '|')   ++pflag;
                    else if(str[i] == '<')
                    {
                        if(str[i+1] == '<') ++dlrflag;
                        else                ++lrflag;
                    }
                    else if(str[i] == '>')
                    {
                        if(str[i+1] == '>') ++drrflag;
                        else                ++rrflag;
                    }
                }
                // printf("%d %d %d\n",pflag,lrflag,rrflag);
        signal(SIGCHLD,procExit);
        c_pid = 0;
        if(str[strlen(str) - 1] == '&' || str[strlen(str) - 2] == '&')  bgflag = 1;
        if(pflag && (rrflag || drrflag || lrflag) == 0)         dopipe(str);
        else if((rrflag || drrflag || lrflag) && pflag == 0)    doredir(str);
        else if(pflag != 0 && (rrflag || drrflag || lrflag))    doboth(str);
		else if(strcmp(str,"quit")==0 || strcmp(str,"quit ")==0)	noerror = 1;
		else if(strncmp(str,"cd",2)==0 && bgflag == 0)		changedir(str);
		else if(strncmp(str,"pwd",3)==0 && bgflag == 0)	    pathpwd();
		else if(strncmp(str,"echo",4)==0 && bgflag == 0)	doecho(str);
		else if(strncmp(str,"pinfo",5)==0 && bgflag == 0)	getpinfo(str);
		else if(strncmp(str,"ls",2)==0 && bgflag == 0)		dols(str);
        else if(strncmp(str,"set",3)==0)                    sete(str);
        else if(strncmp(str,"unset",5)==0)                  unsete(str);
        else if(strncmp(str,"overkill",8)==0)               overkill();
        else if(strncmp(str,"jobs",4)==0)                   jobs();
        else if(strncmp(str,"kjob",4)==0)                   kjob(str);
        else if(strncmp(str,"fg",2)==0)                     fg(str);
        else if(strncmp(str,"bg",2)==0)                     bg(str);
		else						
		{
			if(strlen(str) == 0)	continue;
			callpro(str);
		}
		}
	}
	
}

int main()
{
    signal(SIGTSTP, sigHandler);
    signal(SIGINT, sigHandler);
	shell();

	return 0;
}