/**
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This file is part of openstreetmap-cgimap (https://github.com/zerebubuth/openstreetmap-cgimap/).
 *
 * Copyright (C) 2009-2023 by the CGImap developer community.
 * For a full list of authors see the git log.
 */

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <memory>

#include "cgimap/logger.hpp"
#include <fmt/core.h>

using std::string;
using std::ostream;
using std::ofstream;



namespace logger {

static std::unique_ptr<ostream> stream;
static pid_t pid;

void initialise(const string &filename) {
  stream = std::make_unique<ofstream>(filename.c_str(), std::ios_base::out | std::ios_base::app);
  pid = getpid();
}

void message(const string &m) {
  if (stream) {
    time_t now = time(0);
    *stream << "[" << std::put_time( std::gmtime( &now ), "%FT%T") << " #" << pid << "] " << m
            << std::endl;
  }
}

}
