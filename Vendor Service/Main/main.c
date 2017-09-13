/*
主函数文件
测试步骤：
1.确保已经连接上开发板并安装上PL2303的驱动程序，并且自设备管理器下可以看到串口号
2.打开工程目录下Tool文件夹里的“SYD BLE Utility.exe”程序
3.在UART里可以看到串口号，选择正确的串口
4.点击“open”按钮，此时在"SYD8801 Evaluation board"里提示:"Wait..."
5.此时按下开发板上的复位键，提示就会变成“Ready”
注意：下面的6和7只用在第一次下载中下载就行，以后将不需要下载
6.在“Ble service”的"Ble service(Upgrade)"中点击“Open Service File”按钮选择本工程文件夹的“Ble_Vendor_Service.map”文件
7.在“Ble service”的"Ble service(Upgrade)"中点击“Write to Flah”按钮,即可看到下载进度，等待下载完成,这样服务的配置文件就下载完成了
8.在“Firmware”的"FW(Upgrade)"中点击“Open FW File”按钮选择本工程在“Objects”文件夹的“Module_samples.hex”文件   
9.在“Firmware”的"FW(Upgrade)"中点击“Write to Flah”按钮,即可看到下载进度，等待下载完成
10.下载完成后按下复位键即可看到本实验现象
11.再次下载：点击"SYD8801 Evaluation board"的Detect按钮，然后从第5步再次操作即可
备注：对于已经下载了串口程序的板子，有时候一点击“open”按钮就进入了readey状态，这时候一般是下不进程序的，这时候可以一直按着复位键然后点击“open”按钮，当进入waite状态时候放开复位键，这样就可以下载程序了

作者：北京盛源达科技有限公司
日期：2016/7/8
*/
#define  _HID_C

#include "ARMCM0.h"
#include "syd8801_timer.h"
#include "syd8801_gpio.h"
#include "syd8801_debug.h"
#include "syd8801_delay.h"
#include "config.h"
#include "ota.h"
#include "key.h"
#include "led.h"
#include "lib.h"
#include "SYD8801EVB_service.h"
#include <string.h>

#define MOTOR BIT20

//#define	ADV_INTERVAL			1638		//50ms
#define	ADV_INTERVAL			16384		//500ms

static struct gap_att_report_handle *g_report;   //GATT属性列表头,传入到协议栈中进行初始化

static uint8_t start_tx=0;
static uint8_t start_adv=1;
uint8_t timer1s_inting=0;

//test
static uint16_t cnt_1s=0;
#ifdef USER_MARCHE_STATE
MARCHE_STATE   march_state;
#endif

void BLE_SendData(uint8_t *buf, uint8_t len);
static void gpio_int_callback(void);

uint8_t ADV_DATA[] = {	0x02, // length
					  	0x01, // AD Type: Flags
					  	0x05, // LE Limited Discoverable Mode & BR/EDR Not Supported
					  	0x03, // length
					  	0x03, //AD Type: Complete list of 16-bit UUIDs 
					  	0x12, // UUID: Human Interface Device (0x1812)
					  	0x18, // UUID: Human Interface Device (0x1812)
					  	0x08, // length
					  	0x09, // AD Type: Complete local name
					  	'S',
					  	'Y',
					  	'D',
					  	'8',
					  	'8',
					  	'0',
					  	'1',
					  	};

uint16_t ADV_DATA_SZ = sizeof(ADV_DATA); 
uint8_t SCAN_DATA[]={0x00};
uint16_t SCAN_DATA_SZ = 0; 

static void ble_init(void);
static void gpio_init(void);

static uint32_t	adv_times;
static uint16_t	adv_timer_cnt=0;
#define ADV_TIME_OUT 30 //广播超时

static  POWER_CTRL_TYPE * PW_CTRL = ((POWER_CTRL_TYPE *)PWRON_BASE);
void SystemSleepSYD(enum WAKEUP_TYPE wakeup ,uint8_t timer_wakeup_en,uint8_t gpio_wakeup_en,uint32_t io)
{
	PW_CTRL->WAKE_PIN_EN  |= io ;
	PW_CTRL->TMR_WAKE_EN = timer_wakeup_en;
	PW_CTRL->PIN_WAKE_EN = gpio_wakeup_en;
	PW_CTRL->DSLP_WAKE_EN = wakeup;

	if(wakeup==SLEEP_WAKEUP)
	{
		PW_CTRL->MCU_SLEEP=1;	//执行这句话后马上进入浅睡眠模式，任何东西都不会丢失
	}
	else
	{
		PW_CTRL->DSLP_SYS = 1; // 执行这句话后马上进入深度睡眠模式，唤醒将会进入复位状态，任何东西都将会丢失
		led_close(LEDALL);
		while(1);
	}
}


//static void PowerDown()
//{
//	struct gap_wakeup_config pw_cfg;

//	GPIO_Set_Input(KEY1,KEY1,KEY1);   //key1 反相
//	PIN_CONFIG->PIN_12_POL = PIN_INPUT_IVERTED;
//	
//	if(GPIO_Pin_Read(KEY1))  return;

//	pw_cfg.wakeup_type = POWERDOWN_WAKEUP;
//	pw_cfg.timer_wakeup_en = 0;
//	pw_cfg.gpi_wakeup_en = 1;
//	pw_cfg.gpi_wakeup_cfg = KEY1;
//	WakeupConfig(&pw_cfg);

//	GPIO_IRQ_CTRL->GPIO_INT_CLR = GPIO_IRQ_CTRL->GPIO_INT;
//	SystemPowerDown();	
//}

static void setup_adv_data()
{
	struct gap_adv_params adv_params;	
	
	adv_params.type = ADV_IND;
	adv_params.channel = 0x07; // advertising channel : 37 & 38 & 39
	adv_params.interval = 0x6b; // advertising interval : 66.8ms (107 * 0.625ms)
	adv_params.timeout = 0x1e; // timeout : 30s

	SetAdvParams(&adv_params);	
				
	ADV_DATA[0x02] = 0x05; // LE Limited Discoverable Mode & BR/EDR Not Supported
			
	SetAdvData(ADV_DATA, ADV_DATA_SZ, SCAN_DATA, SCAN_DATA_SZ);
}

static void ble_gatt_read(struct gap_att_read_evt evt)
{
	if(evt.uuid == DEVICE_NAME_UUID)
	{
		uint8_t gatt_buf[]={'S', 'Y', 'D', '8', '8', '0', '1'};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == APPEARANCE_UUID)
	{
		uint8_t gatt_buf[]={0xff, 0xff};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == MANUFACTURER_NAME_STRING_UUID)
	{
		uint8_t gatt_buf[]={'S','Y','D',' ','I', 'n', 'c', '.'};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == MODEL_NUMBER_STRING_UUID)
	{
		uint8_t gatt_buf[]={'B', 'L', 'E', ' ', '1', '.', '0'};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == SERIAL_NUMBER_STRING_UUID)
	{
		uint8_t gatt_buf[]={'1','.','0','.','0'};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == HARDWARE_REVISION_STRING_UUID)
	{
		uint8_t gatt_buf[]={'2','.','0','0'};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == FIRMWARE_REVISION_STRING_UUID)
	{
		uint8_t gatt_buf[]={'3','.','0','0'};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == SOFTWARE_REVISION_STRING_UUID)
	{
		uint8_t gatt_buf[]={'4','.','0','0'};
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == PNP_ID_UUID)
	{
		uint8_t gatt_buf[]={ 0x02, 			  //		Vendor ID Source
						    0x3a,0x09,		//		USB Vendor ID
						    0x05,0x0a,		//		Product ID
						    0x02,0x00		//		Product Version
												 };
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 

		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	}
	else if(evt.uuid == OTA_Read_Write_UUID)
	{
		uint8_t sz=0;
		uint8_t rsp[sizeof(struct hci_evt)]={0};

		ota_rsp(rsp, &sz);

		SetGATTReadRsp(sz, rsp);
	}
	else if(evt.uuid == BATTERY_LEVEL_UUID)
	{
		static uint8_t j=100;
		uint8_t gatt_buf[1];
		uint8_t gatt_buf_sz = sizeof(gatt_buf); 
    gatt_buf[0]=j;
		SetGATTReadRsp(gatt_buf_sz, gatt_buf);
	  j--;
	}
}

static void ble_gatt_write(struct gap_att_write_evt evt)
{
	if(evt.uuid== UART_Write_UUID)
	{
		// rx data
	}
	else if(evt.uuid== OTA_Read_Write_UUID)
	{
		ota_cmd(evt.data, evt.sz);
	}
	
		#ifdef USER_MARCHE_STATE
		march_state.state =0x8e;
		march_state.data.att_evt.sz=evt.sz;
		memcpy(march_state.data.att_evt.data,evt.data,evt.sz);    //拷贝命令内容
		#endif
}

void BLE_SendData(uint8_t *buf, uint8_t len)
{
	if(start_tx == 1)
	{
		struct gap_att_report report;

		report.primary = UART;
		report.uuid = UART_NOTIFY_UUID;
		report.hdl = 0x0001F;
		report.value = BLE_GATT_NOTIFICATION;

		GATTDataSend(BLE_GATT_NOTIFICATION, &report, len, buf) ;
	}
}

static void timer0_adv_callback()
{	
	//led_turn(LED2); 
	//timer1s_inting=1;   //1s
	adv_timer_cnt++;
	if(adv_timer_cnt>=32768/ADV_INTERVAL){
		adv_timer_cnt=0;
		timer1s_inting=1;   //1s
	}
		
	if(start_adv)
	{
			if(adv_times)
			{
				RFWakeup();
				ble_init();
				StartAdv(); 

				adv_times--;
			}
			else
			{
//				dbg_printf("Adv Completely!\r\n\r\n");
				timer_0_disable();
				SystemSleepSYD(SLEEP_WAKEUP,1,1,KEY0);
				//io_irq_disable();
				//io_irq_enable( KEY0, &gpio_int_callback);
			}
			#ifdef USER_MARCHE_STATE
			march_state.state =0x80;
			#endif
	}
}

//static void timer0_button_callback()
//{
//	if(start_tx == 0)
//		return ;

//	if(check_button_click() == 1)
//	{
//		uint8_t data[20] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00};
//		BLE_SendData(data, 20);
//	}	
//}

void ble_evt_callback(struct gap_ble_evt *p_evt)
{
	if(p_evt->evt_code == GAP_EVT_ADV_END)
	{		
		RFSleep();
	}
	else if(p_evt->evt_code == GAP_EVT_CONNECTED)
	{
//		struct gap_update_params update;
		
		start_adv=0;
		start_tx = 0;

		#ifdef USER_MARCHE_STATE
		march_state.state =0x81;
		#endif

//		timer_0_enable(256, timer0_button_callback); // 250 * 31.25 us = 8000 us = 8 ms

		/* connection parameters */
/*		update.updateitv_min = 0x0006; //Minimum connection interval (6 * 1.25ms = 7.5 ms)
		update.updateitv_max = 0x0009; //Maximum connection interval (9 * 1.25ms = 11.25 ms)
		update.updatelatency= 0x0022;
		update.updatesvto= 0x00c8; //supervisory timeout (200 * 10 ms = 2s)
		SetConnectionUpdate(&update);	*/

		//SecurityReq(1,1);
		
		led_open(LED1);    //点亮LED1来表示已经打开了连接
	}
	else if(p_evt->evt_code == GAP_EVT_DISCONNECTED)
	{	
		start_adv=1;
		
		led_close(LED1);    //关闭LED1来表示已经断开了连接
		
		ble_init();

		gpio_init();
    
		#ifdef USER_MARCHE_STATE
		march_state.state =0x82;
		march_state.data.datau8=p_evt->evt.disconn_evt.reason;
		#endif
		adv_times = ADV_TIME_OUT*32768/ADV_INTERVAL;		//广播超时时间
		//timer_0_enable(19200, timer0_adv_callback);
		timer_0_enable(ADV_INTERVAL, timer0_adv_callback);
//		PowerDown();
	}
	else if(p_evt->evt_code == GAP_EVT_ATT_HANDLE_CONFIGURE)
	{					
		if(p_evt->evt.att_handle_config_evt.uuid == UART)
		{
			if(p_evt->evt.att_handle_config_evt.value == BLE_GATT_NOTIFICATION)
			{
				start_tx = 1;
			}
			else
			{			
				start_tx = 0;
			}
		}	
		else if(p_evt->evt.att_handle_config_evt.uuid == BATTERY_SERVICE)
		{
			if(p_evt->evt.att_handle_config_evt.value == BLE_GATT_NOTIFICATION)
			{
				struct gap_att_report report;
				uint8_t buf[] = {0x64};

				report.primary = BATTERY_SERVICE;
				report.uuid = BATTERY_LEVEL_UUID;
				report.hdl = BATTERY_LEVEL_VALUE_HANDLE;
				report.value = BLE_GATT_NOTIFICATION;

				buf[0] = 0x64;
				GATTDataSend(BLE_GATT_NOTIFICATION, &report, 0x01, buf) ;
			}
			else
			{			
			}
		}
//		#ifdef USER_MARCHE_STATE
//		march_state.state =0x83;
//		march_state.data.uuid=p_evt->evt.att_handle_config_evt.uuid;
//		#endif
	}
	else if(p_evt->evt_code == GAP_EVT_ATT_WRITE)
	{
		ble_gatt_write(p_evt->evt.att_write_evt);
//		#ifdef USER_MARCHE_STATE
//	    march_state.state =0x84;
//		march_state.data.uuid=p_evt->evt.att_write_evt.uuid;
//		#endif
	}
	else if(p_evt->evt_code == GAP_EVT_ATT_READ)
	{
		ble_gatt_read(p_evt->evt.att_read_evt);
//		#ifdef USER_MARCHE_STATE
//		march_state.state =0x85;
//		march_state.data.uuid=p_evt->evt.att_read_evt.uuid;
//		#endif
	}
	else if(p_evt->evt_code == GAP_EVT_ATT_HANDLE_CONFIRMATION)
	{
		//DBGPRINTF(("Indication Confirmation\r\n"));
//		#ifdef USER_MARCHE_STATE
//		march_state.state =0x86;
//		march_state.data.uuid=p_evt->evt.att_handle_config_evt.uuid;
//		#endif
	}
	else if(p_evt->evt_code == GAP_EVT_ENC_KEY)
	{
		#ifdef USER_MARCHE_STATE
		march_state.state =0x87;
		#endif
	}
	else if(p_evt->evt_code == GAP_EVT_ENC_START)
	{
//		#ifdef USER_MARCHE_STATE
//		march_state.state =0x88;
//		#endif
	}
  else if(p_evt->evt_code == GAP_EVT_CONNECTION_UPDATE_RSP)
	{
		if(p_evt->evt.connection_update_rsp_evt.result == CONN_PARAM_REJECTED){

		}else{
		}
	}
}


static void ble_init()
{	
	struct gap_evt_callback evt;		
	struct smp_pairing_req sec_params;	
	struct gap_wakeup_config pw_cfg;
	struct gap_ble_addr ble_addr;
		
	BleInit();
	
	MCUClockSwitch(SYSTEM_CLOCK_8M_RCOSC);
	#ifdef USER_32K_CLOCK_RCOSC
	ClockSwitch(SYSTEM_32K_CLOCK_RCOSC);
	LPOCalibration();
	#else
	ClockSwitch(SYSTEM_32K_CLOCK_XOSC);
	#endif

	GetGATTReportHandle(&g_report);

	/* security parameters */
	sec_params.io = IO_NO_INPUT_OUTPUT;
	sec_params.oob = OOB_AUTH_NOT_PRESENT;
	sec_params.flags = AUTHREQ_BONDING;
	sec_params.mitm = 0;
	sec_params.max_enc_sz = 16;
	sec_params.init_key = 0;
	sec_params.rsp_key = (SMP_KEY_MASTER_IDEN |SMP_KEY_ADDR_INFO);
	SetSecParams(&sec_params);
	
	evt.evt_mask=(GAP_EVT_CONNECTION_SLEEP|GAP_EVT_CONNECTION_INTERVAL);
	evt.p_callback=&ble_evt_callback;
	SetEvtCallback(&evt);

	/* bluetooth address */
	ble_addr.type = RANDOM_ADDRESS_TYPE;
	ble_addr.addr[0] = 0x55;
	ble_addr.addr[1] = 0x44;
	ble_addr.addr[2] = 0x33;
	ble_addr.addr[3] = 0x22;
	ble_addr.addr[4] = 0x11;
	ble_addr.addr[5] = 0xff;
	SetDevAddr(&ble_addr);

	/* Bond Manager (MAX:10) */
	SetBondManagerIndex(0x00);

	setup_adv_data();
	
	pw_cfg.wakeup_type = SLEEP_WAKEUP;
	pw_cfg.timer_wakeup_en = 1;	
	pw_cfg.gpi_wakeup_en = 1;
	pw_cfg.gpi_wakeup_cfg = 0x00001000;
	WakeupConfig(&pw_cfg);
}

static void gpio_init()
{
	PIN_CONFIG->PIN_0_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_1_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_2_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_3_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_4_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_5_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_6_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_7_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_8_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_9_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_10_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_11_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_12_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_13_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_14_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_15_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_16_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_17_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_18_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_19_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_20_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_21_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_22_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_23_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_24_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_25_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_26_SEL = PIN_SEL_GENERAL_GPIO;
	PIN_CONFIG->PIN_27_SEL = PIN_SEL_GENERAL_GPIO;
	
	PIN_CONFIG->PIN_0_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_1_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_2_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_3_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_4_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_5_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_6_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_7_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_8_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_9_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_10_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_11_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_12_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_13_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_14_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_15_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_16_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_17_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_18_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_19_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_20_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_21_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_22_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_23_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_24_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_25_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_26_PULL_UP = PIN_PULL_UP;
	PIN_CONFIG->PIN_27_PULL_UP = PIN_PULL_UP;
	key_config();
	//led_config(LEDALL);   //配置LED管脚
	//GPIO_Set_Input(KEY0|KEY1|KEY2|KEY3,KEY0|KEY1|KEY2|KEY3,PIN_INPUT_IVERTED);
	//GPIO_Pin_Set(MOTOR);   //默认关灯
}
#ifdef USER_MARCHE_STATE
void march_state_process(void){
		if(march_state.state & 0x80){
			march_state.state &=0x7f;
			led_turn(LED1);     // 翻转LED1  指示蓝牙状态发生了变化
			switch(march_state.state){
				case 1:
					#ifdef _DEBUG_
					DBGPRINTF(("GAP_EVT_CONNECTED"));
					#endif
					break;
				case 2:
				    #ifdef _DEBUG_
					DBGPRINTF(("GAP_EVT_DISCONNECTED(%02x)\r\n",march_state.data.datau8));
					#endif
					break;
//				case 3:
//				    #ifdef _DEBUG_
//					DBGPRINTF(("GAP_EVT_ATT_HANDLE_CONFIGURE uuid:(%02x)\r\n",march_state.data.uuid));
//					#endif
//					break;
//				case 4:
//				    #ifdef _DEBUG_
//					DBGPRINTF(("GAP_EVT_ATT_WRITE uuid:(%02x)\r\n",march_state.data.uuid));
//					#endif
//					break;
//				case 5:
//				    #ifdef _DEBUG_
//					DBGPRINTF(("GAP_EVT_ATT_READ uuid:(%02x)\r\n",march_state.data.uuid));
//					#endif
//					break;
//				case 6:
//				    #ifdef _DEBUG_
//					DBGPRINTF(("GAP_EVT_ATT_HANDLE_CONFIRMATION uuid:(%02x)\r\n",march_state.data.uuid));
//					#endif
//					break;
				case 7:
				    #ifdef _DEBUG_
					DBGPRINTF(("ancs_find_ervice\r\n"));
					#endif
					break;
//				case 8:
//				    #ifdef _DEBUG_
//					DBGPRINTF(("GAP_EVT_ENC_START\r\n"));
//					#endif
//					break;
				case 9:
				    #ifdef _DEBUG_
					DBGPRINTF(("update rsp:%04x\r\n",march_state.data.datau8));
					#endif
					break;
				case 10:
				    #ifdef _DEBUG_
						dbg_printf("ancs end\r\n");
					#endif
					break;
				case 11:
				    #ifdef _DEBUG_
					dbg_printf("not ancs\r\n");
					#endif
					break;
				case 12:
				    #ifdef _DEBUG_
					dbg_printf("not band\r\n");
				  #endif
					break;
				case 13:
				    #ifdef _DEBUG_
					dbg_printf("Security start\r\n");
				  #endif
					break;
				case 14:
				    #ifdef _DEBUG_
					dbg_hexdump("msg:\r\n",march_state.data.att_evt.data,march_state.data.att_evt.sz);
				   // dbg_printf("c:%x l:%x\r\n",march_state.data.att_evt.data[0],march_state.data.att_evt.data[1]);
					#endif
					break;
				default:
					break;
			}
		}
}
#endif
/*
外部中断回调函数 - 上升沿触发中断
*/
static void gpio_int_callback(void)
{
	uint32_t flag = GPIO_IRQ_CTRL->GPIO_INT;
	if(flag & KEY0)
	{
			adv_times = ADV_TIME_OUT*32768/ADV_INTERVAL;		//广播超时时间
	    timer_0_enable(ADV_INTERVAL, timer0_adv_callback);
			//io_irq_enable(KEY0 | KEY1 | KEY2 | KEY3, &gpio_int_callback);
		
		BLE_SendData("KEY0\r\n", 6);
		//SystemReset();
	}
	if(flag & KEY1)
	{
		BLE_SendData("KEY1\r\n", 6);
	}
	if(flag & KEY2)
	{
		BLE_SendData("KEY2\r\n", 6);
	}
	if(flag & KEY3)
	{
		BLE_SendData("KEY3\r\n", 6);
	}
}

int main()
{	
//	uint8_t ble_data[20] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00};;
	
	__disable_irq();
	
	gpio_init();
	
	#ifdef _DEBUG_
	dbg_init();
	dbg_printf("SYD Inc.\r\n");
	#endif

	ble_init();

	StartAdv();
	
	adv_times = ADV_TIME_OUT*32768/ADV_INTERVAL;		//广播超时时间
	timer_0_enable(ADV_INTERVAL, timer0_adv_callback);
	
	io_irq_enable(KEY0 | KEY1 | KEY2 | KEY3, &gpio_int_callback);
	__enable_irq();	
	
	timer1s_inting=0;
	while(1)
	{		
		ble_sched_execute();
		
		if(timer1s_inting){
			timer1s_inting=0;
			//led_turn(LED0);     // 翻转LED0  指示程序运行
			
			cnt_1s++;
			if(cnt_1s>=180)//最长三分钟校准一次
			{
					cnt_1s=0;
					#ifdef USER_32K_CLOCK_RCOSC
					LPOCalibration();   //这是内部RC32k晶振的校准函数	经过该函数后定时器能够得到一个比较准确的值
					//timer_calender_handle();  内部晶振即使校准依旧有一个固定的偏差
					#endif		
			}  
			
		}
				
		#ifdef USER_MARCHE_STATE
    march_state_process();
		#endif
		
		if(!adv_times)  led_close(LEDALL);
		SystemSleep();
	}	
}

