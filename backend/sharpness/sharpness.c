#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "sharpness.h"

#define DOWNSCALE_FILE "/sys/devices/platform/jz-lcd.0/sharpness_downscaling"
#define UPSCALE_FILE "/sys/devices/platform/jz-lcd.0/sharpness_upscaling"

#ifndef MAX_SHARPNESS
#define MAX_SHARPNESS 32
#endif

static int current_value = -1;
static FILE *dev_file_upscale = NULL;
static FILE *dev_file_downscale = NULL;


//TonyJih : I'll sync upscale and downscale for now, change here if needed in future.
static int dev_file_open(void)
{
	if (!dev_file_upscale) {
		dev_file_upscale = fopen(UPSCALE_FILE, "r+");
		if (!dev_file_upscale)
			return -1;
		// Make stream unbuffered.
		setbuf(dev_file_upscale, NULL);
	}

	if (!dev_file_downscale) {
		dev_file_downscale = fopen(DOWNSCALE_FILE, "r+");
		if (!dev_file_downscale)
			return -1;
		// Make stream unbuffered.
		setbuf(dev_file_downscale, NULL);
	}
	
	return 0;
}

static void dev_file_close(void)
{
	if (dev_file_upscale) {
		fclose(dev_file_upscale);
		dev_file_upscale = NULL;
	}

	if (dev_file_downscale) {
		fclose(dev_file_downscale);
		dev_file_downscale = NULL;
	}
	
}

static int get_sharpness(void)
{
	int val;
	int num_read;

	if (dev_file_open() < 0)
		return -1;

	num_read = fscanf(dev_file_upscale, "%d\n", &val);
	if (num_read == 0)
		return -1;

	return val;
}

static void set_sharpness(int value)
{
	if (dev_file_open() < 0)
		return ;

	fprintf(dev_file_upscale, "%d\n", value);
	fflush(dev_file_upscale);
	fprintf(dev_file_downscale, "%d\n", value);
	fflush(dev_file_downscale);
}


static sharp_adjust(int event_value, int delta)
{
	int new_value;

	if (event_value == 0) { // key release
		dev_file_close();
		return;
	}

	if (event_value == 1) { // new key press (not a repeat)
		// Make sure we fetch an up-to-date value in the next step.
		dev_file_close();
		current_value = -1;
	}

	if (current_value == -1) {
		current_value = get_sharpness();
		if (current_value == -1)
			return;
	}

	new_value = current_value + delta;
	if (new_value < 0)
		new_value = 0;
	else if (new_value > MAX_SHARPNESS)
		new_value = MAX_SHARPNESS;

	if (new_value != current_value) {
		current_value = new_value;
		set_sharpness(current_value);
	}
}

void sharp_up(int event_value)
{
	sharp_adjust(event_value, 1);
}

void sharp_down(int event_value)
{
	sharp_adjust(event_value, -1);
}
