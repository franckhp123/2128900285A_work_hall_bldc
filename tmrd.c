
#include "tmrd.h"
#include "macrodriver.h"
#include "userdefine.h"

/***********************************************************************************************************************
 * Function Name: R_TMRD0_Create
 * Description  : This function initializes the TMRD0 module.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
void R_TMRD0_Create(void)
{
    TRD0EN = 1U; // 允许提供定时器RD输入时钟

    // CSEL1=1在和TRDGRA1寄存器比较匹配后还继续计数;CSEL0=1在和TRDGRA0寄存器比较匹配后还继续计数;
    TRDSTR |= _08_TMRD_TRD1_COUNT_CONTINUES | _04_TMRD_TRD0_COUNT_CONTINUES;

    // TSTART1=0:TRD1停止计数;TSTART0=0:TRD0停止计数
    TRDSTR &= (uint8_t) ~(_02_TMRD_TRD1_COUNT_START | _01_TMRD_TRD0_COUNT_START);

    TRDMK0 = 1U; // 禁止TRD0中断 INTTRD0
    TRDIF0 = 0U; // 清TRD0中断标志位

    TRDMK1 = 1U; // 禁止TRD1中断 INTTRD1
    TRDIF1 = 0U; // 清TRD0中断标志位

    TRDPR10 = 0U; // 设置INTTRD0优先级为1
    TRDPR00 = 1U;

    TRDPR11 = 0U; // 设置INTTRD1优先级为1
    TRDPR01 = 1U;

    // ELC事件输入0(ELCOBE0)允许强制截止定时器RD的脉冲输出; ELC事件输入1(ELCOBE1)允许强制截止定时器RD的脉冲输出;
    TRDELC = _02_TMRD0_CUTOFF_ENABLED | _20_TMRD1_CUTOFF_ENABLED;

    /* regset TRDMR ContentOnly=[4-7] */
    // TRDGRD1是TRDGRB1的buf;TRDGRC1是TRDGRA1的buf;TRDGRD0是TRDGRB0的buf;TRDGRC0是TRDGRA0的buf;TRD0和TRD1独立运行
    TRDMR = _80_TMRD_TRDGRD1_BUFFER | _40_TMRD_TRDGRC1_BUFFER | _20_TMRD_TRDGRD0_BUFFER | _10_TMRD_TRDGRC0_BUFFER;

    // 选择互补PWM 模式（在TRD0 和TRDGRA0 寄存器比较匹配时，将数据从缓冲寄存器传送到通用寄存器）;正向电平输出:初始输出L,高电平有效;反向电平输出:初始输出L,高电平有效
    TRDFCR |= _03_TMRD_TRANSFER_TMRD0_MATCH | _04_TMRD_NORMAL_PHASE_LEVEl_LH | _08_TMRD_COUNTER_PHASE_LEVEl_LH;

    // 输出禁止(TRDIOA0引脚为I/O);输出禁止(TRDIOC0引脚为I/O);
    // TRDIOB0(WH)允许输出;TRDIOD0(WL)允许输出;TRDIOA1(VH)允许输出;
    // TRDIOB1(UH)允许输出;TRDIOC1(VL)允许输出;TRDIOD1(UL)允许输出;
    TRDOER1 = _01_TMRD_TRDIOA0_OUTPUT_DISABLE | _00_TMRD_TRDIOB0_OUTPUT_ENABLE | _04_TMRD_TRDIOC0_OUTPUT_DISABLE |
              _00_TMRD_TRDIOD0_OUTPUT_ENABLE | _00_TMRD_TRDIOA1_OUTPUT_ENABLE | _00_TMRD_TRDIOB1_OUTPUT_ENABLE |
              _00_TMRD_TRDIOC1_OUTPUT_ENABLE | _00_TMRD_TRDIOD1_OUTPUT_ENABLE;

    // 定时器 RD 输出控制寄存器:初始输出L电平
    TRDOCR = _00_TMRD_TRDIOC0_INITIAL_OUTPUT_L;

    // 定时器RD0数字滤波器功能选择寄存器配置:禁止强制截止
    TRDDF0 = _00_TMRD_TRDIOB_FORCEDCUTOFF_DISABLE | _00_TMRD_TRDIOD_FORCEDCUTOFF_DISABLE;
    // 定时器RD1数字滤波器功能选择寄存器配置:禁止强制截止
    TRDDF1 = _00_TMRD_TRDIOA_FORCEDCUTOFF_DISABLE | _00_TMRD_TRDIOB_FORCEDCUTOFF_DISABLE |
             _00_TMRD_TRDIOC_FORCEDCUTOFF_DISABLE | _00_TMRD_TRDIOD_FORCEDCUTOFF_DISABLE;

    // 禁止清除（自由运行）;计数源选择fclk(24M)或fHOCO(48M)当FRQSEL4=1选fHOCO
    TRDCR0 = _00_TMRD_COUNTER_CLEAR_DISABLE | _00_TMRD_INETNAL_CLOCK_FCLK_FHOCO;
    // // 计数源选择fclk(24M)或fHOCO(48M)当FRQSEL4=1选fHOCO
    // TRDCR1 = _00_TMRD_INETNAL_CLOCK_FCLK_FHOCO;

    /* regset TRDIER0 */
    // TRDIER0 = _02_TMRD_IMIB_ENABLE | _01_TMRD_IMIA_ENABLE;
    TRDIER0 = _02_TMRD_IMIB_ENABLE;
    /* regset TRDIER1 */
    // TRDIER1 = _10_TMRD_OVIE_ENABLE | _02_TMRD_IMIB_ENABLE | _01_TMRD_IMIA_ENABLE;

    /*----------------------------------------------------*/
    // 设置pwm频率
    TRD0 = SET_DEATH_TIME_VALUE;//设置死区时间

    TRDGRA0 = SET_FREQUENCY_PWM_VALUE;//设置pwm频率
    //设置通道初始值
    TRDGRB0 = SET_FREQUENCY_PWM_VALUE;
    TRDGRC0 = SET_FREQUENCY_PWM_VALUE;
    TRDGRD0 = SET_FREQUENCY_PWM_VALUE;
    TRDGRA1 = SET_FREQUENCY_PWM_VALUE;
    TRDGRB1 = SET_FREQUENCY_PWM_VALUE;
    TRDGRC1 = SET_FREQUENCY_PWM_VALUE;
    TRDGRD1 = SET_FREQUENCY_PWM_VALUE;

    /*----------------------------------------------------*/
    // 设置输出io
    POM1 &= 0xDFU; // 0b1101 1111普通输出模式 P1.5(WH)
    P1 &= 0xDFU;   // P1.5=0
    PM1 &= 0xDFU;  // P1.5输出口
    /* Set TRDIOD0 pin */
    P1 &= 0xEFU;  // P1.4=0
    PM1 &= 0xEFU; // P1.4(WL)输出口
    /* Set TRDIOA1 pin */
    P1 &= 0xF7U;  // P1.3=0
    PM1 &= 0xF7U; // P1.3(VH)输出口
    /* Set TRDIOB1 pin */
    P1 &= 0xFBU;  // P1.2=0
    PM1 &= 0xFBU; // P1.2(UH)输出口
    /* Set TRDIOC1 pin */
    P1 &= 0xFDU;  // P1.1=0
    PM1 &= 0xFDU; // P1.1(VL)输出口
    /* Set TRDIOD1 pin */
    POM1 &= 0xFEU; // P1.0(UL)普通输出模式
    P1 &= 0xFEU;   // P1.0=0
    PM1 &= 0xFEU;  // P1.0输出口
}
/***********************************************************************************************************************
 * Function Name: R_TMRD0_Start
 * Description  : This function starts TMRD0 counter.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
void R_TMRD0_Start(void)
{
    volatile uint8_t trdsr_dummy;

    /* intpregbitset INTTRD0 intp=IF,bitvalue=0 */
    TRDIF0 = 0U; /* clear INTTRD0 interrupt flag */
    /* intpregbitset INTTRD1 intp=IF,bitvalue=0 */
    TRDIF1 = 0U;          /* clear INTTRD1 interrupt flag */
    trdsr_dummy = TRDSR0; /* read TRDSR0 before write 0 */
    TRDSR0 = 0x00U;       /* clear TRD0 each interrupt request */
    trdsr_dummy = TRDSR1; /* read TRDSR1 before write 0 */
    TRDSR1 = 0x00U;       /* clear TRD1 each interrupt request */
    /* intpregbitset INTTRD0 intp=MK,bitvalue=0 */
    TRDMK0 = 0U; /* enable INTTRD0 interrupt */
    /* intpregbitset INTTRD1 intp=MK,bitvalue=0 */
    TRDMK1 = 0U; /* enable INTTRD1 interrupt */
    /* regset CSEL1|CSEL0 OPMode=byvalue */
    TRDSTR |= _08_TMRD_TRD1_COUNT_CONTINUES | _04_TMRD_TRD0_COUNT_CONTINUES;
    /* regset TSTART0=1|TSTART1=1 OPMode=OrEqual */
    TRDSTR |= _02_TMRD_TRD1_COUNT_START | _01_TMRD_TRD0_COUNT_START;
}
/***********************************************************************************************************************
 * Function Name: R_TMRD0_Stop
 * Description  : This function stops TMRD0 counter.
 * Arguments    : None
 * Return Value : None
 ***********************************************************************************************************************/
void R_TMRD0_Stop(void)
{
    volatile uint8_t trdsr_dummy;

    /* regset CSEL1=1|CSEL0=1 OPMode=byvalue */
    TRDSTR |= _08_TMRD_TRD1_COUNT_CONTINUES | _04_TMRD_TRD0_COUNT_CONTINUES;
    /* regset TSTART0=0|TSTART1=0 opmode=byvalue */
    TRDSTR &= (uint8_t) ~(_02_TMRD_TRD1_COUNT_START | _01_TMRD_TRD0_COUNT_START);
    /* intpregbitset INTTRD0 disable */
    TRDMK0 = 1U; /* disable INTTRD0 interrupt */
    TRDIF0 = 0U; /* clear INTTRD0 interrupt flag */
    /* intpregbitset INTTRD1 disable */
    TRDMK1 = 1U;          /* disable INTTRD1 interrupt */
    TRDIF1 = 0U;          /* clear INTTRD1 interrupt flag */
    trdsr_dummy = TRDSR0; /* read TRDSR0 before write 0 */
    TRDSR0 = 0x00U;       /* clear TRD0 each interrupt request */
    trdsr_dummy = TRDSR1; /* read TRDSR1 before write 0 */
    TRDSR1 = 0x00U;       /* clear TRD1 each interrupt request */
}
