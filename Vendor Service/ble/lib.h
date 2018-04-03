#ifndef _BLE_LIB_H_
#define _BLE_LIB_H_

#include "ARMCM0.h"

#define MAX_ATT_REPORT_HDL 20
#define MAX_ATT_DATA_SZ	23
#define BD_ADDR_SZ 			6
#define MAX_KEY_SZ			16
#define MIN_KEY_SZ			7
#define MAX_RAND_SZ		8
#define MAX_EDIV_SZ			2

enum _RETURN_STATUS_ {
	_PARAM_ERROR_ = 0x0000,
	_NO_ERROR_ = 0x0001,
};

enum _BLE_ADDRESS_TYPE_{
	PUBLIC_ADDRESS_TYPE	= 0x00,
	RANDOM_ADDRESS_TYPE 	= 0x01,
};

enum _GAP_EVT_{
	GAP_EVT_ADV_END						= 0x0001,
	GAP_EVT_CONNECTED					= 0x0002,
	GAP_EVT_DISCONNECTED					= 0x0004,
	GAP_EVT_ENC_KEY						= 0x0008,
	GAP_EVT_PASSKEY_REQ					= 0x0010,
	GAP_EVT_SHOW_PASSKEY_REQ			= 0x0020,
	GAP_EVT_CONNECTION_INTERVAL			= 0x0040,
	GAP_EVT_CONNECTION_SLEEP			= 0x0080,
	GAP_EVT_ATT_READ						= 0x0100,
	GAP_EVT_ATT_WRITE					= 0x0200,
	GAP_EVT_ATT_PREPARE_WRITE			= 0x0400,
	GAP_EVT_ATT_EXECUTE_WRITE			= 0x0800,
	GAP_EVT_ATT_HANDLE_CONFIRMATION	= 0x1000,
	GAP_EVT_ATT_HANDLE_CONFIGURE		= 0x2000,
	GAP_EVT_ENC_START					= 0x4000,
	GAP_EVT_CONNECTION_UPDATE_RSP =0x8000,
};

enum _CONNECTION_UPDATE_RSP_{
	CONN_PARAM_ACCEPTED =0x0000,
	CONN_PARAM_REJECTED =0x0001,
};

enum _ADV_CH_PKT_TYPE_{
	ADV_IND 			= 0x00,
	ADV_DIRECT_IND 	= 0x01,
	ADV_NOCONN_IND	= 0x02,
	SCAN_REQ			= 0x03,
	SCAN_RSP			= 0x04,
	CONNECT_REQ		= 0x05,
	ADV_SCAN_IND		= 0x06,
};

enum BLE_SEND_TYPE {
	BLE_GATT_NOTIFICATION	= 0x0001,
	BLE_GATT_INDICATION		= 0x0002,
};

enum WAKEUP_TYPE { 
	SLEEP_WAKEUP			= 0,
	POWERDOWN_WAKEUP	= 1,
};

enum SMP_IO_CAPABILITY {
	IO_DISPLAY_ONLY		= 0x00,
	IO_DISPLAY_YESNO		= 0x01,
	IO_KEYBOARD_ONLY		= 0x02,
	IO_NO_INPUT_OUTPUT	= 0x03,	
	IO_KEYBOARD_DISPLAY	= 0x04,	
};

enum SMP_OOB_FLAG {
	OOB_AUTH_NOT_PRESENT= 0x00,
	OOB_AUTH_PRESENT		= 0x01,
};

enum SMP_BONDING_FLAGS {
	AUTHREQ_NO_BONDING	= 0x00,
	AUTHREQ_BONDING		= 0x01,
};

enum SMP_KEY_DISTRIBUTION {
	SMP_KEY_MASTER_IDEN	= 0x01,
	SMP_KEY_ADDR_INFO	= 0x02,
	SMP_KEY_SIGNIN_INFO	= 0x04,
};

enum _SYSTEM_CLOCK_SEL_{
	SYSTEM_CLOCK_32M_RCOSC	= 0x00,
	SYSTEM_CLOCK_16M_RCOSC	= 0x01,
	SYSTEM_CLOCK_8M_RCOSC	= 0x02,
	SYSTEM_CLOCK_4M_RCOSC	= 0x03,
	SYSTEM_CLOCK_16M_XOSC	= 0x04,
};

enum _32K_CLOCK_SEL_{
	SYSTEM_32K_CLOCK_RCOSC	= 0x00,
	SYSTEM_32K_CLOCK_XOSC	= 0x01,
};

enum ATT_CMD_CODE {
	ATT_ERR_RSP					= 0x01,
	ATT_MTU_REQ					= 0x02,
	ATT_MTU_RSP					= 0x03,
	ATT_FIND_INFO_REQ				= 0x04,
	ATT_FIND_INFO_RSP				= 0x05,
	ATT_FIND_BY_TYPE_VALUE_REQ	= 0x06,
	ATT_FIND_BY_TYPE_VALUE_RSP	= 0x07,	
	ATT_READ_BY_TYPE_REQ			= 0x08,
	ATT_READ_BY_TYPE_RSP			= 0x09,
	ATT_READ_REQ					= 0x0A,
	ATT_READ_RSP					= 0x0B,
	ATT_READ_BLOB_REQ			= 0x0C,
	ATT_READ_BLOB_RSP				= 0x0D,
	ATT_READ_MULTIPLE_REQ		= 0x0E,
	ATT_READ_MULTIPLE_RSP		= 0x0F,
	ATT_READ_BY_GROUP_TYPE_REQ	= 0x10,
	ATT_READ_BY_GROUP_TYPE_RSP	= 0x11,
	ATT_WRITE_REQ					= 0x12,
	ATT_WRITE_RSP					= 0x13,
	ATT_WRITE_CMD					= 0x52,
	ATT_PREPARE_WRITE_REQ		= 0x16,
	ATT_PREPARE_WRITE_RSP		= 0x17,
	ATT_EXECUTE_WRITE_REQ		= 0x18,
	ATT_EXECUTE_WRITE_RSP		= 0x19,
	ATT_HANDLE_VAL_NOTIFICATION	= 0x1B,
	ATT_HANDLE_VAL_INDICATION	= 0x1D,
	ATT_HANDLE_VAL_CONFIRMATION	= 0x1E,
	ATT_SIGNED_WRITE_CMD			= 0xD2,
};

#pragma pack(1)
struct gap_att_report {
	uint16_t	primary;
	uint16_t	uuid;
	uint16_t	hdl;
	uint16_t	config;
	uint16_t	value;
};

struct gap_att_report_handle {
	uint8_t	cnt;
	struct	gap_att_report report[MAX_ATT_REPORT_HDL];
};

struct gap_disconnected_evt {
	uint8_t	reason;
};

struct gap_ble_addr {
	uint8_t	type;
	uint8_t	addr[BD_ADDR_SZ];
};

struct gap_key_params{
	uint8_t	ediv[MAX_EDIV_SZ];
	uint8_t	rand[MAX_RAND_SZ];	
	uint8_t	ltk[MAX_KEY_SZ];
	uint8_t	iden[MAX_KEY_SZ];
};

struct gap_att_read_evt {
	uint16_t	primary;
	uint16_t	uuid;
	uint16_t	hdl;
	uint16_t	offset;
};

struct gap_att_write_evt {
	uint16_t	primary;
	uint16_t	uuid;
	uint16_t	hdl;
	uint8_t	sz;
	uint8_t	data[MAX_ATT_DATA_SZ];
};

struct gap_att_handle_configure_evt {
	uint16_t	uuid;
	uint16_t	hdl;
	uint16_t	value;
};

struct gap_connection_update_rsp_evt {
	uint16_t	result;
};

struct gap_ble_evt {
	uint8_t	evt_type;
	uint16_t	evt_code;
	union
	{
		struct gap_disconnected_evt 	disconn_evt;
		struct gap_ble_addr			bond_dev_evt;
		struct gap_key_params		enc_key_evt;
		struct gap_att_read_evt		att_read_evt;
		struct gap_att_write_evt		att_write_evt;
		struct gap_att_handle_configure_evt		att_handle_config_evt;
		struct gap_connection_update_rsp_evt    connection_update_rsp_evt;
	} evt; 
};

struct gap_evt_callback {
	uint16_t	evt_mask;
	void 	(*p_callback)(struct gap_ble_evt *p_evt);
};

struct gap_wakeup_config {
	uint8_t wakeup_type;
	uint8_t timer_wakeup_en;
	uint8_t gpi_wakeup_en;
	uint32_t gpi_wakeup_cfg;
};

struct gap_adv_params {
	uint8_t				type;
	struct gap_ble_addr	peer_addr;
	uint8_t       policy;
	uint8_t				channel;
	uint16_t      interval;  /**< Between 0x0020 and 0x4000 in 0.625 ms units (20ms to 10.24s) */
	uint16_t      timeout;	 /**< Between 0x0001 and 0x3FFF in seconds, 0x0000 is disable */
};

struct gap_update_params {
	uint16_t  updateitv_min;
	uint16_t  updateitv_max;
	uint16_t  updatelatency;
	uint16_t  updatesvto;
};

struct smp_pairing_req{
	uint8_t io;
	uint8_t oob;
	uint8_t flags:2;
	uint8_t mitm:1;
	uint8_t rsvd:5;
	uint8_t max_enc_sz;
	uint8_t init_key;
	uint8_t rsp_key;
};

struct gap_link_params {	
	uint16_t	interval;
	uint16_t	latency;
	uint16_t	svto;
};

struct gap_ll_evt_callback {
	uint8_t		evt_callback;
	uint8_t		skip_rom_api;
	uint16_t	evt_mask;
	void 	(*p_callback)(struct gap_ble_evt *p_evt);
};

struct gap_bond_dev {			
	struct gap_ble_addr 		addr;
	struct gap_key_params	key;	
};

struct att_err_rsp{
	uint8_t opcode;
	uint16_t hdl;
	uint8_t err;
};

struct att_find_by_type_val_req{
	uint16_t start_hdl;
	uint16_t end_hdl;
	uint16_t att_type;
	uint8_t att_val[MAX_ATT_DATA_SZ-7];
};

struct att_mtu_rsp{
	uint16_t mtu;
};

struct att_find_info_16{
	uint16_t hdl;
	uint8_t uuid[2];
};

struct att_find_info_128{
	uint16_t hdl;
	uint8_t uuid[16];
};

union  att_find_info_payload {
	struct att_find_info_16   uuid16[5];
	struct att_find_info_128 uuid128;
};

struct att_find_info_rsp{
	uint8_t format;
	union att_find_info_payload pair;
};

struct att_find_by_type_val_rsp{
	uint8_t list[MAX_ATT_DATA_SZ-1];
};

struct att_read_by_type_16{
	uint16_t hdl;
	uint8_t property;
	uint16_t val_hdl;
	uint8_t char_uuid[2];
};

struct att_read_by_type_128{
	uint16_t hdl;
	uint8_t property;
	uint16_t val_hdl;
	uint8_t char_uuid[16];
};

union  att_read_by_type_payload {
	struct att_read_by_type_16   uuid16[3];
	struct att_read_by_type_128 uuid128;
};

struct att_read_by_type_rsp{
	uint8_t length;
	union att_read_by_type_payload pair;
};

struct att_read_by_type_include_rsp{
	uint8_t length;
	uint16_t hdl;
	uint8_t buf[MAX_ATT_DATA_SZ-2];
};

struct att_read_by_type_pair_val{
	uint16_t hdl;
	uint8_t val[10];
};

struct att_read_by_type_val_rsp{
	uint8_t length;
	struct att_read_by_type_pair_val pair[1];
};

struct att_read_by_type_service_16{
	uint16_t hdl;
	uint8_t uuid[2];
};

struct att_read_by_type_service_128{
	uint16_t hdl;
	uint8_t uuid[16];
};

union  att_read_by_type_service_payload {
	struct att_read_by_type_service_16   uuid16[3];
	struct att_read_by_type_service_128 uuid128;
};

struct att_read_by_type_service_rsp{
	uint8_t length;
	union att_read_by_type_service_payload pair;
};

struct att_read_by_type_chartextend_rsp{
	uint8_t length;
	uint16_t hdl;
	uint8_t val[MAX_ATT_DATA_SZ-4];
};

struct att_read_rsp{
	uint8_t buf[MAX_ATT_DATA_SZ-1];
};

struct att_read_blob_rsp{
	uint8_t buf[MAX_ATT_DATA_SZ-1];
};

struct att_read_multiple_rsp{
	uint8_t val[MAX_ATT_DATA_SZ-1];
};

struct att_read_by_group_type_16{
	uint16_t hdl;
	uint16_t end_hdl;
	uint8_t uuid[2];
};

struct att_read_by_group_type_128{
	uint16_t hdl;
	uint16_t end_hdl;
	uint8_t uuid[16];
};

union  att_read_by_group_type_payload {
	struct att_read_by_group_type_16   uuid16[3];
	struct att_read_by_group_type_128 uuid128;
};

struct att_read_by_group_type_rsp{
	uint8_t length;
	union  att_read_by_group_type_payload pair;
};

struct att_hdl_val_notifivation{
	uint16_t hdl;
	uint8_t buf[MAX_ATT_DATA_SZ-3];
};

struct att_hdl_val_indication{
	uint16_t hdl;
	uint8_t buf[MAX_ATT_DATA_SZ-3];
};

struct attc_ble_evt {
	uint8_t	attc_code;
	uint8_t	attc_sz;
	union
	{
		struct att_err_rsp AttErrRsp;
		struct att_mtu_rsp AttMtuRsp;
		struct att_find_info_rsp AttFindInfoRsp;
		struct att_find_by_type_val_rsp AttFindByTypeValRsp;
		struct att_read_by_type_rsp AttReadByTypeRsp;
		struct att_read_by_type_include_rsp AttReadByTypeIncludeRsp;
		struct att_read_by_type_val_rsp AttReadByTypeValRsp;
		struct att_read_by_type_service_rsp AttReadByTypeServiceRsp;
		struct att_read_by_type_chartextend_rsp AttReadByTypeChartExtendRsp;
		struct att_read_rsp AttReadRsp;
		struct att_read_blob_rsp AttReadBlobRsp;
		struct att_read_multiple_rsp AttReadMultipleRsp;
		struct att_read_by_group_type_rsp AttReadByGroupTypeRsp;
		struct att_hdl_val_notifivation AttHdlValNotification;
		struct att_hdl_val_indication AttHdlValIndication;
	} attc; 
};


#pragma pack()

#ifdef _BLE_LIB_C
#else
	/* ble protocol */
	extern uint8_t BleInit(void);
	extern uint8_t StartAdv(void);	
	extern uint8_t DisConnect(void);	
	extern uint8_t SetDevAddr(struct gap_ble_addr* p_dev_addr);	
	extern uint8_t GetDevAddr(struct gap_ble_addr* p_dev_addr);
	extern uint8_t SetAdvData(uint8_t *p_adv, uint8_t adv_sz,uint8_t *p_scan, uint8_t sacn_sz);
	extern uint8_t SetAdvParams(struct gap_adv_params *p_adv_params);
	extern uint8_t SetSecParams(struct smp_pairing_req *p_sec_params);
	extern uint8_t SecurityReq(uint8_t flag, uint8_t mitm);
	extern uint8_t SetConnectionUpdate(struct gap_update_params *p_update_params);	
	extern uint8_t GetLinkParameters(struct gap_link_params* p_link);
	extern uint8_t ConnectionLatencyMode(uint8_t en);
	extern uint8_t SetPasskey(uint32_t passkey);			
	extern uint8_t GetGATTReportHandle(struct gap_att_report_handle** p_hdl);
	extern uint8_t SetGATTReadRsp(uint8_t len,uint8_t *p_data);
	extern uint8_t GATTDataSend(uint8_t type, struct gap_att_report* p_report, uint8_t len, uint8_t *p_data);
	extern uint8_t SetEvtCallback(struct gap_evt_callback* p_callback);	
	extern void ble_sched_execute(void);
	extern void DelayUS(uint32_t dly);
	extern uint8_t Rand(void);
	
	/* system clock */
	extern uint8_t MCUClockSwitch(uint8_t sel);
	extern uint8_t GetMCUClock(uint8_t *p_sel);
	extern uint8_t ClockSwitch(uint8_t sel);	
	extern uint8_t GetClock(uint8_t *p_sel);
	extern uint8_t LPOCaliWaitUS(uint32_t delay);
	extern uint8_t LPOCalibration(void);

	/* sleep & power down */
	extern uint8_t WakeupConfig(struct gap_wakeup_config *p_cfg);
	extern void SystemSleep(void);
	extern void SystemPowerDown(void);
	extern void SystemReset(void);
	extern uint8_t RFSleep(void);
	extern uint8_t RFWakeup(void);

	/* bond Manager */
	extern uint8_t SetBondManagerIndex(uint8_t idx);
	extern uint8_t GetBondDevice(struct gap_bond_dev *p_device);
	extern uint8_t DelBondDevice(void);	
	extern uint8_t DelAllBondDevice(void);

	/* read&write flash */
	extern uint8_t ReadProfileData(uint16_t addr , uint16_t len, uint8_t *p_buf);
	extern uint8_t WriteProfileData(uint16_t addr , uint16_t len, uint8_t *p_buf);
	extern uint8_t ble_flash_erase(uint32_t address, uint8_t num);
	extern uint8_t ble_flash_read(uint32_t address, uint8_t len, uint8_t *p_buf);
	extern uint8_t ble_flash_write(uint32_t address, uint8_t len, uint8_t *p_buf);
	
	/* fw upgrade */
	extern uint8_t CodeErase(void);
	extern uint8_t CodeWrite(uint16_t offset , uint16_t len, uint8_t *p_buf);
	extern uint8_t CodeUpdate(uint8_t *p_desc, uint8_t *p_ver, uint16_t sz, uint16_t checksum);

	/* attc */
	extern uint8_t SetATTCCallback(void* p_callback);
	extern uint8_t ATTC_MTUReq(uint16_t mtu);
	extern uint8_t ATTC_FindInfoReq(uint16_t start_hdl, uint16_t end_hdl);
	extern uint8_t ATTC_FindByTypeValueReq(uint16_t start_hdl, uint16_t end_hdl, uint16_t type, uint8_t val_sz, uint8_t *p_val);
	extern uint8_t ATTC_ReadByTypeReq(uint16_t start_hdl, uint16_t end_hdl, uint16_t type_sz, uint8_t *p_type);
	extern uint8_t ATTC_ReadReq(uint16_t hdl);
	extern uint8_t ATTC_ReadBlobReq(uint16_t hdl, uint16_t offset);
	extern uint8_t ATTC_ReadMultipleReq(uint8_t hdl_sz, uint8_t *p_hdl);
	extern uint8_t ATTC_ReadByGroupTypeReq(uint16_t start_hdl, uint16_t end_hdl, uint16_t type_sz, uint8_t *p_type);
	extern uint8_t ATTC_WriteReq(uint16_t hdl, uint16_t sz, uint8_t *p_buf);
	extern uint8_t ATTC_WriteCmdReq(uint16_t hdl, uint16_t sz, uint8_t *p_buf);
	extern uint8_t ATTC_PrepareWriteReq(uint16_t hdl, uint16_t offset, uint16_t sz, uint8_t *p_buf);
	extern uint8_t ATTC_ExecuteWriteReq(uint8_t flags);
	extern uint8_t ATTC_Confirmation(void);
	
	extern uint8_t AttTxPacket(uint8_t code, uint8_t *p_buf, uint16_t len);

	extern void smp_aes_encrypt(uint8_t *k, uint8_t* p, uint8_t* c);
#endif

#endif


