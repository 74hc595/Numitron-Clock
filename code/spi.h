#ifndef SPI_H_
#define SPI_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "LPC8xx.h"

void spi_init(LPC_SPI_TypeDef *spi);

/**
 * Sends a 64-bit number MSB first, without asserting slave select.
 * The value transmitted by the slave is ignored.
 */
void spi_send64_no_ss(LPC_SPI_TypeDef *spi, uint64_t value);

/**
 * Sends an 8-bit number MSB first, asserting slave select, then returns
 * one byte of data from the slave.
 */
uint8_t spi_read_reg8(LPC_SPI_TypeDef *spi, uint8_t reg);

/**
 * Sends an 8-bit register address followed by an 8-bit data byte, while
 * asserting slave select.
 */
void spi_write_reg8(LPC_SPI_TypeDef *spi, uint8_t reg, uint8_t value);

void spi_read_bytes(LPC_SPI_TypeDef *spi, uint8_t reg, int nbytes, uint8_t *bytes);

void spi_write_bytes(LPC_SPI_TypeDef *spi, uint8_t reg, int nbytes, const uint8_t *bytes);

#ifdef __cplusplus
}
#endif
#endif
