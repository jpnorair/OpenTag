#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_


// Hardware and Software versions
#define HW_TYPE					4	// RI-ACC-ADR2
#define HW_VERSION				1
#define GENERAL_SW_VERSION	    1
#define UNDER_SW_VERSION		0

//-------------------------------------------------------------------------------------------------------------

#define   		TRUE		0
#define 		FALSE		1

#define			LOW 		0
#define 		HIGH		1

#define			HALT		0
#define 		CONTINUE	1

#define			USEC		0
#define 		MSEC		1

//-------------------------------------------------------------------------------------------------------------
// Defines for first command byte
#define SETUP_MODE 			1
#define NORMAL_MODE	 		0
#define PPM					1
#define PWM					0
#define LF					1
#define UHF					0
#define x26					1
#define x22					0

// Standard modes
#define IMMO				0
#define PE_WP				1
#define PE_noWP_22	  		2
#define PE_noWP_26	  		3
#define BBUP				4

#define ONE_BYTE				0
#define TWO_BYTE				1
	
#define  NO_USB_DATA           	0
#define  USB_DATA_STARTED   	1
#define  USB_DATA_RECEIVED     	2


//-------------------------------------------------------------------------------------------------------------

void Init(void);

//void UplinkLF(void);
//void DownlinkLF(void);

void UplinkUSB(void);
int  DownlinkUSB(void);
void NormalModeUSB(void);

void LF_Init(void);

void Timer_Init(void);
void TimerAWait(unsigned int uiDurationUSEC, unsigned char ucStop);
void Timer_Wait(unsigned int uiDurationUSEC, unsigned char ucStop);
void SetOutput(unsigned char volatile *usPortOut, unsigned char ucBit, unsigned char ucState, unsigned int uiDuration, unsigned char ucDurationUnit, unsigned char ucHaltProgram);
//-------------------------------------------------------------------------------------------------------------

// Defines for max. data sizes
//#define RX_BUF_SIZE   		60	
#define RX_BUF_SIZE   		30		// 60 before
#define EXTRA_DATA_SIZE 	4		// data bytes for extra functionality (either WP or BBUP)
#define TX_DATA_SIZE		18	   	// 38 before TX data bytes

//-------------------------------------------------------------------------------------------------------------

// Transponder Access Command
#define		TAC_COMMAND_BYTE		  		0x00

// Enhanced Commands
#define		MSP430Read_CU_Status		  	0xB4
#define		MSP430_PowerDownMode		  	0xB8
#define     MSP430_CU_GetData             	0xB0
#define     MSP430_CU_SendData           	0xB1
#define		MSP430_CLKA_OFF      		  	0x9C
#define		MSP430_CLKA_ON      		  	0x9D
#define		MSP430_OSC_OFF      		  	0x94
#define		MSP430_OSC_134_ON   		  	0x95

#define		MAC_Cmd    		          		0x40
#define		MAC_READ    		          	0x00
#define		MAC_PROGRAM    		          	0x40
#define		MAC_BYTE0    		          	0x01
#define		MAC_BYTE1    		          	0x02
#define		MAC_BYTE2    		          	0x04
#define		MAC_BYTE3    		          	0x08
#define		MAC_BYTE4    		          	0x10


#define		CU_Encryption  		          	0x84
#define		CU_CRC_Calculation	          	0x80
#define		CU_Oscillator_Off		  		0x94	
#define		CU_Oscillator_134KHz		  	0x95
#define		CU_Oscillator_134KHz_DIV4	  	0x96
#define		CU_CLKA_Off			  			0x9C
#define		CU_CLKA_On			  			0x9D

//-------------------------------------------------------------------------------------------------------------
#define     PCU_PUSH_BUTTON     1
#define     PCU_MSP_ACCESS      2
//-------------------------------------------------------------------------------------------------------------

#define		Page1		0x04		// SELECTIVE ADDRESS
#define		Page2	  	0x08		// USER DATA 1
#define		Page3 		0x0C		// UNIQUE IDENTIFICATION
#define		Page4		0x10		// ENCRYPTION KEY
#define		Page6	  	0x18		// ISSUER KEY
#define		Page7 		0x1C		// MUTUAL AUTHENTICATION KEY
#define		Page8		0x20		// USER DATA 2
#define		Page9  		0x24		// USER DATA 3
#define		Page10		0x28		// USER DATA 4
#define		Page11		0x2C		// USER DATA 5   only accessable with muthual key and encryption
#define		Page12		0x30		// USER DATA 6   only accessable with muthual key and encryption
#define		Page13		0x34		// USER DATA 7
#define		Page14		0x38		// USER DATA 8
#define		Page15		0x3C		// USER DATA 9
#define		Page16		0x40		// TRANSMISSION IDENT. CODE
#define		Page17		0x44		// ROLL CODE
#define		Page18		0x48		// PUSH BUTTON MASK
#define		Page28		0x70		// ACCESS MODE BYTE
#define		Page29		0x74		// INCREMENT COUNTER
#define		Page30		0x78		// CONFIGURATION

#define		Page40		0xA0		// User data 10
#define		Page41		0xA4		// User data 11
#define		Page42		0xA8		// User data 12
#define		Page43		0xAC		// User data 13
#define		Page44		0xB0		// User data 14
#define		Page45		0xB4		// User data 15
#define		Page46		0xB8		// User data 16
#define		Page47		0xBC		// User data 17
#define		Page48		0xC0		// User data 18
#define		Page49		0xC4		// User data 19
#define		Page50		0xC8		// User data 20
#define		Page51		0xCC		// User data 21
#define		Page52		0xD0		// User data 22
#define		Page53		0xD4		// User data 23
#define		Page54		0xD8		// User data 24
#define		Page55		0xDC		// User data 25


//-------------------------------------------------------------------------------------------------------------

struct
_st_TRP_CONTENT
{
  unsigned char ucPCU_Mode;
  unsigned char Command;
  unsigned char SerialNumber[4];
  unsigned char KeyNumber;	      // equal to User data 1
  unsigned char SelectiveAddress;
  unsigned char PushButtonMask;
  unsigned char RollingCode[5];
};



//-------------------------------------------------------------------------------------------------------------
/* Burst Length Coding times, in USEC. Also incorrectly called Pulse Position Modulation (PPM). */
struct stBLC
{
	unsigned short usToff;					// off time
	unsigned short usTonL;					// on time of low bit
	unsigned short usTonH;					// on time of high bit
	unsigned short usTonSC;					// on time of stop condition (end of TX data)
};


/* Pulse Width Modulation (PWM) times */
struct stPWM
{
	unsigned short usToffH;					// off time of high bit
	unsigned short usTonH;					// on time of high bit
	unsigned short usToffL;					// off time of low bit
	unsigned short usTonL;					// on time of low bit
};

struct stTimerCtrl
{
  unsigned int  bTriggerRead     : 1;
  unsigned int  bContinue        : 1;
  unsigned int  bTimerAFlag      : 1;
  unsigned int  bWaitForTimeout  : 1;
  unsigned int  bTimerBFlag      : 1;
  unsigned int  bTimerBFlag5     : 1;
  unsigned int  bCCR0_TimeOut    : 1;
  unsigned int  bCCR1_TimeOut    : 1;
  unsigned int  bCCR2_TimeOut	 : 1;
  unsigned int  b2USB_DATA		 : 2;

  unsigned int  uiCCR0_Count_1ms;
  //unsigned char ucCCR3_Count_1ms;
};

struct stASCIIbuffer
{
	unsigned char ucData[2*RX_BUF_SIZE];	// because each byte takes two ASCII characters
	unsigned char ucLength;					// current length of array
	unsigned char ucLengthField;			// The packet length indicated in the Length field.	
	unsigned char ucLengthActual;			// The actual packet length (not the one in the Length field)
	unsigned char ucRXbytes;				// No. of RX bytes requested
};

struct stLFpacket				
{
	// The main data to transmit, apart from overhead (such as Start, Length, LRC)
	unsigned short usPowerBurst[4];			// up to 4 Power Bursts, each 8 or 16 bit long
	
	unsigned char ucEXTRAbytes;					// No. of data BYTES for extra functionality (either WP or BBUP)
	unsigned char ucTXbytes;						// No. of data BYTES for TX
	unsigned char ucRXbytes;						// No. of RX bytes demanded
	
	unsigned char ucEXTRAdata[EXTRA_DATA_SIZE];	// data for extra functionality (either WP or BBUP)
	unsigned char ucTXdata[TX_DATA_SIZE];				// TX data
};


union unCMD
{
	// First Command Byte (CMD) as a bit-field
	unsigned short usValue;
	struct
	{
		// The eight bits, starting with bit0.
		unsigned short Setup_Normal 	: 1;		// true (1): Setup Mode				false(0): Normal Mode
		unsigned short PPM_PWM			: 1;		// true (1): PPM					false(0): PWM
		unsigned short LF_UHF			: 1;		// true (1): LF Response			false(0): UHF Response
		unsigned short x26_x22			: 1;		// true (1): No WP for 37126		false(0): No WP for 37122
		unsigned short Select0			: 1;		// select mode low bit
		unsigned short Select1			: 1;		// select mode high bit
		unsigned short TwoPB_OnePB		: 1;		// true (1): Two Byte Power Burst	false(0): One Byte Power Burst
		unsigned short ByteFollows   	: 1;		// true (1): Command Byte follows	false(0): Command finished
		
		unsigned char  ucMode;						// the mode chosen, derived from bits 4 and 5 (Select0, Select1)
	} field;
};

union unCMD2
{
	// Second Command Byte (CMD2) as a bit-field
	unsigned short usValue;
	struct
	{
		unsigned short FDXASK_HDXFSK	: 1;		// true (1): FDX / ASK				false(0): HDX / FSK
		unsigned short Biphase_Man		: 1;		// true (1): Biphase ASK			false(0): Manchester ASK
		unsigned short ASK4k_2k			: 1;		// true (1): 4kbit/s ASK			false(0): 2kbit/s AKS
		unsigned short Anticollision 	: 1;		// true (1): Anticollision			false(0): No Anticollision
		unsigned short KeepTXon  		: 1;		// true (1): Keep TX on after PB2	false(0): Normal : Switch TX off
		unsigned short Bit5				: 1;													
		unsigned short Bit6    			: 1;
		unsigned short ByteFollows   	: 1;		// true (1): Command Byte follows	false(0): Command finished
	} field;
};

#endif /*FUNCTIONS_H_*/
