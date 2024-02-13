

  /*   0 */		_M(MSG_READY=0, 		"__Ready", "Ready"),
  // Presence detection
  /*   1 */   	_M(MSG_DETECT_START, 	"__Detect", 	"G  Detection started"),
  /*   2 */     _M(MSG_DETECT_FOUND,	"CLFound",		"G  Found presence canton=%d loco=%d"),
  /*   3 */     _RESERVED(r3),
  /*   4 */     _RESERVED(r4),
  /*   5 */     _RESERVED(r5),
  /*   6 */     _RESERVED(r6),
  /*   7 */     _RESERVED(r7),

// Auto
  /*   8 */		_M(MSG_AUTO_START, 		"T_A:Start",	"T%d AUTO: Start"),
  /*   9 */		_M(MSG_AUTO_CHDIR, 		"T_A:Chdir",	"T%d AUTO: Chdir"),
  /*  10 */		_M(MSG_AUTO_DONE,		"T_A:Done",		"T%d AUTO: Done"),
  /*  11 */		_M(MSG_AUTO_UNEXPSTOP, 	"T_A:Stp?",		"T%d AUTO: Unexpected stop"),
  /*  12 */     _RESERVED(r12),
  /*  13 */     _RESERVED(r13),
  /*  14 */     _RESERVED(r14),
  /*  15 */     _RESERVED(r15),

// Ctrl
  /*  16 */     _M(MSG_TRAIN_ON,		"T_ON",			"T%d Train ON"),
  /*  17 */		_M(MSG_TRAIN_OFF,      	"T_OFF",        "T%d Train off"),

