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

job job_create(char *cmd, int jid)
{
    job j;
    j.jid = jid;
    j.cmd = new_copy(cmd);
    j.stat = "waiting";
    j.estat = -1;
    j.start = j.stop = NULL;
    sprintf(j.fnout, "%d.out", j.jid);
    sprintf(j.fnerr, "%d.err", j.jid);
    return j;
}

void total_jobs(job *jobs, int n, char *mode)
{
    int i;
    if (jobs != NULL && n != 0)
    {
        if (strcmp(mode, "showjobs") == 0)
        {
            printf("jobid\tcommand\t\t\t\t\tstatus\n");
            for (i = 0; i < n; ++i)
            {
                if (strcmp(jobs[i].stat, "complete") != 0)
                    printf("%d\t%s\t%s\n", jobs[i].jid + 1, jobs[i].cmd, jobs[i].stat);
            }
        }
        else if (strcmp(mode, "submithistory") == 0)
        {
            printf("jobid\tcommand\t\t\t\t\tstarttime\t\t\tendtime\n");
            for (i = 0; i < n; ++i)
            {
                if (strcmp(jobs[i].stat, "complete") == 0)
                    printf("%d\t%s\t%s\t%s\t%s\n", jobs[i].jid + 1, jobs[i].cmd, jobs[i].start, jobs[i].stop, jobs[i].stat);
            }
        }
    }
}

queue *queue_init(int n)
{
    queue *q = malloc(sizeof(queue));
    q->size = n;
    q->buffer = malloc(sizeof(job *) * n);
    q->start = 0;
    q->end = 0;
    q->count = 0;

    return q;
}

int queue_insert(queue *q, job *jp)
{
    if ((q == NULL) || (q->count == q->size))
        return -1;

    q->buffer[q->end % q->size] = jp;
    q->end = (q->end + 1) % q->size;
    ++q->count;

    return q->count;
}

job *queue_delete(queue *q)
{
    if ((q == NULL) || (q->count == 0))
        return (job *)-1;

    job *j = q->buffer[q->start];
    q->start = (q->start + 1) % q->size;
    --q->count;

    return j;
}

void queue_destroy(queue *q)
{
    free(q->buffer);
    free(q);
}

int get_line(char *s, int n)
{
    int i, c;
    for (i = 0; i < n - 1 && (c = getchar()) != '\n'; ++i)
    {
        if (c == EOF)
            return -1;
        s[i] = c;
    }
    s[i] = '\0';
    return i;
}

int is_gap(char c)
{
    return (c == ' ' ||
            c == '\t' ||
            c == '\n' ||
            c == '\r' ||
            c == '\x0b' ||
            c == '\x0c');
}

char *left_strip(char *s)
{
    int i;

    i = 0;
    while (is_gap(s[i]))
        ++i;

    return s + i;
}

char *new_copy(char *s)
{
    int i, c;
    char *copy;

    i = -1;
    copy = malloc(sizeof(char) * strlen(s));
    while ((c = s[++i]) != '\0')
        copy[i] = c;
    copy[i] = '\0';

    return copy;
}

char *new_copy_newline(char *s)
{
    int i, c;
    char *copy;

    i = -1;
    copy = malloc(sizeof(char) * strlen(s));
    while ((c = s[++i]) != '\0' && c != '\n')
        copy[i] = c;
    copy[i] = '\0';

    return copy;
}

char *current_datetime_str()
{
    time_t tim = time(NULL);
    return new_copy_newline(ctime(&tim));
}

char **get_args(char *line)
{
    char *copy = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(copy, line);

    char *arg;
    char **args = malloc(sizeof(char *));
    int i = 0;
    while ((arg = strtok(copy, " \t")) != NULL)
    {
        args[i] = malloc(sizeof(char) * (strlen(arg) + 1));
        strcpy(args[i], arg);
        args = realloc(args, sizeof(char *) * (++i + 1));
        copy = NULL;
    }
    args[i] = NULL;
    return args;
}

int open_log(char *fn)
{
    int fd;
    if ((fd = open(fn, O_CREAT | O_APPEND | O_WRONLY, 0755)) == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    return fd;
}