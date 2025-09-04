/*******************************************************************************
 * @file
 * @brief termios.h wrapper for easier use.
 * @warning This library is not thread-safe or signal-safe.
 ******************************************************************************/

#ifndef UART_H
#define UART_H

/* Standard includes */
#include <stdint.h>
#include <stddef.h>

/* Linux specific includes */
#include <termios.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Stop bits configuration
 * @{
 */
/** @details Sets 1 stop bit per word. */
#define SINGLE_STOP_BIT ((uint8_t)0)
/** @details Sets 2 stop bits per word. */
#define DOUBLE_STOP_BIT ((uint8_t)1)
/**@}*/

/**
 * @name Parity configuration
 * @{
 */
/** @details Sets no parity. */
#define PARITY_NONE ((uint8_t)0)
/** @details Sets even parity. */
#define PARITY_EVEN ((uint8_t)1)
/** @details Sets odd parity. */
#define PARITY_ODD  ((uint8_t)2)
/**@}*/

/**
 * @name Utility macros for vtime
 * @{
 */
/** @details vtime counts in units of 0.1 seconds. */
#define VTIME_TIME_CONSTANT (0.1)
/** @details Convert seconds to vtime. */
#define SEC_TO_VTIME(SEC) ((cc_t)((SEC) / VTIME_TIME_CONSTANT))
/**@}*/

/**
 * @brief Indicates success or failure status.
 * @details Used as the return type for all the functions.
 */
typedef enum
{
	eUART_OK,              /**< Ok (success!).                */
	eUART_NULL_PARAM,      /**< NULL param passed.            */
	eUART_NO_PATH,         /**< UARTDevice::pname is NULL.    */
	eUART_BAD_FD,          /**< UARTDevice::fd < 0.           */
	eUART_OPEN_ERR,        /**< Failed to open device.        */
	eUART_NOT_TTY,         /**< Device is not a tty.          */
	eUART_BAD_BAUD,        /**< Failed to set baud.           */
	eUART_BAD_BITS,        /**< Failed to set bits per word.  */
	eUART_BAD_STOP_BITS,   /**< Failed to set stop bits.      */
	eUART_BAD_PARITY,      /**< Failed to set parity.         */
	eUART_SETATTR_ERR,     /**< Failed to set tty attributes. */
	eUART_GETATTR_ERR,     /**< Failed to get tty attributes. */
	eUART_BAUD_MISMATCH,   /**< Got invalid baud setting.     */
	eUART_PARITY_MISMATCH, /**< Got invalid parity setting.   */
	eUART_WRITE_ERR,       /**< Failed to write.              */
	eUART_READ_ERR,        /**< Failed to read.               */
	eUART_READ_TIMEOUT     /**< Read timed out.               */
} UARTStatus;

/**
 * @brief Holds UART device properties. 
 * @details Used by all the functions as the first parameter.
 * @note Modifying vtime and vmin will affect read in the following way:\n
 * vtime > 0 | vmin > 0: wait vtime between characters before timing out or
 *                       until vmin characters are read.\n
 * vtime = 0 | vmin = 0: read as much as possible or timeout if the full read
 *                       cant be complete immediately.\n
 * vtime > 0 | vmin = 0: block until either vtime passes or at least one byte
 *                       is read.\n
 * vtime = 0 | vmin > 0: block until at least vmin characters are read.\n
 */
typedef struct
{
	char*    pname;     /**< Path to the UART device E.g.: "/dev/ttyS2". */
	int      fd;        /**< UART fd. Set only with #uart_open and
							 #uart_close. */
	speed_t  baud;      /**< Valid baud values are: B0, B50, B75, B110, B134,
							 B150, B200, B300, B600, B1200, B1800, B2400, B4800,
							 B9600, B19200, B38400, B57600, B115200, B230400,
							 B460800. */
	tcflag_t bits;      /**< Valid bits per word are: CS5, CS6, CS7, CS8. */
	uint8_t  stop_bits; /**< Valid stop bits are: SINGLE_STOP_BIT,
							 DOUBLE_STOP_BIT. */
	uint8_t  parity;    /**< Valid parity is: PARITY_NONE, PARITY_EVEN,
							 PARITY_ODD. */
	cc_t     vmin;      /**< Minimal number of bytes to be available for read to
							 return */
	cc_t     vtime;     /**< Max read timeout in 0.1 second units. (25.5S max)*/
} UARTDevice;

/**
 * @brief Opens the UART device.
 * @details Uses pdevice->pname to specify the file that needs to be open and
 *          stores the resulting file descriptor in pdevice->fd and makes sure
 *          its actually a tty.
 * @param pdevice A pointer to a UARTDevice instance with the
 *                #UARTDevice::pname field set to a valid path.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_NO_PATH if pdevice->pname is NULL, eUART_OPEN_ERR if the
 *          open() syscall failed, eUART_NOT_TTY if the file is not a tty.
 */
UARTStatus uart_open(UARTDevice* pdevice);

/**
 * @brief Closes the UART device.
 * @details Uses pdevice->fd to close the UART device.
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative.
 */
UARTStatus uart_close(const UARTDevice* pdevice);

/**
 * @brief Set the UART device configuration.
 * @details Sets the UART in raw mode and then calls #uart_set_baud,
 *          #uart_set_bits, #uart_set_stop_bits, #uart_set_parity,
 *          #uart_set_read_timeout one after another.
 * @param pdevice A pointer to an UARTDevice instance with the #UARTDevice::fd,
 *                #UARTDevice::baud, #UARTDevice::bits, #UARTDevice::stop_bits,
 *                #UARTDevice::parity, #UARTDevice::vmin, #UARTDevice::vtime
 *                fields all set to valid values.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_SETATTR_ERR if
 *          setting attriubtes failed, eUART_GETATTR_ERR if getting attributes
 *          failed, eUART_BAD_BAUD if baud is invalid, eUART_BAD_BITS if bits
 *          are invalid, eUART_BAD_STOP_BITS if stop_bits are invalid,
 *          eUART_BAD_PARITY if parity is invalid.
 */
UARTStatus uart_set_configuration(const UARTDevice* pdevice);

/**
 * @brief Set the UART devices baud.
 * @details Set the baud rate to one of the following valid values:
 *          B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800,
 *          B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400,
 *          B460800.
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd,
 * 				  #UARTDevice::baud fields all set to valid values.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_SETATTR_ERR if
 *          setting attriubtes failed, eUART_GETATTR_ERR if getting attributes
 *          failed, eUART_BAD_BAUD if baud is invalid.
 */
UARTStatus uart_set_baud(const UARTDevice* pdevice);

/**
 * @brief Set the UART devices bits per word.
 * @details Set the bits per word to one of the following valid values:
 *          CS5, CS6, CS7, CS8.
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd,
 * 				  #UARTDevice::bits fields all set to valid values.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_SETATTR_ERR if
 *          setting attriubtes failed, eUART_GETATTR_ERR if getting attributes
 *          failed, eUART_BAD_BITS if bits are invalid.
 */
UARTStatus uart_set_bits(const UARTDevice* pdevice);

/**
 * @brief Set the UART devices stop bits.
 * @details Set the stop bits to one of the following valid values:
 *          SINGLE_STOP_BIT, DOUBLE_STOP_BIT.
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd,
 * 				  #UARTDevice::stop_bits fields all set to valid values.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_SETATTR_ERR if
 *          setting attriubtes failed, eUART_GETATTR_ERR if getting attributes
 *          failed, eUART_BAD_STOP_BITS if stop_bits are invalid.
 */
UARTStatus uart_set_stop_bits(const UARTDevice* pdevice);

/**
 * @brief Set the UART devices parity.
 * @details Set the parity to one of the following valid values:
 *          PARITY_NONE, PARITY_EVEN, PARITY_ODD.
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd,
 * 				  #UARTDevice::parity fields all set to valid values.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_SETATTR_ERR if
 *          setting attriubtes failed, eUART_GETATTR_ERR if getting attributes
 *          failed, eUART_BAD_PARITY if parity is invalid
 */
UARTStatus uart_set_parity(const UARTDevice* pdevice);

/**
 * @brief Sets the UART devices timeout.
 * @details Set the read timeout in accordance with the vmin and vtime values,
 *          see #UARTDevice struct for more details.
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd,
 * 				  #UARTDevice::vmin, #UARTDevice::vtime fields all set to valid
 *                values.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_SETATTR_ERR if
 *          setting attriubtes failed, eUART_GETATTR_ERR if getting attributes
 *          failed.
 */
UARTStatus uart_set_read_timeout(const UARTDevice* pdevice);

/**
 * @brief Get the UART device configuration.
 * @details Equivalent to calling #uart_get_baud, #uart_get_bits,
 *          #uart_get_stop_bits, #uart_get_parity, #uart_get_read_timeout
 * @param pdevice A pointer to an UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_GETATTR_ERR if
 *          getting attributes failed, eUART_BAUD_MISMATCH if RX and TX bauds
 *          are not the same, eUART_PARITY_MISMATCH if parity is not enabled
 *          correctly.
 */
UARTStatus uart_get_configuration(UARTDevice* pdevice);

/**
 * @brief Get the UART devices baud.
 * @details Store the baud in pdevice->baud.
 * @param pdevice A pointer to an UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_GETATTR_ERR if
 *          getting attributes failed, eUART_BAUD_MISMATCH if RX and TX bauds
 *          are not the same.
 */
UARTStatus uart_get_baud(UARTDevice* pdevice);

/**
 * @brief Get the UART devices bits per word.
 * @details Store the bits per word in pdevice->bits.
 * @param pdevice A pointer to an UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_GETATTR_ERR if
 *          getting attributes failed.
 */
UARTStatus uart_get_bits(UARTDevice* pdevice);

/**
 * @brief Get the UART devices stop bits.
 * @details Store the stop bits in pdevice->stop_bits.
 * @param pdevice A pointer to an UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_GETATTR_ERR if
 *          getting attributes failed.
 */
UARTStatus uart_get_stop_bits(UARTDevice* pdevice);

/**
 * @brief Get the UART devices parity.
 * @details Store the parity in pdevice->parity.
 * @param pdevice A pointer to an UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_GETATTR_ERR if
 *          getting attributes failed, eUART_PARITY_MISMATCH if parity is not
 *          enabled correctly.
 */
UARTStatus uart_get_parity(UARTDevice* pdevice);

/**
 * @brief Get the UART devices read timeout.
 * @details Store the read timeout in pdevice->vmin and pdevice->vtime.
 * @param pdevice A pointer to an UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice is NULL,
 *          eUART_BAD_FD if #UARTDevice::fd is negative, eUART_GETATTR_ERR if
 *          getting attributes failed.
 */
UARTStatus uart_get_read_timeout(UARTDevice* pdevice);

/**
 * @brief Write in blocking mode.
 * @details Writes pnbytes from pbuf in blocking mode.
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @param pbuf A pointer to a transmit buffer.
 * @param pnbytes The number of bytes to write from pbuf.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice, pbuf or
 *          pnbytes is NULL, eUART_BAD_FD if #UARTDevice::fd is negative,
 *          eUART_WRITE_ERR if the write failed.
 */
UARTStatus uart_write_all(const UARTDevice* pdevice, const uint8_t* pbuf,
	size_t* pnbytes);

/**
 * @brief Read in blocking mode or with timeout.
 * @details Reads pnbytes from pbuf in blocking mode or until timeout if
 *          #UARTDevice::vtime is set. 
 * @param pdevice A pointer to a UARTDevice instance with the #UARTDevice::fd
 *                field set to a valid non negative value.
 * @param pbuf A pointer to a receive buffer.
 * @param pnbytes The number of bytes to read from pbuf.
 * @returns eUART_OK on success. Otherwise, eUART_NULL_PARAM if pdevice, pbuf or
 *          pnbytes is NULL, eUART_BAD_FD if #UARTDevice::fd is negative,
 *          eUART_READ_ERR if the read failed or eUART_READ_TIMEOUT if the read
 *          timed out.
 */
UARTStatus uart_read_all(const UARTDevice* pdevice, uint8_t* pbuf,
	size_t* pnbytes);

#ifdef __cplusplus
}
#endif

#endif
