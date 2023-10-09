
#ifndef __ADC_H
#define __ADC_H

#include "macrodriver.h"
#include "userdefine.h"
#include "tau.h"


#pragma interrupt r_adc_interrupt(vect=INTAD)

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/
/*
    Analog input channel specification register (ADS) 
*/
/* Specification of analog input channel (ADISS,ADS4,ADS3,ADS2,ADS1,ADS0) */
#define _00_AD_INPUT_CHANNEL_0                  (0x00U) /* ANI0 */
#define _01_AD_INPUT_CHANNEL_1                  (0x01U) /* ANI1 */
#define _02_AD_INPUT_CHANNEL_2                  (0x02U) /* ANI2 */
#define _03_AD_INPUT_CHANNEL_3                  (0x03U) /* ANI3 */
#define _10_AD_INPUT_CHANNEL_16                 (0x10U) /* ANI16 */
#define _11_AD_INPUT_CHANNEL_17                 (0x11U) /* ANI17 */
#define _12_AD_INPUT_CHANNEL_18                 (0x12U) /* ANI18 */
#define _13_AD_INPUT_CHANNEL_19                 (0x13U) /* ANI19 */
#define _14_AD_INPUT_PGAO                       (0x14U) /* PGAO is used to be the input channel */
#define _80_AD_INPUT_TEMPERSENSOR               (0x80U) /* temperature sensor output is used to be the input channel */
#define _81_AD_INPUT_INTERREFVOLT               (0x81U) /* internal reference voltage output is used to be the input channel */
#define _00_AD_INPUT_CHANNEL_0_3                (0x00U) /* ANI0 - ANI3 */
#define _10_AD_INPUT_CHANNEL_16_19              (0x10U) /* ANI16 - ANI19 */
#define _11_AD_INPUT_CHANNEL_17_PGAO            (0x11U) /* ANI17 - PGAO */

/*
    A/D converter mode register 0 (ADM0) 
*/
/* A/D conversion operation control (ADCS) */
#define _00_AD_CONVERSION_DISABLE               (0x00U) /* stops conversion operation */
#define _80_AD_CONVERSION_ENABLE                (0x80U) /* enables conversion operation */
/* Specification of the A/D conversion channel selection mode (ADMD) */
#define _00_AD_OPERMODE_SELECT                  (0x00U) /* select mode */
#define _40_AD_OPERMODE_SCAN                    (0x40U) /* scan mode */
/* A/D conversion clock selection (FR2,FR1,FR0) */
#define _00_AD_CONVERSION_CLOCK_64              (0x00U) /* fCLK/64 */
#define _08_AD_CONVERSION_CLOCK_32              (0x08U) /* fCLK/32 */
#define _10_AD_CONVERSION_CLOCK_16              (0x10U) /* fCLK/16 */
#define _18_AD_CONVERSION_CLOCK_8               (0x18U) /* fCLK/8 */
#define _20_AD_CONVERSION_CLOCK_6               (0x20U) /* fCLK/6 */
#define _28_AD_CONVERSION_CLOCK_5               (0x28U) /* fCLK/5 */
#define _30_AD_CONVERSION_CLOCK_4               (0x30U) /* fCLK/4 */
#define _38_AD_CONVERSION_CLOCK_2               (0x38U) /* fCLK/2 */
/* A/D conversion time mode selection (LV1,LV0) */
#define _00_AD_TIME_MODE_NORMAL_1               (0x00U) /* normal 1 mode */
#define _02_AD_TIME_MODE_NORMAL_2               (0x02U) /* normal 2 mode */
#define _04_AD_TIME_MODE_LOWVOLTAGE_1           (0x04U) /* low-voltage 1 mode */
#define _06_AD_TIME_MODE_LOWVOLTAGE_2           (0x06U) /* low-voltage 2 mode */
/* A/D voltage comparator operation control (ADCE) */
#define _00_AD_COMPARATOR_DISABLE               (0x00U) /* stops A/D voltage comparator operation */
#define _01_AD_COMPARATOR_ENABLE                (0x01U) /* enables A/D voltage comparator operation */

/*
    A/D converter mode register 1 (ADM1) 
*/
/* A/D conversion trigger mode selection (ADTMD1,ADTMD0) */
#define _00_AD_TRIGGER_SOFTWARE                 (0x00U) /* software trigger mode */
#define _80_AD_TRIGGER_HARDWARE_NOWAIT          (0x80U) /* hardware trigger mode (no-wait) */
#define _C0_AD_TRIGGER_HARDWARE_WAIT            (0xC0U) /* hardware trigger mode (wait) */
/* AD convertion mode selection (ADSCM) */
#define _00_AD_CONVMODE_SEQSELECT               (0x00U) /* sequential convertion mode */
#define _20_AD_CONVMODE_ONESELECT               (0x20U) /* one-shot convertion mode */
/* Hardware trigger signal selection (ADTRS1,ADTRS0) */
#define _00_AD_TRIGGER_INTTM01                  (0x00U) /* INTTM01 */
#define _01_AD_TRIGGER_ELC                      (0x01U) /* ELC */
#define _03_AD_TRIGGER_INTIT                    (0x03U) /* INTIT */

/*
    A/D converter mode register 2 (ADM2) 
*/
/* AD VREF(+) selection (ADREFP1,ADREFP0) */
#define _00_AD_POSITIVE_VDD                     (0x00U) /* use AVDD as VREF(+) */
#define _40_AD_POSITIVE_AVREFP                  (0x40U) /* use AVREFP as VREF(+) */
#define _80_AD_POSITIVE_INTERVOLT               (0x80U) /* use internal reference voltage as VREF(+) */
/* AD VREF(-) selection (ADREFM) */
#define _00_AD_NEGATIVE_VSS                     (0x00U) /* use AVSS as VREF(-) */
#define _20_AD_NEGATIVE_AVREFM                  (0x20U) /* use AVREFM as VREF(-) */
/* AD conversion result upper/lower bound value selection (ADRCK) */
#define _00_AD_AREA_MODE_1                      (0x00U) /* generates INTAD when ADLL <= ADCR <= ADUL */
#define _08_AD_AREA_MODE_2_3                    (0x08U) /* generates INTAD when ADUL < ADCR or ADLL > ADCR */
/* AD wakeup function selection (AWC) */
#define _00_AD_SNOOZE_OFF                       (0x00U) /* stop SNOOZE function */
#define _04_AD_SNOOZE_ON                        (0x04U) /* use SNOOZE function */
/* AD conversion resolution selection (ADTYP) */
#define _00_AD_RESOLUTION_10BIT                 (0x00U) /* 10 bits */
#define _01_AD_RESOLUTION_8BIT                  (0x01U) /* 8 bits */

/*
    A/D port configuration register (ADPC) 
*/
/* Analog I/O (A)/digital I/O (D) switching (ADPC3,ADPC2,ADPC1,ADPC0) */
#define _00_AD_ADPC_4ANALOG                     (0x00U) /* ANI0 - ANI3 */
#define _01_AD_ADPC_0ANALOG                     (0x01U) /* all digital */
#define _02_AD_ADPC_1ANALOG                     (0x02U) /* ANI0 */
#define _03_AD_ADPC_2ANALOG                     (0x03U) /* ANI0 - ANI1 */
#define _04_AD_ADPC_3ANALOG                     (0x04U) /* ANI0 - ANI2 */

/*
    AD test register (ADTES) 
*/
/* AD conversion target (ADTES1,ADTES0) */
#define _00_AD_NORMAL_INPUT                     (0x00U) /* normal mode (ANIxx/temperature sensor output voltage/internal reference voltage) */
#define _02_AD_TEST_AVREFM                      (0x02U) /* use AVREFM as test signal */
#define _03_AD_TEST_AVREFP                      (0x03U) /* use AVREFP as test signal */


/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define _FF_AD_ADUL_VALUE                       (0xFFU) 
#define _00_AD_ADLL_VALUE                       (0x00U) 

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef enum
{
    ADCHANNEL0,
    ADCHANNEL1,
    ADCHANNEL2,
    ADCHANNEL3,
    ADCHANNEL16 = 16U,
    ADCHANNEL17,
    ADCHANNEL18,
    ADCHANNEL19,
    ADPGAO,
    ADTEMPERSENSOR0 = 128U,
    ADINTERREFVOLT
} ad_channel_t;

typedef enum
{
    ADNORMALINPUT,
    ADAVREFM = 2U,
    ADAVREFP
} test_channel_t;

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_ADC_Create(void);
void R_ADC_Start(void);
void R_ADC_Stop(void);
void R_ADC_Set_OperationOn(void);
void R_ADC_Set_OperationOff(void);
void R_ADC_Get_Result(uint16_t * const buffer);
uint16_t AdcSampSingleChannelOnce(uint8_t eChannel);
void Disp_LowSpeedADCInterrupt(void);
void Disp_MidSpeedADCInterrupt(void);
void Disp_MidHighSpeedADCInterrupt(void);
void Disp_HighSpeedADCInterrupt(void);






#endif
