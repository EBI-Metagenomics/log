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

#define XSTR(s) STR(s)
#define STR(s) #s

#define error(msg) log_error(__FILE__ ":" XSTR(__LINE__) ":" msg)

int main(void)
{
    FILE *fd = fopen(TMPDIR "/output.txt", "w");
    NOTNULL(fd);

    log_setup(LOG_INFO, my_print, my_flush, fd);

    error("failed");
    log_flush();

    fclose(fd);

    fd = fopen(TMPDIR "/output.txt", "r");
    NOTNULL(fd);

    char line[128];

    NOTNULL(fgets(line, sizeof line, fd));
    line[strcspn(line, "\n")] = '\0';
    EQ(&line[strlen(line) - 17], "macro.c:28:failed");

    fclose(fd);

    return hope_status();
}
