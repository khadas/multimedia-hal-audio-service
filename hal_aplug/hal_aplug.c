/*
 *  PCM - AMLOGIC Audio HAL plugin
 *
 *  Copyright (c) 2022 by Wei Du <wei.du@amlogic.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm_external.h>
#include "audio_if.h"

#define DEBUG
#ifdef DEBUG
#define DBG(f, ...) \
    fprintf(stderr, "%s:%i %i "f"\n", __FUNCTION__, __LINE__, getpid(), ## __VA_ARGS__);
#else
#define DBG(f, ...)
#endif
//#define VDEBUG
#ifdef VDEBUG
#define VDBG(f, ...) \
    fprintf(stderr, "%s:%i %i "f"\n", __FUNCTION__, __LINE__, getpid(), ## __VA_ARGS__);
#else
#define VDBG(f, ...)
#endif

#define FRAME_SIZE 6

typedef struct {
    snd_pcm_ioplug_t io;
    audio_hw_device_t *device;
    struct audio_stream_out *stream;
    struct audio_config config;

    struct pollfd pfd;

    unsigned periods_done;

    unsigned channels;
    snd_pcm_uframes_t period_size;
    unsigned int rate;
} snd_pcm_hal_t;


static void hal_free(snd_pcm_hal_t *ahal)
{
    free(ahal);
}
static int snd_pcm_hal_close(snd_pcm_ioplug_t *io)
{
    snd_pcm_hal_t *ahal = io->private_data;
    ahal->device->close_output_stream(ahal->device, ahal->stream);
    audio_hw_unload_interface(ahal->device);
    hal_free(ahal);

    return 0;
}

static snd_pcm_sframes_t snd_pcm_hal_pointer(snd_pcm_ioplug_t *io)
{
    snd_pcm_hal_t *ahal = io->private_data;
    snd_pcm_sframes_t hw_pointer = 0;

    switch (io->state) {
    case SND_PCM_STATE_RUNNING:
        hw_pointer = ahal->periods_done*1536;
        break;
    case SND_PCM_STATE_XRUN:
        hw_pointer = -EPIPE;
        break;
    default:
        hw_pointer = 0;
        break;
    }

    return hw_pointer;
}

static int snd_pcm_hal_prepare(snd_pcm_ioplug_t *io)
{
    snd_pcm_hal_t *ahal = io->private_data;
    int ret;

    if (ahal && ahal->stream) {
        ret = ahal->stream->common.standby(&ahal->stream->common);
        if (ret) {
            fprintf(stderr, "%s %d, ret:%x\n",
                    __func__, __LINE__, ret);
            return -1;
        }
    }
    return 0;
}

static int snd_pcm_hal_start(snd_pcm_ioplug_t *io)
{
    snd_pcm_hal_t *ahal = io->private_data;

    return 0;
}

static int snd_pcm_hal_stop(snd_pcm_ioplug_t *io)
{
    snd_pcm_hal_t *ahal = io->private_data;

    return 0;
}

static snd_pcm_sframes_t snd_pcm_hal_write(snd_pcm_ioplug_t *io,
                   const snd_pcm_channel_area_t *areas,
                   snd_pcm_uframes_t offset,
                   snd_pcm_uframes_t size)
{
    void *playback_addr;
    snd_pcm_hal_t *ahal = io->private_data;
    char *buf;
    size_t writebytes;

    buf = (char *) areas->addr + (areas->first +
                    areas->step * offset) / 8;
    writebytes = size * 2 * 2;

    ssize_t s = ahal->stream->write(ahal->stream, buf, writebytes);
    if (s < 0) {
        fprintf(stderr, "stream writing error %d\n", s);
    }

    ahal->periods_done++;
    return size;
}

static snd_pcm_sframes_t snd_pcm_hal_read(snd_pcm_ioplug_t *io,
                  const snd_pcm_channel_area_t *areas,
                  snd_pcm_uframes_t offset,
                  snd_pcm_uframes_t size)
{
    snd_pcm_hal_t *ahal = io->private_data;
    snd_pcm_uframes_t red;

    if (size) {

    } else
        if (io->state == SND_PCM_STATE_XRUN)
            return -EPIPE;
    return 0;
}

static snd_pcm_ioplug_callback_t hal_playback_callback = {
    .close = snd_pcm_hal_close,
    .start = snd_pcm_hal_start,
    .stop = snd_pcm_hal_stop,
    .transfer = snd_pcm_hal_write,
    .pointer = snd_pcm_hal_pointer,
    .prepare = snd_pcm_hal_prepare,
};
static snd_pcm_ioplug_callback_t hal_capture_callback = {
    .close = snd_pcm_hal_close,
    .start = snd_pcm_hal_start,
    .stop = snd_pcm_hal_stop,
    .transfer = snd_pcm_hal_read,
    .pointer = snd_pcm_hal_pointer,
    .prepare = snd_pcm_hal_prepare,
};

#define ARRAY_SIZE(ary) (sizeof(ary)/sizeof(ary[0]))

static int hal_set_hw_constraint(snd_pcm_hal_t *ahal)
{
    unsigned access_list[] = {
        SND_PCM_ACCESS_RW_INTERLEAVED,
    };
    unsigned format_list[] = {
        SND_PCM_FORMAT_S16_LE,
    };

    int err;
    unsigned int rate_min = ahal->rate ? ahal->rate : 44100,
        rate_max = ahal->rate ? ahal->rate : 96000,
        period_bytes_min = ahal->period_size ? FRAME_SIZE * ahal->period_size : 128,
        period_bytes_max = ahal->period_size ? FRAME_SIZE * ahal->period_size : 64*4096;

    if ((err = snd_pcm_ioplug_set_param_list(&ahal->io, SND_PCM_IOPLUG_HW_ACCESS,
                         ARRAY_SIZE(access_list), access_list)) < 0 ||
        (err = snd_pcm_ioplug_set_param_list(&ahal->io, SND_PCM_IOPLUG_HW_FORMAT,
                         ARRAY_SIZE(format_list), format_list)) < 0 ||
        (err = snd_pcm_ioplug_set_param_minmax(&ahal->io, SND_PCM_IOPLUG_HW_CHANNELS,
                         ahal->channels, ahal->channels)) < 0 ||
        (err = snd_pcm_ioplug_set_param_minmax(&ahal->io, SND_PCM_IOPLUG_HW_RATE,
                           rate_min, rate_max)) < 0 ||
        (err = snd_pcm_ioplug_set_param_minmax(&ahal->io, SND_PCM_IOPLUG_HW_PERIOD_BYTES,
                           period_bytes_min, period_bytes_max)) < 0 ||
        (err = snd_pcm_ioplug_set_param_minmax(&ahal->io, SND_PCM_IOPLUG_HW_PERIODS,
                           2, 2)) < 0)
        return err;

    return 0;
}

static int snd_pcm_hal_open_stream(snd_pcm_hal_t *ahal)
{
    int err;
    err = audio_hw_load_interface(&ahal->device);
    if (err) {
        fprintf(stderr, "%s %d error:%d\n", __func__, __LINE__, err);
        return err;
    }

    if (ahal->device->get_supported_devices) {
        uint32_t support_dev = 0;
        support_dev = ahal->device->get_supported_devices(ahal->device);
        VDBG("supported device: %x\n", support_dev);
    }

    int inited = ahal->device->init_check(ahal->device);
    if (inited) {
        VDBG("device not inited, quit\n");
        err = -1;
        goto exit;
    }

    memset(&ahal->config, 0, sizeof(struct audio_config));
    ahal->config.sample_rate = 48000;
    ahal->config.channel_mask = AUDIO_CHANNEL_OUT_STEREO;
    ahal->config.format = AUDIO_FORMAT_PCM_16_BIT;

    err = ahal->device->open_output_stream(ahal->device,
            0, AUDIO_DEVICE_OUT_SPEAKER,
            AUDIO_OUTPUT_FLAG_PRIMARY, &ahal->config,
            &ahal->stream, NULL);
    if (err) {
        VDBG("fail\n");
        goto exit;
    }

    return 0;
exit:
    audio_hw_unload_interface(ahal->device);
    return -1;
}

static int snd_pcm_hal_open(snd_pcm_t **pcmp, const char *name,
                   const char *card,
                   snd_pcm_stream_t stream, int mode,
                   snd_pcm_uframes_t period_size,
                   unsigned int rate)
{
    snd_pcm_hal_t *ahal;
    int err;

    assert(pcmp);
    ahal = calloc(1, sizeof(*ahal));
    if (!ahal)
        return -ENOMEM;

    ahal->channels = 2;
    ahal->period_size = period_size;
    ahal->rate = rate;

    ahal->io.version = SND_PCM_IOPLUG_VERSION;
    ahal->io.name = "ALSA <-> AMLOGIC HAL PCM I/O Plugin";
    ahal->io.callback = stream == SND_PCM_STREAM_PLAYBACK ?
        &hal_playback_callback : &hal_capture_callback;
    ahal->io.private_data = ahal;
    ahal->io.mmap_rw = 0;
    ahal->io.poll_fd = ahal->pfd.fd;
    ahal->io.poll_events = stream == SND_PCM_STREAM_PLAYBACK ? POLLOUT : POLLIN;

    err = snd_pcm_ioplug_create(&ahal->io, name, stream, mode);
    if (err < 0) {
        hal_free(ahal);
        return err;
    }

    err = hal_set_hw_constraint(ahal);
    if (err < 0) {
        snd_pcm_ioplug_delete(&ahal->io);
        return err;
    }

    snd_pcm_hal_open_stream(ahal);
    *pcmp = ahal->io.pcm;

    return 0;
}


SND_PCM_PLUGIN_DEFINE_FUNC(ahal)
{
    snd_config_iterator_t i, next;
    const char *card;
    int err;
    long period_size = 0, rate = 0;

    snd_config_for_each(i, next, conf) {
        snd_config_t *n = snd_config_iterator_entry(i);
        const char *id;
        if (snd_config_get_id(n, &id) < 0)
            continue;

        if (strcmp(id, "comment") == 0 || strcmp(id, "type") == 0)
            continue;
        if (strcmp(id, "card") == 0) {
            continue;
        }
        if (strcmp(id, "period_size") == 0) {
            if (snd_config_get_type(n) != SND_CONFIG_TYPE_INTEGER) {
                SNDERR("Invalid type for %s", id);
                return -EINVAL;
            }
            snd_config_get_integer(n, &period_size);
            continue;
        }
        if (strcmp(id, "rate") == 0) {
            if (snd_config_get_type(n) != SND_CONFIG_TYPE_INTEGER) {
                SNDERR("Invalid type for %s", id);
                return -EINVAL;
            }
            snd_config_get_integer(n, &rate);
            continue;
        }
        SNDERR("Unknown field %s", id);
        return -EINVAL;
    }

    err = snd_pcm_hal_open(pcmp, name, card, stream, mode, period_size, rate);

    return err;
}

SND_PCM_PLUGIN_SYMBOL(ahal);
