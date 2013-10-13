#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__


#define NUM_CHANNELS 2


/*
 * overdrive and distortion
 */

/*
 * overdrive units are between 0x40 (no overdrive)
 * and 0x100 (multiply by four).
 * technically, it can be less than 0x40,
 * but this is not the intention of this effect.
 */
extern unsigned short g_overdrive_level[NUM_CHANNELS];
#define OVERDRIVE_NORMAL_LEVEL 0x40

/* TODO: distortion units */
extern unsigned short g_distortion_level[NUM_CHANNELS];


/*
 * low-pass level
 *
 * units are from zero (nothing passes) to 0x100 (all-pass)
 * TODO: scale the units properly (exponential?)
 */
extern unsigned short g_low_pass_level[NUM_CHANNELS];
#define LOW_PASS_MAX_LEVEL 0x100

/*
 * resonance (goes together with low-pass filter)
 * units are from zero (no resonance) to 0x100 (TODO: consider more)
 */
extern unsigned short g_resonance_level[NUM_CHANNELS];
#define RESONANCE_MAX_LEVEL 0x100


/*
 * high-pass level
 *
 * units are from zero (nothing passes) to 0x100 (all-pass)
 * TODO: scale the units properly (exponential?)
 */
extern unsigned short g_high_pass_level[NUM_CHANNELS];
#define HIGH_PASS_MAX_LEVEL 0x100


/*
 * volume
 *
 * units are between zero (mute) and 0x100 (no change).
 * the intention of this value is to implement effects like tremolo,
 * fade-in, fade-out, etc.
 *
 * do not use this to turn up the volume. use overdrive instead.
 */
extern unsigned short g_volume_factor[NUM_CHANNELS];
#define VOLUME_NORMAL_LEVEL 0x100


/*
 * flanger
 */
extern unsigned short g_flanger_low_freq_limit[NUM_CHANNELS];
extern unsigned short g_flanger_high_freq_limit[NUM_CHANNELS];
extern unsigned short g_flanger_frequency[NUM_CHANNELS];
extern unsigned short g_flanger_mix_level[NUM_CHANNELS];



void parameters_setup();
void parameters_set();
void parameters_counter_increment();



#endif /* __PARAMETERS_H__ */

