#include "log.h"

/* Standard library includes */
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

/* Linux Specific includes */
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

#ifdef _WIN32
    #include <io.h>
    #define fsync _commit
#else
    #include <unistd.h>
#endif

/* ---------------------------- PRIVATE ----------------------------- */

typedef enum iso8601_status_e
{
	ISO8601_STATUS_OK,
	ISO8601_STATUS_ERR
} iso8601_status_e;

typedef enum iso8601_time_length_e
{
	ISO8601_TIME_LENGTH = 18
} iso8601_time_length_e;

typedef enum console_color_length_e
{
	CONSOLE_COLOR_LENGTH = 5
} console_color_length_e;

static const char* console_colors[] = {
	"\033[97m", /* White color           */
	"\033[92m", /* Green color           */
	"\033[33m", /* Yellow color          */
	"\033[91m", /* Red color             */
	"\033[39m"  /* Default console color */
};

static const char* log_level_name[] = {
	"DEBUG",
	"INFO",
	"WARNING",
	"ERROR"
};

static pthread_mutex_t time_mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
static int             log_fd      = -1;

static iso8601_status_e fill_iso8601_time(char* buffer, size_t nbyte)
{
	/* Get current time since epoch */
	time_t now = time(NULL);
	if(now < 0)
	{
		return ISO8601_STATUS_ERR;
	}
	
	/* localtime uses a static local. A critical section is needed 
	   until the local has been parsed into the buffer */
	if(pthread_mutex_lock(&time_mutex))
	{
		return ISO8601_STATUS_ERR;
	}
	
	/* Convert the time since epoch to local time */
	struct tm* ptime = localtime(&now);
	if(ptime == NULL)
	{
		/* Make sure to unlock the mutex here */
		(void)pthread_mutex_unlock(&time_mutex);
		return ISO8601_STATUS_ERR;
	}
	
	/* Fill buffer with ISO8601 time format + NULL */
	int ret = snprintf(
	    buffer, 
	    nbyte, 
	    "%02d-%02d-%02dT%02d:%02d:%02d",
		ptime->tm_mday,
		ptime->tm_mon  + 1,
		ptime->tm_year - 100,
		ptime->tm_hour,
		ptime->tm_min,
		ptime->tm_sec
	);
	
	/* Make sure to unlock the mutex here */
	if(pthread_mutex_unlock(&time_mutex))
	{
		return ISO8601_STATUS_ERR;
	}
	
	if(ret != (ISO8601_TIME_LENGTH - 1))
	{
		return ISO8601_STATUS_ERR;
	}
	
	return ISO8601_STATUS_OK;
}

static void log_to_stdout(const char* log_line,
                          uint32_t    length,
                          uint8_t     level)
{
	/* Copy the log line since snprintf on an overlaping buffer is UB */
	char colored_log_line[LOG_CONFIG_LOG_LINE_MAXLEN] = { 0 };
	int ret = snprintf(colored_log_line, LOG_CONFIG_LOG_LINE_MAXLEN, "%s%s%s",
		console_colors[level], log_line, console_colors[LOG_LEVEL_NONE]
	);
	
	if(ret < (int)(length + CONSOLE_COLOR_LENGTH * 2) ||
	   ret >= LOG_CONFIG_LOG_LINE_MAXLEN)
	{
		perror("Logger failed to color log line");
		return;
	}
	
	/* write to stdout */
	if(write(STDOUT_FILENO, colored_log_line, (unsigned int)ret) != ret)
	{
		perror("Failed to write log line to stdout");
	}
}

static void log_to_file(const char* log_line, uint32_t length)
{
	/* Write out the log line to file */
	if(write(log_fd, log_line, length) != (int)length)
	{
		perror("Failed to write log line to file");
		return;
	}
	
	/* Make sure to flush the buffer to disk with each write */
	int ret = 0;
	do {
		 ret = fsync(log_fd);
	} while(ret < 0 && errno == EINTR);
	
	if(ret < 0)
	{
		perror("Logger failed to flush the buffer to disk");
	}
}

/* ---------------------------- PUBLIC ------------------------------ */

eStatus log_init(void)
{
	if(LOG_SINK_SELECT == LOG_SINK_STDOUT_AND_FILE ||
	   LOG_SINK_SELECT == LOG_SINK_FILE)
	{
		log_fd = open(
			LOG_FILE_PATH,
			LOG_FILE_MODE,
			LOG_FILE_CONFIG_PERMISSIONS
		);
		
		if(log_fd < 0)
		{
			return eSTATUS_SYSTEM_ERROR;
		}
	}
	else if(LOG_SINK_SELECT != LOG_SINK_STDOUT)
	{
		return eSTATUS_INVALID_CONFIG;
	}
	
	return eSTATUS_SUCCESSFUL;
}

void log_exit(void)
{
	if(log_fd >= 0)
	{
		/* No logical action can be taken on a failed close */
		(void)close(log_fd);
	}
}

void log_private(uint8_t level, const char* file, int line, 
                 const char* func, const char* format, ...)
{
	/* Only the format parameter is passed by the user. For debugging,
	   make sure its not NULL */
	assert(file);
	assert(func);
	assert(format);
	
	/* Obtain time string in ISO8601 format */
	char time_str[ISO8601_TIME_LENGTH] = { 0 };
	if(fill_iso8601_time(time_str, ISO8601_TIME_LENGTH))
	{
		perror("Logger failed to obtain time");
		return;
	}
	
	/* Parse user message into a local buffer */
	char user_msg[LOG_CONFIG_USER_MSG_MAXLEN] = { 0 };
	va_list args;
	va_start(args, format);
	int ret = vsnprintf(user_msg, LOG_CONFIG_USER_MSG_MAXLEN, format, args);
	va_end(args);
	
	if(ret < 0 || ret >= LOG_CONFIG_USER_MSG_MAXLEN)
	{
		perror("Logger failed to parse user message");
		return;
	}
	
	/* Generate the log line */
	char log_line[LOG_CONFIG_LOG_LINE_MAXLEN] = { 0 };
	ret = snprintf(log_line, LOG_CONFIG_LOG_LINE_MAXLEN, 
		"[%s][%s]: %s:%d: In function '%s': %s\n",
		time_str, log_level_name[level], file, line, func, user_msg
	);
		
	if(ret < 0 || ret >= LOG_CONFIG_LOG_LINE_MAXLEN)
	{
		perror("Logger failed to generate log line");
		return;
	}
	
	/* io_write may call write multiple times internally. A mutex is
	   needed to guarantee an order of writing when called from
	   multiple threads */
	if(pthread_mutex_lock(&write_mutex))
	{
		perror("Failed to lock write mutex");
		return;
	}
	
	/* Print to the selected log sink */
	if(LOG_SINK_SELECT == LOG_SINK_STDOUT_AND_FILE)
	{
		log_to_stdout(log_line, (uint32_t)ret, level);
		log_to_file(log_line, (uint32_t)ret);
	}
	else if(LOG_SINK_SELECT == LOG_SINK_STDOUT)
	{
		log_to_stdout(log_line, (uint32_t)ret, level);
	}
	else if(LOG_SINK_SELECT == LOG_SINK_FILE)
	{
		log_to_file(log_line, (uint32_t)ret);
	}
	
	if(pthread_mutex_unlock(&write_mutex))
	{
		perror("Failed to unlock write mutex");
		return;
	}
}
