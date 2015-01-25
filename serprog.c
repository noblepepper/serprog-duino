/*   Copyright (C) 2012  Denis 'GNUtoo' Carikli <GNUtoo@no-log.org>
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
#include "protocol.h"
#include "settings.h"

void setup_uart( unsigned long bauds )
{

	int freq = (F_CPU / 4 / bauds -1) /2 ;
	/* Set baud rate */
	UBRR0H = (unsigned char)(freq>>8);
	UBRR0L = (unsigned char)freq;
	/* Enable baud rate doubler */
	UCSR0A |= (1<<U2X0);
	/* Enable receiver, transmitter and RX interrupt enable */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (3<<UCSZ00);
}

void select_chip(void)
{
	SPI_PORT &= ~(1<<SS);
}

void unselect_chip(void)
{
	SPI_PORT |= (1<<SS);
}
void setup_spi(void)
{
	/* set SS low */
	SPI_PORT &= ~(1<<SS);
	/* Enable MOSI,SCK,SS as output like on
	http://en.wikipedia.org/wiki/File:SPI_single_slave.svg */
	DDR_SPI = (1<<MOSI)|(1<<SCK)|(1<<SS);
	/* Enable SPI Master, set the clock to F_CPU / 2 */
	/* CPOL and CPHA are 0 for SPI mode 0 (see wikipedia) */
	/* we use mode 0 like for the linux spi in flashrom*/
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR = (1<<SPI2X);
}

char readwrite_spi(char c)
{
	char r;

	/* transmit c on the SPI bus */
	SPDR = c;
	/* Wait for the transmission to be comlpeted */
	loop_until_bit_is_set(SPSR,SPIF);
	r = SPDR;
	return r;
}

void putchar_uart( unsigned char data )
{
	/* Wait for empty transmit buffer */
	loop_until_bit_is_set(UCSR0A, UDRE0);
	/* Put data into buffer, sends the data */
	_delay_us(10); //8us mostly stable, 10us to be sure
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

/* get 24bit values in little endian */
uint32_t get24_le()
{
	uint32_t val = 0;

	val = (uint32_t)getchar_uart() << 0;
	val |= (uint32_t)getchar_uart() << 8;
	val |= (uint32_t)getchar_uart() << 16;

	return val;
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
			putchar_uart(SUPPORTED_COMMANDS_LOW);
			putchar_uart(0x00);
			putchar_uart(SUPPORTED_COMMANDS_HIGH);
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
			putchar_uart(S_ACK);
			putchar_uart(0xFF);
			putchar_uart(0xFF);
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
			switch (getchar_uart()) {
				case SUPPORTED_BUS:
					putchar_uart(S_ACK);
					break;
				default:
					putchar_uart(S_NAK);
					break;
			}
			break;
		case S_CMD_O_SPIOP:
			cli();
			/* get slen */
			slen = get24_le();
			/* get rlen */
			rlen = get24_le();

			select_chip();
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
			unselect_chip();
			sei();
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
	setup_uart(S_SPEED);
	setup_spi();
	sei();

	for(;;)
	{
		/* We're now interrupt driven */
	}

	return 1; /* Should not happen */
}
