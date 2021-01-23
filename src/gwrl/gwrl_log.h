//
// Created by Dmitry Tretyakov on 21/01/2021.
//

#pragma once

#ifndef GWRL_GWRL_LOG_H
#define GWRL_GWRL_LOG_H
#include "gwrl/shared_config.h"
#include <string.h>
#include <syslog.h>
#include <stdarg.h>

void gwrl_log_init(char *ident);
void gwrl_log_shutdown();
void gwrl_log(int level, const char *fmt, ...);
void gwrl_err(const char *fmt, ...);
void gwrl_warn(const char *fmt, ...);
void gwrl_sys_error(const char *prefix, int errnum);

#endif //GWRL_GWRL_LOG_H
