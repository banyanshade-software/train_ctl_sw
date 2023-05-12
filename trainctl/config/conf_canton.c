// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_canton.h"
#include "conf_canton.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"



#ifdef TRN_BOARD_G4SLV1

unsigned int conf_canton_num_entries(void)
{
    return 4; // 4 
}

static conf_canton_t conf_canton[4] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_4,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_6,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_7,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 1
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_5,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_3,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_2,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 2
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOB,
     .volt_b0 = GPIO_PIN_12,
     .volt_port_b1 = GPIOB,
     .volt_b1 = GPIO_PIN_13,
     .volt_port_b2 = GPIOB,
     .volt_b2 = GPIO_PIN_14,
     .pwm_timer_num = 2,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 3
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_5,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_6,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_7,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  }
};

#endif // TRN_BOARD_G4SLV1




#ifdef TRN_BOARD_G4MASTER1

unsigned int conf_canton_num_entries(void)
{
    return 4; // 4 
}

static conf_canton_t conf_canton[4] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_4,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_6,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_7,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 1
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_5,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_3,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_2,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 2
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOB,
     .volt_b0 = GPIO_PIN_12,
     .volt_port_b1 = GPIOB,
     .volt_b1 = GPIO_PIN_13,
     .volt_port_b2 = GPIOB,
     .volt_b2 = GPIO_PIN_14,
     .pwm_timer_num = 2,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 3
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_5,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_6,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_7,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  }
};

#endif // TRN_BOARD_G4MASTER1




#ifdef TRN_BOARD_UNIT_TEST

unsigned int conf_canton_num_entries(void)
{
    return 6; // 6 
}

static conf_canton_t conf_canton[6] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 1
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 2
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 3
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 4
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 5
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  }
};

#endif // TRN_BOARD_UNIT_TEST




#ifdef TRN_BOARD_UI

unsigned int conf_canton_num_entries(void)
{
    return 0; // 0 
}

static conf_canton_t conf_canton[0] = {
};

#endif // TRN_BOARD_UI




#ifdef TRN_BOARD_MAINV04

unsigned int conf_canton_num_entries(void)
{
    return 6; // 6 
}

static conf_canton_t conf_canton[6] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_4,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_6,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_7,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 1
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_5,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_3,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_2,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 2
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOB,
     .volt_b0 = GPIO_PIN_12,
     .volt_port_b1 = GPIOB,
     .volt_b1 = GPIO_PIN_13,
     .volt_port_b2 = GPIOB,
     .volt_b2 = GPIO_PIN_14,
     .pwm_timer_num = 2,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 3
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_5,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_6,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_7,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 4
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_4,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_3,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_2,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 5
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_11,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_10,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_9,
     .pwm_timer_num = 4,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  }
};

#endif // TRN_BOARD_MAINV04




#ifdef TRN_BOARD_MAINV0

unsigned int conf_canton_num_entries(void)
{
    return 6; // 6 
}

static conf_canton_t conf_canton[6] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_0,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_1,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_2,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 1
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_3,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_4,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_5,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 2
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_6,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_7,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_8,
     .pwm_timer_num = 2,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 3
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_9,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_10,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_11,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 4
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_2,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_3,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_5,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 5
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = GPIO_PIN_0,
     .volt_port_b1 = NULL,
     .volt_b1 = GPIO_PIN_0,
     .volt_port_b2 = NULL,
     .volt_b2 = GPIO_PIN_0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  }
};

#endif // TRN_BOARD_MAINV0




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_canton_num_entries(void)
{
    return 1; // 1 
}

static conf_canton_t conf_canton[1] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  }
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_canton_num_entries(void)
{
    return 0; // 0 
}

static conf_canton_t conf_canton[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_canton_num_entries(void)
{
    return 0; // 0 
}

static conf_canton_t conf_canton[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_canton_num_entries(void)
{
    return 6; // 6 
}

static conf_canton_t conf_canton[6] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 1
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 2
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 3
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 4
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  },
  {     // 5
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  }
};

#endif // TRN_BOARD_SIMU




const conf_canton_t *conf_canton_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_canton_num_entries()) {
        return NULL;
    }
    return &conf_canton[num];
}

static const conf_canton_t canton_template = {
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  };
const conf_canton_t *conf_canton_template(void)
{
  return &canton_template;
}

// canton config store type 0 num 2
int conf_canton_propagate(unsigned int numinst, unsigned int numfield, int32_t value)
{
    if (numinst>=conf_canton_num_entries()) return -1;
    conf_canton_t *conf = &conf_canton[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_canton_reverse:
        conf->reverse = value;
        break;
    case conf_numfield_canton_reverse_bemf:
        conf->reverse_bemf = value;
        break;
    }
    return 0;
}


int32_t conf_canton_default_value(unsigned int numinst, unsigned int numfield, unsigned int boardnum)
{
    (void) boardnum;
    (void) numinst;
    //if (numinst>=conf_canton_num_entries()) return 0;
    switch (numfield) {
    default: return 0;
    case conf_numfield_canton_reverse:
        return 0;
    case conf_numfield_canton_reverse_bemf:
        return 1;
    }
    return 0;
}





// end
