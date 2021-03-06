//FIXME
#include "commandProcessing.h"
#include "errorHandlers.h"
#include <inttypes.h>
#include "digitalIO.h"
#include "analogIN.h"
#include "timers.h"
#include "internalDAC.h"
#include <stm32f4xx_adc.h>
#include "continiousADC.h"

#define GLUE(a,b) (a ## b)

u8_t Cluster::check_for_parameters(u8_t expected)
{
		if( this->nrOfParameters != expected )
		{
			send_error_code(1);
			return false;
		}
		return true;
}

void send_debug_info(char * txt)
{
	OnePacket response;
	sprintf((char *)response.data,"DEBUG:%s",txt);
	response.length = strlen((char *)response.data);
	xQueueSend(sendQueue, &response, 0);
}


void send_ok_resp()
{
	OnePacket response;
	response.data = (uint8_t *)pvPortMalloc(2);
	response.length = 2;
	response.data[0] = 'O';
	response.data[1] = 'K';
	xQueueSend(sendQueue, &response, 0);
}

static u8_t xxx;
void Cluster::parseCommand()
{
	OnePacket response;
	xxx = command;
	if( command == COMMAND_TEST )
	{
		//test commands
		if( !check_for_parameters(2) ) return;
		if( nrOfParameters == 2 && parameters[0] == 2 && parameters[1] == 3 )
		{
				response.data = (uint8_t *)pvPortMalloc(2);
				response.length = 2;
				response.data[0] = 'O';
				response.data[1] = 'K';
				xQueueSend(sendQueue, &response, 0);
		}
		else 
		{
			send_error_code(3);
		}
	}
	else if( command == COMMAND_digitalIOSetPinMode )
	{
		//void digitalIOSetPinMode(u8_t pinNumber, GPIOMode_TypeDef direction, GPIOOType_TypeDef OutType, GPIOPuPd_TypeDef pullUps);
		
		if( !check_for_parameters(4) ) return;
		GPIOMode_TypeDef a;
		GPIOOType_TypeDef b;
		GPIOPuPd_TypeDef c;
		if( parameters[1] == 1 )
			a = GPIO_Mode_OUT;
		else 
			a = GPIO_Mode_IN;
		
		if( parameters[2] == 1 )
			b = GPIO_OType_OD;
		else 
			b = GPIO_OType_PP;
		
		if( parameters[3] == 1 )
			c = GPIO_PuPd_DOWN;
		else if( parameters[3] == 2 )
			c = GPIO_PuPd_UP;
		else
			c = GPIO_PuPd_NOPULL;
		
		digitalIOSetPinMode(parameters[0], a,b,c);
		send_ok_resp();
	}
	else if( command == COMMAND_digitalIOWritePin )
	{
		if( !check_for_parameters(2) ) return;
		//void digitalIOWritePin(uint8_t pinNumber, uint8_t pinValue)
		digitalIOWritePin(parameters[0],parameters[1]);
		send_ok_resp();
	}
	else if( command == COMMAND_digitalIOreadPin )
	{
		if( !check_for_parameters(1) ) return;
		//uint8_t digitalIOreadPin(uint8_t pinNumber);
		response.data = (uint8_t *)pvPortMalloc(1);
		response.length = 1;
		response.data[0] = digitalIOreadPin(parameters[0]);;
		xQueueSend(sendQueue, &response, 0);
		//send_ok_resp();
	}
	else if( command == COMMAND_digitalIOWritePort )
	{
		//void digitalIOWritePort(uint16_t portValue)
		if( !check_for_parameters(1) ) return;
		digitalIOWritePort(parameters[0]);
		send_ok_resp();
	}
	else if( command == COMMAND_digitalIOReadPort )
	{
		//uint16_t digitalIOReadPort()
		if( !check_for_parameters(0) ) return;
		response.data = (uint8_t *)pvPortMalloc(1);
		response.length = 1;
		response.data[0] = digitalIOReadPort();
		xQueueSend(sendQueue, &response, 0);
	}
	else if( command == COMMAND_writeWaveform )
	{
		//void writeWaveform(uint8_t pin, uint8_t * values, uint8_t nrOfSamples, uint32_t period, uint16_t presc, uint8_t repetition)
		// pin - 0
		// nrOfSamples - 1
		// period - 2
		// presc - 3
		// rep -4
		if( !check_for_parameters(5+parameters[1]) ) return;
		writeWaveform(parameters[0], &parameters[5], parameters[1], parameters[2], parameters[3], parameters[4]);
		
		
		
	}
	
	//ANALOG BUILD_IN
	


	else if ( command == COMMAND_analogInit )
	{
		//void analogInit(ADC_TypeDef * ADCx, u8_t resolution, GPIO_TypeDef * GPIOx, uint8_t pin)
		if( !check_for_parameters(4) ) return;
		
		uint32_t res;
		if( parameters[1] == 0 )
			res = ADC_Resolution_12b;
		else if( parameters[1] == 1 )
			res = ADC_Resolution_10b;
		else if( parameters[1] == 2 )
			res = ADC_Resolution_8b;
		else if( parameters[1] == 3 )
			res = ADC_Resolution_6b;
		else 
			send_error_code(4);
			
		
		GPIO_TypeDef * aaa;
		if( parameters[2] == 0 )
			aaa = GPIOA;
		else if( parameters[2] == 1 )
			aaa = GPIOB;
		else if( parameters[2] == 2 )
			aaa = GPIOC;
		else if( parameters[2] == 3 )
			aaa = GPIOD;
		else if( parameters[2] == 4 )
			aaa = GPIOE;
		else 
			send_error_code(4);
		
		if( parameters[0] == 0 )
			analogInit(ADC1, parameters[1], aaa, parameters[3]);
		else if( parameters[0] == 1 )
			analogInit(ADC2, parameters[1], aaa, parameters[3]);
		else if( parameters[0] == 2 )
			analogInit(ADC3, parameters[1], aaa, parameters[3]);
		else 
			send_error_code(4);
		send_ok_resp();
	}
	else if ( command == COMMAND_analogInitChannel )
	{
		//void analogInitChannel(ADC_TypeDef * ADCx, uint8_t channel, uint8_t ADC_SampleTime);
		if( !check_for_parameters(3) ) return;
		if( parameters[0] == 0 )
			analogInitChannel(ADC1, parameters[1], parameters[2]);
		else if( parameters[0] == 1 )
			analogInitChannel(ADC2, parameters[1], parameters[2]);
		else if( parameters[0] == 2 )
			analogInitChannel(ADC3, parameters[1], parameters[2]);
		send_ok_resp();
	}
	else if ( command == COMMAND_analogReadPin )
	{
		
		//uint16_t analogReadPin(ADC_TypeDef * ADCx, uint8_t channel);
		if( !check_for_parameters(2) ) return;
		uint16_t res = -1;
		if( parameters[0] == 0 )
			res = analogReadPin(ADC1, parameters[1]);
		else if( parameters[0] == 1 )
			res = analogReadPin(ADC2, parameters[1]);
		else if( parameters[0] == 2 )
			res = analogReadPin(ADC3, parameters[1]);
		
		TableOf16bits ret;// = (TableOf16bits *)pvPortMalloc(4);
		ret.length = 2;
		ret.data = (uint16_t *)pvPortMalloc(2);
		//ret->data[0] = htons(res);
		ret.data[0] = res;
		response = *((OnePacket *)&ret);
		xQueueSend(sendQueue, &response, 0);
	}
	
	
	// INTERNAL DAC
	
	else if ( command == COMMAND_internalDAC_Init )
	{
		//void internalDAC_Init(uint8_t pin); // 0 - PA4, 1 - PA5
		if( !check_for_parameters(1) ) return;
		internalDAC_Init(parameters[0]);
		send_ok_resp();
		
	}
	else if ( command == COMMAND_internalDAC_Write )
	{
		//void internalDAC_Write(uint8_t pin, uint16_t value, uint8_t align);
		if( !check_for_parameters(3) ) return;
		internalDAC_Write(parameters[0], parameters[1], parameters[2]);
		send_ok_resp();
	}
	
	
	// 			TIMERS
	
	
	else if ( command == COMMAND_getCoreClock )
	{
		//static inline uint32_t getCoreClock();
		if( !check_for_parameters(0) ) return;
		uint32_t clock = getCoreClock();
		
		TableOf16bits ret;//
		ret.data = (uint16_t *)pvPortMalloc(4);
		ret.length = 4;
		ret.data[0] = (clock >> 16);
		ret.data[1] = (clock & 0xFFFF);
		response = *((OnePacket *)&ret);
		xQueueSend(sendQueue, &response, 0);
	}
	else if ( command == COMMAND_TIMER_Init )
	{
		//static inline void TIMER_Init(TIM_TypeDef* TIMx, uint32_t period, uint16_t prescaler);
		if( !check_for_parameters(2) ) return;
		TIMER_Init(TIM3, parameters[0], parameters[1]);
		send_ok_resp();
	}

	
	//		continious ADC mode
	
	else if ( command == COMMAND_continiousADC_init )
	{
		//void continiousADC_init(uint32_t period, uint16_t prescaler)
		if( !check_for_parameters(2) ) return;
		continiousADC_init(parameters[0], parameters[1]);
	}
	else if ( command == COMMAND_continiousADC_stop )
	{
		//void continiousADC_init(uint32_t period, uint16_t prescaler)
		if( !check_for_parameters(0) ) return;
		continiousADC_stop();
		send_ok_resp();
	}
	
	else
	{
		send_error_code(2);
	}
}
