#ifndef _CH341_H_
#define _CH341_H_

#define CH341_USB_VID				0x1A86
#define CH341_USB_PID				0x5512

#define CH341_USB_BULK_ENDPOINT		0x02
#define CH341_PACKET_LENGTH			0x20

#define CH341_USB_TIMEOUT			15000

#define CH341_CMD_SPI_STREAM		0xA8	//SPI command
#define CH341_CMD_UIO_STREAM		0xAB	//UIO command

#define	CH341_CMD_UIO_STM_IN		0x00	// UIO Interface In ( D0 ~ D7 )
#define	CH341_CMD_UIO_STM_DIR		0x40	// UIO interface Dir( set dir of D0~D5 )
#define	CH341_CMD_UIO_STM_OUT		0x80	// UIO Interface Output(D0~D5)
#define	CH341_CMD_UIO_STM_END		0x20	// UIO Interface End Command

bool CH341DeviceInit(void);
void CH341DeviceRelease(void);

bool CH341ChipSelect(unsigned int cs, bool enable);
bool CH341StreamSPI(const unsigned char *in, unsigned char *out, unsigned int size);
bool CH341ReadSPI(unsigned char *out, unsigned int size);
bool CH341WriteSPI(const unsigned char *in, unsigned int size);

static inline bool SPIWrite(const unsigned char *data, unsigned int size)
{
	if (!CH341ChipSelect(0, true))
		return false;
	if (!CH341WriteSPI(data, size))
		return false;
	return CH341ChipSelect(0, false);
}

static inline bool SPIRead(unsigned char *data, unsigned int size)
{
	if (!CH341ChipSelect(0, true))
		return false;
	if (!CH341ReadSPI(data, size))
		return false;
	return CH341ChipSelect(0, false);
}

static inline bool SPIWriteThenRead(const unsigned char *in, unsigned int in_size, unsigned char *out, unsigned int out_size)
{
	if (!CH341ChipSelect(0, true))
		return false;
	if (!CH341WriteSPI(in, in_size))
		return false;
	if (!CH341ReadSPI(out, out_size))
		return false;
	return CH341ChipSelect(0, false);
}

#endif /* _CH341_H_ */
