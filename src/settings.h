#ifndef SERPROG_SETTINGS_H
#define SERPROG_SETTINGS_H

#define SPI_PORT	PORTB
#define SCK		PORTB5		/* port 13 */
#define MISO		PORTB4		/* port 12 */
#define MOSI 		PORTB3		/* port 11 */
#define SS		PORTB2		/* port 10 */
#define DDR_SPI		DDRB

#define S_IFACE_VERSION	0x01		/* Version of the protocol */
#define S_PGM_NAME	"serprog-duino"	/* The program's name */

#endif /* SERPROG_SETTINGS_H */
