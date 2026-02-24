#include "ws281x.h"
#include "delay.h"
#include "stm32f4xx.h"


uint16_t pixelBuffer[PIXEL_NUM][GRB];



/*
*作用：通用定时器2 TIM2_CH2 PWM输出+DMA初始化 
*参数：arr：自动重装值
*说明：定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
*      Ft=定时器工作频率,单位:Mhz
*/
void WS281x_Init(void)
{
	//结构体变量
	GPIO_InitTypeDef            GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
	TIM_OCInitTypeDef           TIM_OCInitStructure;
	DMA_InitTypeDef  			DMA_InitStructure;
	
	//使能RCC时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
 
	//初始化GPIO口
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_7;				
	GPIO_InitStructure.GPIO_Speed   = GPIO_High_Speed;		    
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;			    
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);		

	/* GPIO remap to TIM4*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
	
	/* timer period : T =(arr + 1) * (PSC + 1) / Tck.   arr: period value PSC:prescaler value  Tck: system clock */
	TIM_TimeBaseStructure.TIM_Prescaler		=0;						
	TIM_TimeBaseStructure.TIM_CounterMode	=TIM_CounterMode_Up;	
	TIM_TimeBaseStructure.TIM_Period		=105-1;					/* T = (TIM_Period + 1)*(0+1)/84M  = 800kHz*/   
	TIM_TimeBaseStructure.TIM_ClockDivision	=TIM_CKD_DIV1;		
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);	
	//初始化TIM4 Channel2 PWM模式
	TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_PWM1; 					
 	TIM_OCInitStructure.TIM_OutputState 	= TIM_OutputState_Enable;       
	TIM_OCInitStructure.TIM_Pulse 			= 0;						/* 1 ~ TIM_TimeBaseStructure.TIM_Period */
	TIM_OCInitStructure.TIM_OCPolarity 		= TIM_OCPolarity_High; 		    
	TIM_OCInitStructure.TIM_OutputNState 	= TIM_OutputNState_Disable; //TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_OCNPolarity 	= TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState 	= TIM_OCNIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState 	= TIM_OCNIdleState_Reset;
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);  					
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);  					//使能TIM22在CCR2上的预装载寄存器
	TIM_Cmd(TIM4, DISABLE);  											//失能TIM4，防止第一个脉冲异常
    TIM_DMACmd(TIM4, TIM_DMA_CC2, ENABLE);

	DMA_DeInit(DMA1_Stream3);
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE){}//等待DMA可配置 	
  	/* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel 					= DMA_Channel_2; 	
	DMA_InitStructure.DMA_PeripheralBaseAddr 		= (u32)&(TIM4->CCR2);	
	DMA_InitStructure.DMA_Memory0BaseAddr 			= (u32)pixelBuffer;	
	DMA_InitStructure.DMA_DIR 						= DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize 				= (PIXEL_NUM * GRB);
	DMA_InitStructure.DMA_PeripheralInc 			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc 				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize 		= DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize 			= DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode 						= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority 					= DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode 					= DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold 			= DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst 				= DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst 			= DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);
	DMA_Cmd(DMA1_Stream3, DISABLE);		
	
	/*关闭所有灯珠*/
	WS281x_CloseAll();
	delay_ms(100);
}



void WS281x_CloseAll(void)
{
	uint16_t i;
	uint8_t j;

	for(i = 0; i < PIXEL_NUM; ++i)
	{
		for(j = 0; j < GRB; ++j)
		{
			pixelBuffer[i][j] = WS_LOW;
		}
	}
	WS281x_Show(); 
}


uint32_t WS281x_Color(uint8_t red, uint8_t green, uint8_t blue)
{
	return green << 16 | red << 8 | blue;
}

void WS281x_SetPixelColor(uint16_t n, uint32_t GRBColor)
{
  	uint8_t i;
	
	if(n < PIXEL_NUM)
	{
		for(i = 0; i < GRB; i++)
		{
			pixelBuffer[n][i] = ((GRBColor << i) & 0x800000) ? WS_HIGH : WS_LOW;
		}
	}
}



//下面函数中语句执行的顺序相当关键，不能错。
//从示波器看，非当前的执行顺序，最后一个灯的波形序列会多。
void WS281x_Show(void)
{
//	DMA_Cmd(DMA1_Stream3, DISABLE);                      	//关闭DMA传输 
//	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE){}		//确保DMA可以被设置  
	DMA_SetCurrDataCounter(DMA1_Stream3, PIXEL_NUM * GRB);
	TIM_Cmd(TIM4, ENABLE);
	DMA_Cmd(DMA1_Stream3, ENABLE);
	while(DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3) != SET);
	TIM_Cmd(TIM4, DISABLE);
	TIM_SetCompare2(TIM4, 0); //该句不能加，加上灯不会按正常流程显示
	DMA_Cmd(DMA1_Stream3, DISABLE);
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);

	//RESET
//	TIM_Cmd(TIM4, DISABLE);
//	GPIO_ResetBits(GPIOB, GPIO_Pin_7);	
//	delay_ms(1);	// >50us
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
static uint32_t WS281x_Wheel(uint8_t wheelPos) 
{
	wheelPos = 255 - wheelPos;
	if(wheelPos < 85) 
	{
		return WS281x_Color(255 - wheelPos * 3, 0, wheelPos * 3);
	}
	if(wheelPos < 170)
	{
		wheelPos -= 85;
		return WS281x_Color(0, wheelPos * 3, 255 - wheelPos * 3);
	}
	wheelPos -= 170;
	return WS281x_Color(wheelPos * 3, 255 - wheelPos * 3, 0);
}

// Fill the dots one after the other with a color
void WS281x_ColorWipe(uint32_t c, uint8_t wait)
{
	uint16_t i=0;
	
	for(i=0; i<PIXEL_NUM; i++)
	{
		WS281x_SetPixelColor(i, c);
		WS281x_Show();
		delay_ms(wait);
	}
}

void WS281x_Rainbow(uint8_t wait) 
{
	uint16_t i=0, j=0;

	for(j=0; j<256; j++) 
	{
		for(i=0; i<PIXEL_NUM; i++)
		{
			WS281x_SetPixelColor(i, WS281x_Wheel((i+j) & 255));
		}
		WS281x_Show();
		delay_ms(wait);
	}
}

// Slightly different, this makes the rainbow equally distributed throughout
void WS281x_RainbowCycle(uint8_t wait) 
{
	uint16_t i=0, j=0;

	for(j=0; j<256*5; j++) // 5 cycles of all colors on wheel
	{ 
		for(i=0; i< PIXEL_NUM; i++) 
		{
			WS281x_SetPixelColor(i,WS281x_Wheel(((i * 256 / PIXEL_NUM) + j) & 255));
		}
		WS281x_Show();
		delay_ms(wait);
	}
}

//Theatre-style crawling lights.
void WS281x_TheaterChase(uint32_t c, uint8_t wait) 
{
	uint16_t i=0, j=0, q=0;
	
	for (j=0; j<10; j++) //do 10 cycles of chasing
	{  
		for (q=0; q < 3; q++)
		{
			for (i=0; i < PIXEL_NUM; i=i+3)
			{
				WS281x_SetPixelColor(i+q, c);    //turn every third pixel on
			}
			WS281x_Show();
			delay_ms(wait);
			for (i=0; i < PIXEL_NUM; i=i+3) 
			{
				WS281x_SetPixelColor(i+q, 0);        //turn every third pixel off
			}
		}
	}
}

//Theatre-style crawling lights with rainbow effect
void WS281x_TheaterChaseRainbow(uint8_t wait) 
{
	uint16_t i=0, j=0, q=0;
	
	for (j=0; j < 256; j++) // cycle all 256 colors in the wheel
	{     
		for (q=0; q < 3; q++) 
		{
			for (i=0; i < PIXEL_NUM; i=i+3) 
			{
				WS281x_SetPixelColor(i+q, WS281x_Wheel( (i+j) % 255));    //turn every third pixel on
			}
			WS281x_Show();
			delay_ms(wait);
			for (i=0; i < PIXEL_NUM; i=i+3) 
			{
				WS281x_SetPixelColor(i+q, 0);        //turn every third pixel off
			}
		}
	}
}


