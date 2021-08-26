#include "hope/hope.h"
#include "log/log.h"

static void my_print(char const *msg, void *arg)
{
    FILE *restrict fd = arg;
    fprintf(fd, "%s\n", msg);
}

static void my_flush(void *arg)
{
    FILE *restrict fd = arg;
    fflush(fd);
}

static void test_level(int level)
{
    FILE *fd = fopen(TMPDIR "/output.txt", "w");
    NOTNULL(fd);

    log_setup(level, my_print, my_flush, fd);

    log_put(LOG_DEBUG, "LOG_DEBUG");
    log_put(LOG_INFO, "LOG_INFO");
    log_put(LOG_WARN, "LOG_WARN");
    log_put(LOG_ERROR, "LOG_ERROR");
    log_put(LOG_FATAL, "LOG_FATAL");
    log_flush();

    fclose(fd);

    fd = fopen(TMPDIR "/output.txt", "r");
    NOTNULL(fd);

    char line[32];

    if (level <= LOG_DEBUG)
    {
        NOTNULL(fgets(line, sizeof line, fd));
        line[strcspn(line, "\n")] = '\0';
        EQ(strcmp(line, "LOG_DEBUG"), 0);
    }

    if (level <= LOG_INFO)
    {
        NOTNULL(fgets(line, sizeof line, fd));
        line[strcspn(line, "\n")] = '\0';
        EQ(strcmp(line, "LOG_INFO"), 0);
    }

    if (level <= LOG_WARN)
    {
        NOTNULL(fgets(line, sizeof line, fd));
        line[strcspn(line, "\n")] = '\0';
        EQ(strcmp(line, "LOG_WARN"), 0);
    }

    if (level <= LOG_ERROR)
    {
        NOTNULL(fgets(line, sizeof line, fd));
        line[strcspn(line, "\n")] = '\0';
        EQ(strcmp(line, "LOG_ERROR"), 0);
    }

    if (level <= LOG_FATAL)
    {
        NOTNULL(fgets(line, sizeof line, fd));
        line[strcspn(line, "\n")] = '\0';
        EQ(strcmp(line, "LOG_FATAL"), 0);
    }

    fclose(fd);
}

int main(void)
{
    test_level(LOG_DEBUG);
    test_level(LOG_INFO);
    test_level(LOG_WARN);
    test_level(LOG_ERROR);
    test_level(LOG_FATAL);
    return hope_status();
}
