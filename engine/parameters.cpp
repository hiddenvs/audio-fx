#include "engine/parameters.h"
#include "engine/metronome.h"
#include "lradc.h"
#include "stdint.h"
#include "stddef.h"
#include "serial.h"
#include "utils/str.h"

/* effects */
#include "effects/effect_base.h"
#include "effects/low_pass.h"
#include "effects/high_pass.h"
#include "effects/band_pass.h"
#include "effects/tremolo.h"
#include "effects/overdrive.h"
#include "effects/distortion.h"
#include "effects/delay.h"
#include "effects/resonance.h"
#include "effects/passthru.h"


EffectBase* g_effects[MAX_PRESET_COUNT][MAX_EFFECT_COUNT];
unsigned int g_preset_count;

static PassThru g_passthru;
static Resonance g_reso0;
static LowPass  g_low_pass0(&g_reso0);
static Tremolo   g_trem;
static Delay     g_reverb(true, 30000, 30000);
static Distortion g_dist;


void parameters_setup()
{
    int i=0;

    const metronome_op_t metr_ops[8] = {
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE,
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE,
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE,
        METRONOME_OP_LINEAR_FALL, METRONOME_OP_LINEAR_RISE
    };
    unsigned short metr_levels[8] = {
        0x1000, 0x1000, 0x1000, 0x1000,
        0x1000, 0x1000, 0x1000, 0x1000
    };
    /*
    metronome_op_t metr_ops[4] = {
        METRONOME_OP_CONST_FULL, METRONOME_OP_CONST_FULL,
        METRONOME_OP_CONST_FULL, METRONOME_OP_CONST_FULL
    };
    */
    const double metr_levels_lpf[8] = {
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0};
    const double metr_levels_reso[4] = {0xC00, 0xC00, 0xC00, 0xC00};

    const double metr_levels_wah[8] = {
        0xA00, 0xB00, 0xC00, 0xD00,
        0xE00, 0xD00, 0xC00, 0xB00};
    const metronome_op_t metr_ops_wah[8] = {
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION,
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION,
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION,
        METRONOME_OP_LINEAR_TRANSITION, METRONOME_OP_LINEAR_TRANSITION
    };

    memset(g_effects, 0x00, sizeof(g_effects));

    // test - initialise the metronome with some fixed parameters
    metronome_setup(180, 4, 2);

    /* test - initialise the effects with a basic setup */
    //g_reso0.set_ctrl(PARAM_CTRL_FIXED);
    //g_reso0.set_fixed_level(3200);
    //g_reso0.set_fixed_level(0);
    g_reso0.set_ctrl(PARAM_CTRL_FIXED);
    g_reso0.set_fixed_level(0.5);

    g_low_pass0.set_ctrl(PARAM_CTRL_METRONOME);
    g_low_pass0.set_metronome_ops(metr_ops, metr_levels_lpf, 8);
    //g_low_pass0.set_ctrl(PARAM_CTRL_MANUAL);
    //g_low_pass0.set_pot_index(4);
    //g_low_pass0.set_ctrl(PARAM_CTRL_FIXED);
    //g_low_pass0.set_fixed_level(0x800);

    //g_trem.set_ctrl(PARAM_CTRL_LFO);
    //g_trem.set_pot_index(4);
    //g_trem.set_metronome_ops(metr_ops, metr_levels, 8);

    //g_reverb.set_ctrl(PARAM_CTRL_MANUAL);
    //g_reverb.set_pot_index(4);
    g_reverb.set_ctrl(PARAM_CTRL_FIXED);
    g_reverb.set_fixed_level(0.5);

    //g_dist.set_ctrl(PARAM_CTRL_FIXED);
    //g_dist.set_fixed_level(0x800);

    g_effects[0][0] = &g_passthru;
    g_effects[0][1] = (EffectBase*)NULL;

    i = 0;
    //g_effects[1][i++] = &g_reso0;
    //g_effects[1][i++] = &g_low_pass0;
    //g_effects[1][i++] = &g_trem;
    //g_effects[1][i++] = &g_reverb;
    //g_effects[1][i++] = &g_dist;

    g_effects[1][i] = new Delay(true, 50, 80);
    g_effects[1][i]->set_ctrl(PARAM_CTRL_FIXED);
    g_effects[1][i]->set_fixed_level(0.5);
    ((Delay*)g_effects[1][i])->set_lfo(0.00001);
    i++;

    //g_effects[1][i] = new band_pass_t();
    //g_effects[1][i]->set_ctrl(PARAM_CTRL_METRONOME);
    //g_effects[1][i]->set_metronome_ops(metr_ops_wah, metr_levels_wah, 8);
    //i++;

    g_effects[1][i] = (EffectBase*)NULL;

    g_preset_count = 1;

    lradc_setup_channels_for_polling();
}

