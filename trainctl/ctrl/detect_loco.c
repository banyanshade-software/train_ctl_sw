/*
 * detect_loco.c
 *
 *  Created on: Mar 9, 2022
 *      Author: danielbraun
 */


#include "misc.h"
//#include "../train.h"
#include "../config/conf_train.h"
#include "detect_loco.h"




#define NUM_KNOWN 5

static const bemf_anal_t known_loco[NUM_KNOWN] = {
		{loco_8875_v160, {0, -17, -62, -119, -206, -312 }}, //-433, -538}},
					//   {0, -36, -229, -375, -548, -713, -854, -998} },
					//   {3, -17, -60, -112, -201, -301, -411, -510}
					//	{0, -17, -62, -119, -206, -312, -433, -538}

		{loco_8821_v221, {-4, -35, -393, -1224, -1270, -1261 }}, //, -1263, -1256} },
					//   {-4, -45, -398, -1205, -1265, -1263, -1265, -1252}
					//   {-4, -45, -398, -1205, -1265, -1263, -1265, -1252}
					//  {-2, -25, -143, -407, -963, -1251, -1275, -1328}

		{loco_8805_br89, {2, -145, -303, -377, -418, -449 }}, //, -467, -477} },
					//   {31, -125, -283, -357, -407, -442, -462, -2102},
					//   {1, -124, -287, -351, -389, -424, -439, -456}
		{loco_8864_v60,   {-5, -23, -304, -1253, -1268, -1265 }}, //, -1269, -1258} }, // old one
					//   {-5, -23, -304, -1253, -1268, -1265, -1269, -1258}
					//   {-9, -22, -306, -1272, -1288, -1280, -1265, -1257}
		{loco_8864_v60b, {-6, -41, -1252, -1319, -1317, -1322 }}, //, -1310, -1281} },

					//   {-4, -30, -594, -1310, -1306, -1297, -1293, -1280} // new one
					//   {-10, -35, -1047, -1304, -1319, -1300, -1298, -777}
					//   {-3, -39, -1235, -1319, -1310, -1304, -1300, -1292}
 	 	 	 	 	// {-6, -41, -1252, -1319, -1317, -1322, -1310, -1281
		//{loco_8864_v60,

};



int16_t detect_loco_find(bemf_anal_t *d)
{
	int besti = -1;
	int32_t besterr = 0x7FFFFFF;
	for (int tst=0; tst<NUM_KNOWN; tst++) {
		int32_t err = 0;
		for (int f = 0; f<DETECT_NUM_FREQS; f++) {
			int32_t de =  d->R[f] - known_loco[tst].R[f];
			err += de * de;
		}
		itm_debug2(DBG_DETECT, "tst", tst, err);
		if (err<besterr) {
			besti = tst;
			besterr = err;
		}
	}
	if (besti<0) {
		itm_debug2(DBG_DETECT, "nobest", besti, besterr);
		FatalError("nobt", "besti -1", Error_Abort);
		return -1;
	}
	itm_debug3(DBG_DETECT, "best", besti, besterr, known_loco[besti].d);
	return known_loco[besti].d;
}


const char *loco_detect_name(enum loco_detect_locos l)
{
	switch (l) {
	case loco_none : 		return "(none)";
	case loco_unknown : 	return "(unknown)";
	case loco_rokuhan_chassis: return "rokuhan";
	case loco_8875_v160:	return "v160";
	case loco_8821_v221:	return "v221";
	case loco_8805_br89:	return "br29";
	case loco_8864_v60:		return "v60";
	case loco_8864_v60b:	return "v60b";
	default: return "(err)";
	}
}

#if 0

// TODO FIXME also defined (but different) in railconfig.c, fix me
#define DEFAULT_TRAIN_CFG(_R,  _kP, _kI, _kD, _P, _S)  { \
                        { /* pidctl_config_t*/ \
                                _kP, _kI, _kD,  /* kP, kI, kD */ \
                        }, \
                        { /* inertia_config_t */ \
                                120, 120        /* dec, acc */ \
                        }, \
                        vpolicy_normal,  /*vpolicy_normal, vpolicy_pure_volt, vpolicy_pure_pwm,*/ \
                        0, /* enable_inertia */        \
                        1, /* enabled */            \
                        1, /* enable_pid */            \
                        0, /* notify_pose */        \
                        0, /* bemfIIR; */            \
                        0, /* postIIR */            \
                        0, /* fix_bemf; */            \
                        0,  /*    uint8_t en_spd2pow; */\
                        20, /*    uint8_t min_power; */ \
                        _R, /* reversed */            \
                        _S, /* slipping */          \
                        _P, /* pose_per_cm */       \
                        10, /* len right*/         \
                        10, /* len left */         \
}


static const conf_train_t trcfg[] ={
    /* loco_unknown=0 */    DEFAULT_TRAIN_CFG(0, 500, 180, 500,   450, 120),
    /*loco_rokuhan_chassis*/DEFAULT_TRAIN_CFG(0, 100, 180, 100,   400, 120),
    /*loco_8875_v160*/      DEFAULT_TRAIN_CFG(1, 500, 180, 500,   450, 120),
    /*loco_8821_v221*/      DEFAULT_TRAIN_CFG(0, 500, 180, 500,   450, 120),
    /*loco_8805_br89*/      DEFAULT_TRAIN_CFG(0, 500, 180, 500,   310, 120),
    /*loco_8864_v60*/       DEFAULT_TRAIN_CFG(0, 500, 180, 500,   450, 120),
    /*loco_8864_v60b*/      DEFAULT_TRAIN_CFG(0, 500, 180, 500,   450, 120),
};

const conf_train_t *detect_loco_conf(enum loco_detect_locos loco)
{
    if (loco>=loco_end) {
    	FatalError("Detect", "bad loco num", Error_DetectBadNum);
        return &trcfg[0];
    }
    return &trcfg[loco];
}
#endif
