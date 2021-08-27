#include "hope/hope.h"
#include "log/log.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define TIMES 50
#define NTHREADS 4

static void *put_thrd(void *s)
{
    int i = (int)(long long)s;

    if (i == 0)
        log_put(LOG_INFO, "put_thrd0");
    else if (i == 1)
        log_put(LOG_INFO, "put_thrd1");
    else if (i == 2)
        log_put(LOG_INFO, "put_thrd2");
    else if (i == 3)
        log_put(LOG_INFO, "put_thrd3");

    pthread_exit(NULL);
    return NULL;
}

static void count_file(char const *restrict filename, int count[4])
{
    FILE *fd = fopen(filename, "r");
    NOTNULL(fd);

    char line[64];
    while (fgets(line, sizeof line, fd))
    {
        int i = 0;
        for (; i < NTHREADS; ++i)
        {
            char str[11] = "put_thrdX\n";
            sprintf(str, "put_thrd%d\n", i);
            if (!strcmp(line, str))
            {
                count[i]++;
                break;
            }
        }
        COND(i < NTHREADS);
    }
    EQ(ferror(fd), 0);

    fclose(fd);
}

int main(void)
{
    if (freopen(TMPDIR "/output.txt", "w", stdout) == NULL)
    {
        perror("freopen() failed");
        return EXIT_FAILURE;
    }

    pthread_t tid[NTHREADS * TIMES];
    for (int i = 0; i < NTHREADS * TIMES; ++i)
    {
        void *j = (void *)(long long)(i % NTHREADS);
        EQ(pthread_create(tid + i, NULL, &put_thrd, j), 0);
    }

    for (int i = 0; i < NTHREADS * TIMES; ++i)
        pthread_join(tid[i], NULL);

    log_flush();
    fclose(stdout);

    int count[NTHREADS] = {0, 0, 0, 0};
    count_file(TMPDIR "/output.txt", count);
    for (int i = 0; i < NTHREADS; ++i)
    {
        EQ(count[i], TIMES);
    }

    return hope_status();
}
