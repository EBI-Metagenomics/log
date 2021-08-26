#ifndef LOG_LOG_H
#define LOG_LOG_H

#define __LOG_ARRSIZE(x) (sizeof(x) / sizeof(x[0]))
#define __LOG_STRADDR(x) ((char const *)(char const(*)[__LOG_ARRSIZE(x)]){&(x)})

typedef void log_print_t(char const *msg, void *arg);
typedef void log_flush_t(void *arg);

enum
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

#define log_put(level, msg) __log_put(level, __LOG_STRADDR(msg))

#define log_debug(msg) log_put(LOG_DEBUG, msg)
#define log_info(msg) log_put(LOG_INFO, msg)
#define log_warn(msg) log_put(LOG_WARN, msg)
#define log_error(msg) log_put(LOG_ERROR, msg)
#define log_fatal(msg) log_put(LOG_FATAL, msg)

void log_flush(void);

void log_setup(int level, log_print_t *print, log_flush_t *flush, void *arg);

void __log_put(int level, char const *msg);

#endif
