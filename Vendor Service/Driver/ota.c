#define  _OTA_C

#include "ota.h"
#include "lib.h"
#include <string.h>

static uint8_t EvtBuf[sizeof(struct hci_evt)];
struct hci_evt *pevt = (struct hci_evt *)EvtBuf;

static void EvtCommandComplete(uint8_t opcode, uint8_t *ret_parm, uint8_t ret_len)
{
	pevt->evtcode = EVT_COMMAND_COMPLETE;
	pevt->evtlen = ret_len + CMD_COMPLETE_HDR_SZ;
	
	pevt->evtparm.EvtCommandComplete.opcode= opcode;

	memcpy((uint8_t *)&pevt->evtparm.EvtCommandComplete.RetParm, ret_parm, ret_len);
}

static void CmdFwErase()
{
	struct ret_fw_erase_cmd	Ret;

	CodeErase();

	Ret.status = ERR_COMMAND_SUCCESS;
	
	EvtCommandComplete(CMD_FW_ERASE, (uint8_t *)&Ret, sizeof (Ret));
}

static void CmdFwWrite(struct cmd_fw_write *p_cmd)
{
	struct ret_fw_erase_cmd	Ret;

	CodeWrite(p_cmd->offset,p_cmd->sz,p_cmd->data);
	
	Ret.status = ERR_COMMAND_SUCCESS;
	
	EvtCommandComplete(CMD_FW_WRITE, (uint8_t *)&Ret, sizeof (Ret));
}

static void CmdFwUpgrade(struct cmd_fw_upgrade *p_cmd)
{
	struct ret_fw_erase_cmd	Ret;

	CodeUpdate(NULL, NULL, p_cmd->sz, p_cmd->checksum);

	Ret.status = ERR_COMMAND_SUCCESS;
	
	EvtCommandComplete(CMD_FW_UPGRADE, (uint8_t *)&Ret, sizeof (Ret));

	*((uint8_t*)(0x50001000 + 0x24)) = 0x01;
}

void ota_cmd(uint8_t *p_cmd, uint8_t sz)
{
	struct hci_cmd *pcmd = (struct hci_cmd*)p_cmd;

	switch(pcmd->opcode)
	{	
		case CMD_FW_ERASE:
			CmdFwErase();
			break;
		case CMD_FW_WRITE:
			CmdFwWrite(&pcmd->cmdparm.CmdFwWrite);
			break;
		case CMD_FW_UPGRADE:
			CmdFwUpgrade(&pcmd->cmdparm.CmdFwUpgrade);
			break;
	}
}

void ota_rsp(uint8_t *p_rsp, uint8_t *p_sz)
{
	memcpy(p_rsp, EvtBuf,  pevt->evtlen + 2);

	*p_sz = pevt->evtlen + 2;		
}


