#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

typedef struct _spi_flash_id
{
	const char *model;
	unsigned int jedec_id;
	unsigned int ext_id;
	unsigned int size;
	unsigned int flags;
} spi_flash_id;

const spi_flash_id *spi_flash_id_lookup(unsigned int jedec_id, unsigned int ext_id);

bool FlashProbe(void);
unsigned int FlashGetSize(void);
bool FlashRead(unsigned int addr, unsigned int len, unsigned char *buf);
bool FlashErase(unsigned int addr, unsigned int len);
bool FlashChipErase(void);
bool FlashWrite(unsigned int addr, unsigned char *buff, unsigned int len);

#define SECTOR_4KB		(4 << 10)
#define SECTOR_32KB		(32 << 10)
#define SECTOR_64KB		(64 << 10)
#define SECTOR_256KB	(256 << 10)

#define SIZE_1MB		(1 << 20)
#define SIZE_2MB		(2 << 20)
#define SIZE_4MB		(4 << 20)
#define SIZE_8MB		(8 << 20)
#define SIZE_16MB		(16 << 20)
#define SIZE_32MB		(32 << 20)
#define SIZE_64MB		(64 << 20)
#define SIZE_128MB		(128 << 20)
#define SIZE_256MB		(256 << 20)

#define SF_64K_BLOCK	0x1
#define SF_32K_BLOCK	0x2
#define SF_256K_BLOCK	0x4
#define SF_4K_SECTOR	0x8
#define SF_4K_PMC		0x10
#define SF_SST			0x20
#define SF_INIT_SR		0x40
#define SF_BP0_2		0x80
#define SF_BP3			0x100
#define SF_BP4			0x200

#define SF_BP0_3		(SF_BP0_2 | SF_BP3)
#define SF_BP0_4		(SF_BP0_2 | SF_BP3 | SF_BP4)
#define SF_BP_ALL		SF_BP0_4

#define SR_BP0_2_MASK	0x1c
#define SR_BP3_MASK		0x20
#define SR_BP4_MASK		0x40


#define SPI_CMD_WRSR				0x01
#define SPI_CMD_RDSR				0x05

#define SPI_CMD_WRBR				0x17

#define SPI_CMD_WRDI				0x04
#define SPI_CMD_WREN				0x06

#define SPI_CMD_PAGE_PROG			0x02
#define SPI_CMD_READ				0x03

#define	SPI_CMD_AAI_WP				0xad

#define SPI_CMD_4KB_PMC_ERASE		0xd7
#define SPI_CMD_SECTOR_ERASE		0x20
#define SPI_CMD_32KB_BLOCK_ERASE	0x52
#define SPI_CMD_64KB_BLOCK_ERASE	0xd8
#define SPI_CMD_CHIP_ERASE			0xc7

#define SPI_CMD_RDID				0x9f

#define SPI_CMD_RESET_ENABLE		0x66
#define SPI_CMD_RESET_DEVICE		0x99

#define SPI_CMD_ENTER_4B_MODE		0xb7
#define SPI_CMD_EXIT_4B_MODE		0xe9

/* EON */
#define SPI_CMD_EXIT_HBL_MODE		0x98

/* Winbond */
#define SPI_CMD_WREAR				0xc5

#define JEDEC_MFR(_id)				(((_id) >> 16) & 0xff)
#define JEDEC_SIZE(_id)				((_id) & 0xff)

#define MFR_SST						0xbf
#define MFR_EON						0x1c
#define MFR_ISSI					0x9d
#define MFR_ATMEL					0x1f
#define MFR_MICROM					0x20
#define MFR_WINBOND					0xef
#define MFR_MACRONIX				0xc2
#define MFR_SPANSION				0x01
#define MFR_GIGADEVICE				0xc8

#define PAGE_SIZE					0x100

#endif /* _SPI_FLASH_H_ */
