/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2011 by Amaury Pouly
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include "audioout-imx233.h"
#include "clkctrl-imx233.h"
#include "rtc-imx233.h"
//#include "pcm_sampr.h"

static int hp_vol_l, hp_vol_r;

static int hp_unmute_cb()
{
    /* unmute HP */
    __REG_CLR(HW_AUDIOOUT_HPVOL) = HW_AUDIOOUT_HPVOL__MUTE;
    return 0;
}

void imx233_audioout_preinit(void)
{
    /* Enable AUDIOOUT block */
    imx233_reset_block(&HW_AUDIOOUT_CTRL);
    /* Enable DAC */
    __REG_CLR(HW_AUDIOOUT_ANACLKCTRL) = HW_AUDIOOUT_ANACLKCTRL__CLKGATE;
    /* Set capless mode */
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__CAPLESS;
    /* Set word-length to 16-bit */
    __REG_SET(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__WORD_LENGTH;
    /* Power up DAC */
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__DAC;
    /* Hold HP to ground to avoid pop, then release and power up HP */
    __REG_SET(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_HOLD_GND;
    __REG_CLR(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__HEADPHONE;
    /* Set HP mode to AB */
    __REG_SET(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_CLASSAB;
    /* change biais to -50% */
    __REG_CLR(HW_AUDIOOUT_TEST) = HW_AUDIOOUT_TEST__HP_I1_ADJ_BM;
    __REG_SET(HW_AUDIOOUT_TEST) = HW_AUDIOOUT_TEST__HP_I1_ADJ_M_50;
    __REG_CLR(HW_AUDIOOUT_REFCTRL) = HW_AUDIOOUT_REFCTRL__BIAS_CTRL_BM;
    __REG_SET(HW_AUDIOOUT_REFCTRL) = 1 << HW_AUDIOOUT_REFCTRL__BIAS_CTRL_BP;
    __REG_SET(HW_AUDIOOUT_REFCTRL) = HW_AUDIOOUT_REFCTRL__RAISE_REF;
    __REG_SET(HW_AUDIOOUT_REFCTRL) = HW_AUDIOOUT_REFCTRL__XTAL_BGR_BIAS;
    /* Stop holding to ground */
    __REG_CLR(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_HOLD_GND;
    /* Set dmawait count to 31 (see errata, workaround random stop) */
    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__DMAWAIT_COUNT_BM;
    __REG_SET(HW_AUDIOOUT_CTRL) = 31 << HW_AUDIOOUT_CTRL__DMAWAIT_COUNT_BP;
    /* start converting audio */
    __REG_SET(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__RUN;
    /* unmute DAC */
    __REG_CLR(HW_AUDIOOUT_DACVOLUME) = HW_AUDIOOUT_DACVOLUME__MUTE_LEFT |
        HW_AUDIOOUT_DACVOLUME__MUTE_RIGHT;
    /* send a few samples to avoid pop */
    HW_AUDIOOUT_DATA = 0;
    HW_AUDIOOUT_DATA = 0;
    HW_AUDIOOUT_DATA = 0;
    HW_AUDIOOUT_DATA = 0;
    /* wait for everything to stabilize before unmuting */
    hp_unmute_cb();
}

void imx233_audioout_postinit(void)
{
}

void imx233_audioout_close(void)
{
    /* Switch to class A */
    __REG_CLR(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_CLASSAB;
    /* Hold HP to ground */
    __REG_SET(HW_AUDIOOUT_ANACTRL) = HW_AUDIOOUT_ANACTRL__HP_HOLD_GND;
    /* Mute HP and power down */
    __REG_SET(HW_AUDIOOUT_HPVOL) = HW_AUDIOOUT_HPVOL__MUTE;
    /* Power down HP */
    __REG_SET(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__HEADPHONE;
    /* Mute DAC */
    __REG_SET(HW_AUDIOOUT_DACVOLUME) = HW_AUDIOOUT_DACVOLUME__MUTE_LEFT
        | HW_AUDIOOUT_DACVOLUME__MUTE_RIGHT;
    /* Power down DAC */
    __REG_SET(HW_AUDIOOUT_PWRDN) = HW_AUDIOOUT_PWRDN__DAC;
    /* Gate off DAC */
    __REG_SET(HW_AUDIOOUT_ANACLKCTRL) = HW_AUDIOOUT_ANACLKCTRL__CLKGATE;
    /* will also gate off the module */
    __REG_CLR(HW_AUDIOOUT_CTRL) = HW_AUDIOOUT_CTRL__RUN;
}

/* volume in half dB
 * don't check input values */
static void set_dac_vol(int vol_l, int vol_r)
{
    /* minimum is -100dB and max is 0dB */
    vol_l = MAX(-200, MIN(vol_l, 0));
    vol_r = MAX(-200, MIN(vol_r, 0));
    /* unmute, enable zero cross and set volume.
     * 0xff is 0dB */
    HW_AUDIOOUT_DACVOLUME =
        (0xff + vol_l) << HW_AUDIOOUT_DACVOLUME__VOLUME_LEFT_BP |
        (0xff + vol_r) << HW_AUDIOOUT_DACVOLUME__VOLUME_RIGHT_BP |
        HW_AUDIOOUT_DACVOLUME__EN_ZCD;
}

/* volume in half dB
 * don't check input values */
static void set_hp_vol(int vol_l, int vol_r)
{
    uint32_t select = (HW_AUDIOOUT_HPVOL & HW_AUDIOOUT_HPVOL__SELECT);
    /* minimum is -57.5dB and max is 6dB in DAC mode
     * and -51.5dB / 12dB in Line1 mode */
    int min = select ? -103 : -115;
    int max = select ? 24 : 12;

    vol_l = MAX(min, MIN(vol_l, max));
    vol_r = MAX(min, MIN(vol_r, max));
    /* unmute, enable zero cross and set volume. Keep select value. */
    HW_AUDIOOUT_HPVOL =
        (max - vol_l) << HW_AUDIOOUT_HPVOL__VOL_LEFT_BP |
        (max - vol_r) << HW_AUDIOOUT_HPVOL__VOL_RIGHT_BP |
        select |
        HW_AUDIOOUT_HPVOL__EN_MSTR_ZCD;
}

static void apply_volume(void)
{
    /* Two cases: line1 and dac */
    if(HW_AUDIOOUT_HPVOL & HW_AUDIOOUT_HPVOL__SELECT)
    {
        /* In line1 mode, the HP is the only way to adjust the volume */
        set_hp_vol(hp_vol_l, hp_vol_r);
    }
    else
    {
        /* In DAC mode we can use both the HP and the DAC volume.
         * Use the DAC for volume <0 and HP for volume >0 */
        set_dac_vol(MIN(0, hp_vol_l), MIN(0, hp_vol_r));
        set_hp_vol(MAX(0, hp_vol_l), MAX(0, hp_vol_r));
    }
}

void imx233_audioout_set_hp_vol(int vol_l, int vol_r)
{
    hp_vol_l = vol_l;
    hp_vol_r = vol_r;
    apply_volume();
}

/* select between DAC and Line1 */
void imx233_audioout_select_hp_input(bool line1)
{
    if(line1)
        __REG_SET(HW_AUDIOOUT_HPVOL) = HW_AUDIOOUT_HPVOL__SELECT;
    else
        __REG_CLR(HW_AUDIOOUT_HPVOL) = HW_AUDIOOUT_HPVOL__SELECT;
    /* reapply volume setting */
    apply_volume();
}
