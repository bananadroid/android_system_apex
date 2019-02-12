/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "apexd"

#include <strings.h>

#include <android-base/logging.h>

#include "apexd.h"
#include "apexd_prepostinstall.h"
#include "apexservice.h"

#include <android-base/properties.h>

namespace {

static constexpr const char* kApexDataStatusSysprop = "apexd.data.status";
static constexpr const char* kApexDataStatusReady = "ready";

int HandleSubcommand(char** argv) {
  if (strcmp("--pre-install", argv[1]) == 0) {
    LOG(INFO) << "Preinstall subcommand detected";
    return android::apex::RunPreInstall(argv);
  }

  if (strcmp("--post-install", argv[1]) == 0) {
    LOG(INFO) << "Postinstall subcommand detected";
    return android::apex::RunPostInstall(argv);
  }

  LOG(ERROR) << "Unknown subcommand: " << argv[1];
  return 1;
}

struct CombinedLogger {
  android::base::LogdLogger logd;

  CombinedLogger() {}

  void operator()(android::base::LogId id, android::base::LogSeverity severity,
                  const char* tag, const char* file, unsigned int line,
                  const char* message) {
    logd(id, severity, tag, file, line, message);
    KernelLogger(id, severity, tag, file, line, message);
  }
};

}  // namespace

int main(int /*argc*/, char** argv) {
  // Use CombinedLogger to also log to the kernel log.
  android::base::InitLogging(argv, CombinedLogger());

  if (argv[1] != nullptr) {
    return HandleSubcommand(argv);
  }

  android::apex::onStart();

  // TODO: add a -v flag or an external setting to change LogSeverity.
  android::base::SetMinimumLogSeverity(android::base::VERBOSE);

  // Wait for /data/apex. The property is set by init.
  android::base::WaitForProperty(kApexDataStatusSysprop, kApexDataStatusReady);
  android::apex::startBootSequence();

  android::apex::binder::CreateAndRegisterService();
  android::apex::binder::StartThreadPool();
  android::apex::binder::JoinThreadPool();
  return 1;
}
