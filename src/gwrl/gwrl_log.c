//
// Created by Dmitry Tretyakov on 21/01/2021.
//

#include "gwrl_log.h"

void gwrl_log_init(char *ident) {
    openlog(ident, LOG_PID, LOG_USER);
}
void gwrl_log_shutdown() {
    closelog();
}
void gwrl_log(int level, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vsyslog(level, format, ap);
    va_end(ap);
}
void gwrl_err(const char *format, ...) {
    va_list ap;
    gwrl_log(LOG_ERR, format, ap);
}
void gwrl_warn(const char *format, ...) {
    va_list ap;
    gwrl_log(LOG_WARNING, format, ap);
}
void gwrl_sys_error(const char * prefix, int errnum) {
    gwrl_log(LOG_ERR, "%s: (%i), %s\n",prefix,errnum, strerror(errnum));
}
