#include "spi.h"

#define SPI_CFG_ENABLE        (1<<0)
#define SPI_CFG_MASTER        (1<<2)
#define SPI_CFG_LSBF          (1<<3)
#define SPI_CFG_CPHA          (1<<4)
#define SPI_CFG_CPOL          (1<<5)
#define SPI_CFG_LOOP          (1<<7)
#define SPI_CFG_SPOL          (1<<8)

#define SPI_STAT_RXRDY        (1<<0)
#define SPI_STAT_TXRDY        (1<<1)
#define SPI_STAT_RXOV         (1<<2)
#define SPI_STAT_TXUR         (1<<3)
#define SPI_STAT_SSA          (1<<4)
#define SPI_STAT_SSD          (1<<5)
#define SPI_STAT_STALLED      (1<<6)
#define SPI_STAT_ENDTRANSFER  (1<<7)
#define SPI_STAT_MSTIDLE      (1<<8)

#define SPI_TXDATCTL_TXSSEL_N (1<<16)
#define SPI_TXDATCTL_EOT      (1<<20)
#define SPI_TXDATCTL_EOF      (1<<21)
#define SPI_TXDATCTL_RXIGNORE (1<<22)
#define SPI_TXDATCTL_LEN(n)   ((n&0xF)<<24)


void spi_init(LPC_SPI_TypeDef *spi)
{
  spi->DIV = 0;
  spi->DLY = 0;
  spi->CFG = SPI_CFG_MASTER;
  spi->CFG |= SPI_CFG_ENABLE;
}


void spi_send64_no_ss(LPC_SPI_TypeDef *spi, uint64_t data)
{
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDATCTL = SPI_TXDATCTL_LEN(15) | SPI_TXDATCTL_TXSSEL_N | SPI_TXDATCTL_RXIGNORE | SPI_TXDATCTL_EOF | ((data >> 48) & 0xFFFF);
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDAT = ((data >> 32) & 0xFFFF);
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDAT = ((data >> 16) & 0xFFFF);
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDATCTL = SPI_TXDATCTL_LEN(15) | SPI_TXDATCTL_TXSSEL_N | SPI_TXDATCTL_RXIGNORE | SPI_TXDATCTL_EOF | SPI_TXDATCTL_EOT | (data & 0xFFFF);
  while (!(spi->STAT & SPI_STAT_MSTIDLE)) {}
}


uint8_t spi_read_reg8(LPC_SPI_TypeDef *spi, uint8_t reg)
{
  uint16_t addr_and_data = reg << 8;
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDATCTL = SPI_TXDATCTL_LEN(15) | SPI_TXDATCTL_EOT | addr_and_data;
  while (!(spi->STAT & SPI_STAT_RXRDY)) {}
  return (spi->RXDAT & 0xFF);
}


void spi_write_reg8(LPC_SPI_TypeDef *spi, uint8_t reg, uint8_t value)
{
  uint16_t addr_and_data = (reg << 8) | value;
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDATCTL = SPI_TXDATCTL_LEN(15) | SPI_TXDATCTL_RXIGNORE | SPI_TXDATCTL_EOT | addr_and_data;
  while (!(spi->STAT & SPI_STAT_MSTIDLE)) {}
}


void spi_read_bytes(LPC_SPI_TypeDef *spi, uint8_t reg, int nbytes, uint8_t *bytes)
{
  /* Send address */
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDATCTL = SPI_TXDATCTL_LEN(7) | SPI_TXDATCTL_EOF | reg;
  while (!(spi->STAT & SPI_STAT_RXRDY)) {}
  spi->RXDAT;

  /* Read bytes */
  while (--nbytes) {
    while (!(spi->STAT & SPI_STAT_TXRDY)) {}
    spi->TXDAT = 0;
    while (!(spi->STAT & SPI_STAT_RXRDY)) {}
    *bytes++ = spi->RXDAT;
  }

  /* Read last byte */
  spi->TXDATCTL = SPI_TXDATCTL_LEN(7) | SPI_TXDATCTL_EOF | SPI_TXDATCTL_EOT;
  while (!(spi->STAT & SPI_STAT_RXRDY)) {}
  *bytes++ = spi->RXDAT;
}


void spi_write_bytes(LPC_SPI_TypeDef *spi, uint8_t reg, int nbytes, const uint8_t *bytes)
{
  /* Send address */
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDATCTL = SPI_TXDATCTL_LEN(7) | SPI_TXDATCTL_RXIGNORE | SPI_TXDATCTL_EOF | reg;

  /* Write bytes */
  while (--nbytes) {
    while (!(spi->STAT & SPI_STAT_TXRDY)) {}
    spi->TXDAT = *bytes++;
  }

  /* Write last byte */
  while (!(spi->STAT & SPI_STAT_TXRDY)) {}
  spi->TXDATCTL = SPI_TXDATCTL_LEN(7) | SPI_TXDATCTL_RXIGNORE | SPI_TXDATCTL_EOF | SPI_TXDATCTL_EOT | (*bytes);
  while (!(spi->STAT & SPI_STAT_MSTIDLE)) {}
}
