/**
******************************************************************************
* @file     main.c
* @author   AE team
* @version  V1.0.3
* @date     10/04/2019
* @brief
******************************************************************************
* @copy
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, MindMotion SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2019 MindMotion</center></h2>
*/

#include "HAL_device.h"
#include "HAL_conf.h"
#include "stdio.h"

char printBuf[100];
extern u32 SystemCoreClock;
void GPIO_Configuration(void);
void delay_init(void);
void uart_initwBaudRate(u32 bound);
void ADC1_SingleChannel(uint8_t ADC_Channel_x);
void delay_ms(__IO uint32_t nTime);
u16 Get_Adc_Average(uint8_t ADC_Channel_x, uint8_t times);
void UartSendGroup(u8* buf, u16 len);
void TimingDelay_Decrement(void);

static __IO uint32_t TimingDelay;


/********************************************************************************************************
**������Ϣ ��main(void)
**�������� ��
**������� ��
**������� ��
**    ��ע ��
********************************************************************************************************/
int main(void)
{
    u16 ADCVAL;
    float fValue;
    delay_init();

    uart_initwBaudRate(9600);
    /*����ADC1Ϊ����ת��ģʽ,ͨ��1ʹ��*/

    ADC1_SingleChannel( ADC_Channel_1);
    while(1) {
        ADCVAL = Get_Adc_Average(ADC_Channel_1, 5);
        fValue = ((float)ADCVAL / 4095) * 3.3;
        UartSendGroup((u8*)printBuf, sprintf(printBuf, "ADC1_CH_1=%fV\r\n", fValue));
        delay_ms(200);
    }

}

/********************************************************************************************************
**������Ϣ ��delay_init(void)
**�������� ��systick��ʱ������ʼ��
**������� ����
**������� ����
********************************************************************************************************/
void delay_init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000)) {
        /* Capture error */
        while (1);
    }
    /* Configure the SysTick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x0);//SysTick�ж����ȼ�����
}

/********************************************************************************************************
**������Ϣ ��SysTick_Handler(void)
**�������� ��������жϺ�����Systick���еݼ�
**������� ����
**������� ����
********************************************************************************************************/
void SysTick_Handler(void)
{
    TimingDelay_Decrement();
}

/********************************************************************************************************
**������Ϣ ��TimingDelay_Decrement(void)
**�������� ����1ms���ٶȵݼ�
**������� ��pclk2������ϵͳʱ��Ϊ8MHz������Ӧ����8
**������� ����
********************************************************************************************************/
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00) {
        TimingDelay--;
    }
}

/********************************************************************************************************
**������Ϣ ��uart_initwBaudRate(u32 bound)
**�������� ��UART��ʼ��
**������� ��bound
**������� ����
********************************************************************************************************/
void uart_initwBaudRate(u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    UART_InitTypeDef UART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_UART1, ENABLE);                       //ʹ��UART1ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);                         //����GPIOAʱ��
    //UART ��ʼ������
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

    UART_InitStructure.UART_BaudRate = bound;                                   //���ڲ�����
    UART_InitStructure.UART_WordLength = UART_WordLength_8b;                    //�ֳ�Ϊ8λ���ݸ�ʽ
    UART_InitStructure.UART_StopBits = UART_StopBits_1;                         //һ��ֹͣλ
    UART_InitStructure.UART_Parity = UART_Parity_No;                            //����żУ��λ
    UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;//��Ӳ������������
    UART_InitStructure.UART_Mode = UART_Mode_Rx | UART_Mode_Tx;	                //�շ�ģʽ

    UART_Init(UART1, &UART_InitStructure);                                      //��ʼ������1
    UART_Cmd(UART1, ENABLE);                                                    //ʹ�ܴ���1

    //UART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                                   //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                             //�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);                                      //��ʼ��GPIOA.9

    //UART1_RX	  GPIOA.10��ʼ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                                  //PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                       //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);                                      //��ʼ��GPIOA.10

}

/********************************************************************************************************
**������Ϣ ��void ADC1_SingleChannel(uint8_t ADC_Channel_x)
**�������� ������ADC1����ת��ģʽ
**������� ��ADC_Channel_x , xΪ0~8
**������� ����
**    ��ע ��
********************************************************************************************************/
void ADC1_SingleChannel(uint8_t ADC_Channel_x)
{
    ADC_InitTypeDef  ADC_InitStructure;

    GPIO_Configuration();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* Initialize the ADC_PRESCARE values */
    ADC_InitStructure.ADC_PRESCARE = ADC_PCLK2_PRESCARE_16;
    /* Initialize the ADC_Mode member */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Single;
    /* Initialize the ADC_DataAlign member */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    /* Initialize the ADC_ExternalTrigConv member */
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_Init(ADC1, &ADC_InitStructure);

    /*��������ͨ��*/
    ADC_RegularChannelConfig(ADC1, DISABLE_ALL_CHANNEL, 0, 0);
    /*ʹ��ѡ��ͨ��,�����������*/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_x, 0, ADC_SampleTime_28_5Cycles);

    ADC_Cmd(ADC1, ENABLE);
}

/********************************************************************************************************
**������Ϣ ��void GPIO_Configuration(void)
**�������� ��GPIO����
**������� ��
**������� ��
**    ��ע ��
********************************************************************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin  =  GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    /*��PA1����Ϊģ������*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/********************************************************************************************************
**������Ϣ ��ADC1_SingleChannel_Get()
**�������� ����ȡADC1ת������
**������� ��ADC_Channel_x , xΪ0~8
*puiADData ,ADC1ʵ��ת������
**������� ��ucStatus ,0 ��ʾ���ݻ�ȡʧ��,1 ��ʾ�ɹ�
********************************************************************************************************/
u16 ADC1_SingleChannel_Get(uint8_t ADC_Channel_x)
{
    u16 puiADData;
    /*ADCR�Ĵ�����ADSTλʹ�ܣ��������ת��*/
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    puiADData = ADC_GetConversionValue(ADC1);
    return puiADData;
}

/********************************************************************************************************
**������Ϣ ��Get_Adc_Average(uint8_t ADC_Channel_x,uint8_t times)
**�������� ����ȡ����ADC1����ֵ��ƽ��ֵ
**������� ��ADC_Channel_x , xΪ0~8
**������� ��puiADDataΪADC������ֵ
********************************************************************************************************/
u16 Get_Adc_Average(uint8_t ADC_Channel_x, uint8_t times)
{
    u32 temp_val = 0;
    u8 t;
    u8 delay;
    for(t = 0; t < times; t++) {
        temp_val += ADC1_SingleChannel_Get(ADC_Channel_x);
        for(delay = 0; delay < 100; delay++);
    }
    return temp_val / times;
}

/********************************************************************************************************
**������Ϣ ��delay_ms(__IO uint32_t nTime)
**�������� ������Ӧ�õ�����ʱ��ʹ��systick
**������� ��nTime����ʱ
**������� ����
********************************************************************************************************/
void delay_ms(__IO uint32_t nTime)
{
    TimingDelay = nTime;

    while(TimingDelay != 0);
}

/********************************************************************************************************
**������Ϣ ��void UartSendByte(u8 dat)
**�������� ��UART��������
**������� ��dat
**������� ��
**    ��ע ��
********************************************************************************************************/
void UartSendByte(u8 dat)
{
    UART_SendData(UART1, dat);
    while(!UART_GetFlagStatus(UART1, UART_FLAG_TXEPT));
}

/********************************************************************************************************
**������Ϣ ��void UartSendGroup(u8* buf,u16 len)
**�������� ��UART��������
**������� ��buf,len
**������� ��
**    ��ע ��
********************************************************************************************************/
void UartSendGroup(u8* buf, u16 len)
{
    while(len--)
        UartSendByte(*buf++);
}

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/*-------------------------(C) COPYRIGHT 2019 MindMotion ----------------------*/

