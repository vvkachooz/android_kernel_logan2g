/******************************************************************************
 ** Copyright (c) 
 ** File Name:		sensor_hi253.c 										  *
 ** Author: 													  *
 ** DATE:															  *
 ** Description:   This file contains driver for sensor HI253. 
 ** Spreadtrum_8800s+Hynix_HI253_V1.2														 
 ******************************************************************************

 ******************************************************************************
 ** 					   Edit History 									  *
 ** ------------------------------------------------------------------------- *
 ** DATE		   NAME 			DESCRIPTION 							  *
 ** 2012-04-05	  
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 ** 						Dependencies									  *
 **---------------------------------------------------------------------------*/
//#include "sensor_cfg.h"
//#include "sensor_drv.h"
//#include "i2c_drv.h"
//#include "os_api.h"
//#include "chip.h"
//#include "dal_dcamera.h"
//#include <linux/delay.h>
//#include "sensor.h"
//#include <mach/common.h>
//#include <mach/sensor_drv.h>

#include "common/sensor.h"
#include "common/sensor_drv.h"
#include "common/jpeg_exif_header_k.h"


/**---------------------------------------------------------------------------*
 ** 						Compiler Flag									  *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** 						Const variables 								  *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** 						   Macro Define
 **---------------------------------------------------------------------------*/
#define HI253_I2C_ADDR_W		         0x20//0x40
#define HI253_I2C_ADDR_R			0x20//0x41
//#define HI253_WRITE_DELAY		          0x04

//#define SENSOR_GAIN_SCALE		16

//LOCAL uint32 s_preview_mode;
 
/**---------------------------------------------------------------------------*
 ** 					Local Function Prototypes							  *
 **---------------------------------------------------------------------------*/

LOCAL uint32_t HI253_InitExifInfo(void);
//LOCAL uint32_t set_hi253_ae_awb_enable(uint32_t ae_enable, uint32_t awb_enable);
LOCAL uint32_t set_hi253_ae_enable(uint32_t enable);
//LOCAL uint32_t set_hi253_hmirror_enable(uint32_t enable);
//LOCAL uint32_t set_hi253_vmirror_enable(uint32_t enable);
LOCAL uint32_t set_hi253_preview_mode(uint32_t preview_mode);
LOCAL uint32_t _hi253_Power_On(uint32_t power_on);
LOCAL uint32_t HI253_Identify(uint32_t param);
LOCAL uint32_t HI253_Before_Snapshot(uint32_t param);  
LOCAL uint32_t HI253_After_Snapshot(uint32_t param);
LOCAL uint32_t HI253_set_brightness(uint32_t level);
LOCAL uint32_t HI253_set_contrast(uint32_t level);
//LOCAL uint32_t HI253_set_sharpness(uint32_t level);
//LOCAL uint32_t HI253_set_saturation(uint32_t level);
LOCAL uint32_t HI253_set_image_effect(uint32_t effect_type);
//LOCAL uint32_t HI253_read_ev_value(uint32_t value);
//LOCAL uint32_t HI253_write_ev_value(uint32_t exposure_value);
//LOCAL uint32_t HI253_read_gain_value(uint32_t value);
//LOCAL uint32_t HI253_write_gain_value(uint32_t gain_value);
//LOCAL uint32_t HI253_read_gain_scale(uint32_t value);
//LOCAL uint32_t HI253_set_frame_rate(uint32_t param);
LOCAL uint32_t HI253_set_work_mode(uint32_t mode);
LOCAL uint32_t HI253_set_whitebalance_mode(uint32_t mode);
LOCAL uint32_t set_hi253_video_mode(uint32_t mode);
LOCAL uint32_t set_hi253_ev(uint32_t level);
LOCAL uint32_t _hi253_GetResolutionTrimTab(uint32_t param);
LOCAL uint32_t HI253_GetExifInfo(uint32_t param);
/*modify begin tianxiaohui 2012-03-30*/
LOCAL uint32_t HI253_flash(uint32_t param); //tianxiaohui
static uint32_t  g_flash_mode_en = 0; //tianxiaohui
/*modify end tianxiaohui 2012-03-30*/

LOCAL void    HI253_Write_Group_Regs( SENSOR_REG_T* sensor_reg_ptr );
/**---------------------------------------------------------------------------*
 ** 						Local Variables 								 *
 **---------------------------------------------------------------------------*/

/*lint -save -e533 */
 const SENSOR_REG_T HI253_YUV_COMMON[]=
 {
/////// Start Sleep ///////
    {0x01, 0x79}, //sleep on
    {0x08, 0x0f}, //Hi-Z on
    {0x01, 0x78}, //sleep off

    {0x03, 0x00}, // Dummy 750us START
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00}, // Dummy 750us END

    {0x0e, 0x03}, //PLL On
    {0x0e, 0x73}, //PLLx2

    {0x03, 0x00}, // Dummy 750us START
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00}, // Dummy 750us END

    {0x0e, 0x00}, //PLL off
    {0x01, 0x71}, //sleep on
    {0x08, 0x00}, //Hi-Z off

    {0x01, 0x73},
    {0x01, 0x71},

    // PAGE 20
    {0x03, 0x20}, //page 20
    {0x10, 0x1c}, //ae off

    // PAGE 22
    {0x03, 0x22}, //page 22
    {0x10, 0x69}, //awb off


    //Initial Start
    /////// PAGE 0 START ///////
    {0x03, 0x00},
    {0x10, 0x11}, // Sub1/2_Preview2 Mode_H binning
    {0x11, 0x90}, // aiden: old 0x90
    {0x12, 0x04},

    {0x0b, 0xaa}, // ESD Check Register
    {0x0c, 0xaa}, // ESD Check Register
    {0x0d, 0xaa}, // ESD Check Register

    {0x20, 0x00}, // Windowing start point Y
    {0x21, 0x04},
    {0x22, 0x00}, // Windowing start point X
    {0x23, 0x07},

    {0x24, 0x04},
    {0x25, 0xb0},
    {0x26, 0x06},
    {0x27, 0x40}, // WINROW END

    {0x40, 0x01}, //Hblank 408
    {0x41, 0x68}, 
    {0x42, 0x00}, //Vblank 20
    {0x43, 0x58},

    {0x45, 0x04},
    {0x46, 0x18},
    {0x47, 0xd8},

    //BLC
    {0x80, 0x2e},
    {0x81, 0x7e},
    {0x82, 0x90},
    {0x83, 0x00},
    {0x84, 0x0c},
    {0x85, 0x00},
    {0x90, 0x0c}, //BLC_TIME_TH_ON
    {0x91, 0x0c}, //BLC_TIME_TH_OFF 
    {0x92, 0x98}, //BLC_AG_TH_ON
    {0x93, 0x90}, //BLC_AG_TH_OFF
    {0x94, 0x75},
    {0x95, 0x70},
    {0x96, 0xdc},
    {0x97, 0xfe},
    {0x98, 0x38},

    //OutDoor  BLC
    {0x99, 0x45},
    {0x9a, 0x45},
    {0x9b, 0x45},
    {0x9c, 0x45},

    //Dark BLC
    {0xa0, 0x45},
    {0xa2, 0x45},
    {0xa4, 0x45},
    {0xa6, 0x45},

    //Normal BLC
    {0xa8, 0x45},
    {0xaa, 0x45},
    {0xac, 0x45},
    {0xae, 0x45},

    {0x03, 0x02}, //Page 02
    {0x10, 0x00}, //Mode_test
    {0x11, 0x00}, //Mode_dead_test
    {0x12, 0x03}, //pwr_ctl_ctl1
    {0x13, 0x03}, //Mode_ana_test
    {0x14, 0x00}, //mode_memory
    {0x16, 0x00}, //dcdc_ctl1
    {0x17, 0x8c}, //dcdc_ctl2
    {0x18, 0x4C}, //analog_func1
    {0x19, 0x00}, //analog_func2
    {0x1a, 0x39}, //analog_func3
    {0x1b, 0x00}, //analog_func4
    {0x1c, 0x09}, //dcdc_ctl3
    {0x1d, 0x40}, //dcdc_ctl4
    {0x1e, 0x30}, //analog_func7
    {0x1f, 0x10}, //analog_func8
    {0x20, 0x77}, //pixel bias
    {0x21, 0xde}, //adc,asp bias
    {0x22, 0xa7}, //main,bus bias
    {0x23, 0x30}, //clamp
    {0x24, 0x4a},		
    {0x25, 0x10},		
    {0x27, 0x3c},		
    {0x28, 0x00},		
    {0x29, 0x0c},		
    {0x2a, 0x80},		
    {0x2b, 0x80},		
    {0x2c, 0x02},		
    {0x2d, 0xa0},		
    {0x2e, 0x00}, // 0x11->0x00 [20110809 update]
    {0x2f, 0x00},// 0xa1->0x00 [20110809 update]		
    {0x30, 0x05}, //swap_ctl
    {0x31, 0x99},		
    {0x32, 0x00},		
    {0x33, 0x00},		
    {0x34, 0x22},		
    {0x38, 0x88},		
    {0x39, 0x88},		
    {0x50, 0x20},		
    {0x51, 0x03},		
    {0x52, 0x01},		
    {0x53, 0xc1},		
    {0x54, 0x10},		
    {0x55, 0x1c},		
    {0x56, 0x11},		
    {0x58, 0x10},		
    {0x59, 0x0e},		
    {0x5d, 0xa2},		
    {0x5e, 0x5a},		
    {0x60, 0x87},		
    {0x61, 0x99},		
    {0x62, 0x88},		
    {0x63, 0x97},		
    {0x64, 0x88},		
    {0x65, 0x97},		
    {0x67, 0x0c},		
    {0x68, 0x0c},		
    {0x69, 0x0c},		
    {0x6a, 0xb4},		
    {0x6b, 0xc4},		
    {0x6c, 0xb5},		
    {0x6d, 0xc2},		
    {0x6e, 0xb5},		
    {0x6f, 0xc0},		
    {0x70, 0xb6},		
    {0x71, 0xb8},		
    {0x72, 0x89},		
    {0x73, 0x96},		
    {0x74, 0x89},		
    {0x75, 0x96},		
    {0x76, 0x89},		
    {0x77, 0x96},		
    {0x7c, 0x85},		
    {0x7d, 0xaf},		
    {0x80, 0x01},		
    {0x81, 0x7f},		
    {0x82, 0x13}, //rx_on1_read
    {0x83, 0x24},		
    {0x84, 0x7D},		
    {0x85, 0x81},		
    {0x86, 0x7D},		
    {0x87, 0x81},		
    {0x88, 0xab},		
    {0x89, 0xbc},		
    {0x8a, 0xac},		
    {0x8b, 0xba},		
    {0x8c, 0xad},		
    {0x8d, 0xb8},		
    {0x8e, 0xae},		
    {0x8f, 0xb2},		
    {0x90, 0xb3},		
    {0x91, 0xb7},		
    {0x92, 0x48},		
    {0x93, 0x54},		
    {0x94, 0x7D},		
    {0x95, 0x81},		
    {0x96, 0x7D},		
    {0x97, 0x81},		
    {0xa0, 0x02},		
    {0xa1, 0x7B},		
    {0xa2, 0x02},		
    {0xa3, 0x7B},		
    {0xa4, 0x7B},		
    {0xa5, 0x02},		
    {0xa6, 0x7B},		
    {0xa7, 0x02},		
    {0xa8, 0x85},		
    {0xa9, 0x8C},		
    {0xaa, 0x85},		
    {0xab, 0x8C},		
    {0xac, 0x10}, //Rx_pwr_off1_read
    {0xad, 0x16}, //Rx_pwr_on1_read
    {0xae, 0x10}, //Rx_pwr_off2_read
    {0xaf, 0x16}, //Rx_pwr_on1_read
    {0xb0, 0x99},		
    {0xb1, 0xA3},		
    {0xb2, 0xA4},		
    {0xb3, 0xAE},		
    {0xb4, 0x9B},		
    {0xb5, 0xA2},		
    {0xb6, 0xA6},		
    {0xb7, 0xAC},		
    {0xb8, 0x9B},		
    {0xb9, 0x9F},		
    {0xba, 0xA6},		
    {0xbb, 0xAA},		
    {0xbc, 0x9B},		
    {0xbd, 0x9F},		
    {0xbe, 0xA6},		
    {0xbf, 0xaa},		
    {0xc4, 0x2c},		
    {0xc5, 0x43},		
    {0xc6, 0x63},		
    {0xc7, 0x79},		
    {0xc8, 0x2d},		
    {0xc9, 0x42},		
    {0xca, 0x2d},		
    {0xcb, 0x42},		
    {0xcc, 0x64},		
    {0xcd, 0x78},		
    {0xce, 0x64},		
    {0xcf, 0x78},		
    {0xd0, 0x0a},		
    {0xd1, 0x09},		
    {0xd2, 0x20},		
    {0xd3, 0x00},	
    	
    {0xd4, 0x0c},		
    {0xd5, 0x0c},		
    {0xd6, 0x98},		
    {0xd7, 0x90},
    		
    {0xe0, 0xc4},		
    {0xe1, 0xc4},		
    {0xe2, 0xc4},		
    {0xe3, 0xc4},		
    {0xe4, 0x00},		
    {0xe8, 0x80},		
    {0xe9, 0x40},		
    {0xea, 0x7f},		
    {0xf0, 0x01}, //sram1_cfg
    {0xf1, 0x01}, //sram2_cfg
    {0xf2, 0x01}, //sram3_cfg
    {0xf3, 0x01}, //sram4_cfg
    {0xf4, 0x01}, //sram5_cfg

    /////// PAGE 3 ///////
    {0x03, 0x03},
    {0x10, 0x10},

    /////// PAGE 10 START ///////
    {0x03, 0x10},
    {0x10, 0x03}, // CrYCbY // For Demoset 0x03
    {0x12, 0x30},
    {0x13, 0x0a}, // contrast on
    {0x20, 0x00},

    {0x30, 0x00},
    {0x31, 0x00},
    {0x32, 0x00},
    {0x33, 0x00},

    {0x34, 0x30},
    {0x35, 0x00},
    {0x36, 0x00},
    {0x38, 0x00},
    {0x3e, 0x58},
    {0x3f, 0x00},

    {0x40, 0x90}, // YOFS
    {0x41, 0x00}, // DYOFS
    {0x48, 0x80}, // Contrast

    {0x60, 0x67},
    {0x61, 0x82}, //7e //8e //88 //80
    {0x62, 0x82}, //7e //8e //88 //80
    {0x63, 0x50}, //Double_AG 50->30
    {0x64, 0x41},

    {0x66, 0x42},
    {0x67, 0x20},

    {0x6a, 0x80}, //8a
    {0x6b, 0x84}, //74
    {0x6c, 0x80}, //7e //7a
    {0x6d, 0x80}, //8e

    //Don't touch//////////////////////////
    //{0x72, 0x84},
    //{0x76, 0x19},
    //{0x73, 0x70},
    //{0x74, 0x68},
    //{0x75, 0x60}, // white protection ON
    //{0x77, 0x0e}, //08 //0a
    //{0x78, 0x2a}, //20
    //{0x79, 0x08},
    ////////////////////////////////////////

    /////// PAGE 11 START ///////
    {0x03, 0x11},
    {0x10, 0x7f},
    {0x11, 0x40},
    {0x12, 0x0a}, // Blue Max-Filter Delete
    {0x13, 0xbb},

    {0x26, 0x31}, // Double_AG 31->20
    {0x27, 0x34}, // Double_AG 34->22
    {0x28, 0x0f},
    {0x29, 0x10},
    {0x2b, 0x30},
    {0x2c, 0x32},

    //Out2 D-LPF th
    {0x30, 0x70},
    {0x31, 0x10},
    {0x32, 0x58},
    {0x33, 0x09},
    {0x34, 0x06},
    {0x35, 0x03},

    //Out1 D-LPF th
    {0x36, 0x70},
    {0x37, 0x18},
    {0x38, 0x58},
    {0x39, 0x09},
    {0x3a, 0x06},
    {0x3b, 0x03},

    //Indoor D-LPF th
    {0x3c, 0x80},
    {0x3d, 0x18},
    {0x3e, 0xa0}, //80
    {0x3f, 0x0c},
    {0x40, 0x09},
    {0x41, 0x06},

    {0x42, 0x80},
    {0x43, 0x18},
    {0x44, 0xa0}, //80
    {0x45, 0x12},
    {0x46, 0x10},
    {0x47, 0x10},

    {0x48, 0x90},
    {0x49, 0x40},
    {0x4a, 0x80},
    {0x4b, 0x13},
    {0x4c, 0x10},
    {0x4d, 0x11},

    {0x4e, 0x80},
    {0x4f, 0x30},
    {0x50, 0x80},
    {0x51, 0x13},
    {0x52, 0x10},
    {0x53, 0x13},

    {0x54, 0x11},
    {0x55, 0x17},
    {0x56, 0x20},
    {0x57, 0x01},
    {0x58, 0x00},
    {0x59, 0x00},

    {0x5a, 0x1f}, //18
    {0x5b, 0x00},
    {0x5c, 0x00},

    {0x60, 0x3f},
    {0x62, 0x60},
    {0x70, 0x06},

    /////// PAGE 12 START ///////
    {0x03, 0x12},
    {0x20, 0x0f},
    {0x21, 0x0f},

    {0x25, 0x00}, //0x30

    {0x28, 0x00},
    {0x29, 0x00},
    {0x2a, 0x00},

    {0x30, 0x50},
    {0x31, 0x18},
    {0x32, 0x32},
    {0x33, 0x40},
    {0x34, 0x50},
    {0x35, 0x70},
    {0x36, 0xa0},

    {0x3b, 0x06},
    {0x3c, 0x06},


    //Out2 th
    {0x40, 0xa0},
    {0x41, 0x40},
    {0x42, 0xa0},
    {0x43, 0x90},
    {0x44, 0x90},
    {0x45, 0x80},

    //Out1 th
    {0x46, 0xb0},
    {0x47, 0x55},
    {0x48, 0xa0},
    {0x49, 0x90},
    {0x4a, 0x90},
    {0x4b, 0x80},

    //Indoor th
    {0x4c, 0xb0},
    {0x4d, 0x40},
    {0x4e, 0x90},
    {0x4f, 0x90},
    {0x50, 0xa0},
    {0x51, 0x80},

    //Dark1 th
    {0x52, 0xb0},
    {0x53, 0x60},
    {0x54, 0xc0},
    {0x55, 0xc0},
    {0x56, 0xc0},
    {0x57, 0x80},

    //Dark2 th
    {0x58, 0x90},
    {0x59, 0x40},
    {0x5a, 0xd0},
    {0x5b, 0xd0},
    {0x5c, 0xe0},
    {0x5d, 0x80},

    //Dark3 th
    {0x5e, 0x88},
    {0x5f, 0x40},
    {0x60, 0xe0},
    {0x61, 0xe0},
    {0x62, 0xe0},
    {0x63, 0x80},

    {0x70, 0x15},
    {0x71, 0x01}, //Don't Touch register

    {0x72, 0x18},
    {0x73, 0x01}, //Don't Touch register

    {0x74, 0x25},
    {0x75, 0x15},


    {0x90, 0x5d}, //DPC
    {0x91, 0x88},		
    {0x98, 0x7d},		
    {0x99, 0x28},		
    {0x9A, 0x14},		
    {0x9B, 0xc8},		
    {0x9C, 0x02},		
    {0x9D, 0x1e},		
    {0x9E, 0x28},		
    {0x9F, 0x07},		
    {0xA0, 0x32},		
    {0xA4, 0x04},		
    {0xA5, 0x0e},		
    {0xA6, 0x0c},		
    {0xA7, 0x04},		
    {0xA8, 0x3c},		

    {0xAA, 0x14},		
    {0xAB, 0x11},		
    {0xAC, 0x0f},		
    {0xAD, 0x16},		
    {0xAE, 0x15},		
    {0xAF, 0x14},		

    {0xB1, 0xaa},		
    {0xB2, 0x96},		
    {0xB3, 0x28},		
    //{0xB6,read}, only//dpc_flat_thres
    //{0xB7,read}, only//dpc_grad_cnt
    {0xB8, 0x78},		
    {0xB9, 0xa0},		
    {0xBA, 0xb4},		
    {0xBB, 0x14},		
    {0xBC, 0x14},		
    {0xBD, 0x14},		
    {0xBE, 0x64},		
    {0xBF, 0x64},		
    {0xC0, 0x64},		
    {0xC1, 0x64},		
    {0xC2, 0x04},		
    {0xC3, 0x03},		
    {0xC4, 0x0c},		
    {0xC5, 0x30},		
    {0xC6, 0x2a},		
    {0xD0, 0x0c}, //CI Option/CI DPC
    {0xD1, 0x80},		
    {0xD2, 0x67},		
    {0xD3, 0x00},		
    {0xD4, 0x00},		
    {0xD5, 0x02},		
    {0xD6, 0xff},		
    {0xD7, 0x18},	

    /////// PAGE 13 START ///////
    {0x03, 0x13},
    //Edge
    {0x10, 0xcb},
    {0x11, 0x7b},
    {0x12, 0x07},
    {0x14, 0x00},

    {0x20, 0x15},
    {0x21, 0x13},
    {0x22, 0x33},
    {0x23, 0x05},
    {0x24, 0x09},

    {0x25, 0x0a},

    {0x26, 0x18},
    {0x27, 0x30},
    {0x29, 0x12},
    {0x2a, 0x50},

    //Low clip th
    {0x2b, 0x00}, //Out2 02
    {0x2c, 0x00}, //Out1 02 //01
    {0x25, 0x06},
    {0x2d, 0x0c},
    {0x2e, 0x12},
    {0x2f, 0x12},

    //Out2 Edge
    {0x50, 0x18}, //0x10 //0x16
    {0x51, 0x1c}, //0x14 //0x1a
    {0x52, 0x1a}, //0x12 //0x18
    {0x53, 0x14}, //0x0c //0x12
    {0x54, 0x17}, //0x0f //0x15
    {0x55, 0x14}, //0x0c //0x12

    //Out1 Edge          //Edge
    {0x56, 0x18}, //0x10 //0x16
    {0x57, 0x1c}, //0x13 //0x1a
    {0x58, 0x1a}, //0x12 //0x18
    {0x59, 0x14}, //0x0c //0x12
    {0x5a, 0x17}, //0x0f //0x15
    {0x5b, 0x14}, //0x0c //0x12

    //Indoor Edge
    {0x5c, 0x0a},
    {0x5d, 0x0b},
    {0x5e, 0x0a},
    {0x5f, 0x08},
    {0x60, 0x09},
    {0x61, 0x08},

    //Dark1 Edge
    {0x62, 0x08},
    {0x63, 0x08},
    {0x64, 0x08},
    {0x65, 0x06},
    {0x66, 0x06},
    {0x67, 0x06},

    //Dark2 Edge
    {0x68, 0x07},
    {0x69, 0x07},
    {0x6a, 0x07},
    {0x6b, 0x05},
    {0x6c, 0x05},
    {0x6d, 0x05},

    //Dark3 Edge
    {0x6e, 0x07},
    {0x6f, 0x07},
    {0x70, 0x07},
    {0x71, 0x05},
    {0x72, 0x05},
    {0x73, 0x05},

    //2DY
    {0x80, 0xfd},
    {0x81, 0x1f},
    {0x82, 0x05},
    {0x83, 0x31},

    {0x90, 0x05},
    {0x91, 0x05},
    {0x92, 0x33},
    {0x93, 0x30},
    {0x94, 0x03},
    {0x95, 0x14},
    {0x97, 0x20},
    {0x99, 0x20},

    {0xa0, 0x01},
    {0xa1, 0x02},
    {0xa2, 0x01},
    {0xa3, 0x02},
    {0xa4, 0x05},
    {0xa5, 0x05},
    {0xa6, 0x07},
    {0xa7, 0x08},
    {0xa8, 0x07},
    {0xa9, 0x08},
    {0xaa, 0x07},
    {0xab, 0x08},

    //Out2 
    {0xb0, 0x22},
    {0xb1, 0x2a},
    {0xb2, 0x28},
    {0xb3, 0x22},
    {0xb4, 0x2a},
    {0xb5, 0x28},

    //Out1 
    {0xb6, 0x22},
    {0xb7, 0x2a},
    {0xb8, 0x28},
    {0xb9, 0x22},
    {0xba, 0x2a},
    {0xbb, 0x28},

    //Indoor 
    {0xbc, 0x25},
    {0xbd, 0x2a},
    {0xbe, 0x27},
    {0xbf, 0x25},
    {0xc0, 0x2a},
    {0xc1, 0x27},

    //Dark1
    {0xc2, 0x1e},
    {0xc3, 0x24},
    {0xc4, 0x20},
    {0xc5, 0x1e},
    {0xc6, 0x24},
    {0xc7, 0x20},

    //Dark2
    {0xc8, 0x18},
    {0xc9, 0x20},
    {0xca, 0x1e},
    {0xcb, 0x18},
    {0xcc, 0x20},
    {0xcd, 0x1e},

    //Dark3 
    {0xce, 0x18},
    {0xcf, 0x20},
    {0xd0, 0x1e},
    {0xd1, 0x18},
    {0xd2, 0x20},
    {0xd3, 0x1e},

    /////// PAGE 14 START ///////
    {0x03, 0x14},
    {0x10, 0x11},

    {0x14, 0x80}, // GX
    {0x15, 0x80}, // GY
    {0x16, 0x80}, // RX
    {0x17, 0x80}, // RY
    {0x18, 0x80}, // BX
    {0x19, 0x80}, // BY

    {0x20, 0x60}, //X 60 //a0
    {0x21, 0x80}, //Y

    {0x22, 0x80},
    {0x23, 0x80},
    {0x24, 0x80},

    {0x30, 0xc8},
    {0x31, 0x2b},
    {0x32, 0x00},
    {0x33, 0x00},
    {0x34, 0x90},

    {0x40, 0x48}, //31
    {0x50, 0x34}, //23 //32
    {0x60, 0x29}, //1a //27
    {0x70, 0x34}, //23 //32

    /////// PAGE 15 START ///////
    {0x03, 0x15},
    {0x10, 0x0f},

    //Rstep H 16
    //Rstep L 14
    {0x14, 0x42}, //CMCOFSGH_Day //4c
    {0x15, 0x32}, //CMCOFSGM_CWF //3c
    {0x16, 0x24}, //CMCOFSGL_A //2e
    {0x17, 0x2f}, //CMC SIGN

    //CMC_Default_CWF
    {0x30, 0x8f},
    {0x31, 0x59},
    {0x32, 0x0a},
    {0x33, 0x15},
    {0x34, 0x5b},
    {0x35, 0x06},
    {0x36, 0x07},
    {0x37, 0x40},
    {0x38, 0x87}, //86

    //CMC OFS L_A
    {0x40, 0x92},
    {0x41, 0x1b},
    {0x42, 0x89},
    {0x43, 0x81},
    {0x44, 0x00},
    {0x45, 0x01},
    {0x46, 0x89},
    {0x47, 0x9e},
    {0x48, 0x28},

    //{0x40, 0x93},
    //{0x41, 0x1c},
    //{0x42, 0x89},
    //{0x43, 0x82},
    //{0x44, 0x01},
    //{0x45, 0x01},
    //{0x46, 0x8a},
    //{0x47, 0x9d},
    //{0x48, 0x28},

    //CMC POFS H_DAY
    {0x50, 0x02},
    {0x51, 0x82},
    {0x52, 0x00},
    {0x53, 0x07},
    {0x54, 0x11},
    {0x55, 0x98},
    {0x56, 0x00},
    {0x57, 0x0b},
    {0x58, 0x8b},

    {0x80, 0x03},
    {0x85, 0x40},
    {0x87, 0x02},
    {0x88, 0x00},
    {0x89, 0x00},
    {0x8a, 0x00},

    /////// PAGE 16 START ///////
    {0x03, 0x16},
    {0x10, 0x31},
    {0x18, 0x5e},// Double_AG 5e->37
    {0x19, 0x5d},// Double_AG 5e->36
    {0x1a, 0x0e},
    {0x1b, 0x01},
    {0x1c, 0xdc},
    {0x1d, 0xfe},

    //GMA Default
    {0x30, 0x00},
    {0x31, 0x0a},
    {0x32, 0x1f},
    {0x33, 0x33},
    {0x34, 0x53},
    {0x35, 0x6c},
    {0x36, 0x81},
    {0x37, 0x94},
    {0x38, 0xa4},
    {0x39, 0xb3},
    {0x3a, 0xc0},
    {0x3b, 0xcb},
    {0x3c, 0xd5},
    {0x3d, 0xde},
    {0x3e, 0xe6},
    {0x3f, 0xee},
    {0x40, 0xf5},
    {0x41, 0xfc},
    {0x42, 0xff},
    //RGMA
    {0x50, 0x00},
    {0x51, 0x09},
    {0x52, 0x1f},
    {0x53, 0x37},
    {0x54, 0x5b},
    {0x55, 0x76},
    {0x56, 0x8d},
    {0x57, 0xa1},
    {0x58, 0xb2},
    {0x59, 0xbe},
    {0x5a, 0xc9},
    {0x5b, 0xd2},
    {0x5c, 0xdb},
    {0x5d, 0xe3},
    {0x5e, 0xeb},
    {0x5f, 0xf0},
    {0x60, 0xf5},
    {0x61, 0xf7},
    {0x62, 0xf8},
    //BGMA
    {0x70, 0x00}, // new gamma for low noise
    {0x71, 0x07},
    {0x72, 0x0c},
    {0x73, 0x18},
    {0x74, 0x31},
    {0x75, 0x4d},
    {0x76, 0x69},
    {0x77, 0x83},
    {0x78, 0x9b},
    {0x79, 0xb1},
    {0x7a, 0xc3},
    {0x7b, 0xd2},
    {0x7c, 0xde},
    {0x7d, 0xe8},
    {0x7e, 0xf0},
    {0x7f, 0xf5},
    {0x80, 0xfa},
    {0x81, 0xfd},
    {0x82, 0xff},

    /////// PAGE 17 START ///////
    {0x03, 0x17},
    {0x10, 0xf7},

    /////// PAGE 20 START ///////
    {0x03, 0x20},
    {0x11, 0x1c},
    {0x18, 0x30},
    {0x1a, 0x08},
    {0x20, 0x01}, //05_lowtemp Y Mean off
    {0x21, 0x30},
    {0x22, 0x10},
    {0x23, 0x00},
    {0x24, 0x00}, //Uniform Scene Off

    {0x28, 0xe7},
    {0x29, 0x0d}, //20100305 ad->0d
    {0x2a, 0xff},
    {0x2b, 0x04}, //f4->Adaptive off

    {0x2c, 0xc2},
    {0x2d, 0xcf},  //fe->AE Speed option
    {0x2e, 0x33},
    {0x30, 0x78}, //f8
    {0x32, 0x03},
    {0x33, 0x2e},
    {0x34, 0x30},
    {0x35, 0xd4},
    {0x36, 0xfe},
    {0x37, 0x32},
    {0x38, 0x04},

    {0x39, 0x22}, //AE_escapeC10
    {0x3a, 0xde}, //AE_escapeC11

    {0x3b, 0x22}, //AE_escapeC1
    {0x3c, 0xde}, //AE_escapeC2

    {0x50, 0x45},
    {0x51, 0x88},

    {0x56, 0x03},
    {0x57, 0xf7},
    {0x58, 0x14},
    {0x59, 0x88},
    {0x5a, 0x04},

    //New Weight For Samsung
    //{0x60, 0xff},
    //{0x61, 0xff},
    //{0x62, 0xea},
    //{0x63, 0xab},
    //{0x64, 0xea},
    //{0x65, 0xab},
    //{0x66, 0xeb},
    //{0x67, 0xeb},
    //{0x68, 0xeb},
    //{0x69, 0xeb},
    //{0x6a, 0xea},
    //{0x6b, 0xab},
    //{0x6c, 0xea},
    //{0x6d, 0xab},
    //{0x6e, 0xff},
    //{0x6f, 0xff},

    {0x60, 0x55}, // AEWGT1
    {0x61, 0x55}, // AEWGT2
    {0x62, 0x6a}, // AEWGT3
    {0x63, 0xa9}, // AEWGT4
    {0x64, 0x6a}, // AEWGT5
    {0x65, 0xa9}, // AEWGT6
    {0x66, 0x6a}, // AEWGT7
    {0x67, 0xa9}, // AEWGT8
    {0x68, 0x6b}, // AEWGT9
    {0x69, 0xe9}, // AEWGT10
    {0x6a, 0x6a}, // AEWGT11
    {0x6b, 0xa9}, // AEWGT12
    {0x6c, 0x6a}, // AEWGT13
    {0x6d, 0xa9}, // AEWGT14
    {0x6e, 0x55}, // AEWGT15
    {0x6f, 0x55}, // AEWGT16

    {0x70, 0x76}, //6e
    {0x71, 0x89}, //00 //-4

    // haunting control
    {0x76, 0x43},
    {0x77, 0xe2}, //04 //f2

    {0x78, 0x23}, //Yth1
    {0x79, 0x42}, //Yth2 //46
    {0x7a, 0x23}, //23
    {0x7b, 0x22}, //22
    {0x7d, 0x23},

    {0x83, 0x01}, //EXP Normal 33.33 fps 
    {0x84, 0x5f}, 
    {0x85, 0x90}, 

    {0x86, 0x01}, //EXPMin 5859.38 fps
    {0x87, 0xf4}, 

    {0x88, 0x04}, //EXP Max 8.00 fps 
    {0x89, 0x93}, 
    {0x8a, 0xe0}, 

    {0x8B, 0x75}, //EXP100 
    {0x8C, 0x30}, 
    {0x8D, 0x61}, //EXP120 
    {0x8E, 0xa8}, 

    {0x9c, 0x17}, //EXP Limit 488.28 fps 
    {0x9d, 0x70}, 
    {0x9e, 0x01}, //EXP Unit 
    {0x9f, 0xf4}, 

    //AE_Middle Time option
    //{0xa0, 0x03},
    //{0xa1, 0xa9},
    //{0xa2, 0x80},

    {0xb0, 0x18},
    {0xb1, 0x14}, //ADC 400->560
    {0xb2, 0xa0}, 
    {0xb3, 0x18},
    {0xb4, 0x1a},
    {0xb5, 0x44},
    {0xb6, 0x2f},
    {0xb7, 0x28},
    {0xb8, 0x25},
    {0xb9, 0x22},
    {0xba, 0x21},
    {0xbb, 0x20},
    {0xbc, 0x1f},
    {0xbd, 0x1f},

    {0xc0, 0x14},
    {0xc1, 0x1f},
    {0xc2, 0x1f},
    {0xc3, 0x18}, //2b
    {0xc4, 0x10}, //08

    {0xc8, 0x80},
    {0xc9, 0x40},

    /////// PAGE 22 START ///////
    {0x03, 0x22},
    {0x10, 0xfd},
    {0x11, 0x2e},
    {0x19, 0x01}, // Low On //
    {0x20, 0x30},
    {0x21, 0x80},
    {0x24, 0x01},
    //{0x25, 0x00}, //7f New Lock Cond & New light stable

    {0x30, 0x80},
    {0x31, 0x80},
    {0x38, 0x11},
    {0x39, 0x34},

    {0x40, 0xf7}, //
    {0x41, 0x55}, //44
    {0x42, 0x33}, //43

    {0x43, 0xf7},
    {0x44, 0x55}, //44
    {0x45, 0x44}, //33

    {0x46, 0x00},
    {0x50, 0xb2},
    {0x51, 0x81},
    {0x52, 0x98},

    {0x80, 0x40}, //3e
    {0x81, 0x20},
    {0x82, 0x3e},

    {0x83, 0x5e}, //5e
    {0x84, 0x1e}, //24
    {0x85, 0x5e}, //54 //56 //5a
    {0x86, 0x22}, //24 //22

    {0x87, 0x40},
    {0x88, 0x30},
    {0x89, 0x3f}, //38
    {0x8a, 0x28}, //2a

    {0x8b, 0x40}, //47
    {0x8c, 0x33}, 
    {0x8d, 0x39}, 
    {0x8e, 0x30}, //2c

    {0x8f, 0x53}, //4e
    {0x90, 0x52}, //4d
    {0x91, 0x51}, //4c
    {0x92, 0x4e}, //4a
    {0x93, 0x4a}, //46
    {0x94, 0x45},
    {0x95, 0x3d},
    {0x96, 0x31},
    {0x97, 0x28},
    {0x98, 0x24},
    {0x99, 0x20},
    {0x9a, 0x20},

    {0x9b, 0x77},
    {0x9c, 0x77},
    {0x9d, 0x48},
    {0x9e, 0x38},
    {0x9f, 0x30},

    {0xa0, 0x60},
    {0xa1, 0x34},
    {0xa2, 0x6f},
    {0xa3, 0xff},

    {0xa4, 0x14}, //1500fps
    {0xa5, 0x2c}, // 700fps
    {0xa6, 0xcf},

    {0xad, 0x40},
    {0xae, 0x4a},

    {0xaf, 0x28},  // low temp Rgain
    {0xb0, 0x26},  // low temp Rgain

    {0xb1, 0x00}, //0x20 -> 0x00 0405 modify
    {0xb4, 0xea},
    {0xb8, 0xa0}, //a2: b-2, R+2  //b4 B-3, R+4 lowtemp
    {0xb9, 0x00},

    // PAGE 20
    {0x03, 0x20}, //page 20
    {0x10, 0x9c}, //ae off

    // PAGE 22
    {0x03, 0x22}, //page 22
    {0x10, 0xe9}, //awb off

    // PAGE 0
    {0x03, 0x00},
    {0x0e, 0x03}, //PLL On
    {0x0e, 0x73}, //PLLx2

    {0x03, 0x00}, // Dummy 750us
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},
    {0x03, 0x00},

    {0x03, 0x00}, // Page 0
    {0x01, 0xc0}, // Sleep Off 0xf8->0x50 for solve green line issue

//    {0xff, 0xff},
}
;
const SENSOR_REG_T HI253_YUV_640X480[]=
{
		{0x03, 0x00},
		{0x10, 0x11},
		{0x12, 0x04},
		
		//scaling VGA
		{0x03, 0x18},//Page 18
		{0x12, 0x20},
		{0x10, 0x07},//Open Scaler Function
		{0x11, 0x00},
		{0x20, 0x05},
		{0x21, 0x00},
		{0x22, 0x01},
		{0x23, 0xe0},
		{0x24, 0x00},
		{0x25, 0x00},
		{0x26, 0x00},
		{0x27, 0x00},
		{0x28, 0x05},
		{0x29, 0x00},
		{0x2a, 0x01},
		{0x2b, 0xe0},
		{0x2c, 0x0a},
		{0x2d, 0x00},
		{0x2e, 0x0a},
		{0x2f, 0x00},
		{0x30, 0x44},
	
//		{0xff, 0xff},

};
SENSOR_REG_T HI253_YUV_1280X960[]=
{  
		{0x03, 0x00},
		{0x10, 0x00}, //0401//0x00},
		{0x12, 0x04},
	
		{0x03, 0x18},
		{0x10, 0x07},
		{0x11, 0x00},
		{0x12, 0x20},
		{0x20, 0x05},
		{0x21, 0x00},
		{0x22, 0x03},
		{0x23, 0xc0},
		{0x24, 0x00},
		{0x25, 0x04},
		{0x26, 0x00},
		{0x27, 0x04},
		{0x28, 0x05},
		{0x29, 0x04},
		{0x2a, 0x03},
		{0x2b, 0xc4},
		{0x2c, 0x0a},
		{0x2d, 0x00},
		{0x2e, 0x0a},
		{0x2f, 0x00},
		{0x30, 0x41}, //41->44
			
		{0x03,0x00},//dummy1    
		{0x03,0x00},//dummy2        
		{0x03,0x00},//dummy3
		{0x03,0x00},//dummy4    
		{0x03,0x00},//dummy5        
		{0x03,0x00},//dummy6   
		{0x03,0x00},//dummy7    
		{0x03,0x00},//dummy8        
		{0x03,0x00},//dummy9
		{0x03,0x00},//dummy10                 
	  //{SENSOR_WRITE_DELAY, 0x32},    //delay 10ms   martin_add
//		{0xff, 0xff}
};

SENSOR_REG_T HI253_YUV_1600X1200[]=
{
	
		{0x03,0x00},
		{0x10,0x00}, //0401//0x00},//100208 Vsync type2
		{0x12,0x04},
	
		{0x03, 0x18},
		{0x10, 0x00},
		{0x11, 0x00},
		{0x12, 0x00},
	  //{SENSOR_WRITE_DELAY, 0x32},   //delay 10ms martin_add
//		{0xff, 0xff},

};
LOCAL SENSOR_TRIM_T s_HI253_Resolution_Trim_Tab[]=
{	
		// COMMON INIT
		{0, 0, 0, 0, 0, 64},
		
		// YUV422 PREVIEW 1	
		{0, 0, 0, 0, 0, 64},
		{0, 0, 0, 0, 0, 64},
		
		{0, 0, 0, 0, 0, 64},
		{0, 0, 0, 0, 0, 64},
		
		// YUV422 PREVIEW 2 
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0}
};

LOCAL SENSOR_REG_TAB_INFO_T s_hi253_resolution_Tab_YUV[]=
{	
    // COMMON INIT
    {ADDR_AND_LEN_OF_ARRAY(HI253_YUV_COMMON),640,480,24,SENSOR_IMAGE_FORMAT_YUV422},
    // YUV422 PREVIEW 1
    {ADDR_AND_LEN_OF_ARRAY(HI253_YUV_640X480),640,480,24,SENSOR_IMAGE_FORMAT_YUV422},
    {ADDR_AND_LEN_OF_ARRAY(HI253_YUV_1280X960), 1280, 960, 24, SENSOR_IMAGE_FORMAT_YUV422},
    {ADDR_AND_LEN_OF_ARRAY(HI253_YUV_1600X1200), 1600, 1200,24,SENSOR_IMAGE_FORMAT_YUV422},
    {PNULL,	0, 0,	0, 0, 0},

    // YUV422 PREVIEW 2 
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0},
    {PNULL, 0, 0, 0, 0, 0}

};

LOCAL EXIF_SPEC_PIC_TAKING_COND_T s_hi253_exif;

LOCAL SENSOR_IOCTL_FUNC_TAB_T s_hi253_ioctl_func_tab = 
{
    // Internal 
    PNULL,
    PNULL,//_hi253_Power_On,
    PNULL,
    HI253_Identify,

    PNULL,			// write register
    PNULL,			// read  register	
    PNULL,
    PNULL,//_hi253_GetResolutionTrimTab,

    // External
    set_hi253_ae_enable,
    PNULL,//set_hi253_hmirror_enable,
    PNULL,//set_hi253_vmirror_enable,

    HI253_set_brightness,
    HI253_set_contrast,
    PNULL,//HI253_set_sharpness,
    PNULL,//HI253_set_saturation,

    set_hi253_preview_mode,	
    HI253_set_image_effect,

    HI253_Before_Snapshot,
    HI253_After_Snapshot,
    
    HI253_flash, //tianxiaohui
    
    //PNULL,

    PNULL,//HI253_read_ev_value,
    PNULL,//HI253_write_ev_value,
    PNULL,//HI253_read_gain_value,
    PNULL,//HI253_write_gain_value,
    PNULL,//HI253_read_gain_scale,
    PNULL,//HI253_set_frame_rate,

    PNULL,
    PNULL,
    HI253_set_whitebalance_mode,
    PNULL,	// get snapshot skip frame num from customer, input SENSOR_MODE_E paramter

    PNULL,	// set ISO level					 0: auto; other: the appointed level
    set_hi253_ev, // Set exposure compensation	 0: auto; other: the appointed level

    PNULL, // check whether image format is support
    PNULL, //change sensor image format according to param
    PNULL, //change sensor image format according to param

    // CUSTOMER FUNCTION	                      
    HI253_GetExifInfo,  	// function 3 for custumer to configure                      
    PNULL,	// function 4 for custumer to configure 	
    PNULL,	//Set anti banding flicker	 0: 50hz;1: 60	
    PNULL,	//set_hi253_video_mode, // set video mode

    PNULL,   // pick out the jpeg stream from given buffer
};

LOCAL SENSOR_EXTEND_INFO_T hi253_ext_info = {
    256,    //jpeg_seq_width
    0//1938       //jpeg_seq_height
};

SENSOR_INFO_T g_hi253_yuv_info =
{
	HI253_I2C_ADDR_W,				// salve i2c write address
	HI253_I2C_ADDR_R, 				// salve i2c read address
		
	0,								// bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
										// bit2: 0: i2c register addr  is 8 bit, 1: i2c register addr  is 16 bit
										// other bit: reseved
	SENSOR_HW_SIGNAL_PCLK_P|\
	SENSOR_HW_SIGNAL_VSYNC_N|\
	SENSOR_HW_SIGNAL_HSYNC_P,		// bit0: 0:negative; 1:positive -> polarily of pixel clock
										// bit2: 0:negative; 1:positive -> polarily of horizontal synchronization signal
										// bit4: 0:negative; 1:positive -> polarily of vertical synchronization signal
										// other bit: reseved											
												
		// preview mode
	SENSOR_ENVIROMENT_NORMAL|\
	SENSOR_ENVIROMENT_NIGHT|\
	SENSOR_ENVIROMENT_SUNNY,		
		
		// image effect
	SENSOR_IMAGE_EFFECT_NORMAL|\
	SENSOR_IMAGE_EFFECT_BLACKWHITE|\
	SENSOR_IMAGE_EFFECT_RED|\
	SENSOR_IMAGE_EFFECT_GREEN|\
	SENSOR_IMAGE_EFFECT_BLUE|\
	SENSOR_IMAGE_EFFECT_YELLOW|\
	SENSOR_IMAGE_EFFECT_NEGATIVE|\
	SENSOR_IMAGE_EFFECT_CANVAS,
		
		// while balance mode
	0,
			
	7,								// bit[0:7]: count of step in brightness, contrast, sharpness, saturation
										// bit[8:31] reseved
		
	SENSOR_LOW_PULSE_RESET,			// reset pulse level
	100,								// reset pulse width(ms)
		
	SENSOR_HIGH_LEVEL_PWDN,			// 1: high level valid; 0: low level valid
		
	2,								// count of identify code
	{{0x04, 0x92},						// supply two code to identify sensor.
	{0x04, 0x92}},						// for Example: index = 0-> Device id, index = 1 -> version id	
										
	SENSOR_AVDD_2800MV,				// voltage of avdd	
	
	1600,							// max width of source image
	1200,							// max height of source image
	"HI253",						// name of sensor												
	
	SENSOR_IMAGE_FORMAT_YUV422,		// define in SENSOR_IMAGE_FORMAT_E enum,
										// if set to SENSOR_IMAGE_FORMAT_MAX here, image format depent on SENSOR_REG_TAB_INFO_T
	SENSOR_IMAGE_PATTERN_YUV422_YUYV,	// pattern of input image form sensor;			
	
	s_hi253_resolution_Tab_YUV,	// point to resolution table information structure
	&s_hi253_ioctl_func_tab,		// point to ioctl function table
				
	PNULL,							// information and table about Rawrgb sensor
	&hi253_ext_info,				// extend information about sensor	
	SENSOR_AVDD_2800MV,                     // iovdd
	SENSOR_AVDD_1800MV,                      // dvdd
	0,                     // skip frame num before preview 
	0,                      // skip frame num before capture        
      0,                      // deci frame num during preview    
      0,                      // deci frame num during video preview
      0,                     // threshold enable(only analog TV)  
      0,                    // threshold mode 0 fix mode 1 auto mode      
      0,                     // threshold start postion
      0,                     // threshold end postion       
      0

};

/**---------------------------------------------------------------------------*
 ** 							Function  Definitions
 **---------------------------------------------------------------------------*/
LOCAL void HI253_WriteReg( uint8_t  subaddr, uint8_t data )
{
		Sensor_WriteReg_8bits(subaddr,data);
}

LOCAL uint8_t HI253_ReadReg( uint8_t  subaddr)
{
		uint8_t value = 0;
		value = Sensor_ReadReg(subaddr);
		return value;
}

LOCAL uint32_t HI253_Identify(uint32_t param)
{
		uint32_t i;
		uint32_t nLoop;
		uint8_t ret;
		uint32_t err_cnt = 0;	
		uint8_t reg[2] 	= {0x04, 0x04};
		uint8_t value[2] 	= {0x92, 0x92};
	
		printk("HI253_Identify");
		for(i = 0; i<2; )
		{
			nLoop = 1000;
			ret = HI253_ReadReg(reg[i]);
			if( ret != value[i])
			{
				err_cnt++;
				if(err_cnt>3)			
				{
					printk("It is not HI253\n");
					return SENSOR_FAIL;
				}
				else
				{
					//Masked by frank.yang,SENSOR_Sleep() will cause a  Assert when called in boot precedure
					//SENSOR_Sleep(10);
					while(nLoop--);
					continue;
				}
			}
	        	err_cnt = 0;
			i++;
		}
		printk("HI253 identify: It is HI253\n");

		HI253_InitExifInfo();
		
		return SENSOR_SUCCESS;

}

#if 0
LOCAL uint32_t _hi253_Power_On(uint32_t power_on)
{
		SENSOR_AVDD_VAL_E		dvdd_val=g_hi253_yuv_info.dvdd_val;
		SENSOR_AVDD_VAL_E		avdd_val=g_hi253_yuv_info.avdd_val;
		SENSOR_AVDD_VAL_E		iovdd_val=g_hi253_yuv_info.iovdd_val;  
	//	BOOLEAN 				power_down=g_hi253_yuv_info.power_down_level;	    
		BOOLEAN 				reset_level=g_hi253_yuv_info.reset_pulse_level;
		uint32_t 				         reset_width=g_hi253_yuv_info.reset_pulse_width;	    
	
		if(SENSOR_TRUE==power_on)
		{
			Sensor_SetVoltage(dvdd_val, avdd_val, iovdd_val);
		//	GPIO_SetSensorPower (power_on);
		//	GPIO_SetSensorPwdn(!power_down);/*lint !e730*/
	
			// Open Mclk in default frequency
			Sensor_SetMCLK(SENSOR_DEFALUT_MCLK);   
	
			msleep(20);
			Sensor_SetResetLevel(reset_level);
			msleep(reset_width);
			Sensor_SetResetLevel(!reset_level);/*lint !e730*/
			msleep(100);
		}
		else
		{
		//	GPIO_SetSensorPwdn(power_down);
			Sensor_SetMCLK(SENSOR_DISABLE_MCLK);    
		//	GPIO_SetSensorPower (power_on);
	
			Sensor_SetVoltage(SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED);        
		}
	
		SENSOR_PRINT("SENSOR: _hi253_Power_On(1:on, 0:off): %d", power_on);    
	
		return SENSOR_SUCCESS;
}
#endif

LOCAL uint32_t HI253_GetExifInfo(uint32_t param)
{
	return (uint32_t) & s_hi253_exif;
}
LOCAL uint32_t HI253_InitExifInfo(void)
{
#if 1
	EXIF_SPEC_PIC_TAKING_COND_T *exif_ptr = &s_hi253_exif;

	memset(&s_hi253_exif, 0, sizeof(EXIF_SPEC_PIC_TAKING_COND_T));

	SENSOR_PRINT("SENSOR: HI253_InitExifInfo \n");

	exif_ptr->valid.FNumber = 1;
	exif_ptr->FNumber.numerator = 14;
	exif_ptr->FNumber.denominator = 5;

	exif_ptr->valid.ExposureProgram = 1;
	exif_ptr->ExposureProgram = 0x04;

	//exif_ptr->SpectralSensitivity[MAX_ASCII_STR_SIZE];
	//exif_ptr->ISOSpeedRatings;
	//exif_ptr->OECF;

	//exif_ptr->ShutterSpeedValue;

	exif_ptr->valid.ApertureValue = 1;
	exif_ptr->ApertureValue.numerator = 14;
	exif_ptr->ApertureValue.denominator = 5;

	//exif_ptr->BrightnessValue;
	//exif_ptr->ExposureBiasValue;

	exif_ptr->valid.MaxApertureValue = 1;
	exif_ptr->MaxApertureValue.numerator = 14;
	exif_ptr->MaxApertureValue.denominator = 5;

	//exif_ptr->SubjectDistance;
	//exif_ptr->MeteringMode;
	//exif_ptr->LightSource;
	//exif_ptr->Flash;

	exif_ptr->valid.FocalLength = 1;
	exif_ptr->FocalLength.numerator = 289;
	exif_ptr->FocalLength.denominator = 100;

	//exif_ptr->SubjectArea;
	//exif_ptr->FlashEnergy;
	//exif_ptr->SpatialFrequencyResponse;
	//exif_ptr->FocalPlaneXResolution;
	//exif_ptr->FocalPlaneYResolution;
	//exif_ptr->FocalPlaneResolutionUnit;
	//exif_ptr->SubjectLocation[2];
	//exif_ptr->ExposureIndex;
	//exif_ptr->SensingMethod;

	exif_ptr->valid.FileSource = 1;
	exif_ptr->FileSource = 0x03;

	//exif_ptr->SceneType;
	//exif_ptr->CFAPattern;
	//exif_ptr->CustomRendered;

	exif_ptr->valid.ExposureMode = 1;
	exif_ptr->ExposureMode = 0x00;

	exif_ptr->valid.WhiteBalance = 1;
	exif_ptr->WhiteBalance = 0x00;

	//exif_ptr->DigitalZoomRatio;
	//exif_ptr->FocalLengthIn35mmFilm;
	//exif_ptr->SceneCaptureType;
	//exif_ptr->GainControl;
	//exif_ptr->Contrast;
	//exif_ptr->Saturation;
	//exif_ptr->Sharpness;
	//exif_ptr->DeviceSettingDescription;
	//exif_ptr->SubjectDistanceRange;
#endif
	return SENSOR_SUCCESS;
}

/******************************************************************************/
// Description: get ov7670 rssolution trim tab
// Global resource dependence: 
// Author:
// Note:
/******************************************************************************/
LOCAL uint32_t _hi253_GetResolutionTrimTab(uint32_t param)
{
  	 return (uint32_t)s_HI253_Resolution_Trim_Tab;
}
LOCAL uint32_t set_hi253_ae_enable(uint32_t enable)
{
		if(0x00==enable)
		{
			HI253_WriteReg(0x03,0x20);
			HI253_WriteReg(0x10,0x1c);// AE Off
		}
		else if(0x01==enable)
		{
			HI253_WriteReg(0x03, 0x20);//page 3
			HI253_WriteReg(0x10, 0x9c);//ae on
		}
	
		SENSOR_PRINT("HI253_test set_ae_enable: enable = %d", enable);
	
		return 0;
}
#if 0
LOCAL uint32_t set_hi253_ae_awb_enable(uint32_t ae_enable, uint32_t awb_enable)
{
		if(0x00==ae_enable)
		{
			HI253_WriteReg(0x03,0x20);
			HI253_WriteReg(0x10,0x1c);// AE Off
		}
		else if(0x01==ae_enable)
		{
			HI253_WriteReg(0x03, 0x20);//page 3
			HI253_WriteReg(0x10, 0x9c);//ae on
		}
	
		if(0x00==awb_enable)
		{
			HI253_WriteReg(0x03,0x22);
			HI253_WriteReg(0x10,0x69);// AWB Off
		}
		else if(0x01==awb_enable)
		{
			HI253_WriteReg(0x03, 0x22);
			HI253_WriteReg(0x10, 0xe9); //AWB ON
		}        
	
		SENSOR_PRINT("HI253_test set_ae_awb_enable: ae=%d awb=%d", ae_enable, awb_enable);
		return 0;
}
#endif
#if 0
LOCAL uint32_t set_front_sensor_yrotate_HI253(void)
{
		uint8_t iFilpY;
		uint8_t ss = 0;
	
		HI253_WriteReg(0x03,0x00);
		iFilpY = HI253_ReadReg(0x11);
		ss = iFilpY & 0x02;
		if(ss == 0x02)
		{
			iFilpY = iFilpY & 0xfd;
		}
		else
		{
			iFilpY |= 0x02;
		}
		HI253_WriteReg(0x03,0x00);    
		HI253_WriteReg(0x11, iFilpY);
	
	    return 0;
}

LOCAL uint32_t set_back_sensor_yrotate_HI253(void)
{
		uint8_t iFilpY;
		uint8_t ss = 0;
	#if 1
		HI253_WriteReg(0x03,0x00);
		iFilpY = HI253_ReadReg(0x11);
		ss = iFilpY & 0x02;
		if(ss == 0x02)
		{
			iFilpY = iFilpY & 0xfd;
		}
		else
		{
			iFilpY |= 0x02;
		}
		HI253_WriteReg(0x03,0x00);    
		HI253_WriteReg(0x11, iFilpY);
	
	#endif
		return 0;
}
#endif

#if 0
LOCAL uint32_t set_hi253_hmirror_enable(uint32_t enable)
{
		if(enable)	
		{
			set_front_sensor_yrotate_HI253();
		}
		else	
		{
			set_back_sensor_yrotate_HI253();
		}
	
		SENSOR_PRINT("set_hi253_hmirror_enable: enable = %d", enable);		
		return 0;
}
#endif
#if 0
LOCAL uint32_t set_front_sensor_xrotate_HI253(void)
{
		uint8_t iFilpX,iHV_Mirror;
		uint8_t ss = 0;
	
		HI253_WriteReg(0x03,0x00);
		iHV_Mirror = HI253_ReadReg(0x11);
		ss = iHV_Mirror & 0x01;
		if(ss == 0x01)
		{
			iHV_Mirror = iHV_Mirror & 0xfe;
			iFilpX =  0x31;
		}
		else
		{
			iHV_Mirror |= 0x01;
			HI253_WriteReg(0x03,0x00);    
			HI253_WriteReg(0x11, iHV_Mirror);	
		}
		return 0;
}

LOCAL uint32_t set_back_sensor_xrotate_HI253(void)
{
		uint8_t iFilpX,iHV_Mirror;
		uint8_t ss = 0;	
	
		HI253_WriteReg(0x03,0x00);
		iHV_Mirror = HI253_ReadReg(0x11);
		ss = iHV_Mirror & 0x01;
		if(ss == 0x01)
		{
			iHV_Mirror = iHV_Mirror & 0xfe;
			iFilpX =  0x31;
		}
		else
		{
			iHV_Mirror |= 0x01;
		}
	
		HI253_WriteReg(0x03,0x00);    
		HI253_WriteReg(0x11, iHV_Mirror);
	
		return 0;
}
#endif
#if 0
LOCAL uint32_t set_hi253_vmirror_enable(uint32_t enable)
{
		if(enable)	
		{
			set_front_sensor_xrotate_HI253();
		}
		else	
		{
			set_back_sensor_xrotate_HI253();
		}
		SENSOR_PRINT("set_hi253_vmirror_enable: enable = %d", enable);
		return 0;
}
#endif
/******************************************************************************/
// Description: set brightness 
// Global resource dependence: 
// Author:
// Note:
//		level  must smaller than 8
/******************************************************************************/
const SENSOR_REG_T HI253_brightness_tab[][3]=
{
    {
    {0x03,0x10}, //-3
    {0x40,0xb0}, 
    {0xff,0xff},
    },
    {
    {0x03,0x10}, //-2
    {0x40,0xa0}, 
    {0xff,0xff},
    },
    {
    {0x03,0x10}, //-1
    {0x40,0x90}, 
    {0xff,0xff},
    },
    {
    {0x03,0x10}, //0
    {0x40,0x00}, 
    {0xff,0xff},
    },
    {
    {0x03,0x10}, // 1
    {0x40,0x10}, 
    {0xff,0xff},
    },
    {
    {0x03,0x10}, // 2
    {0x40,0x20}, 
    {0xff,0xff},
    },
    {
    {0x03,0x10}, // +3
    {0x40,0x30}, 
    {0xff,0xff},
    },

};


LOCAL uint32_t HI253_set_brightness(uint32_t level)
{
		if(level > 6)
		{
			return 0;
		}
		
		SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)HI253_brightness_tab[level];
		
		SENSOR_ASSERT(PNULL != sensor_reg_ptr);
		
		HI253_Write_Group_Regs(sensor_reg_ptr);
		SENSOR_PRINT("HI253_test set_hi253_brightness_tab: level = %d", level);
		return 0;
}


const SENSOR_REG_T HI253_contrast_tab[][3]=
{
    {
    {0x03,0x10},  ///-3
    {0x48,0x80}, 
    {0xff,0xff}
    },
    {
    {0x03,0x10},  ///-2
    {0x48,0x60}, 
    {0xff,0xff}
    },
    {
    {0x03,0x10},  ///-1
    {0x48,0x70}, 
    {0xff,0xff}
    },	 
    {
    {0x03,0x10},  ///0
    {0x48,0x80}, // 80
    {0xff,0xff}
    },
    {
    {0x03,0x10},  ///1
    {0x48,0x90}, 
    {0xff,0xff}
    },
    {
    {0x03,0x10},  ///2
    {0x48,0xb0}, 
    {0xff,0xff}
    },
    {
    {0x03,0x10},  ///3
    {0x48,0xc0}, 
    {0xff,0xff}
    },
};

LOCAL void HI253_Write_Group_Regs( SENSOR_REG_T* sensor_reg_ptr )
{
		uint32_t i;
		
		for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
		{
		    Sensor_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
		}

}

LOCAL uint32_t HI253_set_contrast(uint32_t level)
{

		if(level > 6)
			return 0;
		
		SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)HI253_contrast_tab[level];
		
		SENSOR_ASSERT(PNULL != sensor_reg_ptr);
		
		HI253_Write_Group_Regs(sensor_reg_ptr);
		
		Sensor_SetSensorExifInfo(SENSOR_EXIF_CTRL_CONTRAST, (uint32) level);
		
		SENSOR_PRINT("HI253_test tianxiaohui set_hi253_contrast_tab: level = %d\n", level);
    return 0;
}

#if 0
LOCAL uint32_t HI253_set_sharpness(uint32_t level)
{	
	return 0;
}

LOCAL uint32_t HI253_set_saturation(uint32_t level)
{	
	return 0;
}
#endif
/******************************************************************************/
// Description: set brightness 
// Global resource dependence: 
// Author:
// Note:
//		level  must smaller than 8
/******************************************************************************/
LOCAL uint32_t set_hi253_preview_mode(uint32_t  preview_mode)
{
		SENSOR_PRINT("HI253_test set_hi253_preview_mode: preview_mode = %d", preview_mode);
		
		switch (preview_mode)
		{
			case SENSOR_PARAM_ENVIRONMENT_NORMAL: 
			{
				HI253_set_work_mode(0);
				break;
			}
			case SENSOR_PARAM_ENVIRONMENT_NIGHT:
			{
				HI253_set_work_mode(1);
				break;
			}
			case SENSOR_PARAM_ENVIRONMENT_SUNNY:
			{
				HI253_set_work_mode(0);
				break;
			}
				case SENSOR_PARAM_ENVIRONMENT_SPORTS:
			{
				HI253_set_work_mode(0);
				break;
			}
				case SENSOR_PARAM_ENVIRONMENT_LANDSCAPE:
			{
				HI253_set_work_mode(0);
				break;
			}
			default:
			{
				HI253_set_work_mode(0);
				break;
			}
				
		}

		
		return 0;
}



/******************************************************************************/
// Description:
// Global resource dependence: 
// Author:
// Note:
//		
/******************************************************************************/
const SENSOR_REG_T HI253_image_effect_tab[][7]=
{
    {
    {0x03,0x10},   //default
    {0x11,0x03},
    {0x12,0X30},						   
    {0x13,0x0a},
    //{0x40,0x20},
    {0x44,0x80},	
    {0x45,0x80},	
    {0xff,0xff},
    },
    {
    {0x03,0x10},	//gray						
    {0x11,0x03},	 
    {0x12,0x33},
    {0x13,0x02},	 
    {0x44,0x80},					   
    {0x45,0x80},					 	   
    {0xff,0xff},
    },
    {
    {0x03,0x10},   //  red						   
    {0x11,0x03},   
    {0x12,0x33},
    {0x13,0x02},
    {0x44,0x70},					   
    {0x45,0xb8},				   	 
    {0xff,0xff},
    },
    {
    {0x03,0x10},	// green									  
    {0x11,0x03},//embossing effect off 						
    {0x12,0x33},//auto bright on								
    {0x13,0x22},//binary effect off/solarization effect on 	
    {0x44,0x30},//ucon 										
    {0x45,0x50},//vcon 
    {0xff,0xff},
    },
    {
    {0x03,0x10},// blue								  
    {0x11,0x03},//embossing effect off 						
    {0x12,0x33},//auto bright on								
    {0x13,0x22},//binary effect off/solarization effect on 	
    {0x44,0xb0},//ucon 										
    {0x45,0x40},//vcon 
    {0xff,0xff},
    },
    {
    {0x03,0x10},// yellow								  
    {0x11,0x03},//embossing effect off 						
    {0x12,0x33},//auto bright on								
    {0x13,0x22},//binary effect off/solarization effect on 	
    {0x44,0x10},//ucon 										
    {0x45,0x98},//vcon 
    {0xff,0xff},
    },	  
    {
    {0x03,0x10},	// colorinv										   
    {0x11,0x03},//embossing effect off 						
    {0x12,0x38},//auto bright on/nagative effect on			
    {0x13,0x02},//binary effect off													
    {0x44,0x80},//ucon 										
    {0x45,0x80},//vcon  
    {0xff,0xff},
    },
    {
    {0x03,0x10},//sepia							  
    {0x11,0x03},//embossing effect off 						
    {0x12,0x33},//auto bright on								
    {0x13,0x22},//binary effect off/solarization effect on 	
    {0x44,0x40},//ucon  40										
    {0x45,0xa8},//vcon 	   	 
    {0xff,0xff},
    },	  
};


LOCAL uint32_t HI253_set_image_effect(uint32_t effect_type)
{
    if(effect_type > 7)
    	return 0;
	  SENSOR_PRINT("HI253_test SENSOR: set_image_effect: effect_type_start = %d\n", effect_type);
		SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)HI253_image_effect_tab[effect_type];
	
		SENSOR_ASSERT(PNULL != sensor_reg_ptr);
	
		HI253_Write_Group_Regs(sensor_reg_ptr);
		SENSOR_PRINT("HI253_test SENSOR: set_image_effect: effect_type_end = %d\n", effect_type);
	
		return 0;
}


LOCAL uint32_t HI253_After_Snapshot (uint32_t param)
{
		#if 1 
		#define PLL_ADDR_    0x0e
		
			SENSOR_PRINT("HI253_After_Snapshot ");  
		#if 0
			set_hi253_ae_awb_enable(0x01, 0x01);
		
			HI253_WriteReg(PLL_ADDR_, 0x73); 
		#endif 
		
			//HI253_WriteReg(PLL_ADDR_, 0x73);  //tianxiaohui  
			msleep(200); // wait 2 frame the sensor working normal if no delay the lum is incorrect
		#endif
			return 0;    
}

LOCAL uint32_t HI253_Before_Snapshot(uint32_t param)
{      
		/*modify begin tianxiaohui 2012-03-30*/
		if(g_flash_mode_en) //tianxiaohui
		{
			//Sensor_SetFlash(0);
			Sensor_SetFlash(1);
			g_flash_mode_en = 0;
		}
		/*modify end tianxiaohui 2012-03-30*/
		
		SENSOR_PRINT("HI253_test sensor:HI253_Before_Snapshot"); 
	
		if(SENSOR_MODE_SNAPSHOT_ONE_SECOND == param)
		{   
			Sensor_SetMode(param);         
		} 
		if(SENSOR_MODE_SNAPSHOT_ONE_FIRST == param)
		{  
			Sensor_SetMode(param);
		}	
		else
		{
			SENSOR_PRINT("HI253_test HI253_Before_Snapshot:dont set any");
		}
		msleep(200);
		SENSOR_PRINT("HI253_test sensor:HI253_Before_Snapshot2"); 
	
		return 0;
}
#if 0
LOCAL uint32_t HI253_read_ev_value(uint32_t value)
{
	return 0;
}

LOCAL uint32_t HI253_write_ev_value(uint32_t exposure_value)
{	
	return 0;	
}

LOCAL uint32_t HI253_read_gain_value(uint32_t value)
{	
	return 0;
}

LOCAL uint32_t HI253_write_gain_value(uint32_t gain_value)
{	
	return 0;
}

LOCAL uint32_t HI253_read_gain_scale(uint32_t value)
{
	return SENSOR_GAIN_SCALE;	
}

LOCAL uint32_t HI253_set_frame_rate(uint32_t param)
{	
	return 0;
}
#endif
/******************************************************************************/
// Description:
// Global resource dependence: 
// Author:
// Note:
//		mode 0:normal;	 1:night 
/******************************************************************************/
const SENSOR_REG_T HI253_mode_tab[][9]=
{
		{
			{0x03, 0x20},
			{0x10, 0x1c},
			{0x18, 0x38},
			{0x88, 0x04},
			{0x89, 0x93},
			{0x8a, 0xe0},    
			{0x10, 0x9c},
			{0x18, 0x30},
			{0xff, 0xff}  
		} , // 30fps 24M normal
		{
			{0x03, 0x20},
			{0x10, 0x1c},
			{0x18, 0x38},
			{0x88, 0x09},
			{0x89, 0x27},
			{0x8a, 0xc0},	 
			{0x10, 0x9c},
			{0x18, 0x30},
			{0xff, 0xff}
		} , // 6fps 24M normal 
};

LOCAL uint32_t HI253_set_work_mode(uint32_t mode)
{
		uint16_t i;
		SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)HI253_mode_tab[mode];
    SENSOR_ASSERT(mode <= 1);
    SENSOR_ASSERT(PNULL != sensor_reg_ptr);
		if(mode > 1)
		{
			SENSOR_PRINT("HI253_test HI253_set_work_mode:param error,mode=%d .\n",mode);
			return SENSOR_FAIL;
		}
		
		for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value); i++)
		{
			Sensor_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
		}

	Sensor_SetSensorExifInfo(SENSOR_EXIF_CTRL_SCENECAPTURETYPE, (uint32) mode);

		SENSOR_PRINT("HI253_test sensor:HI253_set_work_mode: mode = %d", mode);
		return 0;
		
}

const SENSOR_REG_T HI253_WB_mode_tab[][11]=
{
	    //AUTO
	    {
		{0x03, 0x22},
		{0x80, 0x38}, //37
		{0x82, 0x38}, // 		
		{0x83, 0x5e},
		{0x84, 0x1e}, //26 //22 //0x24 --> 0x20 (by ��å�Ӵ�, 20100513)
		{0x85, 0x5e}, //54 //59 //55
		{0x86, 0x25}, //0x20 --> 0x15 (by ��å�Ӵ�, 20100513)
		{0xff, 0xff}            
	    },    
	    //INCANDESCENCE:
	    {
	        
		{0x03, 0x22},
		{0x80, 0x2a},
		{0x82, 0x3f},
		{0x83, 0x35},
		{0x84, 0x28},
		{0x85, 0x45},
		{0x86, 0x3b},        
		{0xff, 0xff}         
	    },
	    //U30	not used
	    {
		{0x03, 0x22},
		{0x10, 0x7B},
		{0x80, 0x33},
		{0x81, 0x20},
		{0x82, 0x3D},
		{0x83, 0x2E},
		{0x84, 0x24},
		{0x85, 0x43},
		{0x86, 0x3D}, 
		{0x10, 0x7B},       
		{0xff, 0xff}            
	    },      
	    //FLUORESCENT:
	    {
		{0x03, 0x22},
		{0x80, 0x20},
		{0x82, 0x4d},
		{0x83, 0x25},
		{0x84, 0x1b},
		{0x85, 0x55},
		{0x86, 0x48},        
		{0xff, 0xff}          
	    },
	    //SUN:
	    {
		{0x03, 0x22},
		{0x80, 0x3d},
		{0x82, 0x2e},
		{0x83, 0x40},
		{0x84, 0x33},
		{0x85, 0x33},
		{0x86, 0x28},
		{0xff, 0xff}            
	    },
	    //CLOUD:
	    {
		{0x03, 0x22},
		{0x80, 0x50},
		{0x82, 0x25},
		{0x83, 0x55},
		{0x84, 0x4b},
		{0x85, 0x28},
		{0x86, 0x20},        
		{0xff, 0xff}            
	    }
};

LOCAL uint32_t HI253_set_whitebalance_mode(uint32_t mode )
{

  	if(mode>6)
  		return 0;
	
		SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)HI253_WB_mode_tab[mode];
	
		SENSOR_ASSERT(PNULL != sensor_reg_ptr);
	
		HI253_Write_Group_Regs(sensor_reg_ptr);

	Sensor_SetSensorExifInfo(SENSOR_EXIF_CTRL_LIGHTSOURCE, (uint32) mode);
	Sensor_SetSensorExifInfo(SENSOR_EXIF_CTRL_WHITEBALANCE, (uint32) mode);

		SENSOR_PRINT("HI253_test SENSOR: set_awb_mode: mode = %d", mode);
	
		return 0;
}
/******************************************************************************/
// Description: set video mode
// Global resource dependence: 
// Author:
// Note:
//		 
/******************************************************************************/
const SENSOR_REG_T HI253_video_mode_nand_tab[][32]=
{
	 {
		{0x01, 0xc1}, //sleep off	
		{0x03, 0x00},
		{0x10, 0x11}, //0401//0x00}
		{0x11, 0x94},
	
		{0x03, 0x20}, //Page 20
		{0x10, 0x1c},
		{0x2a, 0x03}, //for fix//0xff);
		{0x2b, 0x35}, //for fix//0x04); 
		{0x30, 0x78},
	
		{0x83, 0x01},
		{0x84, 0x5f},
		{0x85, 0x00},
	
		{0x88, 0x03},
		{0x89, 0xa8},
		{0x8a, 0x00}, 
	
		{0x91, 0x03}, //EXP Fix 7.5 fps
		{0x92, 0xd0}, 
		{0x93, 0x00},
		
		{0x03, 0x20}, //Page 20
		{0x10, 0x9c},
	
		{0x03, 0x00},//dummy1    
		{0x03, 0x00},//dummy2        
		{0x03, 0x00},//dummy3
		{0x03, 0x00},//dummy4    
		{0x03, 0x00},//dummy5        
		{0x03, 0x00},//dummy6   
		{0x03, 0x00},//dummy7    
		{0x03, 0x00},//dummy8        
		{0x03, 0x00},//dummy9
		{0x03, 0x00},//dummy10    
		
		{0x01, 0xc0},//dummy10    
		{0xff, 0xff}        
	 } , // 15fps pclk=24M  normal
	 {
		{0x01, 0xc1}, //sleep off	
		{0x03, 0x00},
		{0x10, 0x11}, //0401//0x00}
		{0x11, 0x94},
	
		{0x03, 0x20}, //Page 20
		{0x10, 0x1c},
		{0x2a, 0x03}, //for fix//0xff);
		{0x2b, 0x35}, //for fix//0x04); 
		{0x30, 0x78},
	
		{0x83, 0x01},
		{0x84, 0x5f},
		{0x85, 0x00},
	
		{0x88, 0x06},
		{0x89, 0x66},
		{0x8a, 0x00}, 
	
		{0x91, 0x06}, //EXP Fix 7.5 fps
		{0x92, 0x8a}, 
		{0x93, 0x00},
		
		{0x03, 0x20}, //Page 20
		{0x10, 0x9c},
	
		{0x03, 0x00},//dummy1    
		{0x03, 0x00},//dummy2        
		{0x03, 0x00},//dummy3
		{0x03, 0x00},//dummy4    
		{0x03, 0x00},//dummy5        
		{0x03, 0x00},//dummy6   
		{0x03, 0x00},//dummy7    
		{0x03, 0x00},//dummy8        
		{0x03, 0x00},//dummy9
		{0x03, 0x00},//dummy10    
		
		{0x01, 0xc0},//dummy10    
		{0xff, 0xff}        
	 }
};  

LOCAL uint32_t set_hi253_video_mode(uint32_t mode)
{
/*
		//uint8 data=0x00;
		uint16_t i;
		SENSOR_REG_T* sensor_reg_ptr = PNULL;
	
	  SENSOR_ASSERT(mode <= 1);
		if(mode > 1)
		{
			SENSOR_PRINT("set_hi253_video_mode:param error,mode=%d .\n",mode);
			return SENSOR_FAIL;
		}
	
		sensor_reg_ptr = (SENSOR_REG_T*)HI253_video_mode_nand_tab[mode];
	
	    SENSOR_ASSERT(PNULL != sensor_reg_ptr);
	
		for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value); i++)
		{
			Sensor_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
		}
	
		msleep(150);
	
		SENSOR_PRINT("HI253_test SENSOR: set_hi253_video_mode: mode = %d", mode);
		*/return 0;
		
}

const SENSOR_REG_T HI253_ev_tab[][3]=
{

    {
    {0x03,0x10},//-3
    {0x4a,0x50},
    {0xff,0xff},
    },

    {
    {0x03,0x10},//-2
    {0x4a,0x60},
    {0xff,0xff},
    },

    {
    {0x03,0x10},//-1
    {0x4a,0x70},
    {0xff,0xff},
    },

    {
    {0x03,0x10},//00	
    {0x4a,0x80},
    {0xff,0xff},
    },

    {
    {0x03,0x10},//01
    {0x4a,0x90},
    {0xff,0xff},
    },

    {
    {0x03,0x10},//02
    {0x4a,0xa0},
    {0xff,0xff},
    },

    {
    {0x03,0x10},//03
    {0x4a,0xb0},
    {0xff,0xff},
    }

};


LOCAL uint32_t set_hi253_ev(uint32_t level)
{
		if(level > 6)
		{
			return 0;
		}
		
		SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)HI253_ev_tab[level];
		
		SENSOR_ASSERT(PNULL != sensor_reg_ptr);
		
		HI253_Write_Group_Regs(sensor_reg_ptr);
		SENSOR_PRINT("HI253_test SENSOR: set_hi253_EV_tab: level = %d", level);
    return 0;
}

/*modify begin tianxiaohui 2012-03-30*/
LOCAL uint32_t HI253_flash(uint32_t param)
{
	printk("HI253_flash:param=%d .\n",param);

	/* enable flash, disable in HI253_After_Snapshot */
	g_flash_mode_en = param;
	
	Sensor_SetFlash(param);

	printk("HI253_flash:end .\n");
}
/*modify end tianxiaohui 2012-03-30*/
struct sensor_drv_cfg sensor_hi253 = {
        .sensor_pos = CONFIG_DCAM_SENSOR_POS_HI253,
        .sensor_name = "hi253",
        .driver_info = &g_hi253_yuv_info,
};

static int __init sensor_hi253_init(void)
{
        return dcam_register_sensor_drv(&sensor_hi253);
}

subsys_initcall(sensor_hi253_init);
