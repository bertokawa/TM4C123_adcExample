/*
 * main.c
 */

// ISTO � um teste de dropbox

/*
 *	nome: 0_adcExample
 *
 *	Descrição:
 *		Programa para testar o uso do ADC no tm6c123gh6pm.
 *		Exemplo pego do site de Valvano Cap14: ADC Conveters
 *
 *	Funcionamento:
 *		Será ligado um sensor LDR na entrada do ADC e a saida será um LED.
 *		A cor do LED dependerá do valor lido pelo sensor.
 *
 */

/*
 * pg799 - ADC
 * pg649 - GPIOs
 *
 *  AIN0 - PE3(6)
 *  AIN1 - PE2(7)
 *  AIN2 - PE1(8)
 *  AIN3 - PE0(9)
 *  AIN4 - PD3(64)
 *  AIN5 - PD2(63)
 *  AIN6 - PD1(62)
 *  AIN7 - PD0(61)
 *  AIN8 - PE5(60)
 *  AIN9 - PE4(59)
 * AIN10 - PB4(58)
 * AIN11 - PB5(57)
 *
 * Clock Rate: 16MHz
 *
 *
 */

#include <tm4c123gh6pm.h>
#include <inttypes.h>

#define ADC_INPUT 0x040;

//---------------------
//-------FUNÇÕES-------

void initADC();
void initSequencer();

//-------------------------------

unsigned long valorADC = 0;

//--------------------
//---IMPLEMENTAÇÕES---

void initADC() {
	SYSCTL_RCGCGPIO_R |= 1 << 4;		// ativar clock na GPIO (pg340)
	SYSCTL_RCGCADC_R |= 0b11;			// ativar ADC clock (pg352): bit0 (ADC0), bit1 (ADC1)
	
	GPIO_PORTE_DIR_R &= ~ADC_INPUT;		// PE4 como input
	GPIO_PORTE_AFSEL_R |= ADC_INPUT;	// setar AFSEL para ADC inputs pins, enable alternate function on PE4 (pg1344)
	GPIO_PORTE_DEN_R &= ~ADC_INPUT; 	//AINx - GPIO_DEN_R; // configurar AINx para receber sinal analogico, limpando o DEN (digital enable) da GPIO (pg682)
	GPIO_PORTE_AMSEL_R |= ADC_INPUT; 	// desabilitar a ANALOG ISOLATION dos pinos de entradas (pg687)
	//ADCSSPRI; // configurar a prioridade de aquisição de dados 0(high) para 3 (low). (p823)
}

void initSequencer() {
	ADC0_ACTSS_R &= ~0b1000; // Desabilitar o sequencer para configuraçao (p821)
	ADC0_PP_R |= ADC_PP_MSR_125K; // configurar para 125k samples (p889)
	ADC0_SSPRI_R = 0x123;			// Sequencer 3 is highest priority
	ADC0_EMUX_R |= ADC_EMUX_EM3_PROCESSOR;			// Configurar o trigger para o sample sequencer (p833): software trigger
	//ADCTSSEL_R; // ADC Trigger Source Select - selecionar o PWM quando utilizado (p839)
	ADC0_SSMUX3_R &= ~0x000F;		// Sonfigurar o correspondente entrada de cada sample dos sample sequence (p851): clear SS3 field
	ADC0_SSMUX3_R += 9;				// Seta o canl AIN9 (PE4)
	ADC0_SSCTL3_R = 0x0006;			// configurar os bits de controle, setar o bit final END. (p853): [no TS0 D0, yes IE0 END0
	//ADCIM_R; // se for utilizar interrupções, setar o MASK bit (p825)
	ADC0_ACTSS_R |=  0b1000; // Habilitar o senquencer (p821)
}

unsigned long adcInSeq3 () {
	unsigned long result;
	ADC0_PSSI_R = 0x0008;					// 1) initiate
	while ((ADC0_RIS_R & 0x08) == 0) {};	// 2) wait for conversion done
	result = ADC0_SSFIFO3_R & 0xFFF;		// 3) read result
	ADC0_ISC_R = 0x008;						// 4) acknowledge completion
	
	return result;
}

int main(void) {
	SYSCTL_RCGCGPIO_R |= 1 << 5;
	
	GPIO_PORTF_DIR_R  |= 0b1110;
	GPIO_PORTF_DEN_R  |= 0b1110;

	initADC();
	initSequencer();

	while (1) {
		valorADC = adcInSeq3();
		
		if (valorADC >= 1000) {
			GPIO_PORTF_DATA_R = 0b1110;
		} else if (valorADC < 1000 && valorADC > 0) {
			GPIO_PORTF_DATA_R = 0b1100;
		} else if (valorADC == 0) {
			GPIO_PORTF_DATA_R = 0;
		} else {
			GPIO_PORTF_DATA_R = 0b10;
		}
	}
	
	return 0;
}
