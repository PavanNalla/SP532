#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>

#define LINELEN 1000
#define JOBSLEN 1000
#define JOBQLEN 100

typedef struct job
{
    int jid;
    pthread_t tid;
    char *cmd;
    char *stat;
    int estat;
    char *start;
    char *stop;
    char fnout[10];
    char fnerr[10];
} job;

typedef struct queue
{
    int size;
    job **buffer;
    int start;
    int end;
    int count;
} queue;

job job_create(char *cmd, int jid);
void total_jobs(job *jobs, int n, char *mode);

queue *queue_init(int n);
int queue_insert(queue *q, job *jp);
job *queue_delete(queue *q);
void queue_destroy(queue *q);

int get_line(char *s, int n);
int is_gap(char c);
char *left_strip(char *s);
char *new_copy(char *s);
char *new_copy_newline(char *s);
char *current_datetime_str();
char **get_args(char *line);
int open_log(char *fn);

int argValue;
int jobvalue;
job JOBS[JOBSLEN];
queue *job_queue;

void *process_job(void *arg)
{
    job *jp;
    char **args;
    pid_t pid;

    jp = (job *)arg;

    ++jobvalue;
    jp->stat = "working";
    jp->start = current_datetime_str();

    pid = fork();
    if (pid == 0) 
    {
        args = get_args(jp->cmd);
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) 
    {
        waitpid(pid, &jp->estat, WUNTRACED);
        jp->stat = "complete";
        jp->stop = current_datetime_str();
    }
    else
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    --jobvalue;
    return NULL;
}

void *process_jobs(void *arg)
{
    job *jp; 

    jobvalue = 0;
    while(2)
    {
        if (job_queue->count > 0 && jobvalue < argValue)
        {
            jp = queue_delete(job_queue);

            pthread_create(&jp->tid, NULL, process_job, jp);

            pthread_detach(jp->tid);
        }
        sleep(1); 
    }
    return NULL;
}

void main_Process()
{
    int i;
    char line[LINELEN];
    char *kw;
    char *cmd;

    i = 0;
    while (printf("Enter Command> ") && get_line(line, LINELEN) != -1)
    {
        if ((kw = strtok(new_copy(line), " \t\n\r\x0b\x0c")) != NULL)
        {
            if (strcmp(kw, "submit") == 0)
            {
                if (i >= JOBSLEN)
                    printf("Job history full; restart the program to schedule more\n");
                else if (job_queue->count >= job_queue->size)
                    printf("Job queue full; try again after more jobs complete\n");
                else
                {
                    cmd = left_strip(strstr(line, "submit") + 6);
                    JOBS[i] = job_create(cmd, i);
                    queue_insert(job_queue, JOBS + i);
                    printf("job %d added to the queue\n", i + 1);
                    i++;
                }
            }
            else if (strcmp(kw, "showjobs") == 0 ||
                     strcmp(kw, "submithistory") == 0)
                total_jobs(JOBS, i, kw);
        }
    }
    kill(0, SIGINT);
}

int main(int argc, char **argv)
{
    pthread_t tid;

    if (argc != 2)
    {
        printf("Usage: %s CONCURRENCY\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    argValue = atoi(argv[1]);
    if (argValue < 1)
        argValue = 1;
    else if (argValue > 8)
        argValue = 8;

    job_queue = queue_init(JOBQLEN);

    pthread_create(&tid, NULL, process_jobs, NULL);

    main_Process();

    exit(EXIT_SUCCESS);
}