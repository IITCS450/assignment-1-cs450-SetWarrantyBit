#define _POSIX_C_SOURCE 199309L
#include "common.h"
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <cmd> [args]\n",a); exit(1);}
static double d(struct timespec a, struct timespec b){
 return (b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9;}
int main(int c,char**v){
    struct timespec start, end;
    int pid;
    int stat;
    double duration;

    if(c < 2) {usage(v[0]);}
    if(clock_gettime(CLOCK_MONOTONIC, &start) < 0) {DIE("clock_gettime");}

    pid = fork();
    if(pid < 0) DIE("fork");
    if(pid == 0) { /*child*/
        execvp(v[1], &v[1]);
        DIE("execvp");
    } else { /*parent*/
        if(waitpid(pid, &stat, 0) < 0) {DIE("waitpid");}
        if(clock_gettime(CLOCK_MONOTONIC, &end) < 0) {DIE("clock_gettime");}

        duration = d(start, end);

        if(WIFEXITED(stat)) {printf("pid=%d elapsed=%.3f exit=%d\n", pid, duration, WEXITSTATUS(stat));}
        else if(WIFSIGNALED(stat)) {printf("pid=%d elapsed=%.3f exit=%d\n", pid, duration, WTERMSIG(stat));}
    }
    return 0;
}
