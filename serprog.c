/*    Copyright (C) 2012  Denis 'GNUtoo' Carikli <GNUtoo@no-log.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define S_ACK 0x06
#define S_NAK 0x15
#define S_CMD_NOP               0x00            /* No operation                                 */
#define S_CMD_Q_IFACE           0x01            /* Query interface version                      */
#define S_CMD_Q_CMDMAP          0x02            /* Query supported commands bitmap              */
#define S_CMD_Q_PGMNAME         0x03            /* Query programmer name                        */
#define S_CMD_Q_SERBUF          0x04            /* Query Serial Buffer Size                     */
#define S_CMD_Q_BUSTYPE         0x05            /* Query supported bustypes                     */
#define S_CMD_Q_CHIPSIZE        0x06            /* Query supported chipsize (2^n format)        */
#define S_CMD_Q_OPBUF           0x07            /* Query operation buffer size                  */
#define S_CMD_Q_WRNMAXLEN       0x08            /* Query Write to opbuf: Write-N maximum length */
#define S_CMD_R_BYTE            0x09            /* Read a single byte                           */
#define S_CMD_R_NBYTES          0x0A            /* Read n bytes                                 */
#define S_CMD_O_INIT            0x0B            /* Initialize operation buffer                  */
#define S_CMD_O_WRITEB          0x0C            /* Write opbuf: Write byte with address         */
#define S_CMD_O_WRITEN          0x0D            /* Write to opbuf: Write-N                      */
#define S_CMD_O_DELAY           0x0E            /* Write opbuf: udelay                          */
#define S_CMD_O_EXEC            0x0F            /* Execute operation buffer                     */
#define S_CMD_SYNCNOP           0x10            /* Special no-operation that returns NAK+ACK    */
#define S_CMD_Q_RDNMAXLEN       0x11            /* Query read-n maximum length                  */
#define S_CMD_S_BUSTYPE         0x12            /* Set used bustype(s).                         */
#define S_CMD_O_SPIOP           0x13            /* Perform SPI operation.                       */

#define SPI_PORT PORTB
#define SCK PORTB5 /* port 13 */
#define MISO PORTB4 /* port 12 */
#define MOSI PORTB3 /* port 11 */
#define SS PORTB2 /* port 10 */
#define DDR_SPI DDRB

#define S_IFACE_VERSION		0x01		/* Version of the protocol */
#define S_PGM_NAME		"serprog-duino" /* The program's name */
#define S_SPEED			57600		/* Serial speed */

void setup_uart( unsigned int bauds )
{

	int freq = F_CPU / 16 / bauds -1;
	/* Set baud rate */
	UBRR0H = (unsigned char)(freq>>8);
	UBRR0L = (unsigned char)freq;
	/* Disable baud rate doubler */
	UCSR0A = (0<<U2X0);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<TXCIE0)|(1<<RXCIE0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void setup_spi(void)
{
	/* Enable MOSI,SCK,SS as output like on
	http://en.wikipedia.org/wiki/File:SPI_single_slave.svg */
	DDR_SPI = (1<<MOSI)|(1<<SCK)|(1<<SS)|(0<<MISO);
	/* Enable SPI Master, set the clock to F_CPU / 16 */
	/* CPOL and CPHA are 0 for SPI mode 0 (see wikipedia) */
	/* we use mode 0 like for the linux spi in flashrom*/
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(1<<SPIE)|(0<<DORD);
	SPSR = (1<<SPI2X);
	/* Hold SS low for enabling the slave of the BIOS chip */
	SPI_PORT = (0<<SS);
	/* Mode 0 */
}

char readwrite_spi(char c)
{
	/* transmit c on the SPI bus */
	SPDR = c;
	/* Wait for the transmission to be comlpeted */
	loop_until_bit_is_set(SPSR,SPIF);
	return SPDR;
}

void putchar_uart( unsigned char data )
{
	/* Wait for empty transmit buffer */
	loop_until_bit_is_set(UCSR0A, UDRE0);
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

char getchar_uart(void)
{
	/* Wait for the char to arrive in the buffer */
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

void word_uart(char * str)
{
	int i;
	for (i=0;i<strlen(str);i++){
		putchar_uart(str[i]);
	}
}

/* get addresses in big endian */
void getaddr_be(uint32_t* addr)
{
	*addr = getchar_uart() << 0;
	*addr |= getchar_uart() << 8;
	*addr |= getchar_uart() << 16;
	*addr |= 0 << 24;
}

void handle_command(unsigned char command)
{
	int i;
	char c;
	uint32_t slen = 0; /* write len */
	uint32_t rlen = 0; /* read len */
	switch (command){
		case S_CMD_NOP:
			putchar_uart(S_ACK);
			break;
		case S_CMD_Q_IFACE:
			putchar_uart(S_ACK);
			putchar_uart(S_IFACE_VERSION);
			/* little endian multibyte value to complete to 16bit */
			putchar_uart(0);
			break;
		case S_CMD_Q_CMDMAP:
			putchar_uart(S_ACK);
			/* little endian */
			putchar_uart(0b00101111);
			putchar_uart(0b00000000);
			putchar_uart(0b00001001);
			for (i=0;i<29;i++){
				putchar_uart(0x0);
			}
			break;
		case S_CMD_Q_PGMNAME:
			putchar_uart(S_ACK);
			word_uart(S_PGM_NAME);
			for (i=strlen(S_PGM_NAME);i<16;i++){
				putchar_uart(0);
			}
			break;
		case S_CMD_Q_SERBUF:
			break;
		case S_CMD_Q_BUSTYPE:
			putchar_uart(S_ACK);
			putchar_uart(0b00001000);
			break;
		case S_CMD_Q_CHIPSIZE:
			break;
		case S_CMD_Q_OPBUF:
			break;
		case S_CMD_Q_WRNMAXLEN:
			break;
		case S_CMD_R_BYTE:
			break;
		case S_CMD_R_NBYTES:
			break;
		case S_CMD_O_INIT:
			break;
		case S_CMD_O_WRITEB:
			break;
		case S_CMD_O_WRITEN:
			break;
		case S_CMD_O_DELAY:
			break;
		case S_CMD_O_EXEC:
			break;
		case S_CMD_SYNCNOP:
			putchar_uart(S_NAK);
			putchar_uart(S_ACK);
			break;
		case S_CMD_Q_RDNMAXLEN:
			break;
		case S_CMD_S_BUSTYPE:
			break;
		case S_CMD_O_SPIOP:
			/* get slen */
			getaddr_be(&slen);
			/* get rlen */
			getaddr_be(&rlen);

			/* SPI is configured in little endian */
			while (slen--){
				c = getchar_uart();
				readwrite_spi(c);
			}
			putchar_uart(S_ACK);
			/* receive TODO: handle errors */
			while (rlen--){
				putchar_uart(readwrite_spi(0x0));
			}
			break;
		default:
			break;
	}
}

ISR(USART_RX_vect)
{
	handle_command(UDR0);
}

int main (void)
{
	setup_uart(57600);
	setup_spi();
	sei(); /* enable interupts */

	while (1)
	{
	}
	return 1;
}
