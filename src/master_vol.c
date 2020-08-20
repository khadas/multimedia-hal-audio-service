#include <stdio.h>
#include <stdlib.h>
#include <audio_if.h>

int main(int argc, char **argv)
{
    audio_hw_device_t *device;
    int ret;
    float vol;
    char cmd[256];

    if (argc < 3) {
        printf("Usage set volume             : master_vol s <volume: 0.0-1.0>\n");
        printf("      set master mute/unmute : master_vol m <0, 1>\n");
        return -1;
    }

    vol = atof(argv[1]);
    if ((vol < 0) || (vol > 1.0)) {
        printf("Volume must be between [0.0, 1.0]\n");
        return -1;
    }

    ret = audio_hw_load_interface(&device);
    if (ret) {
        fprintf(stderr, "audio_hw_load_interface failed: %d\n", ret);
        return ret;
    }

    ret = device->init_check(device);
    if (ret) {
        printf("device not inited, quit\n");
        audio_hw_unload_interface(device);
        return -1;
    }

    if (argv[1][0] == 's') {
        vol = atof(argv[2]);
        if ((vol < 0) || (vol > 1.0)) {
            printf("Volume must be between [0.0, 1.0]\n");
            audio_hw_unload_interface(device);
            return -1;
        }
        ret = device->set_master_volume(device, vol);
        printf("Volume %f set\n", vol);
    } else if (argv[1][0] == 'm') {
        ret = device->set_master_mute(device, atoi(argv[2]) == 1);
    } else {
        printf("Invalid command, use 's' or 'm' command.\n");
        ret = -2;
    }

    audio_hw_unload_interface(device);

    return ret;
}

