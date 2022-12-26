/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/**
 * @file logging_stack.h
 * @brief Utility header file that exposes macros for configuring logging
 * implementation of logging macros (LogError, LogWarn, LogInfo, LogDebug).
 */

#ifndef LOGGING_H
#define LOGGING_H

/* Include header for logging level macros. */
#include "FreeRTOSConfig.h"
#include "logging_levels.h"
#include "logging_task.h"

/* Standard Include. */
#include <stdint.h>
#include <stdio.h>

/**
 * @brief The name of the library or demo to add as metadata in log messages
 * from the library or demo.
 *
 * This metadata aids in identifying the module source of log messages.
 * The metadata is logged in the format `[ <LIBRARY-NAME> ]` as a prefix to the
 * log messages.
 * Refer to #LOG_METADATA_FORMAT for the complete format of the metadata prefix
 * in log messages.
 */

/**
 * @brief Common macro that maps all the logging interfaces,
 * (#LogDebug, #LogInfo, #LogWarn, #LogError) to the platform-specific logging
 * function.
 *
 * @note The default definition of this macro generates logging via a
 * printf-like vLoggingPrintf function.
 */

/**
 * Disable definition of logging interface macros when generating doxygen
 * output, to avoid conflict with documentation of macros at the end of the
 * file.
 */
/* Check that LIBRARY_LOG_LEVEL is defined and has a valid value. */
#if !defined(LIBRARY_LOG_LEVEL) ||                                             \
    ((LIBRARY_LOG_LEVEL != LOG_NONE) && (LIBRARY_LOG_LEVEL != LOG_ERROR) &&    \
     (LIBRARY_LOG_LEVEL != LOG_WARN) && (LIBRARY_LOG_LEVEL != LOG_INFO) &&     \
     (LIBRARY_LOG_LEVEL != LOG_DEBUG))
#error                                                                         \
    "Please define LIBRARY_LOG_LEVEL as either LOG_NONE, LOG_ERROR, LOG_WARN, LOG_INFO, or LOG_DEBUG."
#else
#if LIBRARY_LOG_LEVEL == LOG_DEBUG
/* All log level messages will logged. */
#define LogError(message, ...) vLoggingPrintfError(message, ##__VA_ARGS__)
#define LogWarn(message, ...) vLoggingPrintfWarn(message, ##__VA_ARGS__)
#define LogInfo(message, ...) vLoggingPrintfInfo(message, ##__VA_ARGS__)
#define LogDebug(message, ...) vLoggingPrintfDebug(message, ##__VA_ARGS__)

#elif LIBRARY_LOG_LEVEL == LOG_INFO
/* Only INFO, WARNING and ERROR messages will be logged. */
#define LogError(message, ...) vLoggingPrintfError(message, ##__VA_ARGS__)
#define LogWarn(message, ...) vLoggingPrintfWarn(message, ##__VA_ARGS__)
#define LogInfo(message, ...) vLoggingPrintfInfo(message, ##__VA_ARGS__)
#define LogDebug(message, ...)

#elif LIBRARY_LOG_LEVEL == LOG_WARN
/* Only WARNING and ERROR messages will be logged.*/
#define LogError(message, ...) vLoggingPrintfError(message, ##__VA_ARGS__)
#define LogWarn(message, ...) vLoggingPrintfWarn(message, ##__VA_ARGS__)
#define LogInfo(message, ...)
#define LogDebug(message, ...)

#elif LIBRARY_LOG_LEVEL == LOG_ERROR
/* Only ERROR messages will be logged. */
#define LogError(message, ...) vLoggingPrintfError(message, ##__VA_ARGS__)
#define LogWarn(message, ...)
#define LogInfo(message, ...)
#define LogDebug(message, ...)

#else /* if LIBRARY_LOG_LEVEL == LOG_ERROR */

#define LogError(message)
#define LogWarn(message)
#define LogInfo(message)
#define LogDebug(message)

#endif /* if LIBRARY_LOG_LEVEL == LOG_ERROR */
#endif /* if !defined( LIBRARY_LOG_LEVEL ) || ( ( LIBRARY_LOG_LEVEL !=         \
          LOG_NONE ) && ( LIBRARY_LOG_LEVEL != LOG_ERROR ) && (                \
          LIBRARY_LOG_LEVEL != LOG_WARN ) && ( LIBRARY_LOG_LEVEL != LOG_INFO ) \
          && ( LIBRARY_LOG_LEVEL != LOG_DEBUG ) ) */

#endif /* ifndef LOGGING_STACK_H */