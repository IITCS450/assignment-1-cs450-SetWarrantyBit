/*
#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <pid>\n",a); exit(1);}
static int isnum(const char*s){for(;*s;s++) if(!isdigit(*s)) return 0; return 1;}
int main(int c,char**v){
 if(c!=2||!isnum(v[1])) usage(v[0]);
 printf("TODO: implement procinfo\nTesting if remote ssh works\n");
 return 0;
}
*/
/* src/procinfo.c */
#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

static void usage(const char *a){fprintf(stderr,"Usage: %s <pid>\n",a); exit(1);}
static int isnum(const char*s){for(;*s;s++) if(!isdigit(*s)) return 0; return 1;}

int main(int c, char** v){
    /*init vars*/
    char path[256];
    char buffer[1024];
    FILE *f_status;
    char state = 'X';
    int ppid = 0;
    long vmrss = 0;
    FILE *f_cmd;
    size_t len;
    size_t i;
    FILE *f_stat;
    char *toks;
    unsigned long utime = 0; 
    unsigned long stime = 0;
    long clk_tck;
    double total_seconds;
    
    if(c!=2 || !isnum(v[1])) usage(v[0]);
    /* c    num of args
       v[0] ./bin/procinfo
       v[1] pid
       v[2] end of array (NULL) */

    /* print pid (v[1]) */
    printf("PID:%s\n", v[1]);

    /* open /proc/<pid>/status for State, PPid, VmRSS
       path = /proc/(pid)/status */
    snprintf(path, sizeof(path), "/proc/%s/status", v[1]);
    f_status = fopen(path, "r");    /* read (path) */
    if (!f_status) {    /* failed to open f_status */
        DIE_MSG("Err. Check PID");
    }



    while (fgets(buffer, sizeof(buffer), f_status)) {
        /* if line shows state */
        if (strncmp(buffer, "State:", 6) == 0) {
            char *p = strchr(buffer, ':');
            if (p) {
                while (isspace(*++p)){};
                state = *p;
            }
        }

        /* if line shows parent pid */
        else if (strncmp(buffer, "PPid:", 5) == 0) {
            sscanf(buffer, "PPid: %d", &ppid);
        }

        /* if line shows VmRSS */
        else if (strncmp(buffer, "VmRSS:", 6) == 0) {
            sscanf(buffer, "VmRSS: %ld", &vmrss);
        }
    }
    fclose(f_status);    /* close f_status */
    /* print stuff, but VmRSS for later */
    printf("State: %c\n", state);
    printf("PPID: %d\n", ppid);

    /* open /proc/(pid)/cmdline*/
    snprintf(path, sizeof(path), "/proc/%s/cmdline", v[1]);
    f_cmd = fopen(path, "r");
    if (f_cmd) {
        /* replace \0 with spaces in cmdline */
        len = fread(buffer, 1, sizeof(buffer)-1, f_cmd);
        buffer[len] = '\0';
        fclose(f_cmd);
        
        if (len > 0) {
            for (i = 0; i < len; i++) {
                if (buffer[i] == '\0') buffer[i] = ' '; 
            }
            printf("Cmd: %s\n", buffer);
        } else {
             /* cmdline empty */
             printf("Cmd: \n"); 
        }
    } else {
        printf("Cmd: \n");
    }

    /* Open /proc/<pid>/stat */
    snprintf(path, sizeof(path), "/proc/%s/stat", v[1]);
    f_stat = fopen(path, "r");
    if (f_stat) {
        if (fgets(buffer, sizeof(buffer), f_stat)) {
            /* find last ) */
            toks = strrchr(buffer, ')');
            if (toks) {
                utime = 0, stime = 0;
                sscanf(toks + 2, "%*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
                
                clk_tck = sysconf(_SC_CLK_TCK);
                total_seconds = (double)(utime + stime) / clk_tck;
                printf("CPU: %lu %.3f\n", utime + stime, total_seconds);
            }
        }
        fclose(f_stat);
    }

    /* Print VmRSS */
    printf("VmRSS: %ld\n", vmrss);

    return 0;
}