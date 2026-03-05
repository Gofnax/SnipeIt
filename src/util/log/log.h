#ifndef LOG_H
#define LOG_H 

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "log_config.h"
#include "status.h"

/**
 * @brief  Initializes the logger.
 * @return Log status code.
 * @retval eSTATUS_SUCCESSFUL     log initialization complete
 * @retval eSTATUS_INVALID_CONFIG invalid log sink in log_config
 * @retval eSTATUS_SYSTEM_ERROR   failed to open log file
 * @note   Not thread safe. Should be called only once on program exit
 *         anyway.
 */
eStatus log_init(void);

/**
 * @brief Closes the logger.
 * @note  Not thread safe. Should be called only once on program exit
 *        anyway.
 */
void log_exit(void);

/**
 * @brief   Private function for logging a message.
 * @details Called by the macros LOG_DEBUG, LOG_INFO, LOG_WARNING and
 *          LOG_ERROR.
 * @warning The user should never call this function directly!
 * @param level  level to log at
 * @param file   always __FILE__
 * @param line   always __LINE__
 * @param func   always __func__
 * @param format message format (same as printf)
 * @param ...    arguments to be used by the format (same as printf)
 * @note  This function is thread safe!
 */
void log_private(uint8_t level, const char* file, int line, 
                 const char* func, const char* format, ...);

#if LOG_LEVEL == LOG_LEVEL_DEBUG
	#define LOG_DEBUG(...)                                         \
		log_private(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, \
			__VA_ARGS__)
#else
	#define LOG_DEBUG(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
	#define LOG_INFO(...)                                         \
		log_private(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, \
			__VA_ARGS__)
#else
	#define LOG_INFO(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
	#define LOG_WARNING(...)                                         \
		log_private(LOG_LEVEL_WARNING, __FILE__, __LINE__, __func__, \
			__VA_ARGS__)
#else
	#define LOG_WARNING(...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
	#define LOG_ERROR(...)                                         \
		log_private(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, \
			__VA_ARGS__)
#else
	#define LOG_ERROR(...)
#endif

#endif
