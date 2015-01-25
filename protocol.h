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

#ifndef SERPROG_PROTOCOL_H
#define SERPROG_PROTOCOL_H

#define S_ACK			0x06
#define S_NAK			0x15

#define S_CMD_NOP		0x00UL /* No operation                                 */
#define S_CMD_Q_IFACE		0x01UL /* Query interface version                      */
#define S_CMD_Q_CMDMAP		0x02UL /* Query supported commands bitmap              */
#define S_CMD_Q_PGMNAME		0x03UL /* Query programmer name                        */
#define S_CMD_Q_SERBUF		0x04UL /* Query Serial Buffer Size                     */
#define S_CMD_Q_BUSTYPE		0x05UL /* Query supported bustypes                     */
#define S_CMD_Q_CHIPSIZE	0x06UL /* Query supported chipsize (2^n format)        */
#define S_CMD_Q_OPBUF		0x07UL /* Query operation buffer size                  */

#define S_CMD_Q_WRNMAXLEN	0x08UL /* Query Write to opbuf: Write-N maximum length */
#define S_CMD_R_BYTE		0x09UL /* Read a single byte                           */
#define S_CMD_R_NBYTES		0x0AUL /* Read n bytes                                 */
#define S_CMD_O_INIT		0x0BUL /* Initialize operation buffer                  */
#define S_CMD_O_WRITEB		0x0CUL /* Write opbuf: Write byte with address         */
#define S_CMD_O_WRITEN		0x0DUL /* Write to opbuf: Write-N                      */
#define S_CMD_O_DELAY		0x0EUL /* Write opbuf: udelay                          */
#define S_CMD_O_EXEC		0x0FUL /* Execute operation buffer                     */

#define S_CMD_SYNCNOP		0x10UL /* Special no-operation that returns NAK+ACK    */
#define S_CMD_Q_RDNMAXLEN	0x11UL /* Query read-n maximum length                  */
#define S_CMD_S_BUSTYPE		0x12UL /* Set used bustype(s).                         */
#define S_CMD_O_SPIOP		0x13UL /* Perform SPI operation.                       */

/*
 * we must split in 3 parts because else avr-gcc doesn't seem to
 *  be able to compute stuff like 1<<S_CMD_SYNCNOP (it returns 0)
 */
#define SUPPORTED_COMMANDS_LOW ( ( \
        (1<<S_CMD_NOP) | (1<<S_CMD_Q_IFACE) | (1<<S_CMD_Q_CMDMAP) \
        | (1<<S_CMD_Q_PGMNAME) | (1<<S_CMD_Q_SERBUF) | (1<<S_CMD_Q_BUSTYPE) \
        ) & 0xff)
#define SUPPORTED_COMMANDS_HIGH ( ( ( \
        (1<<(S_CMD_SYNCNOP - 16)) | (1<<(S_CMD_O_SPIOP - 16)) | (1<<(S_CMD_S_BUSTYPE - 16)) \
        ) & 0xff ) )

#define SUPPORTED_BUS 0x08

#endif /* SERPROG_PROTOCOL_H */
