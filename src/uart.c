#include "uart.h"

/* Linux specific includes */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

/* -------------------------------PRIVATE-START------------------------------ */
#define FIRST_VALID_FD (0) /* All valid fd's will be greater then 0 */

#define UART_ERR     (-1) /* UART error value when using syscalls */
#define UART_TIMEOUT (0)  /* UART read timed out */

#define NO_FLAGS ((tcflag_t)0) /* Set no flags */

/* Internal function for validating UART parameters */
static inline UARTStatus uart_check_param(const UARTDevice* pdevice)
{
	if(pdevice == NULL)
	{
		return eUART_NULL_PARAM;
	}

	if(pdevice->fd < FIRST_VALID_FD)
	{
		return eUART_BAD_FD;
	}

	return eUART_OK;
}

// Gets the attributes of a tty if its associated with a valid UARTDevice object
static inline UARTStatus uart_get_attributes(const UARTDevice* pdevice,
	struct termios* ptty)
{
	UARTStatus status = uart_check_param(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	if(tcgetattr(pdevice->fd, ptty) == UART_ERR)
	{
		return eUART_GETATTR_ERR;
	}

	return eUART_OK;
}

// Wait until all queued output has been written, queued input has been
// discarded and then set the attributes.
static inline UARTStatus uart_set_attributes(int fd, const struct termios* ptty)
{
	if(tcsetattr(fd, TCSAFLUSH, ptty) == UART_ERR)
	{
		return eUART_SETATTR_ERR;
	}

	return eUART_OK;
}

// Make the tty a raw uart interface
static inline UARTStatus uart_make_raw(const UARTDevice* pdevice)
{
	UARTStatus status = uart_check_param(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	struct termios tty = {
		.c_iflag = 0,                // Turn off all input processing
		.c_oflag = NO_FLAGS,         // Turn off all output processing
		.c_cflag = (CLOCAL | CREAD), // Ignore modem and enable reads
		.c_lflag = NO_FLAGS          // Turn off all local flags
	};

	return uart_set_attributes(pdevice->fd, &tty);
}
/* -------------------------------PRIVATE-END-------------------------------- */

UARTStatus uart_open(UARTDevice* pdevice)
{
	if(pdevice == NULL)
	{
		return eUART_NULL_PARAM;
	}

	if(pdevice->pname == NULL)
	{
		return eUART_NO_PATH;
	}

	pdevice->fd = open(pdevice->pname, O_RDWR | O_NOCTTY);
	if(pdevice->fd == UART_ERR)
	{
		return eUART_OPEN_ERR;
	}

	if(!isatty(pdevice->fd))
	{
		close(pdevice->fd);
		pdevice->fd = UART_ERR;
		return eUART_NOT_TTY;
	}

	return eUART_OK;
}

UARTStatus uart_close(const UARTDevice* pdevice)
{
	UARTStatus status = uart_check_param(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	close(pdevice->fd);

	return eUART_OK;
}

UARTStatus uart_set_configuration(const UARTDevice* pdevice)
{
	UARTStatus status = uart_make_raw(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	status = uart_set_baud(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	status = uart_set_bits(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	status = uart_set_stop_bits(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	status = uart_set_parity(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	return uart_set_read_timeout(pdevice);
}

UARTStatus uart_set_baud(const UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	// Set RX and TX baud to the same value
	if(cfsetispeed(&tty, pdevice->baud) == UART_ERR ||
	   cfsetospeed(&tty, pdevice->baud) == UART_ERR)
	{
		return eUART_BAD_BAUD;
	}

	return uart_set_attributes(pdevice->fd, &tty);
}

UARTStatus uart_set_bits(const UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	// Make sure the bits option is valid
	if(pdevice->bits != CS5 && pdevice->bits != CS6 &&
	   pdevice->bits != CS7 && pdevice->bits != CS8)
	{
		return eUART_BAD_BITS;
	}

	tty.c_cflag &= (tcflag_t)~(CSIZE); // Disable previous bits per word
	tty.c_cflag |= pdevice->bits;      // Set new bits per word

	return uart_set_attributes(pdevice->fd, &tty);
}

UARTStatus uart_set_stop_bits(const UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	if(pdevice->stop_bits == SINGLE_STOP_BIT)
	{
		tty.c_cflag &= (tcflag_t)~(CSTOPB); // Sets 1 stop bit
	}
	else if(pdevice->stop_bits == DOUBLE_STOP_BIT)
	{
		tty.c_cflag |= CSTOPB; // Sets 2 stop bits
	}
	else
	{
		return eUART_BAD_STOP_BITS;
	}

	return uart_set_attributes(pdevice->fd, &tty);
}

UARTStatus uart_set_parity(const UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	if(pdevice->parity != PARITY_NONE)
	{
		if(pdevice->parity == PARITY_EVEN)
		{
			tty.c_cflag &= (tcflag_t)~(PARODD); // Set even parity
		}
		else if(pdevice->parity == PARITY_ODD)
		{
			tty.c_cflag |= PARODD; // Set odd parity
		}
		else
		{
			return eUART_BAD_PARITY;
		}
		tty.c_iflag |= INPCK;  // Enable input parity check
		tty.c_cflag |= PARENB; // Enable output parity generation
		tty.c_iflag |= IGNPAR; // Ignore bytes with parity or framing errors
	}
	else
	{
		tty.c_iflag &= (tcflag_t)~(INPCK);  //Disable input parity check
		tty.c_cflag &= (tcflag_t)~(PARENB); //Disable output parity generation
	}

	return uart_set_attributes(pdevice->fd, &tty);
}

UARTStatus uart_set_read_timeout(const UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	tty.c_cc[VMIN]  = pdevice->vmin;  // Minimal number of read bytes
	tty.c_cc[VTIME] = pdevice->vtime; // Timeout in 0.1 second units

	return uart_set_attributes(pdevice->fd, &tty);
}

UARTStatus uart_get_configuration(UARTDevice* pdevice)
{
	UARTStatus status = uart_get_baud(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	status = uart_get_bits(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	status = uart_get_stop_bits(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	status = uart_get_parity(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	return uart_get_read_timeout(pdevice);
}

UARTStatus uart_get_baud(UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	// Make sure both RX and TX baud are the same
	pdevice->baud = cfgetospeed(&tty);
	if(pdevice->baud != cfgetispeed(&tty))
	{
		return eUART_BAUD_MISMATCH;
	}

	return eUART_OK;
}

UARTStatus uart_get_bits(UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	pdevice->bits = tty.c_cflag & CSIZE; // Get bits per word

	return eUART_OK;
}

UARTStatus uart_get_stop_bits(UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	// Check if single or double stop bits are used
	if(tty.c_cflag & CSTOPB)
	{
		pdevice->stop_bits = DOUBLE_STOP_BIT;
	}
	else
	{
		pdevice->stop_bits = SINGLE_STOP_BIT;
	}

	return eUART_OK;
}

UARTStatus uart_get_parity(UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	// Check if both input parity check and output parity generation is enabled.
	if((tty.c_iflag & INPCK) && (tty.c_cflag & PARENB))
	{// Check if the parity is odd
		if(tty.c_cflag & PARODD)
		{
			pdevice->parity = PARITY_ODD;
		}
		else
		{
			pdevice->parity = PARITY_EVEN;
		}
	} // Check if parity is disabled
	else if(!(tty.c_iflag & INPCK) && !(tty.c_cflag & PARENB))
	{
		pdevice->parity = PARITY_NONE;
	}
	else
	{
		return eUART_PARITY_MISMATCH;
	}

	return eUART_OK;
}

UARTStatus uart_get_read_timeout(UARTDevice* pdevice)
{
	struct termios tty = { 0 };
	UARTStatus status = uart_get_attributes(pdevice, &tty);
	if(status != eUART_OK)
	{
		return status;
	}

	pdevice->vmin  = tty.c_cc[VMIN];  // Minimal number of bytes
	pdevice->vtime = tty.c_cc[VTIME]; // Timeout in 0.1 seconds

	return eUART_OK;
}

UARTStatus uart_write_all(const UARTDevice* pdevice, const uint8_t* pbuf,
	size_t* pnbytes)
{
	UARTStatus status = uart_check_param(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	// These parameters are not checked by uart_param_check
	if(pbuf == NULL || pnbytes == NULL)
	{
		return eUART_NULL_PARAM;
	}

	while(*pnbytes > 0)
	{
		ssize_t bytes_sent = write(pdevice->fd, pbuf, *pnbytes);
		if(bytes_sent == UART_ERR)
		{
			return eUART_WRITE_ERR;
		}
		*pnbytes -= (size_t)bytes_sent;
		pbuf     += bytes_sent;
	}

	return eUART_OK;
}

UARTStatus uart_read_all(const UARTDevice* pdevice, uint8_t* pbuf,
	size_t* pnbytes)
{
	UARTStatus status = uart_check_param(pdevice);
	if(status != eUART_OK)
	{
		return status;
	}

	// These parameters are not checked by uart_param_check
	if(pbuf == NULL || pnbytes == NULL)
	{
		return eUART_NULL_PARAM;
	}

	while(*pnbytes > 0)
	{
		ssize_t bytes_read = read(pdevice->fd, pbuf, *pnbytes);
		if(bytes_read == UART_ERR)
		{
			return eUART_READ_ERR;
		}
		else if(bytes_read == UART_TIMEOUT)
		{
			return eUART_READ_TIMEOUT;
		}
		*pnbytes -= (size_t)bytes_read;
		pbuf     += bytes_read;
	}

	return eUART_OK;
}
