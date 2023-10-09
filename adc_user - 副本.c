
#include "adc.h"  
#include "GlobalVar.h"  
#include "sau.h"

//AD采样(10位/1024):单次选择等待转化完成,非中断方式
u16 AdcSampSingleChannelOnce(u8 eChannel)
{
   ADS = eChannel;
   ADIF = 0;        //清INTAD中断标志   
   ADCS = 1;        //允许AD转换运行
   while(!ADIF);
   ADIF = 0;
   ADCS = 0;        //禁止AD转换

   return(ADCR>>6); //转换结果存贮在ADCR高10位  
}


/***********************************************************************************************************************
* Function Name: r_adc_interrupt
* Description  : None
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_adc_interrupt(void)       
{
   switch(g_u8FluxToBemfFlag) //速度判断  单个AD测试时间5.8us  
   {
      case 0:  //低速:磁链换相+反电势过零检测:3次过零→→→↓ 
	  	     //Disp_LowSpeedADCInterrupt();    
	  	        g_u16ADCBuffer[g_u8AdcIndex] = (u16)(ADCR>>4U); //读取高10位,TMRD中断中触发AD采样前已配置Index=1通道为反电势通道
				if(g_u8AdcIndex < 5)        					 
				{
				   g_u8AdcIndex++;                         
				   if(g_u8AdcIndex == 2)	  { ADS = eaAdFifoChannelBuf[1]; ADIF = 0U; } //反电势相电压	
				   //if(g_u8AdcIndex == 2)	  { ADS = eaAdFifoChannelBuf[2]; ADIF = 0U; } //导通相相电压
				   else if(g_u8AdcIndex == 3) { ADS = IPeak_ADCH; ADIF = 0U;  } //母线电流	 
				   else if(g_u8AdcIndex == 4) { ADS = BusVdc_ADCH; ADIF = 0U; } //母线电压(可在PWM OFF时采集)  
				   else if(g_u8AdcIndex == 5) { ADS = eaComAdChannel[g_u8CommAdcIndex]; ADIF = 0U; } //采集公共通道模拟量数据		  
				}
				else  //ADC采完了两组数据 每一组进4次中断两组读完后得到总结果总耗时27US(总共进入8次中断)
				{
				   R_ADC_Stop();
				   uFluxAdRes.u16Bemf		= g_u16ADCBuffer[2];	//获得反电势电压AD(放大4倍)结果为12位AD:对应电压为3.3*AD/4096 
				   uFluxAdRes.u16Conduction = g_u16ADCBuffer[1];	//获得导通相电压AD(放大4倍)结果为12位AD:对应电压为3.3*AD/4096 
				   uFluxAdRes.u16PhaseCurr	= g_u16ADCBuffer[3];	//获得峰值电流结果为12位AD(内部运放32倍):对应电压为3.3*AD/4096	  
				   g_sPeakIdc.Ins			= g_u16ADCBuffer[3]>>2; //用于过流保护	 
				   g_u16L_dIs_dt			= g_u16ADCBuffer[4]<<1; //母线电压实际值/3(结果放大1024倍):(AD*3.3/4096)*7.6(分压比1.5k:10K)/3*1024
				   g_sBusVdc.Ins			= g_u16ADCBuffer[4]>>2; //换算成10位AD(1024)采样	

				   uBemfAdRes.u16Conduction = uFluxAdRes.u16Conduction>>2; //导通相转换为1024格式  
				   uBemfAdRes.aBemf[0]      = uFluxAdRes.u16Bemf>>2;       //反电势转换为1024格式
				   g_u16ZeroCompareVal      = uBemfAdRes.u16Conduction>>1; //得出比较电压,母线电压一半   
				   
				   if(g_u8CommAdcIndex == 0) //循环通道用于公共通道数据
				   { g_sMosTmp.Ins = g_u16ADCBuffer[5]>>2; g_u8CommAdcIndex++; }			 
				   else if(g_u8CommAdcIndex == 1)
				   { g_u8CommAdcIndex++; }	 //后续再定义读取数据	 
				   else if(g_u8CommAdcIndex == 2)
				   { g_u8CommAdcIndex = 0; }               

                   #if 0
				   //Debug 强拉   
				   if(++g_sMotor.u8TimeCntForceChgPhase >= 80) //2对级8msDebug OK  
				   {  
				      g_sMotor.u8TimeCntForceChgPhase = 0;              
					  BLDC_ForceChangePhase(&g_sMotor); //强拉换相             
				   }
                   #endif

				   #if 1   
				   //磁链+强拉    
				   BLDC_FluxCalcuCommutation(&g_sMotor,&uFluxAdRes); //耗时11.8us磁链运算换相                	  
				   if(g_bCommutationOK)  //磁链换相成功
				   { g_sMotor.u8TimeCntFluxToForce = 0; }     //清磁链切强拉间隔时间(两对级:100ms)  
				   else //磁链换相不成功,100ms切强拉      											 
				   {
                      if(++g_sMotor.u8TimeCntFluxToForce >= 100)               
                      {
                         g_sMotor.u8TimeCntFluxToForce = 0;
						 BLDC_ForceChangePhase(&g_sMotor);
                      }
					  #if 0//if(++g_sMotor.u8TimeForceChgPhase >= 80) //磁链换相不成功强拉(2对级电机) 
					  if(++g_sMotor.u8TimeForceChgPhase >= 100) //磁链换相不成功强拉(4对级电机) 
					  {  
						 g_sMotor.u8TimeForceChgPhase = 0;                     
						 BLDC_ForceChangePhase(&g_sMotor); //强拉换相                              
					  } 
					  #endif  
				   }  
				   #endif

				   #if 1  
				   if(g_bCommutationOK == 1)  //低速磁链or强拉换相成功,计算换相间隔时间   
				   {
                      /********计算换相间隔时间**********/  
					  g_u16NowZeroTimerVal = TCR00;  //计算过零间隔时间     	  
					  g_aTzc[2] = g_aTzc[1];
					  g_aTzc[1] = g_aTzc[0];  
					  if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal)//上次过零时刻点>本次过零时刻点
					  { g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
					  else 
					  { g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
					  g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次换相时间值
					  /***********************************/
                   }
                   #endif
				   
				   if(g_u8FluxBemfStat == eSTAG0_1stINT) //flux判断执行时间  
				   {
                      if(g_bCommutationOK == 1) //磁链换相成功      
                      {
                         if(++g_u8CommutationCnt >= 3) //有三次磁链换相     
						 { 
                            g_u8CommutationCnt = 0; //磁链换相计数清0
                            g_u8ZeroCrosscnt = 0;   //反电势换相清0
                            g_u8CommAdcIndex = 0;   //公共通道采样  
                            g_u8FluxBemfStat = eSTAG1_1stINT; //切换到电机运行子状态2,阶段1(磁链+反电势加速)PWM周期第一次中断

							//g_u8CommAdcIndex = 0;        //公共通道采样    
                            //g_u8FluxToBemfFlag = 1;      //采样及计算方式更改    
                            //g_u8PwmToADelayTimeFlag = 1; //tmrd中断延迟触发ADC采样时间 
						 }
					  }
                   }
				   else        
				   {
                     //if(g_sMotor.uWorkFlag.bfluxCommuSucess) //磁链换相成功才计算过零点 
                     {
                       //g_sMotor.uWorkFlag.bfluxCommuSucess = 0;
					   if(BLDC_u8BemfZeroCrossDetect_LowDuty(&g_sMotor,g_u16ZeroCompareVal,uBemfAdRes.aBemf[0])) 
                       {
                         DebugIOHigh;           
					     DebugIOLow;    
						 g_sMotor.uWorkFlag.bfluxCommuSucess = 0;
						 #if 0
						 /********计算换相间隔时间**********/
					     g_u16NowZeroTimerVal = TCR00;  //计算过零间隔时间 	  
					     g_aTzc[2] = g_aTzc[1];
					     g_aTzc[1] = g_aTzc[0];
					     if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal)//上次过零时刻点>本次过零时刻点
					     { g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
					     else 
					     { g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
					     g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次换相时间值
					     /***********************************/ 
						 #endif
                         g_u8ZeroCrosscnt++;
			             g_sMotor.uWorkFlag.bGetBemfZeroCross = 0; //清过零检测标志
			             g_sMotor.uWorkFlag.bBemfFalling = 0;
			             g_sMotor.uWorkFlag.bBemfRising = 0;
					   }
					 }
					  if(g_bCommutationOK == 1) //反电势判断不用于换相       
			          {
                         if(g_u8ZeroCrosscnt >= 3)
                         {
                            g_u8ZeroCrosscnt = 0;        //反电势检测次数       
                            g_u8CommAdcIndex = 0;        //公共通道采样        
                            //g_u8FluxToBemfFlag = 1;      //采样及计算方式更改    
                            g_u8PwmToADelayTimeFlag = 1; //tmrd中断延迟触发ADC采样时间    
                         }
			          }
                   }
				   #if 0
				   if(g_bCommutationOK == 1)               
				   {
                      //UartSendData(0xaa);
					if(BLDC_u8BemfZeroCrossDetect_LowDuty(&g_sMotor,g_u16ZeroCompareVal,uBemfAdRes.aBemf[0]))
				    {
					  //UartSendData(0xaa); //Debug 10%占空比无过零信号产生;15%占空比有稳定过零信号产生;  
					  //DebugIOFlash;
					  g_bCommutationOK = 0;     
					  DebugIOHigh;    
					  DebugIOLow;
                      /********计算换相间隔时间**********/
					  g_u16NowZeroTimerVal = TCR00;  //计算过零间隔时间 	  
					  g_aTzc[2] = g_aTzc[1];
					  g_aTzc[1] = g_aTzc[0];
					  if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal)//上次过零时刻点>本次过零时刻点
					  { g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
					  else 
					  { g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
					  g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次换相时间值
					  /***********************************/
					  //UartSendData(g_aTzc[0]>>10);   
					  
					  g_sMotor.uWorkFlag.bGetBemfZeroCross = 0; //清过零检测标志   
					  g_sMotor.uWorkFlag.bBemfFalling = 0;		//清反电势下坡标志
					  g_sMotor.uWorkFlag.bBemfRising = 0;		//清反电势上坡标志
			 
					  if(++g_u8ZeroCrosscnt >= 6)  //累计6次过零切到反电势
					  {
						 g_u8ZeroCrosscnt = 0;		  //过零计数清0 					  
						 g_u8CommAdcIndex = 0;		  //公共通道采样计数清0 			
						 g_u8PwmToADelayTimeFlag = 1; //tmrd中断延迟触发ADC采样时间   
						 g_u8FluxToBemfFlag = 1;		//采样及计算方式更改   

						 #if 0
						 g_bCommutationOK = 0;   //换相未完成  
			             g_u8NoAdcStartFlag = 1; //未执行完换相即使进入tmrd也不允许启动ADC 
			             TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/7;  //超前30-180/7=30-26=4度
			             R_TAU0_Channel1_Start();  //启动定时器通道1超前换相   
			             #endif
						 
						 //g_bCommutationOK = 1;		  //置换相成功标志 
						 //g_u8FluxToBemfFlag = 2;		//采样及计算方式更改   
					  }  
				     }
				   }
				   #endif
				}
			    break;       
			 
      case 1:              
	  	     g_u16ADCBuffer[g_u8AdcIndex] = (u16)(ADCR>>6U); //实际AD结果(高6位)                                
             if(g_u8AdcIndex < 5)            
             {
                g_u8AdcIndex++;                
                if(g_u8AdcIndex == 2)      { ADS = eaAdFifoChannelBuf[1]; ADIF = 0U;}  //反电势相电压AD通道  
                //if(g_u8AdcIndex == 2)      { ADS = eaAdFifoChannelBuf[2]; ADIF = 0U;}  //导通相电压AD通道
				else if(g_u8AdcIndex == 3) { ADS = IPeak_ADCH; ADIF = 0U; }             //母线电流通道为外部放大器
                else if(g_u8AdcIndex == 4) { ADS = BusVdc_ADCH; ADIF = 0U; }            //母线电压(可在PWM OFF时采集)
                else if(g_u8AdcIndex == 5) { ADS = eaComAdChannel[g_u8CommAdcIndex]; ADIF = 0U; } //公共通道
             }
             else   
             {
                R_ADC_Stop();  
				uBemfAdRes.aBemf[0]      = g_u16ADCBuffer[2];    //获得反电势相AD          
                uBemfAdRes.u16Conduction = g_u16ADCBuffer[1];    //获得导通相AD  
                uFluxAdRes.u16Bemf       = g_u16ADCBuffer[2]<<2; //获反电势相电压AD(12位AD格式)磁链
                uFluxAdRes.u16Conduction = g_u16ADCBuffer[1]<<2; //获得导通相电压AD(12位AD格式)磁链      
                uFluxAdRes.u16PhaseCurr  = g_u16ADCBuffer[3]<<2; //获得峰值电流(转换成4096格式)外部放大器21倍:对应电压为3.3*AD/4096 
                g_sPeakIdc.Ins = g_u16ADCBuffer[3]; //获得峰值电流AD,用于过流保护  
                g_sBusIdc.Ins  = g_u16ADCBuffer[4]; //获得母线电压AD
                if(g_u8CommAdcIndex == 0)      { g_sMosTmp.Ins = g_u16ADCBuffer[5]; g_u8CommAdcIndex++; }
	            else if(g_u8CommAdcIndex == 1) {  g_u8CommAdcIndex++; }
	            else if(g_u8CommAdcIndex == 2) {  g_u8CommAdcIndex = 0; }  
                g_u16ZeroCompareVal = uBemfAdRes.u16Conduction>>1; //得出比较电压,母线电压一半              
				//g_u16ZeroCompareVal = g_sBusIdc.Ins>>1; //得出比较电压,母线电压一半                

				if(g_bCommutationOK == 1) //上次换相成功,才允许计算过零点准备下一次换相                      
	            {   
                   if(BLDC_u8BemfZeroCrossDetect_MidDuty(&g_sMotor,g_u16ZeroCompareVal,uBemfAdRes.aBemf[0])) //有过零点
                   {
                      //UartSendData(0xbb);                  
                      DebugIOHigh;         
					  DebugIOLow;
			          g_u16NowZeroTimerVal = TCR00; //读取本次过零点时刻值      
                      g_aTzc[2] = g_aTzc[1];
			          g_aTzc[1] = g_aTzc[0];    
			          if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal) //上次过零时刻点>本次过零时刻点  
			          { g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
			          else 
			          { g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
                      g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次过零点时刻值(作为上次过零时刻值)       
              
			          //DebugIOFlash; 
			          //UartSendData(((g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/3)>>5); //Debug:Duty=50发送值为21-->*32=672                                                 
			          g_bCommutationOK = 0;   //换相未完成
			          g_u8NoAdcStartFlag = 1; //未执行完换相即使进入tmrd也不允许启动ADC     
                      if(g_sPeakIdc.Ins >= 325) //电流>5A:5A(2倍额定电流)(3.3V*325/1024/21/0.01)
			          { TDR01 = g_aTzc[0]>>5; } //延时2度,超前30-2=28度      
			          else
			          //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>6; }     //超前30-60/64=29度      
                      //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>5; }     //超前30-60/32=28度         
                      //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>4; } //超前30-60/16=30-4=26度            
                      //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>3; } //超前30-60/8=30-7.5=22.5度       
                      //{ TDR01 = g_aTzc[0]>>3; }
			          //{ TDR01 = g_aTzc[0]>>5; }   
		        	  //{ TDR01 = g_aTzc[0]>>4; }
					  //{ TDR01 = g_aTzc[0]>>3; }
			          //{ TDR01 = g_aTzc[0]>>2; } 
			          //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/6; } //超前30-180/6=30-30=0度 
			          //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/7; } //超前30-180/7=30-26=4度                
			          //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/8; } //超前30-180/8=30-22=8度
			          //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/9; } //超前30-180/9=30-20=10度 
			          { TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/10; } //超前30-180/10=30-18=12度 
					  //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/12; } //超前30-180/12=30-15=15度  
					  //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/16; } //超前30-180/16=30-11=19度
					  //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/18; } //超前30-180/18=30-10=20度
					  //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/20; } //超前30-180/20=30-9=21度     
					  //{ TDR01 = (g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/32; } //超前30-180/32=30-6=24度    
			          //{ TDR01 = g_aTzc[0]>>1; }
			          //{ TDR01 = g_aTzc[0]>>6; }    
                      //{ TDR01 = g_aTzc[0]>>3+g_aTzc[0]>>5; }  //超前30-7.5-1.875=20.625度(无续流脉冲)    
			          //{ TDR01 = g_aTzc[0]>>3+g_aTzc[0]>>4; }  //超前30-7.5-3.75=18.75度(无续流脉冲)
                      //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>2; } //超前30-60/4=30-15=15度(转不起来)   
                      //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>1; } //超前30-60/2=0度 
			          //{ TDR01 = g_aTzc[0]>>4; } //超前30-60/16=26.25度 
			          //{ TDR01 = g_aTzc[0]>>3; } //超前30-8=22度  
			          //{ TDR01 = (g_aTzc[0]>>3)-2; } //超前30-8=22度   
			          //{ TDR01 = g_aTzc[0]>>1; } //超前30-30=0度       
			          //{ TDR01 = g_aTzc[0]>>2; } //超前30-60/4=15度(50%占空比空载相电压美)                       
                      R_TAU0_Channel1_Start();  //启动定时器通道1超前换相     
                      if(g_sDrive.u16TargetPwmDuty > cDUTY_50)                      
			          {
                         g_u8CommAdcIndex = 0;        //公共通道采样                  
                         g_u8FluxToBemfFlag = 2;      //换相点处理方式 bemf         
                         g_u8PwmToADelayTimeFlag = 2; //tmrd延时采样时间    
			          }
			       }
	            }
             }   
	  	      //Disp_MidSpeedADCInterrupt();     
	         break; //中速:<50%占空比←←←←←←←←←←←←|←← 
	  case 2: Disp_MidHighSpeedADCInterrupt(); break; //中高速>50%占空比←←←|>80%↓|<40%→→↑
	  case 3: Disp_HighSpeedADCInterrupt();    break; //高速>=80%占空比|<70%↑|←←←
	  default:break;     
   }
}
/*
   g_u16ADCBuffer[1]:存贮反电势AD数据            
   g_u16ADCBuffer[2]:导通相
   g_u16ADCBuffer[3]:母线电流   
   ADCR为16位寄存器,高10位存放结果(分辨率为10),AD中断内转换结果将10位数据少右移2位(应右移4实际右移2)转换成12位数据格式化
*/
#if 0
void Disp_LowSpeedADCInterrupt(void)                                 
{
   g_u16ADCBuffer[g_u8AdcIndex] = (u16)(ADCR>>4U); //读取高10位,TMRD中断中触发AD采样前已配置Index=1通道为反电势通道
   if(g_u8AdcIndex < 5)                     
   {
      g_u8AdcIndex++;
      if(g_u8AdcIndex == 2)      { ADS = eaAdFifoChannelBuf[1]; ADIF = 0U;  } //反电势相电压                        
	  else if(g_u8AdcIndex == 3) { ADS = IPeak_ADCH; ADIF = 0U; } //母线电流    
	  else if(g_u8AdcIndex == 4) { ADS = BusVdc_ADCH; ADIF = 0U; } //母线电压(可在PWM OFF时采集)  
	  else if(g_u8AdcIndex == 5) { ADS = eaComAdChannel[g_u8CommAdcIndex]; ADIF = 0U; } //采集公共通道模拟量数据         
   }
   else  //ADC采完了两组数据 每一组进4次中断两组读完后得到总结果总耗时27US(总共进入8次中断)
   {
      R_ADC_Stop();
	  uFluxAdRes.u16Conduction = g_u16ADCBuffer[1];    //获得导通相电压AD(放大4倍)结果为12位AD:对应电压为3.3*AD/4096  
      uFluxAdRes.u16Bemf       = g_u16ADCBuffer[2];    //获得反电势电压AD(放大4倍)结果为12位AD:对应电压为3.3*AD/4096  
      uFluxAdRes.u16PhaseCurr  = g_u16ADCBuffer[3];    //获得峰值电流结果为12位AD(内部运放32倍):对应电压为3.3*AD/4096    
      g_sPeakIdc.Ins           = g_u16ADCBuffer[3]>>2; //用于过流保护   
	  g_u16L_dIs_dt            = g_u16ADCBuffer[4]<<1; //母线电压实际值/3(结果放大1024倍):(AD*3.3/4096)*7.6(分压比1.5k:10K)/3*1024
	  g_sBusVdc.Ins            = g_u16ADCBuffer[4]>>2; //换算成10位AD(1024)采样         
      
	  if(g_u8CommAdcIndex == 0) //循环通道用于公共通道数据
	  { g_sMosTmp.Ins = g_u16ADCBuffer[5]>>2; g_u8CommAdcIndex++; }             
	  else if(g_u8CommAdcIndex == 1)
	  { g_u8CommAdcIndex++; }   //后续再定义读取数据    
	  else if(g_u8CommAdcIndex == 2)
	  { g_u8CommAdcIndex = 0; }        
	  
      //++g_sMotor.u8TimeForceChgPhase;                                              
	  BLDC_FluxCalcuCommutation(&g_sMotor,&uFluxAdRes); //耗时11.8us磁链运算换相     
      if(g_bCommutationOK) { g_sMotor.u8TimeForceChgPhase = 0; }       
	  else                                              //磁链换相不成功强拉时间计数 
	  {
	     if(++g_sMotor.u8TimeForceChgPhase >= 80) //磁链换相不成功强拉时间计数    
	     {  
	        g_sMotor.u8TimeForceChgPhase = 0;
		    BLDC_ForceChangePhase(&g_sMotor); //强拉换相  
	     }
	  }
	  
      if(BLDC_u8BemfZeroCrossDetect_LowDuty(&g_sMotor,uFluxAdRes.u16Conduction>>1,uFluxAdRes.u16Bemf))
	  {
         //UartSendData(0xaa); //Debug 10%占空比无过零信号产生;15%占空比有稳定过零信号产生;  
         //DebugIOFlash;     
         g_u16NowZeroTimerVal = TCR00;  //计算过零间隔时间       
		 g_aTzc[2] = g_aTzc[1];
		 g_aTzc[1] = g_aTzc[0];
		 if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal)//上次过零时刻点>本次过零时刻点
		 { g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
		 else 
		 { g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
		 g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次换相时间值
         //UartSendData(g_aTzc[0]>>10);   
		 
		 g_sMotor.uWorkFlag.bGetBemfZeroCross = 0; //清过零检测标志   
		 g_sMotor.uWorkFlag.bBemfFalling = 0;      //清反电势下坡标志
		 g_sMotor.uWorkFlag.bBemfRising = 0;       //清反电势上坡标志

		 if(++g_u8ZeroCrosscnt >= 6)  //累计6次过零切到反电势
		 {
            g_u8ZeroCrosscnt = 0;        //过零计数清0                       
            g_u8CommAdcIndex = 0;        //公共通道采样计数清0             
            //g_u8PwmToADelayTimeFlag = 1; //tmrd中断延迟触发ADC采样时间 
            g_bCommutationOK = 1;        //置换相成功标志
            //g_u8FluxToBemfFlag = 1;      //采样及计算方式更改  
            //g_u8FluxToBemfFlag = 2;      //采样及计算方式更改
		 }
      }
	  

	  
#if 0
	  ++g_sMotor.u8TimeForceChgPhase;

      if(g_sMotor.u8TimeForceChgPhase >= 40) // 40*100us=4ms       
      {
         g_sMotor.u8TimeForceChgPhase = 0;
		 BLDC_ForceChangePhase(&g_sMotor); //强拉换相        
      }
	  else //强拉4ms时间内优先磁链换相          
	  {
         BLDC_FluxCalcuCommutation(&g_sMotor,&uFluxAdRes); //耗时11.8us磁链运算换相,需要ISR_PwmAdcInt_EndHandle更新上
         if(g_bCommutationOK) g_sMotor.u8TimeForceChgPhase = 0;    
      }
#endif
   }
}
#endif


#if 0
void Disp_MidSpeedADCInterrupt(void) //<50%占空比       
{
   g_u16ADCBuffer[g_u8AdcIndex] = (u16)(ADCR>>6U); //实际AD结果(高6位)                   
   if(g_u8AdcIndex < 5)    
   {
      g_u8AdcIndex++;     
      if(g_u8AdcIndex == 2)      { ADS = eaAdFifoChannelBuf[2]; ADIF = 0U; }  //导通相电压AD通道
      else if(g_u8AdcIndex == 3) { ADS = IPeak_ADCH; ADIF = 0U; }             //母线电流通道为外部放大器
      else if(g_u8AdcIndex == 4) { ADS = BusVdc_ADCH; ADIF = 0U; }            //母线电压(可在PWM OFF时采集)
      else if(g_u8AdcIndex == 5) { ADS = eaComAdChannel[g_u8CommAdcIndex]; ADIF = 0U; } //公共通道
   }
   else
   {
      R_ADC_Stop();
      uBemfAdRes.aBemf[0]      = g_u16ADCBuffer[1];    //获得反电势相AD  
      uBemfAdRes.u16Conduction = g_u16ADCBuffer[2];    //获得导通相AD
      uFluxAdRes.u16Bemf       = g_u16ADCBuffer[1]<<2; //获反电势相电压AD(12位AD格式)磁链
      uFluxAdRes.u16Conduction = g_u16ADCBuffer[2]<<2; //获得导通相电压AD(12位AD格式)磁链      
      uFluxAdRes.u16PhaseCurr  = g_u16ADCBuffer[3]<<2; //获得峰值电流(转换成4096格式)外部放大器21倍:对应电压为3.3*AD/4096 
      g_sPeakIdc.Ins = g_u16ADCBuffer[3]; //获得峰值电流AD,用于过流保护  
      g_sBusIdc.Ins  = g_u16ADCBuffer[4]; //获得母线电压AD
      if(g_u8CommAdcIndex == 0)      { g_sMosTmp.Ins = g_u16ADCBuffer[5]; g_u8CommAdcIndex++; }
	  else if(g_u8CommAdcIndex == 1) {  g_u8CommAdcIndex++; }
	  else if(g_u8CommAdcIndex == 2) {  g_u8CommAdcIndex = 0; }  
      g_u16ZeroCompareVal = uBemfAdRes.u16Conduction>>1; //得出比较电压,母线电压一半

	  if(g_bCommutationOK == 1) //上次换相成功,才允许计算过零点准备下一次换相      
	  {   
         if(BLDC_u8BemfZeroCrossDetect_MidDuty(&g_sMotor,g_u16ZeroCompareVal,uBemfAdRes.aBemf[0])) //有过零点
         {
            //计算换相间隔时间                     
            #if 0  
            u16TimerRegBuf = TCR00;                 
			g_aTzc[2] = g_aTzc[1];
			g_aTzc[1] = g_aTzc[0];
			g_aTzc[0] = u16LastZeroCrossTimerReg-u16TimerRegBuf;   
			u16LastZeroCrossTimerReg = u16TimerRegBuf;
			#endif     
			//UartSendData(0xbb);    
			#if 1  
			g_u16NowZeroTimerVal = TCR00; //读取本次过零点时刻值      
            g_aTzc[2] = g_aTzc[1];
			g_aTzc[1] = g_aTzc[0];    
			if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal) //上次过零时刻点>本次过零时刻点  
			{ g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
			else 
			{ g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
            g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次过零点时刻值(作为上次过零时刻值)       
            #endif  
			//DebugIOFlash; 
			//UartSendData(((g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/3)>>5); //Debug:Duty=50发送值为21-->*32=672                                                 
			
			g_bCommutationOK = 0;   //换相未完成
			g_u8NoAdcStartFlag = 1; //未执行完换相即使进入tmrd也不允许启动ADC     

			if(g_sPeakIdc.Ins >= 325) //电流>5A:5A(2倍额定电流)(3.3V*325/1024/21/0.01)
			{ TDR01 = g_aTzc[0]>>5; } //延时2度,超前30-2=28度      
			else
			//{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>6; }     //超前30-60/64=29度    
            //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>5; }     //超前30-60/32=28度         
            //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>4; } //超前30-60/16=30-4=26度            
            //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>3; } //超前30-60/8=30-7.5=22.5度       
            //{ TDR01 = g_aTzc[0]>>3; }
			//{ TDR01 = g_aTzc[0]>>5; }   
			//{ TDR01 = g_aTzc[0]>>4; }
			//{ TDR01 = g_aTzc[0]>>2; }  
			//{ TDR01 = g_aTzc[0]>>1; }
			{ TDR01 = g_aTzc[0]>>6; }  
            //{ TDR01 = g_aTzc[0]>>3+g_aTzc[0]>>5; }  //超前30-7.5-1.875=20.625度(无续流脉冲)    
			//{ TDR01 = g_aTzc[0]>>3+g_aTzc[0]>>4; }  //超前30-7.5-3.75=18.75度(无续流脉冲)
            //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>2; } //超前30-60/4=30-15=15度(转不起来)   
            //{ TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>1; } //超前30-60/2=0度 
			//{ TDR01 = g_aTzc[0]>>4; } //超前30-60/16=26.25度 
			//{ TDR01 = g_aTzc[0]>>3; } //超前30-8=22度  
			//{ TDR01 = (g_aTzc[0]>>3)-2; } //超前30-8=22度   
			//{ TDR01 = g_aTzc[0]>>1; } //超前30-30=0度       
			//{ TDR01 = g_aTzc[0]>>2; } //超前30-60/4=15度(50%占空比空载相电压美)                       
            R_TAU0_Channel1_Start();  //启动定时器通道1超前换相     

            if(g_sDrive.u16TargetPwmDuty > cDUTY_50)                      
			{
               g_u8CommAdcIndex = 0;        //公共通道采样                
               g_u8FluxToBemfFlag = 2;      //换相点处理方式 bemf         
               g_u8PwmToADelayTimeFlag = 2; //tmrd延时采样时间    
			}
			 
		 }
	  }
   }
}
#endif

void Disp_MidHighSpeedADCInterrupt(void) //>50% <80%占空比  
{
   g_u16ADCBuffer[g_u8AdcIndex] = (u16)(ADCR>>6U); //实际AD结果(高6位)
   if(g_u8AdcIndex < 5)
   {
      g_u8AdcIndex++; //ADS = eaAdFifoChannelBuf[1]  
	  if(g_u8AdcIndex == 2)      { ADS = eaAdFifoChannelBuf[1]; ADIF = 0U; }
	  else if(g_u8AdcIndex == 3)    { ADS = eaAdFifoChannelBuf[1]; ADIF = 0U; }
	  else if(g_u8AdcIndex == 4) { ADS = eaAdFifoChannelBuf[1]; ADIF = 0U; }  
      //else if(g_u8AdcIndex == 4)      { ADS = BusVdc_ADCH; ADIF = 0U; }            //母线电压(可在PWM OFF时采集)
      else if(g_u8AdcIndex == 5) { ADS = eaComAdChannel[g_u8CommAdcIndex]; ADIF = 0U;} //公共通道
   }
   else
   {
      R_ADC_Stop();
      uBemfAdRes.aBemf[0] = g_u16ADCBuffer[1];      
	  uBemfAdRes.aBemf[1] = g_u16ADCBuffer[2];   
      uBemfAdRes.aBemf[2] = g_u16ADCBuffer[3];
      g_sBusVdc.Ins       = g_u16ADCBuffer[4];
	  if(g_u8CommAdcIndex == 0)      { g_u8CommAdcIndex++; }
	  else if(g_u8CommAdcIndex == 1) { g_sMosTmp.Ins = g_u16ADCBuffer[5]; g_u8CommAdcIndex++; }
	  else if(g_u8CommAdcIndex == 2) { g_u8CommAdcIndex++; }
	  else if(g_u8CommAdcIndex == 3) { g_sPeakIdc.Ins = g_u16ADCBuffer[5];g_u8CommAdcIndex = 0; }

	  if(g_bCommutationOK == 1) //上次换相成功,才允许计算过零点准备下一次换相
	  {
         //注意Demo板里母线电压分压比同相电压传递参数时需/2
		 if(BLDC_u8BemfZeroCrossDetect_MidHighDuty(&g_sMotor,g_sBusVdc.Ins>>1,uBemfAdRes.aAllData))
         {
            //计算换相间隔时间   
            #if 0
            u16TimerRegBuf = TCR00; //24M/2^4=1.5M-->1clock=1/1.5M=0.667us                   
            g_aTzc[1] = g_aTzc[0];
			g_aTzc[0] = u16LastZeroCrossTimerReg-u16TimerRegBuf;  
			u16LastZeroCrossTimerReg = u16TimerRegBuf;
            #endif
			//UartSendData(0xcc);  
			g_u16NowZeroTimerVal = TCR00; //读取本次过零点时刻值       
			g_aTzc[2] = g_aTzc[1];
			g_aTzc[1] = g_aTzc[0];
			if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal) //上次过零时刻点>本次过零时刻点  
			{ g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
			else 
			{ g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
            g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次过零点时刻值(作为上次过零时刻值) 
			
			g_bCommutationOK = 0;
			g_u8NoAdcStartFlag = 1;   //未执行完换相即使进入tmrd也不允许启动adc  

			//UartSendData(((g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/3)>>5); //Debug:Duty=60发送值为18-->*32=576 
			
			//TDR01 = g_aTzc[0]>>3;     //超前30-60/8=22.5度  
			TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>3; //超前30-60/8=30-7.5=22.5度   
			//TDR01 = ((g_aTzc[0]+g_aTzc[1])>>1)>>2; //超前30-60/4=30-15=15度   
			R_TAU0_Channel1_Start();  //启动CH1定时器,超前换相  
			if(g_sDrive.u16TargetPwmDuty >= cDUTY_80)
			{
               g_u8FluxToBemfFlag = 3;
               g_u8CommAdcIndex = 0; //公共通道采样
               g_u8PwmToADelayTimeFlag = 2;//tmrd延时采样时间
			}
			else if(g_sDrive.u16TargetPwmDuty <= cDUTY_40)  
			{
               g_u8FluxToBemfFlag = 1;
               g_u8CommAdcIndex = 0; //公共通道采样
               g_u8PwmToADelayTimeFlag = 1;//tmrd延时采样时间         
			}
		 }
	  }
   }
}

void Disp_HighSpeedADCInterrupt(void)   //>80%占空比
{
   g_u16ADCBuffer[g_u8AdcIndex] = (u16)(ADCR>>6U);
   if(g_u8AdcIndex < 7)
   {
      g_u8AdcIndex++;
      if(g_u8AdcIndex == 6)      { ADS = BusVdc_ADCH; ADIF = 0; }
	  else if(g_u8AdcIndex == 7) { ADS = eaComAdChannel[g_u8CommAdcIndex]; ADIF = 0; }
   }
   else
   {
      R_ADC_Stop();
      uBemfAdRes.aBemf[0] = g_u16ADCBuffer[1];
	  uBemfAdRes.aBemf[1] = g_u16ADCBuffer[2];
	  uBemfAdRes.aBemf[2] = g_u16ADCBuffer[3];
	  uBemfAdRes.aBemf[3] = g_u16ADCBuffer[4];
	  uBemfAdRes.aBemf[4] = g_u16ADCBuffer[5];
	  g_sBusVdc.Ins       = g_u16ADCBuffer[6];    

	  if(g_u8CommAdcIndex == 0)      { g_u8CommAdcIndex++; }
	  else if(g_u8CommAdcIndex == 1) { g_sMosTmp.Ins = g_u16ADCBuffer[7]; g_u8CommAdcIndex++;   }
	  else if(g_u8CommAdcIndex == 2) { g_sMosTmp.Ins = g_u16ADCBuffer[7]; g_u8CommAdcIndex++;   }
	  else if(g_u8CommAdcIndex == 3) { g_sMosTmp.Ins = g_u16ADCBuffer[7]; g_u8CommAdcIndex = 0; }

	  if(g_bCommutationOK == 1)
	  {
         if(BLDC_u8BemfZeroCrossDetect_HighDuty(&g_sMotor,g_sBusVdc.Ins>>1,uBemfAdRes.aAllData))  
		 {
            //计算换相间隔时间
            #if 0
            u16TimerRegBuf = TCR00; //上一次过零点到本次过零点定时器值
			g_aTzc[2] = g_aTzc[1];
			g_aTzc[1] = g_aTzc[0];
			g_aTzc[0] = u16LastZeroCrossTimerReg-u16TimerRegBuf; //上1次减本次的值  
			u16LastZeroCrossTimerReg = u16TimerRegBuf; //存贮本次过零点定时器点值    
			#endif
			//UartSendData(0xdd);
			g_u16NowZeroTimerVal = TCR00; //读取本次过零点时刻值              
			g_aTzc[2] = g_aTzc[1];
			g_aTzc[1] = g_aTzc[0];
			if(g_u16LastZeroTimerVal >= g_u16NowZeroTimerVal) //上次过零时刻点>本次过零时刻点  
			{ g_aTzc[0] = g_u16LastZeroTimerVal - g_u16NowZeroTimerVal; }  
			else 
			{ g_aTzc[0] = _FFFF_TAU_TDR00_VALUE-g_u16NowZeroTimerVal+g_u16LastZeroTimerVal; }
            g_u16LastZeroTimerVal = g_u16NowZeroTimerVal; //存贮本次过零点时刻值(作为上次过零时刻值) 
            
			g_bCommutationOK = 0;
		    g_u8NoAdcStartFlag = 1;  //未执行完换相即使进入tmrd也不允许启动adc      

			//UartSendData(((g_aTzc[0]+g_aTzc[1]+g_aTzc[2])/3)>>5); //Debug:Duty=80发送值为15-->*32=480   

			//TDR01 = g_aTzc[0]>>4;    //超前30-60/16=26度    
			//TDR01 = g_aTzc[0]>>2;    //超前30-60/4=15度  
			//TDR01 = g_aTzc[0]>>3;    //超前30-60/8=22.5度 
			//TDR01 = g_aTzc[0]>>5;    //超前30-60/32=28度 
			//TDR01 = g_aTzc[0]>>1;    //超前30-60/2=0度 
			 TDR01 = g_aTzc[0]/3;   //超前30-60/3=10度  
            R_TAU0_Channel1_Start(); //启动CH1定时器 超前换相       
            R_ADC_Stop();
			if(g_sDrive.u16TargetPwmDuty <= cDUTY_70) 
			{
               g_u8FluxToBemfFlag = 2;      //换相点处理方式 bemf  
               g_u8CommAdcIndex = 0;        //公共通道采样
               g_u8PwmToADelayTimeFlag = 2; //tmrd延时采样时间
			}
		 }
      }
   }
}  
 



