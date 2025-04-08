//
// Created by Alether on 4/4/2025.
//
module;

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <memory>

export module logger;

const std::string CONSOLE_LOG_NAME = "console_log";

export namespace ale::logger {

void init() { auto console_logger = spdlog::stdout_color_st(CONSOLE_LOG_NAME); }

std::shared_ptr<spdlog::logger> get() { return spdlog::get(CONSOLE_LOG_NAME); }

}; // namespace ale::logger
