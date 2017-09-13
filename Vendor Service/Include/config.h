#ifndef _CONFIG_H_
#define _CONFIG_H_ 

#include "lib.h"

//#define  _DEBUG_

#define USER_32K_CLOCK_RCOSC

#define USER_MARCHE_STATE

#ifdef USER_MARCHE_STATE

typedef struct  {
	uint8_t	sz;
	uint8_t	data[MAX_ATT_DATA_SZ];
}gap_att_array;


/*
marche_state.state:
bit8: 1状态有更新 0：状态无更新
bit[7:0] : 具体状态
0: 保留
1：蓝牙已连接
2:    蓝牙断开连接
3:    GAP_EVT_ATT_HANDLE_CONFIGURE
4:    GAP_EVT_ATT_WRITE
5:    GAP_EVT_ATT_READ
6:    GAP_EVT_ATT_HANDLE_CONFIRMATION
7:    GAP_EVT_ENC_KEY
8:    GAP_EVT_ENC_START
9:    GAP_EVT_CONNECTION_UPDATE_RSP.CONN_PARAM_REJECTED
10:  ancs end
11:  not ancs
12:  not band
13:  Security OK
14:  app commerd

*/
typedef struct{
	uint8_t state;
	union
	{
		uint8_t datau8;
	    uint16_t	uuid;
		uint16_t	hdl;
		gap_att_array att_evt;
	}data;
} MARCHE_STATE;

extern MARCHE_STATE   march_state;

#endif

#endif
