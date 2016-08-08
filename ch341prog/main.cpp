
#include "stdafx.h"

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "ch341.h"
#include "spi_flash.h"

static void ShowUsage(void)
{
	puts(
		"Usage:\n"
		"  probe\n"
		"  read <file> [<addr> [size]]\n"
		"  erase [chip | <addr> <size>]\n"
		"  write [erase] [verify] <file> [addr] [size]\n");
}

static int DoFlashRead(int argc, char *argv[])
{
	unsigned int addr = 0, size;
	const char *filename;
	unsigned char *buff;
	FILE *f;

	if (!FlashProbe())
		return -ENODEV;

	size = FlashGetSize();

	if (!argc)
	{
		fprintf(stderr, "Error: please specify a filename.\n");
		return -EINVAL;
	}

	filename = argv[0];

	argc--;
	argv++;

	if (argc)
	{
		if (!isdigit(argv[0][0]))
		{
			fprintf(stderr, "Please input a numeric flash address!\n");
			return -EINVAL;
		}

		addr = strtoul(argv[0], NULL, 0);

		if (addr >= FlashGetSize())
		{
			fprintf(stderr, "Error: start address exceeds the flash size!\n");
			return -EINVAL;
		}

		argc--;
		argv++;

		if (!argc)
			size = FlashGetSize() - addr;
	}

	if (argc)
	{
		if (!isdigit(argv[0][0]))
		{
			fprintf(stderr, "Please input a numeric size!\n");
			return -EINVAL;
		}

		size = strtoul(argv[0], NULL, 0);

		if (addr + size > FlashGetSize())
		{
			fprintf(stderr, "Error: end address exceeds the flash size!\n");
			return -EINVAL;
		}
	}

	buff = new unsigned char[size];
	if (!buff)
	{
		fprintf(stderr, "Error: unable to allocate memory!\n");
		return -ENOMEM;
	}

	printf("Reading flash from %xh, size %xh ...\n", addr, size);

	if (!FlashRead(addr, size, buff))
	{
		printf("Operation failed.\n");
		delete[] buff;
		return -EFAULT;
	}

	printf("Saving to file %s ...\n", filename);

	f = fopen(filename, "wb");
	if (!f)
	{
		fprintf(stderr, "Error: unable to open/create file! error %d\n", errno);
		delete[] buff;
		return -errno;
	}

	if (fwrite(buff, 1, size, f) != size)
	{
		fprintf(stderr, "Error: failed to write to file! error %d\n", errno);
		delete[] buff;
		return -errno;
	}

	fclose(f);

	printf("Done.\n");

	return 0;
}

static int DoFlashChipErase(int argc, char *argv[])
{
	if (!FlashProbe())
		return -ENODEV;

	printf("Erasing entire flash, please wait ...\n");

	if (!FlashChipErase())
	{
		printf("Operation failed.\n");
		return -EFAULT;
	}

	printf("Done.\n");

	return 0;
}

static int DoFlashErase(int argc, char *argv[])
{
	unsigned int addr = 0, size;

	if (!FlashProbe())
		return -ENODEV;

	size = FlashGetSize();

	if (!isdigit(argv[0][0]))
	{
		fprintf(stderr, "Please input a numeric flash address!\n");
		return -EINVAL;
	}

	addr = strtoul(argv[0], NULL, 0);

	if (addr >= FlashGetSize())
	{
		fprintf(stderr, "Error: start address exceeds the flash size!\n");
		return -EINVAL;
	}

	if (!isdigit(argv[1][0]))
	{
		fprintf(stderr, "Please input a numeric size!\n");
		return -EINVAL;
	}

	size = strtoul(argv[1], NULL, 0);

	if (addr + size > FlashGetSize())
	{
		fprintf(stderr, "Error: end address exceeds the flash size!\n");
		return -EINVAL;
	}

	printf("Erasing flash from %xh, size %xh ...\n", addr, size);

	if (!FlashErase(addr, size))
	{
		printf("Operation failed.\n");
		return -EFAULT;
	}

	printf("Done.\n");

	return 0;
}

static int DoFlashWrite(int argc, char *argv[])
{
	int need_erase = 0, need_verify = 0, size_set = 0, pass = 1;
	unsigned int addr = 0, size, filelen, i;
	const char *filename;
	unsigned char *buff, *buff_check;
	FILE *f;

	if (!FlashProbe())
		return -ENODEV;

	size = FlashGetSize();

	if (!argc)
	{
_insufficinet_param:
		fprintf(stderr, "Error: please specify a filename.\n");
		return -EINVAL;
	}

	if (!strcmp(argv[0], "erase"))
	{
		need_erase = 1;
		argc--;
		argv++;
	}

	if (!argc)
		goto _insufficinet_param;

	if (!strcmp(argv[0], "verify"))
	{
		need_verify = 1;
		argc--;
		argv++;
	}

	if (!argc)
		goto _insufficinet_param;

	filename = argv[0];

	argc--;
	argv++;

	if (argc)
	{
		if (!isdigit(argv[0][0]))
		{
			fprintf(stderr, "Please input a numeric flash address!\n");
			return -EINVAL;
		}

		addr = strtoul(argv[0], NULL, 0);

		if (addr >= FlashGetSize())
		{
			fprintf(stderr, "Error: start address exceeds the flash size!\n");
			return -EINVAL;
		}

		argc--;
		argv++;

		if (!argc)
			size = FlashGetSize() - addr;
	}

	if (argc)
	{
		if (!isdigit(argv[0][0]))
		{
			fprintf(stderr, "Please input a numeric size!\n");
			return -EINVAL;
		}

		size = strtoul(argv[0], NULL, 0);

		if (addr + size > FlashGetSize())
		{
			fprintf(stderr, "Error: end address exceeds the flash size!\n");
			return -EINVAL;
		}

		size_set = 1;
	}

	printf("Reading file %s ...\n", filename);

	f = fopen(filename, "rb");
	if (!f)
	{
		fprintf(stderr, "Error: unable to open file! error %d\n", errno);
		return -errno;
	}

	fseek(f, 0, SEEK_END);
	filelen = ftell(f);

	fseek(f, 0, SEEK_SET);

	if (filelen > size)
	{
		fprintf(stderr, "Warning: file size is larger than write size.\n");
	}
	else if (filelen < size)
	{
		if (size_set)
			fprintf(stderr, "Warning: write size is larger than file size, write size truncated to 0x%x.\n", filelen);
		size = filelen;
	}

	buff = new unsigned char[size];
	if (!buff)
	{
		fprintf(stderr, "Error: unable to allocate memory!\n");
		return -ENOMEM;
	}

	if (fread(buff, 1, size, f) != size)
	{
		fprintf(stderr, "Error: failed to read file! error %d\n", errno);
		delete[] buff;
		return -errno;
	}

	fclose(f);

	printf("Done.\n\n");

	if (need_erase)
	{
		printf("Erasing flash from %xh, size %xh ...\n", addr, size);

		if (!FlashErase(addr, size))
		{
			printf("Operation aborted.\n");
			delete[] buff;
			return -EFAULT;
		}

		printf("Done.\n\n");
	}

	printf("Writing flash at %xh, size %xh ...\n", addr, size);

	if (!FlashWrite(addr, buff, size))
	{
		printf("Operation aborted.\n");
		delete[] buff;
		return -EFAULT;
	}

	printf("Done.\n");

	if (need_verify)
	{
		printf("\n");

		buff_check = new unsigned char[size];
		if (!buff_check)
		{
			fprintf(stderr, "Error: unable to allocate memory!\n");
			return -ENOMEM;
		}

		printf("Reading flash from %xh, size %xh ...\n", addr, size);

		if (!FlashRead(addr, size, buff_check))
		{
			printf("Operation failed.\n");
			delete[] buff;
			delete[] buff_check;
			return -EFAULT;
		}

		printf("Done.\n\n");

		printf("Verifying ...\n");

		for (i = 0; i < size; i++)
		{
			if (buff[i] != buff_check[i])
			{
				printf("Difference at 0x%08x, read 0x%02x, expected 0x%02x\n", addr + i, buff_check[i], buff[i]);
				pass = 0;
			}
		}

		if (pass)
			printf("Passed.\n");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int argv_c = argc - 1, argv_p = 1;
	int ret = 0;

	printf("Simple CH341 SPI Flash Programmer\nBy HackPascal <hackpascal@gmail.com>\n\n");

	CH341DeviceInit();

	if (argc == 1)
	{
	_show_usage:
		ShowUsage();
		goto cleanup;
	}

	if (!strcmp(argv[argv_p], "probe"))
	{
		FlashProbe();
		goto cleanup;
	}

	if (!strcmp(argv[argv_p], "read"))
	{
		argv_c--;
		argv_p++;

		if (argv_c < 1)
			goto _show_usage;

		ret = DoFlashRead(argv_c, argv + argv_p);
		goto cleanup;
	}

	if (!strcmp(argv[argv_p], "erase"))
	{
		argv_c--;
		argv_p++;

		if (argv_c < 1)
			goto _show_usage;

		if (!strcmp(argv[argv_p], "chip"))
		{
			ret = DoFlashChipErase(argv_c, argv + argv_p);
			goto cleanup;
		}

		if (argv_c < 2)
			goto _show_usage;

		ret = DoFlashErase(argv_c, argv + argv_p);
		goto cleanup;
	}

	if (!strcmp(argv[argv_p], "write"))
	{
		argv_c--;
		argv_p++;

		if (argv_c < 1)
			goto _show_usage;

		ret = DoFlashWrite(argv_c, argv + argv_p);
		goto cleanup;
	}

	goto _show_usage;

cleanup:
	CH341DeviceRelease();

    return ret;
}
