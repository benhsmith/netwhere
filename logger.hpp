/*
 * Copyright (c) 2017, Ben Smith
 * All rights reserved.
 *
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

/**
 * @brief simple logging function
 *
 * I'd prefer to use a more sophisticated logger, but for now...
 */

#include <time.h>
#include <iostream>
#include <string>

template<typename T>
void log(const T& message) {
  char now_str[256];
  time_t now = time(nullptr);
  strftime(now_str, sizeof(now_str), "[%F %T]", localtime(&now));

  std::cerr << now_str << " -- " << message << std::endl;
}

#ifdef ENABLE_LOGGING
#define LOG(x) log(x);
#else
#define LOG(x)
#endif

#endif

