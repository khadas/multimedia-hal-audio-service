/*
 * Copyright (C) 2016 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "audio_if.h"

int quit_flag = 0;
audio_hw_device_t *device;
struct audio_stream_in *stream;

void handler(int sig)
{
    quit_flag = 1;
}

void capture_read_input_stream()
{
    int ret = 0;
    int read_len = 0;
    int flen = 0;
    char *buffer = NULL;

    buffer = (char *)calloc(1, 4*1024 * sizeof(char));
    FILE *fp = fopen("/data/audio/hal_capture.raw", "a+");
    if (fp) {
        while (!quit_flag) {
            ret = stream->read(stream, buffer, 4*1024);
            read_len += (ret/1024);
            if (read_len%10 == 0) {
                printf("data reading:%dk\n", read_len);
            }
            flen = fwrite((char *)buffer, 1, 4*1024, fp);
            memset(buffer, 0, 4*1024);
        }

        printf("read all data:%d k\n", read_len);
        free(buffer);
        buffer = NULL;
        fclose(fp);
    }

    return;
}

void capture_open_input_stream(struct audio_config *config)
{
    int ret = 0;

    printf("open input stream...\n");

    ret = device->open_input_stream(device, 0, AUDIO_DEVICE_IN_BUILTIN_MIC, config, &stream, AUDIO_INPUT_FLAG_RAW, NULL, AUDIO_SOURCE_MIC);
    if (ret) {
        printf("open input stream fail\n");
    } else {
        printf("open input stream success\n");
    }

    return;
}

void capture_close_input_stream()
{
    printf("close input stream...\n");
    device->close_input_stream(device, stream);
    return;
}

int main(int argc, char **argv)
{
    int ret = 0;
    unsigned int support_dev = 0;
    int inited = 0;
    struct audio_config config;

    signal(SIGINT, handler);

    ret = audio_hw_load_interface(&device);
    if (ret) {
        fprintf(stderr, "%s[%d]: load hw interface error:%d\n", __func__, __LINE__, ret);
        return ret;
    }

    printf("hw version: %x\n", device->common.version);
    printf("hal api version: %x\n", device->common.module->hal_api_version);
    printf("module id: %s\n", device->common.module->id);
    printf("module name: %s\n", device->common.module->name);

    if (device->get_supported_devices) {
        support_dev = device->get_supported_devices(device);
        printf("supported device: %x\n", support_dev);
    }

    inited = device->init_check(device);
    if (inited != 0) {
        printf("device not inited, quit\n");
        goto exit;
    }

    memset(&config, 0, sizeof(config));
    config.sample_rate = 16000;
    config.channel_mask = AUDIO_CHANNEL_IN_MONO;
    config.format = AUDIO_FORMAT_PCM_16_BIT;

    capture_open_input_stream(&config);
    capture_read_input_stream(stream);
    capture_close_input_stream();

exit:
    audio_hw_unload_interface(device);
    return 0;
}
