// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_led.h"
#include "conf_led.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"



#ifdef TRN_BOARD_G4SLV1

unsigned int conf_led_num_entries(void)
{
    return 1; // 1 
}

static conf_led_t conf_led[1] = {
  {     // 0
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_G4SLV1




#ifdef TRN_BOARD_G4MASTER1

unsigned int conf_led_num_entries(void)
{
    return 1; // 1 
}

static conf_led_t conf_led[1] = {
  {     // 0
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_G4MASTER1




#ifdef TRN_BOARD_UNIT_TEST

unsigned int conf_led_num_entries(void)
{
    return 3; // 3 
}

static conf_led_t conf_led[3] = {
  {     // 0
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  },
  {     // 1
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  },
  {     // 2
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_UNIT_TEST




#ifdef TRN_BOARD_UI

unsigned int conf_led_num_entries(void)
{
    return 0; // 0 
}

static conf_led_t conf_led[0] = {
};

#endif // TRN_BOARD_UI




#ifdef TRN_BOARD_MAINV04

unsigned int conf_led_num_entries(void)
{
    return 5; // 5 
}

static conf_led_t conf_led[5] = {
  {     // 0
     .port_led = GPIOD,
     .pin_led = GPIO_PIN_8,
     .defprog = 0,
  },
  {     // 1
     .port_led = GPIOE,
     .pin_led = GPIO_PIN_15,
     .defprog = 0,
  },
  {     // 2
     .port_led = GPIOE,
     .pin_led = GPIO_PIN_12,
     .defprog = 0,
  },
  {     // 3
     .port_led = GPIOE,
     .pin_led = GPIO_PIN_10,
     .defprog = 0,
  },
  {     // 4
     .port_led = GPIOE,
     .pin_led = GPIO_PIN_8,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_MAINV04




#ifdef TRN_BOARD_MAINV0

unsigned int conf_led_num_entries(void)
{
    return 3; // 3 
}

static conf_led_t conf_led[3] = {
  {     // 0
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_7,
     .defprog = 0,
  },
  {     // 1
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_7,
     .defprog = 0,
  },
  {     // 2
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_7,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_MAINV0




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_led_num_entries(void)
{
    return 0; // 0 
}

static conf_led_t conf_led[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_led_num_entries(void)
{
    return 6; // 6 
}

static conf_led_t conf_led[6] = {
  {     // 0
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_15,
     .defprog = 0,
  },
  {     // 1
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_3,
     .defprog = 0,
  },
  {     // 2
     .port_led = GPIOC,
     .pin_led = GPIO_PIN_13,
     .defprog = 0,
  },
  {     // 3
     .port_led = GPIOC,
     .pin_led = GPIO_PIN_14,
     .defprog = 0,
  },
  {     // 4
     .port_led = GPIOC,
     .pin_led = GPIO_PIN_15,
     .defprog = 0,
  },
  {     // 5
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_2,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_led_num_entries(void)
{
    return 0; // 0 
}

static conf_led_t conf_led[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_led_num_entries(void)
{
    return 3; // 3 
}

static conf_led_t conf_led[3] = {
  {     // 0
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  },
  {     // 1
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  },
  {     // 2
     .port_led = NULL,
     .pin_led = 0,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_SIMU




const conf_led_t *conf_led_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_led_num_entries()) {
        return NULL;
    }
    return &conf_led[num];
}

// led config store type 0 num 4
int conf_led_propagate(unsigned int numinst, unsigned int numfield, int32_t value)
{
    if (numinst>=conf_led_num_entries()) return -1;
    conf_led_t *conf = &conf_led[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_led_defprog:
        conf->defprog = value;
        break;
    }
    return 0;
}


int32_t conf_led_default_value(unsigned int numinst, unsigned int numfield, unsigned int boardnum)
{
    (void) boardnum;
    (void) numinst;
    //if (numinst>=conf_led_num_entries()) return 0;
    switch (numfield) {
    default: return 0;
    case conf_numfield_led_defprog:
        return 0;
    }
    return 0;
}





// end
