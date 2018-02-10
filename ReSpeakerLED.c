/*
 * RespeakerLED.c
 *
 * Copyright 2018 dimeiza. All Rights Reserved.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.1";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 800000;
static uint16_t delay;

struct LEDPatternRecord {
	char *LEDPatternName;
	int LEDPatternIndex;
};

struct MicHatRecord {
	char *MicName;
	int MicID;
};

static const struct MicHatRecord MicList[] = {
	{ "2mic", 0 },
	{ "4mic", 1 },
};

static const struct LEDPatternRecord LEDPatternList[] = {
	{"LED_TURN_OFF",0,},
	{"ALEXA_LISTENING",1},
	{"ALEXA_THINKING",2},
	{"ALEXA_SPEAKING",3},

	{"GOOGLEASSISTANT_ON_CONVERSATION_TURN_STARTED",4},
	{"GOOGLEASSISTANT_ON_RECOGNIZING_SPEECH_FINISHED",5},
	{"GOOGLEASSISTANT_ON_RESPONDING_STARTED",6},
};


uint8_t led_rgb_data_4Mic[8][48] = {

	//  LED_TURN_OFF
	{255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 	255, 0, 0, 0, 	255, 0, 0, 0, 	255, 0, 0, 0, 	255, 0, 0, 0, 	255, 0, 0, 0, },

	// ALEXA_LISTENING
	{255, 24, 0, 0,255, 24, 0, 0, 255, 24, 0, 0,255, 24, 0, 0,255, 24, 0, 0,255, 24, 0, 0,255, 24, 48, 0,	255, 24, 0, 0,255, 24, 0, 0, 	255, 24, 0, 0, 	255, 24, 0, 0, 	255, 24, 0, 0},

	// ALEXA_THINKING
	{255, 12, 12, 0, 255, 24, 0, 0, 255, 12, 12, 0, 255, 24, 0, 0, 255, 12, 12, 0, 255, 24, 0, 0, 255, 12, 12, 0, 255, 24, 0, 0, 255, 12, 12, 0, 255, 24, 0, 0, 255, 12, 12, 0, 255, 24, 0, 0},

	// ALEXA_SPEAKING
	{255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0},

	// GOOGLEASSISTANT_ON_CONVERSATION_TURN_STARTED
	{255, 48, 0, 0, 	255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 48, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 24, 24, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 48, 0, 255, 0, 0, 0, 255, 0, 0, 0},

	// GOOGLEASSISTANT_ON_RESPONDING_STARTED
	{255, 48, 0, 0, 255, 48, 0, 0, 255, 0, 0, 0, 255, 0, 0, 48, 255, 0, 0, 48, 255, 0, 0, 0, 255, 0, 24, 24, 255, 0, 24, 24, 255, 0,0,0, 255, 0, 48, 0, 255, 0, 48, 0, 255, 0, 0, 0},

	// GOOGLEASSISTANT_ON_RECOGNIZING_SPEECH_FINISHED
	{255, 48, 0, 0, 255, 48, 0, 0, 255, 48, 0, 0, 255, 0, 0, 48, 255, 0, 0, 48, 255, 0, 0, 48, 255, 0, 24, 24, 255, 0, 24, 24, 255, 0, 24, 24, 255, 0, 48, 0, 255, 0, 48, 0, 255, 0, 48, 0},

};

uint8_t led_rgb_data_2Mic[8][12] = {

	//  LED_TURN_OFF
	{255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, },

	// ALEXA_LISTENING
	{255, 48, 0, 0,	255, 255, 255, 255,255, 48, 0, 0,},

	// ALEXA_THINKING
	{255, 48, 0, 0, 255, 24, 24, 0, 255, 48, 0, 0,} ,
	
	// ALEXA_SPEAKING
	{255, 12, 12, 0, 255, 12, 12, 0, 255, 12, 12, 0, },

	// GOOGLEASSISTANT_ON_CONVERSATION_TURN_STARTED
	{255, 0, 48, 0, 255, 0, 0, 0, 255, 0, 0, 0,},

	// GOOGLEASSISTANT_ON_RECOGNIZING_SPEECH_FINISHED
	{255, 0, 48, 0, 255, 48, 0, 0, 255, 0, 0, 0,},

	// GOOGLEASSISTANT_ON_RESPONDING_STARTED
	{255, 0, 48, 0, 255, 48, 0, 0,255, 0, 0, 48, },

};

uint8_t startFrame[] = { 0x00, 0x00, 0x00, 0x00 };

uint8_t endFrame[] =  { 0x00,0x00 };

static int getMicID(char *MicName){
	
	for(int i = 0;i < ARRAY_SIZE(MicList);i++){
		if(!strcmp(MicList[i].MicName,MicName)){
			return MicList[i].MicID;
		}
	}
	return -1;
}
static int getLEDPatternIndex(char *LEDPatternName){
	
	for(int i = 0;i < ARRAY_SIZE(LEDPatternList);i++){
		if(!strcmp(LEDPatternList[i].LEDPatternName,LEDPatternName)){
			return LEDPatternList[i].LEDPatternIndex;
		}
	}
	
	return -1;
}

static int sendSPI(int fd,unsigned char *sendData,int length){
	
	int ret;
	uint8_t rx[256] = {0};

	struct spi_ioc_transfer transferMessage = {
		.tx_buf = (unsigned long)sendData,
		.rx_buf = (unsigned long)rx,
		.len = length,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	
	return ioctl(fd, SPI_IOC_MESSAGE(1), &transferMessage);
}

static void controlLED(int hatID ,int pattern){
	
	uint8_t *LEDPattern = NULL;
	int patternLength = 0; 
	
	int fd = open(device, O_RDWR);
	if (fd < 0){
		pabort("can't open device");
	}
	
	
	switch(hatID){
		case 0:
			LEDPattern = led_rgb_data_2Mic[pattern];
			patternLength = ARRAY_SIZE(led_rgb_data_2Mic[pattern]);
			break;
		case 1:
			LEDPattern = led_rgb_data_4Mic[pattern];
			patternLength = ARRAY_SIZE(led_rgb_data_4Mic[pattern]);
			break;
		default:
			pabort("This hat is not supported.");
			break;
	}
	
	// If we want to use the APA102 RGB LEDs, please write HIGH to GPIO5 first to enable VCC of the LEDs.
	system("gpio -g mode 5 out");
	system("gpio -g write 5 1");
	

	if(sendSPI(fd,startFrame,ARRAY_SIZE(startFrame)) < 1){
		pabort("can't send start frame");
	}

	if(sendSPI(fd,LEDPattern,patternLength) < 1){
		pabort("can't send LED Control Data");
	}
	
	if(sendSPI(fd,endFrame,ARRAY_SIZE(endFrame)) < 1){
		pabort("can't send end frame");
	}

	close(fd);
}

static const struct option lopts[] = {
	{ "device",  1, 0, 'D' },
	{ NULL, 0, 0, 0 },
};

static void print_usage(const char *prog)
{
	printf("Usage: %s [-D] pattern-name\n", prog);
	puts("-D --device   device to use (2mic or 4mic, default:2mic)");

	puts("pattern-name  LED pattern name ");
	for(int i = 0;i < ARRAY_SIZE(LEDPatternList);i++){
		puts(LEDPatternList[i].LEDPatternName);
	}

	exit(1);
}

int main(int argc, char *argv[])
{
	int pattern = 0;
	int hatID = 3;

	int c;
	char *target="2mic";

	c = getopt_long(argc, argv, "D:",lopts, NULL);

	switch (c) {
		case 'D':
			target = optarg;
			break;
	}
	
	hatID = getMicID(target);
	if(hatID == -1){
		print_usage(argv[0]);
	}
	
	if(argv[optind] == NULL){
		print_usage(argv[0]);
	}
	pattern = getLEDPatternIndex(argv[optind]);
	if(pattern == -1){
		print_usage(argv[0]);
	}

	controlLED(hatID,pattern);
	
	return 0;
	
}

