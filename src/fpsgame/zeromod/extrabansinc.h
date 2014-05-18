// before ban struct

#define BAN_TYPE_MASK           0xFF
#define BAN_TYPE_NORMAL         0x00
#define BAN_TYPE_SPECTATOR      0x01
#define BAN_TYPE_MUTE           0x02
#define BAN_TYPE_NAMEMUTE       0x04
#define BAN_TYPE_SPY            0x08

#define BAN_REASON_MASK         0xFF00
#define BAN_REASON_DEFAULT      0x0000
#define BAN_REASON_TEAMKILLS    0x0100

#define BAN_FLAG_MASK           0xFFFF0000
#define BAN_FLAG_PERSIST        0x80000000
