// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_servo.h"
#include "conf_servo.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"



#ifdef TRN_BOARD_UNIT_TEST

unsigned int conf_servo_num_entries(void)
{
    return 0; // 0 
}

static conf_servo_t conf_servo[0] = {
};

#endif // TRN_BOARD_UNIT_TEST




#ifdef TRN_BOARD_UI

unsigned int conf_servo_num_entries(void)
{
    return 0; // 0 
}

static conf_servo_t conf_servo[0] = {
};

#endif // TRN_BOARD_UI




#ifdef TRN_BOARD_MAINV04

unsigned int conf_servo_num_entries(void)
{
    return 0; // 0 
}

static conf_servo_t conf_servo[0] = {
};

#endif // TRN_BOARD_MAINV04




#ifdef TRN_BOARD_MAINV0

unsigned int conf_servo_num_entries(void)
{
    return 0; // 0 
}

static conf_servo_t conf_servo[0] = {
};

#endif // TRN_BOARD_MAINV0




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_servo_num_entries(void)
{
    return 0; // 0 
}

static conf_servo_t conf_servo[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_servo_num_entries(void)
{
    return 2; // 2 
}

static conf_servo_t conf_servo[2] = {
  {     // 0
     .port_power = GPIOD,
     .pin_power = -1,
     .pwm_timer_num = 4,
     .pwm_timer_ch = TIM_CHANNEL_1,
     .direction = 0,
     .min = 1000,
     .max = 3700,
     .spd = 80,
     .pos0 = 5000,
  },
  {     // 1
     .port_power = GPIOD,
     .pin_power = -1,
     .pwm_timer_num = 4,
     .pwm_timer_ch = TIM_CHANNEL_2,
     .direction = 0,
     .min = 2300,
     .max = 7230,
     .spd = 80,
     .pos0 = 5000,
  }
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_servo_num_entries(void)
{
    return 0; // 0 
}

static conf_servo_t conf_servo[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_servo_num_entries(void)
{
    return 2; // 2 
}

static conf_servo_t conf_servo[2] = {
  {     // 0
     .port_power = NULL,
     .pin_power = 0,
     .pwm_timer_num = 4,
     .pwm_timer_ch = TIM_CHANNEL_1,
     .direction = 0,
     .min = 1000,
     .max = 3700,
     .spd = 80,
     .pos0 = 5000,
  },
  {     // 1
     .port_power = NULL,
     .pin_power = 0,
     .pwm_timer_num = 4,
     .pwm_timer_ch = TIM_CHANNEL_2,
     .direction = 0,
     .min = 2300,
     .max = 7230,
     .spd = 80,
     .pos0 = 5000,
  }
};

#endif // TRN_BOARD_SIMU




const conf_servo_t *conf_servo_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_servo_num_entries()) {
        return NULL;
    }
    return &conf_servo[num];
}

// servo config store type 0 num 5
int conf_servo_propagate(unsigned int numinst, unsigned int numfield, int32_t value)
{
    if (numinst>=conf_servo_num_entries()) return -1;
    conf_servo_t *conf = &conf_servo[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_servo_direction:
        conf->direction = value;
        break;
    case conf_numfield_servo_min:
        conf->min = value;
        break;
    case conf_numfield_servo_max:
        conf->max = value;
        break;
    case conf_numfield_servo_spd:
        conf->spd = value;
        break;
    case conf_numfield_servo_pos0:
        conf->pos0 = value;
        break;
    }
    return 0;
}


int32_t conf_servo_default_value(unsigned int numinst, unsigned int numfield, unsigned int boardnum)
{
    (void) boardnum;
    (void) numinst;
    //if (numinst>=conf_servo_num_entries()) return 0;
    switch (numfield) {
    default: return 0;
    case conf_numfield_servo_direction:
        return 0;
    case conf_numfield_servo_min:
        return 0;
    case conf_numfield_servo_max:
        return 0;
    case conf_numfield_servo_spd:
        return 0;
    case conf_numfield_servo_pos0:
        return 0;
    }
    return 0;
}





// end