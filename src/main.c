
#include <string.h>

#include "netx_io_areas.h"
#include "rdy_run.h"
#include "systime.h"

#include "uart.h"
#include "uprintf.h"
#include "version.h"

#include "console_io.h"
#include "serial_vectors.h"

/*-------------------------------------------------------------------------*/

#if ASIC_TYP==10
/* NXHX10-ETM */
static const UART_CONFIGURATION_T tUartCfg =
{
	.uc_rx_mmio = 20U,
	.uc_tx_mmio = 21U,
	.uc_rts_mmio = 0xffU,
	.uc_cts_mmio = 0xffU,
	.us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
#elif ASIC_TYP==56
/* NXHX51-ETM */
static const UART_CONFIGURATION_T tUartCfg =
{
	.uc_rx_mmio = 34U,
	.uc_tx_mmio = 35U,
	.uc_rts_mmio = 0xffU,
	.uc_cts_mmio = 0xffU,
	.us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
#elif ASIC_TYP==4000
static const UART_CONFIGURATION_T tUartCfg =
{
	.uc_rx_mmio = 26U,
	.uc_tx_mmio = 27U,
	.uc_rts_mmio = 0xffU,
	.uc_cts_mmio = 0xffU,
	.us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
#endif


#define IO_UART_UNIT 0

static unsigned char io_uart_get(void)
{
	return (unsigned char)uart_get(IO_UART_UNIT);
}


static void io_uart_put(unsigned int uiChar)
{
	uart_put(IO_UART_UNIT, (unsigned char)uiChar);
}


static unsigned int io_uart_peek(void)
{
	return uart_peek(IO_UART_UNIT);
}


static void io_uart_flush(void)
{
	uart_flush(IO_UART_UNIT);
}


static const SERIAL_COMM_UI_FN_T tSerialVectors_Uart =
{
	.fn =
	{
		.fnGet = io_uart_get,
		.fnPut = io_uart_put,
		.fnPeek = io_uart_peek,
		.fnFlush = io_uart_flush
	}
};


SERIAL_COMM_UI_FN_T tSerialVectors;

/*--------------------------------------------------------------------------*/

#define bits2ul(b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,ba,bb,bc,bd,be,bf) ((b0)|((b1)<<1U)|((b2)<<2U)|((b3)<<3U)|((b4)<<4U)|((b5)<<5U)|((b6)<<6U)|((b7)<<7U)|((b8)<<8U)|((b9)<<9U)|((ba)<<10U)|((bb)<<11U)|((bc)<<12U)|((bd)<<13U)|((be)<<14U)|((bf)<<15U))
static const unsigned short ausPortControlMask[19] =
{
	/*      0 1 2 3 4 5 6 7 8 9 a b c d e f */
	bits2ul(1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0),  /* PORTCONTROL_P0_* */
	bits2ul(1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0),  /* PORTCONTROL_P1_* */
	bits2ul(1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0),  /* PORTCONTROL_P2_* */
	bits2ul(0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P3_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P4_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P5_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P6_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P7_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P8_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P9_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0),  /* PORTCONTROL_P10_* */
	bits2ul(1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0),  /* PORTCONTROL_P11_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1),  /* PORTCONTROL_P12_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0),  /* PORTCONTROL_P13_* */
	bits2ul(1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0),  /* PORTCONTROL_P14_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0),  /* PORTCONTROL_P15_* */
	bits2ul(1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0),  /* PORTCONTROL_P16_* */
	bits2ul(1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0),  /* PORTCONTROL_P17_* */
	bits2ul(1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0)   /* PORTCONTROL_P18_* */
};


#define MSK_PORTCONTROL_CONFIGURATION_UDC   0x0003
#define SRT_PORTCONTROL_CONFIGURATION_UDC   0
#define MSK_PORTCONTROL_CONFIGURATION_DRV   0x0030
#define SRT_PORTCONTROL_CONFIGURATION_DRV   4
#define MSK_PORTCONTROL_CONFIGURATION_CTL   0x0f00
#define SRT_PORTCONTROL_CONFIGURATION_CTL   8
#define MSK_PORTCONTROL_CONFIGURATION_SEL   0x7000
#define SRT_PORTCONTROL_CONFIGURATION_SEL   12

static const char acUdc[4] =
{
	'n',
	'U',
	'N',
	'D'
};

static const unsigned int auiDrv[4] =
{
	4,
	6,
	8,
	12
};

static void show_portcontrol(void)
{
	unsigned int uiPCnt;
	unsigned int uiSCnt;
	volatile unsigned long *pulPortControl;
	unsigned long ulBits;
	unsigned long ulValue;
	unsigned long ulPortControlUdc;
	unsigned long ulPortControlDrv;
	unsigned long ulPortControlCtl;
	unsigned long ulPortControlSel;


	pulPortControl = (volatile unsigned long*)HOSTADDR(PORTCONTROL);

	uprintf("PORTCONTROL:\n");
	uprintf("    00    01    02    03    04    05    06    07    08    09    0a    0b    0c    0d    0e    0f\n");

	/* Loop over all port control values. */
	uiPCnt = 0;
	while( uiPCnt<(sizeof(ausPortControlMask)/sizeof(ausPortControlMask[0])) )
	{
		/* Loop over the bits in the port unit. */
		ulBits = ausPortControlMask[uiPCnt];
		uiSCnt = 0;

		uprintf("%02x: ", uiPCnt);
		while( uiSCnt<16 )
		{
			/* Is this entry valid? */
			if( (ulBits & (1U<<uiSCnt))==0U )
			{
				/* The entry is not valid. */
				uprintf("----- ");
			}
			else
			{
				ulValue = pulPortControl[uiPCnt*16U + uiSCnt];
				ulPortControlUdc = (ulValue & MSK_PORTCONTROL_CONFIGURATION_UDC) >> SRT_PORTCONTROL_CONFIGURATION_UDC;
				ulPortControlDrv = (ulValue & MSK_PORTCONTROL_CONFIGURATION_DRV) >> SRT_PORTCONTROL_CONFIGURATION_DRV;
				ulPortControlCtl = (ulValue & MSK_PORTCONTROL_CONFIGURATION_CTL) >> SRT_PORTCONTROL_CONFIGURATION_CTL;
				ulPortControlSel = (ulValue & MSK_PORTCONTROL_CONFIGURATION_SEL) >> SRT_PORTCONTROL_CONFIGURATION_SEL;

				uprintf("%c%02d%x%d ", acUdc[ulPortControlUdc], auiDrv[ulPortControlDrv], ulPortControlCtl, ulPortControlSel);
			}

			uiSCnt++;
		}

		uprintf("\n");

		uiPCnt++;
	}

	uprintf("\n");
}


/*--------------------------------------------------------------------------*/


static const char *apcMmioFunctions[] =
{
	"GPIO0",                  /* 0x00 */
	"GPIO1",                  /* 0x01 */
	"GPIO2",                  /* 0x02 */
	"GPIO3",                  /* 0x03 */
	"GPIO4",                  /* 0x04 */
	"GPIO5",                  /* 0x05 */
	"GPIO6",                  /* 0x06 */
	"GPIO7",                  /* 0x07 */
	"GPIO8",                  /* 0x08 */
	"GPIO9",                  /* 0x09 */
	"GPIO10",                 /* 0x0a */
	"GPIO11",                 /* 0x0b */
	"GPIO12",                 /* 0x0c */
	"GPIO13",                 /* 0x0d */
	"GPIO14",                 /* 0x0e */
	"GPIO15",                 /* 0x0f */
	"IOLINK0_IN",             /* 0x10 */
	"IOLINK0_OUT",            /* 0x11 */
	"IOLINK0_OE",             /* 0x12 */
	"IOLINK0_WAKEUP",         /* 0x13 */
	"IOLINK1_IN",             /* 0x14 */
	"IOLINK1_OUT",            /* 0x15 */
	"IOLINK1_OE",             /* 0x16 */
	"IOLINK1_WAKEUP",         /* 0x17 */
	"IOLINK2_IN",             /* 0x18 */
	"IOLINK2_OUT",            /* 0x19 */
	"IOLINK2_OE",             /* 0x1a */
	"IOLINK2_WAKEUP",         /* 0x1b */
	"IOLINK3_IN",             /* 0x1c */
	"IOLINK3_OUT",            /* 0x1d */
	"IOLINK3_OE",             /* 0x1e */
	"IOLINK3_WAKEUP",         /* 0x1f */
	"IOLINK4_IN",             /* 0x20 */
	"IOLINK4_OUT",            /* 0x21 */
	"IOLINK4_OE",             /* 0x22 */
	"IOLINK4_WAKEUP",         /* 0x23 */
	"IOLINK5_IN",             /* 0x24 */
	"IOLINK5_OUT",            /* 0x25 */
	"IOLINK5_OE",             /* 0x26 */
	"IOLINK5_WAKEUP",         /* 0x27 */
	"IOLINK6_IN",             /* 0x28 */
	"IOLINK6_OUT",            /* 0x29 */
	"IOLINK6_OE",             /* 0x2a */
	"IOLINK6_WAKEUP",         /* 0x2b */
	"IOLINK7_IN",             /* 0x2c */
	"IOLINK7_OUT",            /* 0x2d */
	"IOLINK7_OE",             /* 0x2e */
	"IOLINK7_WAKEUP",         /* 0x2f */
	"PIO0",                   /* 0x30 */
	"PIO1",                   /* 0x31 */
	"PIO2",                   /* 0x32 */
	"PIO3",                   /* 0x33 */
	"PIO4",                   /* 0x34 */
	"PIO5",                   /* 0x35 */
	"PIO6",                   /* 0x36 */
	"PIO7",                   /* 0x37 */
	"WDG_ACTIVE",             /* 0x38 */
	"EN_IN",                  /* 0x39 */
	"SPI0_CLK",               /* 0x3a */
	"SPI0_CS0N",              /* 0x3b */
	"SPI0_CS1N",              /* 0x3c */
	"SPI0_CS2N",              /* 0x3d */
	"SPI0_MISO",              /* 0x3e */
	"SPI0_MOSI",              /* 0x3f */
	"SPI1_CLK",               /* 0x40 */
	"SPI1_CS0N",              /* 0x41 */
	"SPI1_CS1N",              /* 0x42 */
	"SPI1_CS2N",              /* 0x43 */
	"SPI1_MISO",              /* 0x44 */
	"SPI1_MOSI",              /* 0x45 */
	"I2C0_SCL",               /* 0x46 */
	"I2C0_SDA",               /* 0x47 */
	"I2C1_SCL",               /* 0x48 */
	"I2C1_SDA",               /* 0x49 */
	"I2C2_SCL",               /* 0x4a */
	"I2C2_SDA",               /* 0x4b */
	"UART0_CTSN",             /* 0x4c */
	"UART0_RTSN",             /* 0x4d */
	"UART0_RXD",              /* 0x4e */
	"UART0_TXD",              /* 0x4f */
	"UART1_CTSN",             /* 0x50 */
	"UART1_RTSN",             /* 0x51 */
	"UART1_RXD",              /* 0x52 */
	"UART1_TXD",              /* 0x53 */
	"UART2_CTSN",             /* 0x54 */
	"UART2_RTSN",             /* 0x55 */
	"UART2_RXD",              /* 0x56 */
	"UART2_TXD",              /* 0x57 */
	"UART_XPIC3_CTSN",        /* 0x58 */
	"UART_XPIC3_RTSN",        /* 0x59 */
	"UART_XPIC3_RXD",         /* 0x5a */
	"UART_XPIC3_TXD",         /* 0x5b */
	"CAN_RX",                 /* 0x5c */
	"CAN_TX",                 /* 0x5d */
	"PWM_FAILURE_N",          /* 0x5e */
	"POS_ENC0_A",             /* 0x5f */
	"POS_ENC0_B",             /* 0x60 */
	"POS_ENC0_N",             /* 0x61 */
	"POS_ENC1_A",             /* 0x62 */
	"POS_ENC1_B",             /* 0x63 */
	"POS_ENC1_N",             /* 0x64 */
	"POS_MP0",                /* 0x65 */
	"POS_MP1",                /* 0x66 */
	"XC0_SAMPLE0",            /* 0x67 */
	"XC0_SAMPLE1",            /* 0x68 */
	"XC0_TRIGGER0",           /* 0x69 */
	"XC0_TRIGGER1",           /* 0x6a */
	"XC1_SAMPLE0",            /* 0x6b */
	"XC1_SAMPLE1",            /* 0x6c */
	"XC1_TRIGGER0",           /* 0x6d */
	"XC1_TRIGGER1",           /* 0x6e */
	"MII_MDC",                /* 0x6f */
	"MII_MDIO",               /* 0x70 */
	"XM10_MII_MDC",           /* 0x71 */
	"XM10_MII_MDIO",          /* 0x72 */
	"XM11_MII_MDC",           /* 0x73 */
	"XM11_MII_MDIO",          /* 0x74 */
	"XM10_MII_IRQ",           /* 0x75 */
	"XM11_MII_IRQ",           /* 0x76 */
	"PHY0_LED_PHY_CTRL_LNK",  /* 0x77 */
	"PHY0_LED_PHY_CTRL_ACT",  /* 0x78 */
	"PHY0_LED_SPD",           /* 0x79 */
	"PHY0_LED_DPX",           /* 0x7a */
	"PHY1_LED_PHY_CTRL_LNK",  /* 0x7b */
	"PHY1_LED_PHY_CTRL_ACT",  /* 0x7c */
	"PHY1_LED_SPD",           /* 0x7d */
	"PHY1_LED_DPX",           /* 0x7e */
	"PHY2_LED_PHY_CTRL_LNK",  /* 0x7f */
	"PHY2_LED_PHY_CTRL_ACT",  /* 0x80 */
	"PHY2_LED_LNK",           /* 0x81 */
	"PHY2_LED_ACT",           /* 0x82 */
	"PHY2_LED_SPD",           /* 0x83 */
	"PHY2_LED_DPX",           /* 0x84 */
	"PHY3_LED_PHY_CTRL_LNK",  /* 0x85 */
	"PHY3_LED_PHY_CTRL_ACT",  /* 0x86 */
	"PHY3_LED_LNK",           /* 0x87 */
	"PHY3_LED_ACT",           /* 0x88 */
	"PHY3_LED_SPD",           /* 0x89 */
	"PHY3_LED_DPX",           /* 0x8a */
	"XM00_IO0",               /* 0x8b */
	"XM00_IO1",               /* 0x8c */
	"XM00_IO2",               /* 0x8d */
	"XM00_IO3",               /* 0x8e */
	"XM00_IO4",               /* 0x8f */
	"XM00_IO5",               /* 0x90 */
	"XM00_RX",                /* 0x91 */
	"XM00_TX_OUT",            /* 0x92 */
	"XM01_IO0",               /* 0x93 */
	"XM01_IO1",               /* 0x94 */
	"XM01_IO2",               /* 0x95 */
	"XM01_IO3",               /* 0x96 */
	"XM01_IO4",               /* 0x97 */
	"XM01_IO5",               /* 0x98 */
	"XM01_RX",                /* 0x99 */
	"XM01_TX_OUT",            /* 0x9a */
	"XM10_IO0",               /* 0x9b */
	"XM10_IO1",               /* 0x9c */
	"XM10_IO2",               /* 0x9d */
	"XM10_IO3",               /* 0x9e */
	"XM10_IO4",               /* 0x9f */
	"XM10_IO5",               /* 0xa0 */
	"XM10_RX",                /* 0xa1 */
	"XM10_TX_OUT",            /* 0xa2 */
	"XM11_IO0",               /* 0xa3 */
	"XM11_IO1",               /* 0xa4 */
	"XM11_IO2",               /* 0xa5 */
	"XM11_IO3",               /* 0xa6 */
	"XM11_IO4",               /* 0xa7 */
	"XM11_IO5",               /* 0xa8 */
	"XM11_RX",                /* 0xa9 */
	"XM11_TX_OUT"             /* 0xaa */
};


static void show_mmios(void)
{
	HOSTDEF(ptMmioCtrlArea);
	unsigned int uiCnt;
	unsigned long ulValue;
	unsigned long ulIndex;
	unsigned long ulInvertOut;
	unsigned long ulInvertIn;
	const char *pcFunction;
	unsigned int uiLen;
	unsigned int uiNlCnt;


	uprintf("MMIOs:\n");
	uprintf("      0                       1                       2                       3                       4");

	/* Loop over all MMIOs. */
	uiCnt = 0;
	uiNlCnt = 0;
	while( uiCnt<(sizeof(ptMmioCtrlArea->aulMmio_cfg)/sizeof(ptMmioCtrlArea->aulMmio_cfg[0])) )
	{
		if( uiNlCnt==0U )
		{
			uprintf("\n%02x: ", uiCnt);
		}

		/* Get the MMIO control. */
		ulValue = ptMmioCtrlArea->aulMmio_cfg[uiCnt];
		ulIndex     = (ulValue & MSK_NX4000_mmio0_cfg_mmio_sel) >> SRT_NX4000_mmio0_cfg_mmio_sel;
		ulInvertOut = (ulValue & MSK_NX4000_mmio0_cfg_mmio_out_inv) >> SRT_NX4000_mmio0_cfg_mmio_out_inv;
		ulInvertIn  = (ulValue & MSK_NX4000_mmio0_cfg_mmio_in_inv) >> SRT_NX4000_mmio0_cfg_mmio_in_inv;

		/* Get the pointer to the function text. */
		if( ulIndex==0xff )
		{
			pcFunction = "PIO";
		}
		else if( ulIndex<(sizeof(apcMmioFunctions)/sizeof(apcMmioFunctions[0])) )
		{
			pcFunction = apcMmioFunctions[ulIndex];
		}
		else
		{
			pcFunction = "???";
		}

		/* Get the length of the text. */
		uiLen = strlen(pcFunction);

		uprintf("%c%c%s", (ulInvertOut==0?' ':'#'), (ulInvertIn==0?' ':'#'), pcFunction);
		/* Fill up the text to 22 chars. */
		while( uiLen<22 )
		{
			uprintf(" ");
			uiLen++;
		}

		uiCnt++;

		/* Insert a line feed all 5 entries. */
		uiNlCnt++;
		if( uiNlCnt>=5U )
		{
			uiNlCnt = 0;
		}
	}

	uprintf("\n");
}


/*--------------------------------------------------------------------------*/


static void show_ddr(void)
{
	HOSTDEF(ptDdrCtrlArea);
	HOSTDEF(ptDdrPhyArea);
	unsigned int uiCnt;


	uprintf("DDR CTRL:\n");
	uprintf("    00       01       02       03       04       05       06       07       08       09       0a       0b       0c       0d       0e       0f");
	for(uiCnt=0; uiCnt<(sizeof(ptDdrCtrlArea->aulDDR_CTRL_CTL)/sizeof(ptDdrCtrlArea->aulDDR_CTRL_CTL[0])); uiCnt++)
	{
		if( (uiCnt&15)==0 )
		{
			uprintf("\n%02x: ");
		}

		uprintf("%08x ", ptDdrCtrlArea->aulDDR_CTRL_CTL[uiCnt]);
	}

	uprintf("\n");

	uprintf("DDR PHY:\n");
	uprintf("FUNCCTRL: %08x    DLLCTRL: %08x    ZQCALCTRL: %08x    ZQODTCTRL: %08x\n", ptDdrPhyArea->ulDDR_PHY_FUNCCTRL, ptDdrPhyArea->ulDDR_PHY_DLLCTRL, ptDdrPhyArea->ulDDR_PHY_ZQCALCTRL, ptDdrPhyArea->ulDDR_PHY_ZQODTCTRL);
	uprintf("RDCTRL:   %08x    RDTMG:   %08x    FIFOINIT:  %08x    OUTCTRL:   %08x\n", ptDdrPhyArea->ulDDR_PHY_RDCTRL, ptDdrPhyArea->ulDDR_PHY_RDTMG, ptDdrPhyArea->ulDDR_PHY_FIFOINIT, ptDdrPhyArea->ulDDR_PHY_OUTCTRL);
	uprintf("WLCTRL1:  %08x    WLCTRL2: %08x    MASKSDLY1: %08x    MASKSDLY2: %08x\n", ptDdrPhyArea->ulDDR_PHY_WLCTRL1, ptDdrPhyArea->ulDDR_PHY_WLCTRL2, ptDdrPhyArea->ulDDR_PHY_MASKSDLY1, ptDdrPhyArea->ulDDR_PHY_MASKSDLY2);
	uprintf("ZQCODE:   %08x\n", ptDdrPhyArea->ulDDR_PHY_ZQCODE);
}


/*--------------------------------------------------------------------------*/


static void show_power_and_clocks(void)
{
	HOSTDEF(ptRAPSysctrlArea);


	uprintf("SYSCTRL:\n");
	uprintf("NOCPWRCTRL: %08x\n", ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRCTRL);
	uprintf("NOCPWRMASK: %08x\n", ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRMASK);
	uprintf("NOCPWRSTAT: %08x\n", ptRAPSysctrlArea->ulRAP_SYSCTRL_NOCPWRSTAT);
	uprintf("CLKCFG: %08x\n", ptRAPSysctrlArea->ulRAP_SYSCTRL_CLKCFG);
}


/*--------------------------------------------------------------------------*/



void show_cfg_init(void);
void show_cfg_init(void)
{
	systime_init();
	uart_init(IO_UART_UNIT, &tUartCfg);

	/* Set the serial vectors. */
	memcpy(&tSerialVectors, &tSerialVectors_Uart, sizeof(SERIAL_COMM_UI_FN_T));

	uprintf("\f. *** Show the active hardware configuration. ***\n");
	uprintf("Written by cthelen@hilscher.com in 2016.\n");
	uprintf("V" VERSION_ALL "\n\n");

	rdy_run_setLEDs(RDYRUN_OFF);
}

#ifdef SHOW_HWCONFIG_CONSOLE
void show_cfg_main(void);
void show_cfg_main(void)
{
	const char *pcInput;
	int iExit;

	show_cfg_init();
	console_io_init();
	
	/* Prompt the user for a choice. */
	iExit = 0;
	do
	{
		uprintf("p: show the portcontrol settings\n");
		uprintf("m: show the MMIOs\n");
		uprintf("r: show the RAP ctrl power and clocks\n");
		uprintf("d: show the DDR settings\n");
		uprintf("x: exit and continue booting\n");
		uprintf(">");
		tSerialVectors.fn.fnFlush();
		pcInput = console_io_read_line(80);

		if( strcmp(pcInput, "p")==0 )
		{
			show_portcontrol();
		}
		else if( strcmp(pcInput, "m")==0 )
		{
			show_mmios();
		}
		else if( strcmp(pcInput, "r")==0 )
		{
			show_power_and_clocks();
		}
		else if( strcmp(pcInput, "d")==0 )
		{
			show_ddr();
		}
		else if( strcmp(pcInput, "x")==0 )
		{
			iExit = 1;
		}
		else
		{
			uprintf("Invalid command!\n");
		}
	} while( iExit==0 );
}
#endif

#ifdef SHOW_HWCONFIG_AUTO
void show_cfg_main(void);
void show_cfg_main(void)
{
	show_cfg_init();
	show_portcontrol();
	show_mmios();
	show_power_and_clocks();
	show_ddr();
}
#endif