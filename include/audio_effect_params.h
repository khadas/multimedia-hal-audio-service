#ifndef __AUDIO_EFFECT_PARAMS_H_
#define __AUDIO_EFFECT_PARAMS_H_

#include <hardware/hardware.h>
#include <hardware/audio.h>

#ifdef __cplusplus
extern "C"
{
#endif

//-----------Balance typedef-------------------------------
//Warning:balance is used for 51 bands

#define BALANCE_MAX_BANDS 51

#define BITS    16
#define MAXbit  (1 << (BITS - 1))
#define INT2FLOAT(a) ((float)a / MAXbit)
#define FLOAT2INT(a) (int)((float)a * MAXbit)

typedef enum {
    BALANCE_PARAM_LEVEL = 0,
    BALANCE_PARAM_ENABLE,
    BALANCE_PARAM_LEVEL_NUM,
    BALANCE_PARAM_INDEX,
} Balanceparams;

typedef struct aml_balance_param_s {
    effect_param_t param;
    uint32_t command;
    union {
        int32_t v;
        float f;
        float index[BALANCE_MAX_BANDS];
    };
} aml_balance_param_t;

//-------------VirtualSurround typedef--------------------------
typedef enum {
    VIRTUALSURROUND_PARAM_ENABLE,
    VIRTUALSURROUND_PARAM_EFFECTLEVEL,
} Virtualsurroundparams;

typedef struct aml_virtualsurround_param_s {
    effect_param_t param;
    uint32_t command;
    union {
        int32_t v;
        float f;
    };
} aml_virtualsurround_param_t;

//-------------TrebleBass typedef--------------------------
typedef enum {
    TREBASS_PARAM_INVALID = -1,
    TREBASS_PARAM_BASS_LEVEL,
    TREBASS_PARAM_TREBLE_LEVEL,
    TREBASS_PARAM_ENABLE,
} TREBASSparams;

typedef struct aml_treblebass_param_s {
    effect_param_t param;
    uint32_t command;
    union {
        uint32_t v;
        float f;
    };
} aml_treblebass_param_t;

//-------------Hpeq typedef--------------------------
typedef enum {
    HPEQ_PARAM_INVALID = -1,
    HPEQ_PARAM_ENABLE,
    HPEQ_PARAM_EFFECT_MODE,
    HPEQ_PARAM_EFFECT_CUSTOM,
} HPEQparams;

typedef struct aml_hpeq_param_s {
    effect_param_t param;
    uint32_t command;
    union {
        uint32_t v;
        float f;
        signed char band[5];
    };
} aml_hpeq_param_t;

//-------------Avl typedef--------------------------
typedef enum {
    AVL_PARAM_ENABLE,
    AVL_PARAM_PEAK_LEVEL,
    AVL_PARAM_DYNAMIC_THRESHOLD,
    AVL_PARAM_NOISE_THRESHOLD,
    AVL_PARAM_RESPONSE_TIME,
    AVL_PARAM_RELEASE_TIME,
    AVL_PARAM_SOURCE_IN,
} Avlparams;

typedef struct aml_avl_param_s {
    effect_param_t param;
    uint32_t command;
    union {
        int32_t v;
        float f;
    };
} aml_avl_param_t;

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_EFFECT_PARAMS_H_ */