#include "c11threads.h"
#include "hope/hope.h"
#include "log/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int put_thrd0(void *s)
{
    log_put(LOG_INFO, "put_thrd0");
    thrd_exit(0);
    return 0;
}

static int put_thrd1(void *s)
{
    log_put(LOG_INFO, "put_thrd1");
    thrd_exit(0);
    return 0;
}

static int put_thrd2(void *s)
{
    log_put(LOG_INFO, "put_thrd2");
    thrd_exit(0);
    return 0;
}

static int put_thrd3(void *s)
{
    log_put(LOG_INFO, "put_thrd3");
    thrd_exit(0);
    return 0;
}

typedef int put_thrd(void *s);

#define TIMES 50
#define NTHREADS 4

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
    put_thrd *put_thrds[NTHREADS] = {put_thrd0, put_thrd1, put_thrd2,
                                     put_thrd3};

    thrd_t tid[NTHREADS * TIMES];
    for (int i = 0; i < NTHREADS * TIMES; ++i)
    {
        if (thrd_success != thrd_create(tid + i, put_thrds[i % NTHREADS], NULL))
        {
            return EXIT_FAILURE;
        }
    }

    for (unsigned i = 0; i < NTHREADS * TIMES; ++i)
        thrd_join(tid[i], NULL);

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
