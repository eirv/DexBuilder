/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "slicer/common.h"

#include <stdio.h>
#include <stdlib.h>

#include <set>
#include <utility>

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace slicer {

static void log_(const std::string& msg) {
#ifdef __ANDROID__
  __android_log_write(ANDROID_LOG_ERROR, "slicer", msg.c_str());
#else
  printf("%s", msg.c_str());
  fflush(stdout);
#endif
}

static logger_type log = log_;

void set_logger(logger_type new_logger) { log = new_logger; }

// Helper for the default SLICER_CHECK() policy
void _checkFailed(const char* expr, int line, const char* file) {
  StringStream ss;
  ss << endl << "SLICER_CHECK failed [";
  ss << expr << "] at " << file << ":" << line;
  ss << endl << endl;
  log(ss.str());
  abort();
}

void _checkFailedOp(const void* lhs, const void* rhs, const char* op,
                    const char* suffix, int line, const char* file) {
  StringStream ss;
  ss << endl << "SLICER_CHECK_" << suffix << " failed [";
  ss << lhs << " " << op << " " << rhs;
  ss << "] at " << file << ":" << line;
  log(ss.str());
  abort();
}

void _checkFailedOp(uint32_t lhs, uint32_t rhs, const char* op,
                    const char* suffix, int line, const char* file) {
  StringStream ss;
  ss << endl << "SLICER_CHECK_" << suffix << " failed [";
  ss << lhs << " " << op << " " << rhs;
  ss << "] at " << file << ":" << line;
  log(ss.str());
  abort();
}

// keep track of the failures we already saw to avoid spamming with duplicates
thread_local std::set<std::pair<int, const char*>> weak_failures;

// Helper for the default SLICER_WEAK_CHECK() policy
//
// TODO: implement a modal switch (abort/continue)
//
void _weakCheckFailed(const char* expr, int line, const char* file) {
  auto failure_id = std::make_pair(line, file);
  if (weak_failures.find(failure_id) == weak_failures.end()) {
    StringStream ss;
    ss << endl << "SLICER_WEAK_CHECK failed [";
    ss << expr << "] at " << file << ":";
    ss << line << endl << endl;
    log(ss.str());
    weak_failures.insert(failure_id);
  }
}

// Prints a formatted message and aborts
void _fatal(const std::string& msg) {
  log("SLICER_FATAL: " + msg);
  abort();
}

}  // namespace slicer
