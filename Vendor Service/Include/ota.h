#ifndef _OTA_H_
#define _OTA_H_

#include "ARMCM0.h"

#pragma pack(1)

#define CMD_FW_ERASE		0x16
#define CMD_FW_WRITE		0x17
#define CMD_FW_UPGRADE	0x18

#define ERR_COMMAND_SUCCESS           0x00
#define EVT_COMMAND_COMPLETE		(0x0E)
#define CMD_COMPLETE_HDR_SZ		1
#define MAX_FW_WRITE_SZ			15

struct ret_fw_erase_cmd {
	uint8_t 	status;
};

struct cmd_fw_write {
	uint16_t	offset;
	uint8_t	sz;
	uint8_t	data[MAX_FW_WRITE_SZ];
};

struct ret_fw_write_cmd {
	uint8_t 	status;
};

struct cmd_fw_upgrade {
	uint16_t	sz;
	uint16_t	checksum;
};

struct ret_fw_upgrade_cmd {
	uint8_t 	status;
};

union  cmd_parm {
	struct cmd_fw_write			CmdFwWrite;
	struct cmd_fw_upgrade		CmdFwUpgrade;
};

struct hci_cmd {
	uint8_t	opcode;
	uint8_t	len;
	union cmd_parm cmdparm;
};

union ret_parm {
	struct ret_fw_erase_cmd		RetFwEraseCmd;
	struct ret_fw_write_cmd		RetFwWriteCmd;
	struct ret_fw_upgrade_cmd		RetFwUpgradeCmd;
};

struct evt_command_complete {
	uint8_t	opcode;
	union ret_parm RetParm;
};

union  evt_parm {
	struct evt_command_complete 				EvtCommandComplete;
};

struct hci_evt {
	uint8_t	evtcode;
	uint8_t	evtlen;
	union evt_parm evtparm;
};

#pragma pack()

#ifdef _OTA_C
#else
	extern void ota_cmd(uint8_t *p_cmd, uint8_t sz);
	extern void ota_rsp(uint8_t *p_rsp, uint8_t *p_sz);
#endif

#endif

