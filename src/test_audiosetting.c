/*
 * Copyright (C) 2023 Amlogic Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <AML_Audio_Setting.h>

static void usage()
{
    printf("Usage:\n");
    printf("      set volume          : test_audiosetting volume x (x is vol value, 0~100)\n");
    printf("      get volume          : test_audiosetting volume\n");
    printf("      set mute            : test_audiosetting mute port x (port:hdmi or hp, x is mute:1 or unmute:0)\n");
    printf("      get mute            : test_audiosetting mute port (port:hdmi or hp)\n");
}

int main(int argc, char **argv)
{
    float vol;
    int ret;
    if (argc < 2) {
        usage();
        return -1;
    }
    if (!strncmp("volume", argv[1], sizeof("volume")) && argv[2] != NULL) {
        ret = aml_audio_set_volume(atoi(argv[2]));
        printf("aml_audio_set_volume(%d): ret = %d\n", atoi(argv[2]), ret);
    } else if (!strncmp("volume", argv[1], sizeof("volume"))) {
        ret = aml_audio_get_volume();
        printf("aml_audio_get_volume: %d\n", ret);
    } else if (!strncmp("mute", argv[1], sizeof("mute")) && argv[2] != NULL && argv[3] != NULL) {
        if (!strncmp("hdmi", argv[2], sizeof("hdmi")))
            ret = aml_audio_set_mute(AUDIO_PORT_HDMI, atoi(argv[3]));
        else if (!strncmp("hp", argv[2], sizeof("hp")))
            ret = aml_audio_set_mute(AUDIO_PORT_HEADPHONE, atoi(argv[3]));
        printf("aml_audio_set_mute(%s: %d): ret = %d\n", argv[2], atoi(argv[3]), ret);
    } else if (!strncmp("mute", argv[1], sizeof("mute")) && argv[2] != NULL) {
        if (!strncmp("hdmi", argv[2], sizeof("hdmi")))
            ret = aml_audio_get_mute(AUDIO_PORT_HDMI);
        else if (!strncmp("hp", argv[2], sizeof("hp")))
            ret = aml_audio_get_mute(AUDIO_PORT_HEADPHONE);
        printf("aml_audio_get_mute(%s): %smute\n", argv[2], ret ? "" : "un");
    }
    return 0;
}
