/*
 * Copyright (C) 2021 Amlogic Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "audio_if_client.h"
#include "audio_effect_if.h"
#include "audio_effect_params.h"

#ifdef LOG
#undef LOG
#endif
#define LOG(x...) printf("[AudioEffect] " x)

//-----------Balance parameters-------------------------------
aml_balance_param_t gBalanceParam[] = {
    {{0, 4, 4}, BALANCE_PARAM_LEVEL, {100}},
    {{0, 4, 4}, BALANCE_PARAM_ENABLE, {1}},
    {{0, 4, 4}, BALANCE_PARAM_LEVEL_NUM, {BALANCE_MAX_BANDS}},
    {{0, 4, BALANCE_MAX_BANDS * 4}, BALANCE_PARAM_INDEX, {0}},
};

int balance_level_num = 0;
float index1[BALANCE_MAX_BANDS] = {0};
const char *BalanceStatusstr[] = {"Disable", "Enable"};

//-------------VirtualSurround parameters--------------------------
aml_virtualsurround_param_t gVirtualSurroundParam[] = {
    {{0, 4, 4}, VIRTUALSURROUND_PARAM_ENABLE, {0}},
    {{0, 4, 4}, VIRTUALSURROUND_PARAM_EFFECTLEVEL, {50}},
};

const char *VIRTUALSURROUNDStatusstr[] = {"Disable", "Enable"};

//-------------TrebleBass parameters--------------------------
aml_treblebass_param_t gTrebleBassParam[] = {
    {{0, 4, 4}, TREBASS_PARAM_BASS_LEVEL, {0}},
    {{0, 4, 4}, TREBASS_PARAM_TREBLE_LEVEL, {0}},
    {{0, 4, 4}, TREBASS_PARAM_ENABLE, {1}},
};

const char *TREBASSStatusstr[] = {"Disable", "Enable"};

//-------------Hpeq parameters--------------------------
aml_hpeq_param_t gHPEQParam[] = {
    {{0, 4, 4}, HPEQ_PARAM_ENABLE, {1}},
    {{0, 4, 4}, HPEQ_PARAM_EFFECT_MODE, {0}},
    {{0, 4, 5}, HPEQ_PARAM_EFFECT_CUSTOM, {0}},
};

const char *HPEQStatusstr[] = {"Disable", "Enable"};

//-------------AVL parameters--------------------------
aml_avl_param_t gAvlParam[] = {
    {{0, 4, 4}, AVL_PARAM_ENABLE, {1}},
    {{0, 4, 4}, AVL_PARAM_PEAK_LEVEL, {-18}},
    {{0, 4, 4}, AVL_PARAM_DYNAMIC_THRESHOLD, {-24}},
    {{0, 4, 4}, AVL_PARAM_NOISE_THRESHOLD, {-40}},
    {{0, 4, 4}, AVL_PARAM_RESPONSE_TIME, {512}},
    {{0, 4, 4}, AVL_PARAM_RELEASE_TIME, {2}},
    {{0, 4, 4}, AVL_PARAM_SOURCE_IN, {3}},
};

const char *AvlStatusstr[] = {"Disable", "Enable"};

//-------------function--------------------------
static int Balance_effect_func(int gParamIndex, int gParamValue)
{
    if (balance_level_num == 0) {
        audio_effect_get_parameters(AML_EFFECT_BALANCE, &gBalanceParam[BALANCE_PARAM_LEVEL_NUM].param);
        balance_level_num = gBalanceParam[BALANCE_PARAM_LEVEL_NUM].v;
        LOG("Balance: Level size = %d\n", balance_level_num);
    }
    audio_effect_get_parameters(AML_EFFECT_BALANCE, &gBalanceParam[BALANCE_PARAM_INDEX].param);
    for (int i = 0; i < balance_level_num; i++) {
        index1[i] = gBalanceParam[BALANCE_PARAM_INDEX].index[i];
        //LOG("Balance: index = %f\n", index1[i]);
    }
    switch (gParamIndex) {
    case BALANCE_PARAM_LEVEL:
        if (gParamValue  < 0 || gParamValue > ((balance_level_num - 1) << 1)) {
            LOG("Balance: Level gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gBalanceParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_BALANCE, &gBalanceParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_BALANCE, &gBalanceParam[gParamIndex].param);
        LOG("Balance: Level is %d -> %d\n", gParamValue, gBalanceParam[gParamIndex].v);
        return 0;
    case BALANCE_PARAM_ENABLE:
        if (gParamValue < 0 || gParamValue > 1) {
            LOG("Balance: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gBalanceParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_BALANCE, &gBalanceParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_BALANCE, &gBalanceParam[gParamIndex].param);
        LOG("Balance: Status is %d -> %s\n", gParamValue, BalanceStatusstr[gBalanceParam[gParamIndex].v]);
        return 0;
    default:
        LOG("Balance: ParamIndex = %d invalid\n", gParamIndex);
        return -1;
    }
}

static int VirtualSurround_effect_func(int gParamIndex, int gParamValue)
{
    switch (gParamIndex) {
    case VIRTUALSURROUND_PARAM_ENABLE:
        if (gParamValue < 0 || gParamValue > 1) {
            LOG("VirtualSurround: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gVirtualSurroundParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_VIRTUALSURROUND, &gVirtualSurroundParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_VIRTUALSURROUND, &gVirtualSurroundParam[gParamIndex].param);
        LOG("VirtualSurround: Status is %d -> %s\n", gParamValue, VIRTUALSURROUNDStatusstr[gVirtualSurroundParam[gParamIndex].v]);
        return 0;
    case VIRTUALSURROUND_PARAM_EFFECTLEVEL:
        if (gParamValue < 0 || gParamValue > 100) {
            LOG("VirtualSurround: level gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gVirtualSurroundParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_VIRTUALSURROUND, &gVirtualSurroundParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_VIRTUALSURROUND, &gVirtualSurroundParam[gParamIndex].param);
        LOG("VirtualSurround: level is %d -> %d \n", gParamValue, gVirtualSurroundParam[gParamIndex].v);
        return 0;
    default:
        LOG("VirtualSurround: ParamIndex = %d invalid\n", gParamIndex);
        return -1;
    }
}

static int TrebleBass_effect_func(int gParamIndex, int gParamValue)
{
    switch (gParamIndex) {
    case TREBASS_PARAM_BASS_LEVEL:
        if (gParamValue < 0 || gParamValue > 100) {
            LOG("TrebleBass: Bass gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gTrebleBassParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_TREBLEBASS, &gTrebleBassParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_TREBLEBASS, &gTrebleBassParam[gParamIndex].param);
        LOG("TrebleBass: Bass is %d -> %d level\n", gParamValue, gTrebleBassParam[gParamIndex].v);
        return 0;
    case TREBASS_PARAM_TREBLE_LEVEL:
        if (gParamValue < 0 || gParamValue > 100) {
            LOG("TrebleBass: Treble gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gTrebleBassParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_TREBLEBASS, &gTrebleBassParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_TREBLEBASS, &gTrebleBassParam[gParamIndex].param);
        LOG("TrebleBass: Treble is %d -> %d level\n", gParamValue, gTrebleBassParam[gParamIndex].v);
        return 0;
    case TREBASS_PARAM_ENABLE:
        if (gParamValue < 0 || gParamValue > 1) {
            LOG("TrebleBass: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gTrebleBassParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_TREBLEBASS, &gTrebleBassParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_TREBLEBASS, &gTrebleBassParam[gParamIndex].param);
        LOG("TrebleBass: Status is %d -> %s\n", gParamValue, TREBASSStatusstr[gTrebleBassParam[gParamIndex].v]);
        return 0;
    default:
        LOG("TrebleBass: ParamIndex = %d invalid\n", gParamIndex);
        return -1;
    }
}


static int HPEQ_effect_func(int gParamIndex, int gParamValue, signed char gParamBand[5])
{
    switch (gParamIndex) {
    case HPEQ_PARAM_ENABLE:
         if (gParamValue < 0 || gParamValue > 1) {
            LOG("HPEQ: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gHPEQParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_HPEQ, &gHPEQParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_HPEQ, &gHPEQParam[gParamIndex].param);
        LOG("HPEQ: Status is %d -> %s\n", gParamValue, HPEQStatusstr[gHPEQParam[gParamIndex].v]);
        return 0;
    case HPEQ_PARAM_EFFECT_MODE:
        if (gParamValue < 0 || gParamValue > 6) {
            LOG("Hpeq:gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gHPEQParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_HPEQ, &gHPEQParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_HPEQ, &gHPEQParam[gParamIndex].param);
        LOG("HPEQ: mode is %d -> %d\n", gParamValue, gHPEQParam[gParamIndex].v);
        return 0;
    case HPEQ_PARAM_EFFECT_CUSTOM:
        for (int i = 0; i < 5; i++) {
           if (gParamBand[i]< -10 || gParamBand[i] >10) {
              LOG("Hpeq:gParamBand[%d] = %d invalid\n",i, gParamBand[i]);
              return -1;
           }
        }
        gHPEQParam[gParamIndex].band[0] = gParamBand[0];
        gHPEQParam[gParamIndex].band[1] = gParamBand[1];
        gHPEQParam[gParamIndex].band[2] = gParamBand[2];
        gHPEQParam[gParamIndex].band[3] = gParamBand[3];
        gHPEQParam[gParamIndex].band[4] = gParamBand[4];
        audio_effect_set_parameters(AML_EFFECT_HPEQ, &gHPEQParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_HPEQ, &gHPEQParam[gParamIndex].param);
        return 0;
    default:
        LOG("HPEQ: ParamIndex = %d invalid\n", gParamIndex);
        return -1;
    }
}

static int Avl_effect_func(int gParamIndex, int gParamValue)
{
    switch (gParamIndex) {
    case AVL_PARAM_ENABLE:
         if (gParamValue < 0 || gParamValue > 1) {
            LOG("AVL: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gAvlParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        LOG("Avl: Status is %d -> %s\n", gParamValue, AvlStatusstr[gAvlParam[gParamIndex].v]);
        return 0;
    case AVL_PARAM_PEAK_LEVEL:
         if (gParamValue < -40 || gParamValue > 0) {
            LOG("AVL: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gAvlParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        LOG("Avl: peak_level is %d -> %d\n", gParamValue, gAvlParam[gParamIndex].v);
        return 0;
    case AVL_PARAM_DYNAMIC_THRESHOLD:
         if (gParamValue < -80 || gParamValue > 0) {
            LOG("AVL: dynamic_threshold = %d invalid\n", gParamValue);
            return -1;
        }
        gAvlParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        LOG("Avl: dynamic_threshold is %d -> %d\n", gParamValue, gAvlParam[gParamIndex].v);
        return 0;
     case AVL_PARAM_NOISE_THRESHOLD:
         if (gParamValue > -10) {
            LOG("AVL: noise_threshold = %d invalid\n", gParamValue);
            return -1;
        }
         gAvlParam[gParamIndex].v = gParamValue;
         audio_effect_set_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
         audio_effect_get_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
         LOG("Avl: noise_threshold is %d -> %d\n", gParamValue, gAvlParam[gParamIndex].v);
         return 0;
    case AVL_PARAM_RESPONSE_TIME:
         if (gParamValue < 20 || gParamValue > 2000) {
            LOG("AVL: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gAvlParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        LOG("Avl: Status is %d -> %d\n", gParamValue, gAvlParam[gParamIndex].v);
        return 0;
    case AVL_PARAM_RELEASE_TIME:
         if (gParamValue < 20 || gParamValue > 2000) {
            LOG("AVL: Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gAvlParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        LOG("Avl: Status is %d -> %d\n", gParamValue, gAvlParam[gParamIndex].v);
        return 0;
    case AVL_PARAM_SOURCE_IN:
        if (gParamValue < 0 || gParamValue > 5) {
            LOG("Avl: source_id gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gAvlParam[gParamIndex].v = gParamValue;
        audio_effect_set_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_AVL, &gAvlParam[gParamIndex].param);
        LOG("Avl: source_id is %d -> %d\n", gParamValue, gAvlParam[gParamIndex].v);
        return 0;
   default:
        LOG("Avl: ParamIndex = %d invalid\n", gParamIndex);
        return -1;
    }
}

int GetIntData (int *data)
{
    int status = scanf("%d", data);
    if (status == 0) {
        scanf("%*s");
        LOG("Erorr input! Pls Retry!\n");
        return -1;
    }
    return 0;
}

void PrintHelp(int gEffectIndex, char *name)
{
    if (gEffectIndex == AML_EFFECT_BALANCE) {
        LOG("**********************************Balance***********************************\n");
        LOG("Amlogic Balance EffectIndex: %d\n", (int)AML_EFFECT_BALANCE);
        LOG("Usage: %s %d <ParamIndex> <ParamValue/ParamScale/gParamBand>\n", name, (int)AML_EFFECT_BALANCE);
        LOG("ParamIndex: 0 -> Level\n");
        LOG("ParamValue: 0 ~ 100\n");
        LOG("ParamIndex: 1 -> Enable\n");
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("****************************************************************************\n\n");
    } else if (gEffectIndex == AML_EFFECT_VIRTUALSURROUND) {
        LOG("********************************VirtualSurround*********************************\n");
        LOG("Amlogic VirtualSurround EffectIndex: %d\n", (int)AML_EFFECT_VIRTUALSURROUND);
        LOG("Usage: %s %d <ParamIndex> <ParamValue/ParamScale/gParamBand>\n", name, (int)AML_EFFECT_VIRTUALSURROUND);
        LOG("ParamIndex: 0 -> Enable\n");
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: 1 -> Level\n");
        LOG("ParamValue: 0 ~ 100\n");
        LOG("****************************************************************************\n\n");
    } else if (gEffectIndex == AML_EFFECT_TREBLEBASS) {
        LOG("*********************************TrebleBass*********************************\n");
        LOG("Amlogic Treble/Bass EffectIndex: %d\n", (int)AML_EFFECT_TREBLEBASS);
        LOG("Usage: %s %d <ParamIndex> <ParamValue/ParamScale/gParamBand>\n", name, (int)AML_EFFECT_TREBLEBASS);
        LOG("ParamIndex: 0 -> Bass\n");
        LOG("ParamValue: 0 ~ 100\n");
        LOG("ParamIndex: 1 -> Treble\n");
        LOG("ParamValue: 0 ~ 100\n");
        LOG("ParamIndex: 2 -> Enable\n");
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("****************************************************************************\n\n");
    } else if (gEffectIndex == AML_EFFECT_HPEQ) {
        LOG("*********************************HPEQ***************************************\n");
        LOG("Amlogic HPEQ EffectIndex: %d\n", (int)AML_EFFECT_HPEQ);
        LOG("Usage: %s %d <ParamIndex> <ParamValue/ParamScale/gParamBand>\n", name, (int)AML_EFFECT_HPEQ);
        LOG("ParamIndex: 0 -> Enable\n");
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: 1 -> Mode\n");
        LOG("ParamValue: 0 -> Standard  1 -> Music   2 -> news  3 -> movie   4 -> game   5->user\n");
        LOG("ParamIndex: 2 -> custom\n");
        LOG("ParamValue: -10 ~10 \n");
        LOG("****************************************************************************\n\n");
    } else if (gEffectIndex == AML_EFFECT_AVL) {
        LOG("*********************************Avl****************************************\n");
        LOG("Amlogic AVL EffectIndex: %d\n", (int)AML_EFFECT_AVL);
        LOG("Usage: %s %d <ParamIndex> <ParamValue/ParamScale/gParamBand>\n", name, (int)AML_EFFECT_AVL);
        LOG("ParamIndex: 0 -> Enable\n");
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: 1 -> Max Level in dB\n");
        LOG("ParamScale: -40.0 ~ 0.0\n");
        LOG("ParamIndex: 2 -> Dynamic Threshold in dB\n");
        LOG("ParamScale: 0 ~ -80\n");
        LOG("ParamIndex: 3 -> Noise Threshold in dB\n");
        LOG("ParamScale: -NAN ~ -10\n");
        LOG("ParamIndex: 4 -> Attack Time in ms\n");
        LOG("ParamValue: 20 ~ 2000 ms\n");
        LOG("ParamIndex: 5 -> Release Time in ms\n");
        LOG("ParamValue: 2000~8000 ms\n");
        LOG("****************************************************************************\n\n");
    }
}

int main(int argc,char **argv)
{
    int ret = -1;
    int gEffectIndex = -1;
    int gParamIndex = 0;
    int gParamValue = 0;
    signed char gParamBand[5] = {0};
    audio_hw_device_t *device;

    ret = audio_hw_load_interface(&device);
    if (ret) {
        LOG("%s %d error:%d\n", __func__, __LINE__, ret);
        return ret;
    }

    if (argc < 4) {
        if (argc == 1) {
            LOG("********************Audio Effect Tuning Tool help*****************************\n");
            LOG("Choice an audio effect to get detail help!\n");
            LOG("EffectIndex: 0: Amlogic BALANCE\n");
            LOG("             1: Amlogic VIRTUALSURROUND\n");
            LOG("             2: Amlogic TREBLEBASS\n");
            LOG("             3: Amlogic HPEQ\n");
            LOG("             4: Amlogic AVL\n");
            LOG("Usage: %s <EffectIndex>\n", argv[0]);
            LOG("******************************************************************************\n");
            return 0;
        }
        sscanf(argv[1], "%d", &gEffectIndex);
        PrintHelp(gEffectIndex, argv[0]);
        return 0;
    }

    LOG("start...\n");
    sscanf(argv[1], "%d", &gEffectIndex);
    sscanf(argv[2], "%d", &gParamIndex);

    switch (gEffectIndex) {
    case AML_EFFECT_BALANCE:
        //------------get Balance parameters---------------------------------------
        sscanf(argv[3], "%d", &gParamValue);
        LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
        break;
    case AML_EFFECT_VIRTUALSURROUND:
        //------------get VirtualSurround parameters------------------------------------
        sscanf(argv[3], "%d", &gParamValue);
        LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
        break;
    case AML_EFFECT_TREBLEBASS:
        //------------get TrebleBass parameters------------------------------------
        sscanf(argv[3], "%d", &gParamValue);
        LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
        break;
    case AML_EFFECT_HPEQ:
        //------------get HPEQ parameters------------------------------------------
        if (gParamIndex == HPEQ_PARAM_EFFECT_CUSTOM) {
            for (int i = 0; i < 5; i++) {
                int ParamBand = 0;
                sscanf(argv[i + 3], "%d", &ParamBand);
                gParamBand[i] = (char)ParamBand;
                LOG("EffectIndex:%d, ParamIndex:%d, ParamBand:%d\n", gEffectIndex, gParamIndex, gParamBand[i]);
            }
        } else {
            sscanf(argv[3], "%d", &gParamValue);
            LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
        }
        break;
    case AML_EFFECT_AVL:
        //------------get Avl parameters-------------------------------------------
        sscanf(argv[3], "%d", &gParamValue);
        LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
        break;
    default:
        LOG("EffectIndex = %d is invalid\n", gEffectIndex);
        return -1;
    }

    while (1) {
        switch (gEffectIndex) {
        case AML_EFFECT_BALANCE:
            //------------set Balance parameters---------------------------------------
            if (Balance_effect_func(gParamIndex, gParamValue) < 0)
                LOG("Balance Test failed\n");
            break;
        case AML_EFFECT_VIRTUALSURROUND:
            //------------set VirtualSurround parameters------------------------------------
            if (VirtualSurround_effect_func(gParamIndex, gParamValue) < 0)
                LOG("VirtualSurround Test failed\n");
            break;
        case AML_EFFECT_TREBLEBASS:
            //------------set TrebleBass parameters------------------------------------
            if (TrebleBass_effect_func(gParamIndex, gParamValue) < 0)
                LOG("TrebleBass Test failed\n");
            break;
        case AML_EFFECT_HPEQ:
            //------------set HPEQ parameters------------------------------------------
            if (HPEQ_effect_func(gParamIndex, gParamValue, gParamBand) < 0)
                LOG("HPEQ Test failed\n");
            break;
        case AML_EFFECT_AVL:
            //------------set Avl parameters-------------------------------------------
            if (Avl_effect_func(gParamIndex, gParamValue) < 0)
                LOG("Avl Test failed\n");
            break;
        default:
            break;
        }

    Retry_input:
        LOG("Please enter param: <27> to Exit\n");
        LOG("Please enter param: <EffectIndex> <ParamIndex> <ParamValue/ParamScale>\n");

        /*sleep 100ms to wait new input*/
        usleep(100*1000);

        if (GetIntData(&gEffectIndex) < 0)
            goto Retry_input;

        if (gEffectIndex == 27) {
            LOG("Exit...\n");
            break;
        }

        if (GetIntData(&gParamIndex) < 0)
            goto Retry_input;

        switch (gEffectIndex) {
        case AML_EFFECT_BALANCE:
            //------------get Balance parameters---------------------------------------
            if (GetIntData(&gParamValue) < 0)
                goto Retry_input;
            LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
            break;
        case AML_EFFECT_VIRTUALSURROUND:
            //------------get VirtualSurround parameters------------------------------------
            if (GetIntData(&gParamValue) < 0)
                goto Retry_input;
            LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
            break;
        case AML_EFFECT_TREBLEBASS:
            //------------get TrebleBass parameters------------------------------------
            if (GetIntData(&gParamValue) < 0)
                goto Retry_input;
            LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
            break;
        case AML_EFFECT_HPEQ:
            //------------get HPEQ parameters------------------------------------------
            if (gParamIndex == HPEQ_PARAM_EFFECT_CUSTOM) {
                for (int i = 0; i < 5; i++) {
                    int ParamBand = 0;
                    if (GetIntData(&ParamBand) < 0)
                        goto Retry_input;
                    gParamBand[i] = (char)ParamBand;
                    LOG("EffectIndex:%d, ParamIndex:%d, ParamBand:%d\n", gEffectIndex, gParamIndex, (int)gParamBand[i]);
                }
            } else {
                if (GetIntData(&gParamValue) < 0)
                    goto Retry_input;
                LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
            }
            break;
        case AML_EFFECT_AVL:
            //------------get Avl parameters-------------------------------------------
            if (GetIntData(&gParamValue) < 0)
                goto Retry_input;
            LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
            break;
        default:
            LOG("EffectIndex = %d is invalid\n", gEffectIndex);
            break;
        }
    }

    ret = 0;
Error:
    audio_hw_unload_interface(device);
    return ret;
}

