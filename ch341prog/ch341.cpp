
#include "stdafx.h"

#include <memory.h>

#include <libusb-1.0/libusb.h>

#include "ch341.h"

static struct libusb_device_handle *CH341DeviceHanlde;

bool CH341DeviceInit(void)
{
	int ret;
	unsigned char desc[0x12];

	if (CH341DeviceHanlde)
		return true;

	if ((ret = libusb_init(NULL)))
	{
		fprintf(stderr, "Error: libusb_init failed: %d (%s)\n", ret, libusb_error_name(ret));
		return false;
	}

	if (!(CH341DeviceHanlde = libusb_open_device_with_vid_pid(NULL, CH341_USB_VID, CH341_USB_PID)))
	{
		fprintf(stderr, "Error: CH341 device (%04x/%04x) not found\n", CH341_USB_VID, CH341_USB_PID);
		return false;
	}

#if !defined(_MSC_VER) && !defined(MSYS) && !defined(CYGWIN) && !defined(WIN32) && !defined(MINGW) && !defined(MINGW32)
	if (libusb_kernel_driver_active(CH341DeviceHanlde, 0))
	{
		if ((ret = libusb_detach_kernel_driver(CH341DeviceHanlde, 0)))
		{
			fprintf(stderr, "Error: libusb_detach_kernel_driver failed: %d (%s)\n", ret, libusb_error_name(ret));
			goto cleanup;
		}
	}
#endif

	if ((ret = libusb_claim_interface(CH341DeviceHanlde, 0)))
	{
		fprintf(stderr, "Error: libusb_claim_interface failed: %d (%s)\n", ret, libusb_error_name(ret));
		goto cleanup;
	}

	if (!(ret = libusb_get_descriptor(CH341DeviceHanlde, LIBUSB_DT_DEVICE, 0x00, desc, 0x12)))
	{
		fprintf(stderr, "Warning: libusb_get_descriptor failed: %d (%s)\n", ret, libusb_error_name(ret));
	}

	printf("CH341 %d.%02d found.\n\n", desc[12], desc[13]);

	return true;

cleanup:
	libusb_close(CH341DeviceHanlde);
	CH341DeviceHanlde = NULL;
	return false;
}

void CH341DeviceRelease(void)
{
	if (!CH341DeviceHanlde)
		return;

	libusb_release_interface(CH341DeviceHanlde, 0);
	libusb_close(CH341DeviceHanlde);
	libusb_exit(NULL);

	CH341DeviceHanlde = NULL;
}

static int CH341USBTransferPart(enum libusb_endpoint_direction dir, unsigned char *buff, unsigned int size)
{
	int ret, bytestransferred;

	if (!CH341DeviceHanlde)
		return 0;

	if ((ret = libusb_bulk_transfer(CH341DeviceHanlde, CH341_USB_BULK_ENDPOINT | dir, buff, size, &bytestransferred, CH341_USB_TIMEOUT)))
	{
		fprintf(stderr, "Error: libusb_bulk_transfer for IN_EP failed: %d (%s)\n", ret, libusb_error_name(ret));
		return -1;
	}

	return bytestransferred;
}

static bool CH341USBTransfer(enum libusb_endpoint_direction dir, unsigned char *buff, unsigned int size)
{
	int pos, bytestransferred;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341USBTransferPart(dir, buff + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

#define CH341USBRead(buff, size) CH341USBTransfer(LIBUSB_ENDPOINT_IN, buff, size)
#define CH341USBWrite(buff, size) CH341USBTransfer(LIBUSB_ENDPOINT_OUT, buff, size)



bool CH341ChipSelect(unsigned int cs, bool enable)
{
	unsigned char pkt[4];

	static const int csio[4] = {0x36, 0x35, 0x33, 0x27};

	if (cs > 3)
	{
		fprintf(stderr, "Error: invalid CS pin %d, 0~3 are available\n", cs);
		return false;
	}

	pkt[0] = CH341_CMD_UIO_STREAM;
	if (enable)
		pkt[1] = CH341_CMD_UIO_STM_OUT | csio[cs];
	else
		pkt[1] = CH341_CMD_UIO_STM_OUT | 0x37;
	pkt[2] = CH341_CMD_UIO_STM_DIR | 0x3F;
	pkt[3] = CH341_CMD_UIO_STM_END;

	return CH341USBWrite(pkt, 4);
}

static int CH341TransferSPI(const unsigned char *in, unsigned char *out, unsigned int size)
{
	unsigned char pkt[CH341_PACKET_LENGTH];
	unsigned int i;

	if (!size)
		return 0;

	if (size > CH341_PACKET_LENGTH - 1)
		size = CH341_PACKET_LENGTH - 1;

	pkt[0] = CH341_CMD_SPI_STREAM;

	for (i = 0; i < size; i++)
		pkt[i + 1] = BitSwapTable[in[i]];

	if (!CH341USBWrite(pkt, size + 1))
	{
		fprintf(stderr, "Error: failed to transfer data to CH341\n");
		return -1;
	}

	if (!CH341USBRead(pkt, size))
	{
		fprintf(stderr, "Error: failed to transfer data from CH341\n");
		return -1;
	}

	for (i = 0; i < size; i++)
		out[i] = BitSwapTable[pkt[i]];

	return size;
}

bool CH341StreamSPI(const unsigned char *in, unsigned char *out, unsigned int size)
{
	int pos, bytestransferred;

	if (!size)
		return true;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(in + pos, out + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

bool CH341ReadSPI(unsigned char *out, unsigned int size)
{
	int pos, bytestransferred;
	unsigned char pkt[CH341_PACKET_LENGTH];

	if (!size)
		return true;

	memset(pkt, 0, sizeof (pkt));

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(pkt, out + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

bool CH341WriteSPI(const unsigned char *in, unsigned int size)
{
	int pos, bytestransferred;
	unsigned char pkt[CH341_PACKET_LENGTH];

	if (!size)
		return true;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(in + pos, pkt, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}
