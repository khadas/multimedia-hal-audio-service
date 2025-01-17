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
#include <Virtualx_v4.h>
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

//----------DBX parameters-----------------------------
aml_dbx_param_t gDbxParam[] = {
    {{0, 4, 4}, DBX_PARAM_ENABLE, {1}},
    {{0, 4, 4}, DBX_SET_MODE, {0}},
    {{0, 4, 4}, DBX_Read_Param, {0}},
    {{0, 4, 8}, DBX_Write_Param, {1}},
    {{0, 4, 4}, DBX_Read_Coeff, {0}},
    {{0, 4, 8}, DBX_Write_Coeff, {1}},
    {{0, 4, 8}, DBX_Write_VCF, {1}},
    {{0, 4, 4}, DBX_Mute, {0}},
    {{0, 4, 4}, DBX_Version, {1}},
};
const char *DBXStatusstr[] = {"Disable", "Enable"};

//-------------Virtualx parameter--------------------------
Virtualx_param_t gVirtualxParam[] = {
    {{0, 4, 4}, DTS_PARAM_MBHL_ENABLE_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_BYPASS_GAIN_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_REFERENCE_LEVEL_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_VOLUME_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_VOLUME_STEP_I32, {100}},
    {{0, 4, 4}, DTS_PARAM_MBHL_BALANCE_STEP_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_MBHL_OUTPUT_GAIN_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_MODE_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_MBHL_PROCESS_DISCARD_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_MBHL_CROSS_LOW_I32, {7}},
    {{0, 4, 4}, DTS_PARAM_MBHL_CROSS_MID_I32, {15}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_ATTACK_I32, {5}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_LOW_RELEASE_I32, {250}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_LOW_RATIO_I32, {4}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_LOW_THRESH_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_LOW_MAKEUP_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_MID_RELEASE_I32, {250}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_MID_RATIO_I32, {4}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_MID_THRESH_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_MID_MAKEUP_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_HIGH_RELEASE_I32, {250}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_HIGH_RATIO_I32, {4}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_HIGH_THRESH_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_MBHL_COMP_HIGH_MAKEUP_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_BOOST_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_THRESHOLD_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_SLOW_OFFSET_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_MBHL_FAST_ATTACK_I32, {5}},
    {{0, 4, 4}, DTS_PARAM_MBHL_FAST_RELEASE_I32, {50}},
    {{0, 4, 4}, DTS_PARAM_MBHL_SLOW_ATTACK_I32, {500}},
    {{0, 4, 4}, DTS_PARAM_MBHL_SLOW_RELEASE_I32, {500}},
    {{0, 4, 4}, DTS_PARAM_MBHL_DELAY_I32, {8}},
    {{0, 4, 4}, DTS_PARAM_MBHL_ENVELOPE_FREQUENCY_I32, {20}},
    {{0, 4, 4}, DTS_PARAM_MBHL_APP_FRT_LOWCROSS_F32,{0}},
    {{0, 4, 4}, DTS_PARAM_MBHL_APP_FRT_MIDCROSS_F32,{0}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_ENABLE_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_MONO_MODE_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_MAXGAIN_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_SPKSIZE_I32, {2}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_HP_ENABLE_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_TEMP_GAIN_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_PROCESS_DISCARD_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_HPORDER_I32, {4}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_APP_SPKSIZE_I32,{0}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_APP_HPRATIO_F32,{0}},
    {{0, 4, 4}, DTS_PARAM_TBHDX_APP_EXTBASS_F32,{0}},
    {{0, 4, 4}, DTS_PARAM_VX_ENABLE_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_VX_INPUT_MODE_I32, {4}},
    {{0, 4, 4}, DTS_PARAM_VX_OUTPUT_MODE_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_VX_HEADROOM_GAIN_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_VX_PROC_OUTPUT_GAIN_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_VX_REFERENCE_LEVEL_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TSX_ENABLE_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TSX_PASSIVEMATRIXUPMIX_ENABLE_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TSX_HEIGHT_UPMIX_ENABLE_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TSX_LPR_GAIN_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TSX_CENTER_GAIN_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TSX_HORIZ_VIR_EFF_CTRL_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TSX_HEIGHTMIX_COEFF_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TSX_PROCESS_DISCARD_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TSX_HEIGHT_DISCARD_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_TSX_FRNT_CTRL_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_TSX_SRND_CTRL_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_VX_DC_ENABLE_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_VX_DC_CONTROL_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_VX_DEF_ENABLE_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_VX_DEF_CONTROL_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_LOUDNESS_CONTROL_ENABLE_I32, {1}},
    {{0, 4, 4}, DTS_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS_I32, {-24}},
    {{0, 4, 4}, DTS_PARAM_LOUDNESS_CONTROL_PRESET_I32, {0}},
    {{0, 4, 4}, DTS_PARAM_LOUDNESS_CONTROL_IO_MODE_I32,{0}},
    {{0, 4, 4}, DTS_PARAM_AEQ_ENABLE_I32,{0}},
    {{0, 4, 4}, DTS_PARAM_AEQ_DISCARD_I32,{0}},
    {{0, 4, 4}, DTS_PARAM_AEQ_INPUT_GAIN_I16,{1}},
    {{0, 4, 4}, DTS_PARAM_AEQ_OUTPUT_GAIN_I16,{1}},
    {{0, 4, 4}, DTS_PARAM_AEQ_BYPASS_GAIN_I16,{1}},
    {{0, 4, 4}, DTS_PARAM_AEQ_LR_LINK_I32,{1}},
    {{0, 4, 20}, DTS_PARAM_AEQ_BAND_Fre,{1}}, // band is 5 just for example
    {{0, 4, 20}, DTS_PARAM_AEQ_BAND_Gain,{1}},
    {{0, 4, 20}, DTS_PARAM_AEQ_BAND_Q,{1}},
    {{0, 4, 20}, DTS_PARAM_AEQ_BAND_type,{1}},
    {{0, 4, 4}, DTS_PARAM_CHANNEL_NUM, {1}},
    {{0, 4, 4}, VIRTUALX_PARAM_ENABLE, {1}},
    {{0, 4, 4}, VIRTUALX_PARAM_DIALOGCLARITY_MODE, {1}},
    {{0, 4, 4}, VIRTUALX_PARAM_SURROUND_MODE, {1}},
    {{0, 4, 4}, AUDIO_DTS_PARAM_TYPE_NONE,{1}},
    {{0, 4, 32},AUDIO_DTS_PARAM_TYPE_TRU_SURROUND,{1}},
    {{0, 4, 32}, AUDIO_DTS_PARAM_TYPE_CC3D,{1}},
    {{0, 4, 40},AUDIO_DTS_PARAM_TYPE_TRU_BASS,{1}},
    {{0, 4, 32},AUDIO_DTS_PARAM_TYPE_TRU_DIALOG,{1}},
    {{0, 4, 24},AUDIO_DTS_PARAM_TYPE_DEFINITION,{1}},
    {{0, 4, 12},AUDIO_DTS_PARAM_TYPE_TRU_VOLUME,{1}},
    {{0, 4, 4}, AUDIO_DTS_ALL_PARAM_DUMP, {1}},
};
const char *VXStatusstr[] = {"Disable", "Enable"};


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

static int DBX_effect_func(int gParamIndex,int gParamDbxIndex, int gParamValue)
{
    switch (gParamIndex) {
    case DBX_PARAM_ENABLE:
        if (gParamValue < 0 || gParamValue > 1) {
            LOG("DBX Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gDbxParam[gParamIndex].v = gParamDbxIndex;
        audio_effect_set_parameters(AML_EFFECT_DBX, &gDbxParam[gParamIndex].param);
        audio_effect_get_parameters(AML_EFFECT_DBX, &gDbxParam[gParamIndex].param);
        LOG("DBX: state is %d -> %d\n", gParamValue, gDbxParam[gParamIndex].v);
        return 0;
    case DBX_SET_MODE:
        if (gParamValue < 0 || gParamValue > 2) {
            LOG("DBX Status gParamValue = %d invalid\n", gParamValue);
            return -1;
        }
        gDbxParam[gParamIndex].v = gParamDbxIndex;
        audio_effect_set_parameters(AML_EFFECT_DBX, &gDbxParam[gParamIndex].param);
        LOG("DBX: set mode to %d\n",gDbxParam[gParamIndex].v);
        return 0;
    case DBX_Read_Param:
    case DBX_Read_Coeff:
        if (gParamDbxIndex < 0) {
            LOG("DBX read DBX index %d invalid\n", gParamDbxIndex);
            return -1;
        }
        gDbxParam[gParamIndex].v = gParamDbxIndex; //get dbx lib index
        //value is printed at logcat
        audio_effect_set_parameters(AML_EFFECT_DBX, &gDbxParam[gParamIndex].param);
        return 0;

    case DBX_Write_Param:
    case DBX_Write_Coeff:
    case DBX_Write_VCF:
        if (gParamDbxIndex < 0) {
            LOG("DBX write %d invalid\n",gParamIndex);
                return -1;
        }
        gDbxParam[gParamIndex].value[0] = gParamDbxIndex; //set dbx param index
        gDbxParam[gParamIndex].value[1] = gParamValue; //set dbx param value
        audio_effect_set_parameters(AML_EFFECT_DBX, &gDbxParam[gParamIndex].param);
        return 0;
    case DBX_Mute:
        //it will mute dbx output 500ms
        audio_effect_set_parameters(AML_EFFECT_DBX, &gDbxParam[gParamIndex].param);
        return 0;
    case DBX_Version:
        //it will print DBX version at logcat
        audio_effect_set_parameters(AML_EFFECT_DBX, &gDbxParam[gParamIndex].param);
        return 0;
    default:
        LOG("DBX: ParamIndex = %d invalid\n", gParamIndex);
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
static int Virtualx_effect_func(int gParamIndex, int gParamValue, float gParamScale, float gParaRange[VX_MAX_PARAM_SIZE])
{
    int rc = 0;
    switch (gParamIndex) {
        case VIRTUALX_PARAM_ENABLE:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: Status gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            if (!audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param))
                LOG("Virtualx: Successful\n");
            else
                LOG("Virtualx: Failed\n");
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("Virtualx: Status is %d -> %s, [0x%08X]\n",
                gParamValue, VXStatusstr[gVirtualxParam[gParamIndex].v], FLOAT2INT(gParamValue));
            return 0;
       case DTS_PARAM_MBHL_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: MBHL ENABLE gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_BYPASS_GAIN_I32:
            if (gParamScale < 0 || gParamScale > 1.0) {
                LOG("Vritualx: mbhl bypass gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl bypassgain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_REFERENCE_LEVEL_I32:
            if (gParamScale < 0.0009 || gParamScale > 1.0) {
                LOG("Vritualx: mbhl reference level gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl reference level is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_VOLUME_I32:
            if (gParamScale < 0 || gParamScale > 1.0) {
                LOG("Vritualx: mbhl volume gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl volume is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_VOLUME_STEP_I32:
            if (gParamValue < 0 || gParamValue > 100) {
                LOG("Vritualx: mbhl volumestep gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl volume step is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_BALANCE_STEP_I32:
            if (gParamValue < -10 || gParamValue > 10) {
                LOG("Vritualx: mbhl banlance step gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl balance step is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_OUTPUT_GAIN_I32:
            if (gParamScale < 0 || gParamScale > 1.0) {
                LOG("Vritualx: mbhl output gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            // audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl output gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_BOOST_I32:
            if (gParamScale < 0.001 || gParamScale > 1000) {
                LOG("Vritualx: mbhl boost  gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl boost is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_THRESHOLD_I32:
            if (gParamScale < 0.064 || gParamScale > 1.0) {
                LOG("Vritualx: mbhl threshold  gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl threshold is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_SLOW_OFFSET_I32:
            if (gParamScale < 0.3170 || gParamScale > 3.1619) {
                LOG("Vritualx: mbhl slow offset  gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl slow offset is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_FAST_ATTACK_I32:
            if (gParamScale < 0 || gParamScale > 10) {
                LOG("Vritualx: mbhl fast attack  gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl fast attack is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_FAST_RELEASE_I32:
            if (gParamValue < 10 || gParamValue > 500) {
                LOG("Vritualx: mbhl fast release gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl fast release is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_SLOW_ATTACK_I32:
            if (gParamValue < 100 || gParamValue > 1000) {
                LOG("Vritualx: mbhl slow attack gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl slow attack is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_SLOW_RELEASE_I32:
            if (gParamValue < 100 || gParamValue > 2000) {
                LOG("Vritualx: mbhl slow release gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl slow release is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_DELAY_I32:
            if (gParamValue < 1 || gParamValue > 16) {
                LOG("Vritualx: mbhl delay gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl delay is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_ENVELOPE_FREQUENCY_I32:
            if (gParamValue < 5 || gParamValue > 500) {
                LOG("Vritualx: mbhl envelope freq gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl envelope freq is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_MODE_I32:
            if (gParamValue < 0 || gParamValue > 4) {
                LOG("Vritualx: mbhl mode gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl mode is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_PROCESS_DISCARD_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: mbhl process discard gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl process discard is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_CROSS_LOW_I32:
            if (gParamValue < 0 || gParamValue > 20) {
                LOG("Vritualx: mbhl cross low gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl cross low  is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_CROSS_MID_I32:
            if (gParamValue < 0 || gParamValue > 20) {
                LOG("Vritualx: mbhl cross mid gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl cross mid is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_COMP_ATTACK_I32:
            if (gParamValue < 0 || gParamValue > 100) {
                LOG("Vritualx: mbhl compressor attack time gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor attack time is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_COMP_LOW_RELEASE_I32:
            if (gParamValue < 50 || gParamValue > 2000) {
                LOG("Vritualx: mbhl compressor low release time gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor low release time is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_COMP_LOW_RATIO_I32:
            if (gParamScale < 1.0 || gParamScale > 20.0) {
                LOG("Vritualx: mbhl compressor low ratio gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor low ratio is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_LOW_THRESH_I32:
            if (gParamScale < 0.0640 || gParamScale > 15.8479) {
                LOG("Vritualx: mbhl compressor low threshold gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor low threshold is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_LOW_MAKEUP_I32:
            if (gParamScale < 0.0640 || gParamScale > 15.8479) {
                LOG("Vritualx: mbhl compressor low makeup gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor low makeup is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_MID_RELEASE_I32:
            if (gParamValue < 50 || gParamValue > 2000) {
                LOG("Vritualx: mbhl compressor mid release time gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor mid release time is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_COMP_MID_RATIO_I32:
             if (gParamScale < 1.0 || gParamScale > 20.0) {
                LOG("Vritualx: mbhl compressor mid ratio gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor mid ratio is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_MID_THRESH_I32:
            if (gParamScale < 0.0640 || gParamScale > 15.8479) {
                LOG("Vritualx: mbhl compressor mid threshold gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor mid threshold is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_MID_MAKEUP_I32:
             if (gParamScale < 0.0640 || gParamScale > 15.8479) {
                LOG("Vritualx: mbhl compressor mid makeup gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor mid makeup is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_HIGH_RELEASE_I32:
            if (gParamValue < 50 || gParamValue > 2000) {
                LOG("Vritualx: mbhl compressor high release time gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor high release time is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_MBHL_COMP_HIGH_RATIO_I32:
            if (gParamScale < 1.0 || gParamScale > 20.0) {
                LOG("Vritualx: mbhl compressor high ratio gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor high ratio is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_HIGH_THRESH_I32:
            if (gParamScale < 0.0640 || gParamScale > 15.8479) {
                LOG("Vritualx: mbhl compressor high threshold gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor high threshold is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_COMP_HIGH_MAKEUP_I32:
             if (gParamScale < 0.0640 || gParamScale > 15.8479) {
                LOG("Vritualx: mbhl compressor high makeup gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("mbhl compressor high makeup is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TBHDX_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tbhdx enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TBHDX_MONO_MODE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tbhdx mono mode gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx mono mode is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TBHDX_SPKSIZE_I32:
            if (gParamValue < 0 || gParamValue > 12) {
                LOG("Vritualx: tbhdx spksize gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx spksize is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TBHDX_TEMP_GAIN_I32:
            if (gParamScale < 0.0 || gParamScale > 1.0) {
                LOG("Vritualx: tbhdx temp gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx temp gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TBHDX_MAXGAIN_I32:
            if (gParamScale < 0.0 || gParamScale > 1.0) {
                LOG("Vritualx: tbhdx max gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx max gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TBHDX_PROCESS_DISCARD_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tbhdx process discard gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
           //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx process discard is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TBHDX_HPORDER_I32:
            if (gParamValue < 0 || gParamValue > 8) {
                LOG("Vritualx: tbhdx high pass filter order gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx high pass filter order is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TBHDX_HP_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tbhdx high pass enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tbhdx high pass enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_VX_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: vxlib1 enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("vxlib1 enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_VX_INPUT_MODE_I32:
            if (gParamValue < 0 || gParamValue > 4) {
                LOG("Vritualx: vxlib1 input mode gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("vxlib1 input mode is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_VX_HEADROOM_GAIN_I32:
            if (gParamScale < 0.1250 || gParamScale > 1.0) {
                LOG("Vritualx: vxlib1 headroom gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("vxlib1 headroom gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_VX_PROC_OUTPUT_GAIN_I32:
            if (gParamScale < 0.5 || gParamScale > 4.0) {
                LOG("Vritualx: vxlib1 output gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("vxlib1 output gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TSX_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("vxlib1 tsx enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TSX_PASSIVEMATRIXUPMIX_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx passive matrix upmixer enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("vxlib1 tsx passive matrix upmixer enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TSX_HORIZ_VIR_EFF_CTRL_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx horizontal Effect gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("vxlib1 tsx horizontal Effect is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TSX_FRNT_CTRL_I32:
            if (gParamScale < 0.5 || gParamScale > 2.0) {
                LOG("Vritualx: tsx frnt ctrl gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx frnt ctrl is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TSX_SRND_CTRL_I32:
            if (gParamScale < 0.5 || gParamScale > 2.0) {
                LOG("Vritualx: tsx srnd ctrl gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx srnd ctrl is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TSX_LPR_GAIN_I32:
             if (gParamScale < 0.0 || gParamScale > 2.0) {
                LOG("Vritualx: tsx lprtoctr mix gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx lprtoctr mix gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TSX_HEIGHTMIX_COEFF_I32:
             if (gParamScale < 0.5 || gParamScale > 2.0) {
                LOG("Vritualx: tsx heightmix coeff gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx heightmix coeff is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TSX_CENTER_GAIN_I32:
             if (gParamScale < 1.0 || gParamScale > 2.0) {
                LOG("Vritualx: tsx center gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx center gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TSX_HEIGHT_DISCARD_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx height discard gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx height discard is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TSX_PROCESS_DISCARD_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx process discard gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx process discard is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TSX_HEIGHT_UPMIX_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx height upmix enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx height upmix enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_VX_DC_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx dc enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx dc enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_VX_DC_CONTROL_I32:
             if (gParamScale < 0.0 || gParamScale > 1.0) {
                LOG("Vritualx: tsx dc level gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx dc level is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_VX_DEF_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: tsx def enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx def enable is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_VX_DEF_CONTROL_I32:
             if (gParamScale < 0.0 || gParamScale > 1.0) {
                LOG("Vritualx: tsx def level gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("tsx def level is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_LOUDNESS_CONTROL_ENABLE_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("loudness control = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("loudness control is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS_I32:
            if (gParamValue < -40 || gParamValue > 0) {
                LOG("loudness control target = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("loudness control target is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_LOUDNESS_CONTROL_PRESET_I32:
            if (gParamValue < 0 || gParamValue > 2) {
                LOG("loudness control preset = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            //audio_effect_get_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("loudness control preset is %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TBHDX_APP_SPKSIZE_I32:
            if (gParamValue < 40 || gParamValue > 600) {
                LOG("app spksize = %d invalid\n",gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("app spksize %d, [0x%08X]\n",
                gVirtualxParam[gParamIndex].v, FLOAT2INT(gVirtualxParam[gParamIndex].v));
            return 0;
        case DTS_PARAM_TBHDX_APP_HPRATIO_F32:
            if (gParamScale < 0.0 || gParamScale > 1.0) {
                LOG("app hpratio = %f invalid\n",gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("app hpratio is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_TBHDX_APP_EXTBASS_F32:
            if (gParamScale < 0.0 || gParamScale > 1.0) {
                LOG("app extbass = %f invalid\n",gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("app extbass is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_APP_FRT_LOWCROSS_F32:
            if (gParamScale < 40 || gParamScale > 8000.0) {
                LOG("app low freq = %f invalid\n",gParamScale);
                return -1;
            }
             gVirtualxParam[gParamIndex].f = gParamScale;
            rc =audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            LOG("app low freq is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_MBHL_APP_FRT_MIDCROSS_F32:
            if (gParamScale < 40.0 || gParamScale > 8000.0) {
                LOG("app mid freq = %f invalid\n",gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("app mid freq is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case AUDIO_DTS_PARAM_TYPE_TRU_SURROUND:
            for (int i = 0; i < 8; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case AUDIO_DTS_PARAM_TYPE_CC3D:
            for (int i = 0; i < 8; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case AUDIO_DTS_PARAM_TYPE_TRU_BASS:
            for (int i = 0; i < 10; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case AUDIO_DTS_PARAM_TYPE_TRU_DIALOG:
            for (int i = 0; i < 8; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case AUDIO_DTS_PARAM_TYPE_DEFINITION:
            for (int i = 0; i < 6; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case AUDIO_DTS_PARAM_TYPE_TRU_VOLUME:
            for (int i = 0; i < 3; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case DTS_PARAM_AEQ_ENABLE_I32:
             if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: aeq  enable gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("aeq enable is %d",gVirtualxParam[gParamIndex].v);
            return 0;
        case DTS_PARAM_AEQ_DISCARD_I32:
             if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: aeq  discard gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("aeq discard is %d",gVirtualxParam[gParamIndex].v);
            return 0;
        case DTS_PARAM_AEQ_INPUT_GAIN_I16:
            if (gParamScale < 0 || gParamScale > 1.0) {
                LOG("Vritualx: aeq input gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("aeq input gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_AEQ_OUTPUT_GAIN_I16:
            if (gParamScale < 0 || gParamScale > 1.0) {
                LOG("Vritualx: aeq output gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("aeq output gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_AEQ_BYPASS_GAIN_I16:
            if (gParamScale < 0 || gParamScale > 1.0) {
                LOG("Vritualx: aeq bypass gain gParamValue = %f invalid\n", gParamScale);
                return -1;
            }
            gVirtualxParam[gParamIndex].f = gParamScale;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("aeq bypass gain is %f, [0x%08X]\n",
                gVirtualxParam[gParamIndex].f, FLOAT2INT(gVirtualxParam[gParamIndex].f));
            return 0;
        case DTS_PARAM_AEQ_LR_LINK_I32:
            if (gParamValue < 0 || gParamValue > 1) {
                LOG("Vritualx: aeq lr link gParamValue = %d invalid\n", gParamValue);
                return -1;
            }
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("aeq lr link flag is %d",gVirtualxParam[gParamIndex].v);
            return 0;
        case DTS_PARAM_AEQ_BAND_Fre:
            for (int i = 0; i < 5; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case DTS_PARAM_AEQ_BAND_Gain:
            for (int i = 0; i < 5; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case DTS_PARAM_AEQ_BAND_Q:
            for (int i = 0; i < 5; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case DTS_PARAM_AEQ_BAND_type:
            for (int i = 0; i < 5; i++) {
                gVirtualxParam[gParamIndex].params[i] = gParaRange[i];
                LOG("gVirtualxParam[gParamIndex].params[%d] is %f, [0x%08X]\n",
                    i, gParaRange[i], FLOAT2INT(gParaRange[i]));
            }
            rc = audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("rc is %d\n",rc);
            return 0;
        case DTS_PARAM_CHANNEL_NUM:
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            LOG("set dts channel num is %d \n",gVirtualxParam[gParamIndex].v);
            return 0;
        case AUDIO_DTS_ALL_PARAM_DUMP:
            gVirtualxParam[gParamIndex].v = gParamValue;
            audio_effect_set_parameters(AML_EFFECT_VIRTUALX, &gVirtualxParam[gParamIndex].param);
            return 0;
    default:
        LOG("Virtualx: ParamIndex = %d invalid\n", gParamIndex);
        return -1;
    }
}

// -vx4 set param
int Virtualx_v4_effect_func(void)
{
    VX_Cmd_Param *vxCmd = getVxCmdParamInstance();
    if (!vxCmd) {
        LOG("Warning, VxCmd is NULL!\n");
        return -1;
    }

    //vxParam will update by init_effect_param_data()
    Virtualx_v4_param_t *vxParam = getVxParamInstance();
    int ret =audio_effect_set_parameters(AML_EFFECT_VIRTUALX_v4, &vxParam->param);//Set DTS VX parameters
    LOG("rc is %d\n",ret);

    if ((vxCmd->userId == PARAM_CHANNEL_NUM) ||
        (vxCmd->userId == PARAM_OUT_CHANNEL_NUM) ||
        (vxCmd->userId == VIRTUALX_PARAM_ENABLE) ||
        (vxCmd->userId == VIRTUALX4_PARAM_SURROUND_MODE) ||
        (vxCmd->userId == VIRTUALX4_PARAM_DIALOGCLARITY_MODE) ||
        (vxCmd->userId == AUDIO_PARAM_TYPE_TRU_VOLUME) ||
        (vxCmd->userId == PARAM_LOUDNESS_CONTROL_ENABLE_I32) ||
        (vxCmd->userId == PARAM_LOUDNESS_CONTROL_IO_MODE_I32) ||
        (vxCmd->userId == PARAM_AEQ_ENABLE_I32) ||
        (vxCmd->userId == VIRTUALX_EFFECT_ENABLE))
    {
        LOG("->[%s] ParamIndex:%d ParamValType:Int %s = %d\n", (ret == 0? "OK" : "NG"), vxParam->command, vxCmd->param, vxParam->v);
    }
    else
    {
        LOG("->[%s] ParamIndex:%d ParamValType:CStr %s = %s\n", (ret == 0? "OK" : "NG"), vxParam->command, vxCmd->param, vxParam->str);
    }
    return 0;
}

int GetIntData (int *data)
{
    int status = scanf("%d", data);
    if (status == 0) {
        int status_temp=scanf("%*s");
        if (status_temp < 0)
            LOG("Error input! Pls Retry!\n");
        return -1;
    }
    return 0;
}
int GetFloatData (float *data)
{
    int status = scanf("%f", data);
    if (status == 0) {
        int status_temp = scanf("%*s");
        if (status_temp < 0)
            LOG("Error input! Pls Retry!\n");
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
    } else if (gEffectIndex == AML_EFFECT_VIRTUALX) {
        LOG("*********************************Virtualx***********************************\n");
        LOG("VirtualX EffectIndex: %d\n", (int)AML_EFFECT_VIRTUALX);
        LOG("Usage: %s %d <ParamIndex> <ParamValue/ParamScale/gParamBand>\n", name, (int)AML_EFFECT_VIRTUALX);
        LOG("------------Multi band hard limiter----ParamIndex(from %d to %d)-----\n",
            (int)DTS_PARAM_MBHL_ENABLE_I32, (int)DTS_PARAM_MBHL_APP_FRT_MIDCROSS_F32);
        LOG("ParamIndex: %d -> Mbhl Enable\n", (int)DTS_PARAM_MBHL_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> Mbhl Bypass Gain\n", (int)DTS_PARAM_MBHL_BYPASS_GAIN_I32);
        LOG("ParamScale: 0.0 ~ 1.0\n");
        LOG("ParamIndex: %d -> Mbhl Reference Level\n", (int)DTS_PARAM_MBHL_REFERENCE_LEVEL_I32);
        LOG("ParamScale: 0.0009 ~ 1.0\n");
        LOG("ParamIndex: %d -> Mbhl Volume\n", (int)DTS_PARAM_MBHL_VOLUME_I32);
        LOG("ParamScale: 0.0 ~ 1.0\n");
        LOG("ParamIndex: %d -> Mbhl Volume Step\n", (int)DTS_PARAM_MBHL_VOLUME_STEP_I32);
        LOG("ParamValue: 0 ~ 100\n");
        LOG("ParamIndex: %d -> Mbhl Balance Step\n", (int)DTS_PARAM_MBHL_BALANCE_STEP_I32);
        LOG("ParamValue: -10 ~ 10\n");
        LOG("ParamIndex: %d -> Mbhl Output Gain\n", (int)DTS_PARAM_MBHL_OUTPUT_GAIN_I32);
        LOG("ParamScale: 0.0 ~ 1.0\n");
        LOG("ParamIndex: %d -> Mbhl Mode\n", (int)DTS_PARAM_MBHL_MODE_I32);
        LOG("ParamValue: 0 ~ 4\n");
        LOG("ParamIndex: %d -> Mbhl process Discard\n", (int)DTS_PARAM_MBHL_PROCESS_DISCARD_I32);
        LOG("ParamValue: 0 ~ 1\n");
        LOG("ParamIndex: %d -> Mbhl Cross Low\n", (int)DTS_PARAM_MBHL_CROSS_LOW_I32);
        LOG("ParamValue: 0 ~ 20\n");
        LOG("ParamIndex: %d -> Mbhl Cross Mid\n", (int)DTS_PARAM_MBHL_CROSS_MID_I32);
        LOG("ParamValue: 0 ~ 20\n");
        LOG("ParamIndex: %d -> Mbhl Comp Attack\n", (int)DTS_PARAM_MBHL_COMP_ATTACK_I32);
        LOG("ParamValue: 0 ~ 100\n");
        LOG("ParamIndex: %d -> Mbhl Comp Low Release\n", (int)DTS_PARAM_MBHL_COMP_LOW_RELEASE_I32);
        LOG("ParamValue: 50 ~ 2000\n");
        LOG("ParamIndex: %d -> Mbhl Comp Low Ratio\n", (int)DTS_PARAM_MBHL_COMP_LOW_RATIO_I32);
        LOG("ParamScale: 1.0 ~ 20.0\n");
        LOG("ParamIndex: %d -> Mbhl Comp Low Thresh\n", (int)DTS_PARAM_MBHL_COMP_LOW_THRESH_I32);
        LOG("ParamScale: 0.0640 ~ 15.8479\n");
        LOG("ParamIndex: %d -> Mbhl Comp Low Makeup\n", (int)DTS_PARAM_MBHL_COMP_LOW_MAKEUP_I32);
        LOG("ParamScale: 0.0640 ~ 15.8479\n");
        LOG("ParamIndex: %d -> Mbhl Comp Mid Release\n", (int)DTS_PARAM_MBHL_COMP_MID_RELEASE_I32);
        LOG("ParamValue: 50 ~ 2000\n");
        LOG("ParamIndex: %d -> Mbhl Comp Mid Ratio\n", (int)DTS_PARAM_MBHL_COMP_MID_RATIO_I32);
        LOG("ParamScale: 1.0 ~ 20.0\n");
        LOG("ParamIndex: %d -> Mbhl Comp Mid Thresh\n", (int)DTS_PARAM_MBHL_COMP_MID_THRESH_I32);
        LOG("ParamScale: 0.0640 ~ 15.8479\n");
        LOG("ParamIndex: %d -> Mbhl Comp Mid Makeup\n", (int)DTS_PARAM_MBHL_COMP_MID_MAKEUP_I32);
        LOG("ParamScale: 0.0640 ~ 15.8479\n");
        LOG("ParamIndex: %d -> Mbhl Comp High Release\n", (int)DTS_PARAM_MBHL_COMP_HIGH_RELEASE_I32);
        LOG("ParamValue: 50 ~ 2000\n");
        LOG("ParamIndex: %d -> Mbhl Comp High Ratio\n", (int)DTS_PARAM_MBHL_COMP_HIGH_RATIO_I32);
        LOG("ParamScale: 1.0 ~ 20.0\n");
        LOG("ParamIndex: %d -> Mbhl Comp High Thresh\n", (int)DTS_PARAM_MBHL_COMP_HIGH_THRESH_I32);
        LOG("ParamScale: 0.0640 ~ 15.8479\n");
        LOG("ParamIndex: %d -> Mbhl Comp High Makeup\n", (int)DTS_PARAM_MBHL_COMP_HIGH_MAKEUP_I32);
        LOG("ParamScale: 0.0640 ~ 15.8479\n");
        LOG("ParamIndex: %d -> Mbhl Boost\n", (int)DTS_PARAM_MBHL_BOOST_I32);
        LOG("ParamScale: 0.001 ~ 1000\n");
        LOG("ParamIndex: %d -> Mbhl Threshold\n", (int)DTS_PARAM_MBHL_THRESHOLD_I32);
        LOG("ParamScale: 0.0640 ~ 1.0\n");
        LOG("ParamIndex: %d -> Mbhl Slow Offset\n", (int)DTS_PARAM_MBHL_SLOW_OFFSET_I32);
        LOG("ParamScale: 0.317 ~ 3.1619\n");
        LOG("ParamIndex: %d -> Mbhl Fast Attack\n", (int)DTS_PARAM_MBHL_FAST_ATTACK_I32);
        LOG("ParamScale: 0 ~ 10\n");
        LOG("ParamIndex: %d -> Mbhl Fast Release\n", (int)DTS_PARAM_MBHL_FAST_RELEASE_I32);
        LOG("ParamValue: 10 ~ 500\n");
        LOG("ParamIndex: %d -> Mbhl Slow Attack\n", (int)DTS_PARAM_MBHL_SLOW_ATTACK_I32);
        LOG("ParamValue: 100 ~ 1000\n");
        LOG("ParamIndex: %d -> Mbhl Slow Release\n", (int)DTS_PARAM_MBHL_SLOW_RELEASE_I32);
        LOG("ParamValue: 100 ~ 2000\n");
        LOG("ParamIndex: %d -> Mbhl Delay\n", (int)DTS_PARAM_MBHL_DELAY_I32);
        LOG("ParamValue: 0 ~ 16\n");
        LOG("ParamIndex: %d -> Mbhl Envelope Freq\n", (int)DTS_PARAM_MBHL_ENVELOPE_FREQUENCY_I32);
        LOG("ParamValue: 5 ~ 500\n");
        LOG("ParamIndex: %d -> Mbhl frt lowcross\n", (int)DTS_PARAM_MBHL_APP_FRT_LOWCROSS_F32);
        LOG("ParamScale  40 ~ 8000\n");
        LOG("ParamIndex: %d -> Mbhl frt midcross\n", (int)DTS_PARAM_MBHL_APP_FRT_MIDCROSS_F32);
        LOG("ParamScale  40 ~ 8000\n");
        LOG("------------TruBassHDX-------ParamIndex(from %d to %d)--\n",
            (int)DTS_PARAM_TBHDX_ENABLE_I32, (int)DTS_PARAM_TBHDX_APP_EXTBASS_F32);
        LOG("ParamIndex: %d -> TBHDX Enable\n", (int)DTS_PARAM_TBHDX_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TBHDX Mono Mode\n", (int)DTS_PARAM_TBHDX_MONO_MODE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TBHDX Max Gain\n", (int)DTS_PARAM_TBHDX_MAXGAIN_I32);
        LOG("ParamScale: 0.0 ~ 1.0\n");
        LOG("ParamIndex: %d -> TBHDX Spk Size\n", (int)DTS_PARAM_TBHDX_SPKSIZE_I32);
        LOG("ParamValue: 0 ~ 12\n");
        LOG("ParamIndex: %d -> TBHDX HP Enable\n", (int)DTS_PARAM_TBHDX_HP_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TBHDX Temp Gain\n", (int)DTS_PARAM_TBHDX_TEMP_GAIN_I32);
        LOG("ParamScale: 0.0 ~ 1.0\n");
        LOG("ParamIndex: %d -> TBHDX  Process Discard\n", (int)DTS_PARAM_TBHDX_PROCESS_DISCARD_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TBHDX HP Order\n", (int)DTS_PARAM_TBHDX_HPORDER_I32);
        LOG("ParamValue: 1 ~ 8\n");
        LOG("ParamIndex: %d -> TBHDX app spksize\n", (int)DTS_PARAM_TBHDX_APP_SPKSIZE_I32);
        LOG("ParamValue 40 ~ 600 \n");
        LOG("ParamIndex: %d -> TBHDX app hpratio\n", (int)DTS_PARAM_TBHDX_APP_HPRATIO_F32);
        LOG("ParamScale 0 ~ 1.0\n");
        LOG("ParamIndex: %d -> TBHDX app extbass\n", (int)DTS_PARAM_TBHDX_APP_EXTBASS_F32);
        LOG("ParamScale 0 ~ 1.0\n");
        LOG("------------General Setting-------ParamIndex(from %d to %d)--\n",
            (int)DTS_PARAM_VX_ENABLE_I32, (int)DTS_PARAM_VX_REFERENCE_LEVEL_I32);
        LOG("ParamIndex: %d -> VX Enable\n", (int)DTS_PARAM_VX_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> VX Input Mode\n", (int)DTS_PARAM_VX_INPUT_MODE_I32);
        LOG("ParamValue: 0 ~ 4\n");
        LOG("ParamIndex: %d -> VX Output Mode\n", (int)DTS_PARAM_VX_OUTPUT_MODE_I32);
        LOG("ParamValue: Can't be set!\n");
        LOG("ParamIndex: %d -> VX Head Room Gain\n", (int)DTS_PARAM_VX_HEADROOM_GAIN_I32);
        LOG("ParamScale: 0.125 ~ 1.0\n");
        LOG("ParamIndex: %d -> VX Proc Output Gain\n", (int)DTS_PARAM_VX_PROC_OUTPUT_GAIN_I32);
        LOG("ParamScale: 0.5 ~ 4.0\n");
        LOG("ParamIndex: %d -> VX Reference level\n", (int)DTS_PARAM_VX_REFERENCE_LEVEL_I32);
        LOG("ParamValue: Can't be set!\n");
        LOG("------------TrusurroundX-------ParamIndex(from %d to %d)--\n",
            (int)DTS_PARAM_TSX_ENABLE_I32, (int)DTS_PARAM_TSX_SRND_CTRL_I32);
        LOG("ParamIndex: %d -> TSX Enable\n", (int)DTS_PARAM_TSX_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TSX Passive Matrix Upmixer Enable\n", (int)DTS_PARAM_TSX_PASSIVEMATRIXUPMIX_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TSX Height Upmixer Enable\n", (int)DTS_PARAM_TSX_HEIGHT_UPMIX_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TSX Lpr Gain\n", (int)DTS_PARAM_TSX_LPR_GAIN_I32);
        LOG("ParamScale: 0.000 ~ 2.0\n");
        LOG("ParamIndex: %d -> TSX Center Gain\n", (int)DTS_PARAM_TSX_CENTER_GAIN_I32);
        LOG("ParamScale: 1.0 ~ 2.0\n");
        LOG("ParamIndex: %d -> TSX Horiz Vir Effect Ctrl\n", (int)DTS_PARAM_TSX_HORIZ_VIR_EFF_CTRL_I32);
        LOG("ParamValue: 0 -> default   1 -> mild\n");
        LOG("ParamIndex: %d -> TSX Height Mix Coeff\n", (int)DTS_PARAM_TSX_HEIGHTMIX_COEFF_I32);
        LOG("ParamScale: 0.5 ~ 2.0\n");
        LOG("ParamIndex: %d -> TSX Process Discard\n", (int)DTS_PARAM_TSX_PROCESS_DISCARD_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TSX Height Discard\n", (int)DTS_PARAM_TSX_HEIGHT_DISCARD_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> TSX Frnt Ctrl\n", (int)DTS_PARAM_TSX_FRNT_CTRL_I32);
        LOG("ParamScale: 0.5 ~ 2.0\n");
        LOG("ParamIndex: %d -> TSX Srnd Ctrl\n", (int)DTS_PARAM_TSX_SRND_CTRL_I32);
        LOG("ParamScale: 0.5 ~ 2.0\n");
        LOG("------------Dialog Clarty-------ParamIndex(from %d to %d)--\n",
            (int)DTS_PARAM_VX_DC_ENABLE_I32, (int)DTS_PARAM_VX_DC_CONTROL_I32);
        LOG("ParamIndex: %d -> VX DC Enable\n", (int)DTS_PARAM_VX_DC_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> VX DC Control\n", (int)DTS_PARAM_VX_DC_CONTROL_I32);
        LOG("ParamScale: 0.0 ~ 1.0\n");
        LOG("------------Definition-------ParamIndex(from %d to %d)--\n",
            (int)DTS_PARAM_VX_DEF_ENABLE_I32, (int)DTS_PARAM_VX_DEF_CONTROL_I32);
        LOG("ParamIndex: %d -> VX DEF Enable\n", (int)DTS_PARAM_VX_DEF_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> VX DEF Control\n", (int)DTS_PARAM_VX_DEF_CONTROL_I32);
        LOG("ParamScale: 0.0 ~ 1.0\n");
        LOG("------------TruVolume-------ParamIndex(from %d to %d)--\n",
            (int)DTS_PARAM_LOUDNESS_CONTROL_ENABLE_I32, (int)DTS_PARAM_LOUDNESS_CONTROL_IO_MODE_I32);
        LOG("ParamIndex: %d -> Loudness Control Enable\n", (int)DTS_PARAM_LOUDNESS_CONTROL_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> Loudness Control Target Loudness\n", (int)DTS_PARAM_LOUDNESS_CONTROL_TARGET_LOUDNESS_I32);
        LOG("ParamValue: -40 ~ 0\n");
        LOG("ParamIndex: %d -> Loudness Control Preset\n", (int)DTS_PARAM_LOUDNESS_CONTROL_PRESET_I32);
        LOG("ParamValue: 0 -> light 1 -> mid 2 -> Aggressive \n");
        LOG("ParamIndex: %d -> Loudness Control Mode\n", (int)DTS_PARAM_LOUDNESS_CONTROL_IO_MODE_I32);
        LOG("ParamValue: 0 ~ 4 \n");
        LOG("****************************************************************************\n\n");
        LOG("------------DTSEQ-------ParamIndex(from %d to %d)--\n",
            (int)DTS_PARAM_AEQ_ENABLE_I32, (int)DTS_PARAM_AEQ_BAND_type);
        LOG("ParamIndex: %d -> AEQ  Enable\n", (int)DTS_PARAM_AEQ_ENABLE_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> AEQ  discard\n", (int)DTS_PARAM_AEQ_DISCARD_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> AEQ  input gain\n", (int)DTS_PARAM_AEQ_INPUT_GAIN_I16);
        LOG("ParamScale: 0.000 ~ 1.0\n");
        LOG("ParamIndex: %d -> AEQ  output gain\n", (int)DTS_PARAM_AEQ_OUTPUT_GAIN_I16);
        LOG("ParamScale: 0.000 ~ 1.0\n");
        LOG("ParamIndex: %d -> AEQ  bypass gain\n", (int)DTS_PARAM_AEQ_BYPASS_GAIN_I16);
        LOG("ParamScale: 0.000 ~ 1.0\n");
        LOG("ParamIndex: %d -> AEQ  LR Link\n", (int)DTS_PARAM_AEQ_LR_LINK_I32);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("ParamIndex: %d -> AEQ  fre\n", (int)DTS_PARAM_AEQ_BAND_Fre);
        LOG("ParamValue: 20 ~ 20000\n");
        LOG("ParamIndex: %d -> AEQ  band gain\n", (int)DTS_PARAM_AEQ_BAND_Gain);
        LOG("ParamValue: -12db ~ 12db\n");
        LOG("ParamIndex: %d -> AEQ  band Q\n", (int)DTS_PARAM_AEQ_BAND_Q);
        LOG("ParamValue: 0.25 ~ 16\n");
        LOG("ParamIndex: %d -> AEQ  band type\n", (int)DTS_PARAM_AEQ_BAND_type);
        LOG("ParamValue: 0:Traditional | 1:LowShelf | 2:High Shelf | 9:Null ]\n");
        LOG("ParamIndex: %d -> DTS source channel num \n", (int)DTS_PARAM_CHANNEL_NUM);
        LOG("ParamValue: 2 | 6 \n");
        LOG("------------Debug interface------------\n");
        LOG("ParamIndex: %d -> Get all parameters from VX lib\n", (int)AUDIO_DTS_ALL_PARAM_DUMP);
        LOG("ParamValue: 0 \n");
        LOG("****************************************************************************\n\n");
    } else if (gEffectIndex == AML_EFFECT_DBX) {
        LOG("*********************************DBX****************************************\n");
        LOG("Amlogic DBX EffectIndex: %d\n", (int)AML_EFFECT_DBX);
        LOG("Usage: %s %d <ParamIndex> <ParamValue/ParamScale/gParamBand>\n", name, (int)AML_EFFECT_DBX);
        LOG("ParamValue: 0 -> Disable   1 -> Enable\n");
        LOG("****************************************************************************\n\n");
    }
}

int main(int argc,char **argv)
{
    int ret = -1;
    int gEffectIndex = -1;
    int gParamIndex = 0;
    int gParamValue = 0;
    float gParamScale = 0.0f;
    float grange[VX_MAX_PARAM_SIZE] = {0};
    signed char gParamBand[5] = {0};
    int gParamDBXIndex = 0;
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
            LOG("             5: DTS VIRTUALX\n");
            LOG("             6: DTS VIRTUALX4\n");
            LOG("             7: DBX SOUND EFFECT\n");
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
    case AML_EFFECT_VIRTUALX:
        //------------get Virtualx parameters--------------------------------------
        if ((gParamIndex >= DTS_PARAM_MBHL_BYPASS_GAIN_I32 && gParamIndex <= DTS_PARAM_MBHL_VOLUME_I32)
            || gParamIndex == DTS_PARAM_MBHL_OUTPUT_GAIN_I32
            || (gParamIndex >= DTS_PARAM_MBHL_COMP_LOW_RATIO_I32 && gParamIndex <= DTS_PARAM_MBHL_COMP_LOW_MAKEUP_I32)
            || (gParamIndex >= DTS_PARAM_MBHL_COMP_MID_RATIO_I32 && gParamIndex <= DTS_PARAM_MBHL_COMP_MID_MAKEUP_I32)
            || (gParamIndex >= DTS_PARAM_MBHL_COMP_HIGH_RATIO_I32 && gParamIndex <= DTS_PARAM_MBHL_FAST_ATTACK_I32)
            || gParamIndex == DTS_PARAM_MBHL_APP_FRT_LOWCROSS_F32 || gParamIndex == DTS_PARAM_MBHL_APP_FRT_MIDCROSS_F32
            || gParamIndex == DTS_PARAM_TBHDX_MAXGAIN_I32
            || gParamIndex == DTS_PARAM_TBHDX_TEMP_GAIN_I32 || gParamIndex == DTS_PARAM_VX_HEADROOM_GAIN_I32
            || gParamIndex == DTS_PARAM_TBHDX_APP_HPRATIO_F32 || gParamIndex == DTS_PARAM_TBHDX_APP_EXTBASS_F32
            || gParamIndex == DTS_PARAM_VX_PROC_OUTPUT_GAIN_I32 || gParamIndex == DTS_PARAM_TSX_LPR_GAIN_I32
            || gParamIndex == DTS_PARAM_TSX_CENTER_GAIN_I32 || gParamIndex == DTS_PARAM_TSX_HEIGHTMIX_COEFF_I32
            || gParamIndex == DTS_PARAM_TSX_FRNT_CTRL_I32 || gParamIndex == DTS_PARAM_TSX_SRND_CTRL_I32
            || gParamIndex == DTS_PARAM_VX_DC_CONTROL_I32 || gParamIndex == DTS_PARAM_VX_DEF_CONTROL_I32
            || gParamIndex == DTS_PARAM_AEQ_INPUT_GAIN_I16 || gParamIndex == DTS_PARAM_AEQ_OUTPUT_GAIN_I16
            || gParamIndex == DTS_PARAM_AEQ_BYPASS_GAIN_I16)  {
            sscanf(argv[3], "%f", &gParamScale);
            LOG("EffectIndex:%d, ParamIndex:%d, ParamScale:%f\n", gEffectIndex, gParamIndex, gParamScale);
        } else if (gParamIndex == AUDIO_DTS_PARAM_TYPE_TRU_SURROUND || gParamIndex == AUDIO_DTS_PARAM_TYPE_CC3D ||
            gParamIndex == AUDIO_DTS_PARAM_TYPE_TRU_DIALOG) {
            for (int i = 0; i < 8; i++) {
                sscanf(argv[i + 3], "%f", &grange[i]);
                LOG("EffectIndex:%d, ParamIndex:%d, grange:%f\n", gEffectIndex, gParamIndex, grange[i]);
            }
        } else if (gParamIndex == AUDIO_DTS_PARAM_TYPE_TRU_BASS) {
            for (int i = 0; i < 10; i++) {
                sscanf(argv[i + 3], "%f", &grange[i]);
                LOG("EffectIndex:%d, ParamIndex:%d, grange:%f\n", gEffectIndex, gParamIndex, grange[i]);
            }
        } else if (gParamIndex == AUDIO_DTS_PARAM_TYPE_DEFINITION) {
            for (int i = 0; i < 6; i++) {
                sscanf(argv[i + 3], "%f", &grange[i]);
                LOG("EffectIndex:%d, ParamIndex:%d, grange:%f\n", gEffectIndex, gParamIndex, grange[i]);
            }
        } else if (gParamIndex == AUDIO_DTS_PARAM_TYPE_TRU_VOLUME) {
            for (int i = 0; i < 3; i++) {
                sscanf(argv[i + 3], "%f", &grange[i]);
                LOG("EffectIndex:%d, ParamIndex:%d, grange:%f\n", gEffectIndex, gParamIndex, grange[i]);
            }
        } else if (gParamIndex == DTS_PARAM_AEQ_BAND_Fre || gParamIndex == DTS_PARAM_AEQ_BAND_Gain ||
            gParamIndex == DTS_PARAM_AEQ_BAND_Q || gParamIndex == DTS_PARAM_AEQ_BAND_type) {
            for (int i = 0; i < 5; i++) {
                sscanf(argv[i + 3], "%f", &grange[i]);
                LOG("EffectIndex:%d, ParamIndex:%d, grange:%f\n", gEffectIndex, gParamIndex, grange[i]);
            }
        } else {
            sscanf(argv[3], "%d", &gParamValue);
            LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
        }
        break;
     case AML_EFFECT_VIRTUALX_v4:
        ret = set_param_from_cmd_line(argc, argv);//Parse input parameter
        if (ret < 0) {
            LOG("main() Virtualx parse cmd line for VX Param fail!\n");
        }
        gParamIndex = ret;
        break;;
    case AML_EFFECT_DBX:
        if (gParamIndex == DBX_Write_Param|| gParamIndex== DBX_Write_Coeff ||gParamIndex == DBX_Write_VCF) {
            if (argc < 5) {
                LOG("need 5 parameter,effect_tool EffectInde CommandIndex DBXindex DBXparam");
                goto Retry_input;
            } else {
                sscanf(argv[3], "%d", &gParamDBXIndex);
                sscanf(argv[4], "%d", &gParamValue);
            }
            LOG("DBX_EffectIndex:%d, ParamIndex:%d, gParamDBXIndex: %d, Paramvalue:%x\n", gEffectIndex, gParamIndex, gParamDBXIndex, gParamValue);
        } else {
            gParamValue = 0;
            sscanf(argv[3], "%d", &gParamDBXIndex);
            LOG("DBX_EffectIndex:%d, ParamIndex:%d, gParamDBXIndex:%d\n", gEffectIndex, gParamIndex, gParamDBXIndex);
        }
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
        case AML_EFFECT_VIRTUALX:
            //------------set Virtualx parameters-------------------------------------------
            if (Virtualx_effect_func(gParamIndex, gParamValue,gParamScale,grange) < 0)
                LOG("Virtualx Test failed\n");
            break;
       case AML_EFFECT_VIRTUALX_v4:
            //------------set Virtualx v4 parameters-------------------------------------------
            Virtualx_v4_effect_func();
            break;
       case AML_EFFECT_DBX:
            //------------set DBX sound effect parameters--------------------------------------
            if (DBX_effect_func(gParamIndex, gParamDBXIndex, gParamValue) < 0)
                LOG("DBX Test failed\n");
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

        /* AML_EFFECT_VIRTUALX_v4 gParamIndex support string
         * so GetIntData not used at AML_EFFECT_VIRTUALX_v4
         * use set_param_from_scanf () to parse the parameters of DTS V4
        */
        if (gEffectIndex != AML_EFFECT_VIRTUALX_v4) {
            if (GetIntData(&gParamIndex) < 0)
                goto Retry_input;
        }

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
        case AML_EFFECT_VIRTUALX:
            //------------get Virtualx parameters--------------------------------------
            if ((gParamIndex >= DTS_PARAM_MBHL_BYPASS_GAIN_I32 && gParamIndex <= DTS_PARAM_MBHL_VOLUME_I32)
                || gParamIndex == DTS_PARAM_MBHL_OUTPUT_GAIN_I32
                || (gParamIndex >= DTS_PARAM_MBHL_COMP_LOW_RATIO_I32 && gParamIndex <= DTS_PARAM_MBHL_COMP_LOW_MAKEUP_I32)
                || (gParamIndex >= DTS_PARAM_MBHL_COMP_MID_RATIO_I32 && gParamIndex <= DTS_PARAM_MBHL_COMP_MID_MAKEUP_I32)
                || (gParamIndex >= DTS_PARAM_MBHL_COMP_HIGH_RATIO_I32 && gParamIndex <= DTS_PARAM_MBHL_FAST_ATTACK_I32)
                || gParamIndex == DTS_PARAM_MBHL_APP_FRT_LOWCROSS_F32 || gParamIndex == DTS_PARAM_MBHL_APP_FRT_MIDCROSS_F32
                || gParamIndex == DTS_PARAM_TBHDX_MAXGAIN_I32
                || gParamIndex == DTS_PARAM_TBHDX_TEMP_GAIN_I32 || gParamIndex == DTS_PARAM_VX_HEADROOM_GAIN_I32
                || gParamIndex == DTS_PARAM_TBHDX_APP_HPRATIO_F32 || gParamIndex == DTS_PARAM_TBHDX_APP_EXTBASS_F32
                || gParamIndex == DTS_PARAM_VX_PROC_OUTPUT_GAIN_I32 || gParamIndex == DTS_PARAM_TSX_LPR_GAIN_I32
                || gParamIndex == DTS_PARAM_TSX_CENTER_GAIN_I32 || gParamIndex == DTS_PARAM_TSX_HEIGHTMIX_COEFF_I32
                || gParamIndex == DTS_PARAM_TSX_FRNT_CTRL_I32 || gParamIndex == DTS_PARAM_TSX_SRND_CTRL_I32
                || gParamIndex == DTS_PARAM_VX_DC_CONTROL_I32 || gParamIndex == DTS_PARAM_VX_DEF_CONTROL_I32
                || gParamIndex == DTS_PARAM_AEQ_INPUT_GAIN_I16 || gParamIndex == DTS_PARAM_AEQ_OUTPUT_GAIN_I16
                || gParamIndex == DTS_PARAM_AEQ_BYPASS_GAIN_I16)  {
                if (GetFloatData(&gParamScale) < 0)
                    goto Retry_input;
                LOG("EffectIndex:%d, ParamIndex:%d, ParamScale:%f\n", gEffectIndex, gParamIndex, gParamScale);
            } else {
                if (GetIntData(&gParamValue) < 0)
                    goto Retry_input;
                LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
            }
            break;
        case AML_EFFECT_VIRTUALX_v4:
            //------------get Virtualx parameters--------------------------------------
            ret = set_param_from_scanf(AML_EFFECT_VIRTUALX_v4);
            if (ret < 0) {
                goto Retry_input;
            }
            gParamIndex = ret;
            break;

        case AML_EFFECT_AVL:
            //------------get Avl parameters-------------------------------------------
            if (GetIntData(&gParamValue) < 0)
                goto Retry_input;
            LOG("EffectIndex:%d, ParamIndex:%d, Paramvalue:%d\n", gEffectIndex, gParamIndex, gParamValue);
            break;
        case AML_EFFECT_DBX:
            //-----------get DBX parameters-------------------------------------------
            if (gParamIndex == DBX_Write_Param|| gParamIndex== DBX_Write_Coeff ||gParamIndex == DBX_Write_VCF) {
                if (GetIntData(&gParamDBXIndex) < 0)
                    goto Retry_input;
                if (GetIntData(&gParamValue) < 0)
                    goto Retry_input;
                LOG("DBX EffectIndex:%d, ParamIndex:%d, gParamDBXIndex: %d, Paramvalue:%x\n", gEffectIndex, gParamIndex, gParamDBXIndex, gParamValue);
            } else {
                if (GetIntData(&gParamDBXIndex) < 0)
                    goto Retry_input;
                gParamValue = 0;
                LOG("DBX EffectIndex:%d, ParamIndex:%d, gParamDBXIndex:%d\n", gEffectIndex, gParamIndex, gParamDBXIndex);
            }
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

