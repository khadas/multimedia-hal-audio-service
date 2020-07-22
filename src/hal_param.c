#include <stdio.h>
#include <audio_if.h>

int main(int argc, char **argv)
{
    audio_hw_device_t *device;
    int ret = audio_hw_load_interface(&device);

    if (argc < 2) {
        printf("Usage: hal_param <param string (kvpair)>\n");
        return -1;
    }

    if (ret) {
        fprintf(stderr, "audio_hw_load_interface failed: %d\n", ret);
        return ret;
    }

    int inited = device->init_check(device);
    if (inited) {
        printf("device not inited, quit\n");
        audio_hw_unload_interface(device);
        return -1;
    }

    ret = device->set_parameters(device, argv[1]);

    audio_hw_unload_interface(device);

    printf("Parameters sent to Audio HAL.\n");

    return ret;
}

