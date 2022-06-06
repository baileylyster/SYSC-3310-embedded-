#include "msp.h"


#define DEBOUNCE_TIME 1500

uint8_t state = 0b00000000; //Declare State as 00, floor state. 


int main(void){
		//UART CONFIG STUFF.
		WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;             // Stop watchdog timer

    CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
            CS_CTL1_SELS_3 |                // SMCLK = DCO
            CS_CTL1_SELM_3;                 // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses

    // Configure UART pins
    P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_B_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK
    // Baud Rate calculation
    // 12000000/(16*9600) = 78.125
    // Fractional portion = 0.125
    // User's Guide Table 21-4: UCBRSx = 0x10
    // UCBRFx = int ( (78.125-78)*16) = 2
    EUSCI_A0->BRW = 78;                     // 12000000/16/9600
    EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) |
            EUSCI_A_MCTLW_OS16;

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt

    // Enable sleep on exit from ISR
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCIA0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);

    // Enter LPM0
    __sleep();
    __no_operation();                       // For debugger


		//END UART CONFIG 
		
		//GPIO CONFIG STUFF
		// Set direction to input
    P1->DIR &= ~((1<<1) | (1<<4));
    // Enable pull resistor
    P1->REN |= ((1<<1) | (1<<4));
    // Set pull direction to up
    P1->OUT |= ((1<<1) | (1<<4));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1<<1) | (1<<4));
    
    /* LEDs (P1.0, P2.0, P2.1, P2.2) */
    // Set direction to output
    P1->DIR |= (1<<0);
    P2->DIR |= ((1<<0) | (1<<1) | (1<<2));
    // Ensure that high drive strength is disabled
    P1->DS &= ~(1<<0);
    P2->DS &= ~((1<<0) | (1<<1) | (1<<2));
    // Initialize to driven low
    P1->OUT &= ~(1<<0);
    P2->OUT &= ~((1<<0) | (1<<1) | (1<<2));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1<<0));
    P2->IE &= ~((1<<0) | (1<<1) | (1<<2));
		
		
		//Config NVIC and Interrupts
		P1->IES |= ((1<<1) | (1<<4));
    // Clear interrupt flags
    P1->IFG &= ~((1<<1) | (1<<4));
    // Enable pin interrupts
    P1->IE |= ((1<<1) | (1<<4));
    
    NVIC_SetPriority(PORT1_IRQn, 2);
    NVIC->ICPR[PORT1_IRQn > 31] |= ((PORT1_IRQn % 32) << 1);
    NVIC_EnableIRQ(PORT1_IRQn);
		
		
		
		for (;;) {
        __WFI();
    }
    
		return 0; // never reached (hopefully)
}
//Function that uses the state to set the LEDS.
void updateLED(){
	if(state == 0b00000011){
		P1->OUT &= (1<<0);
		P2->OUT &= ((1<<0) | (1<<1) | (1<<2));
	}
	if(state == 0b00000010){
		P1->OUT &= (1<<0);
		P2->OUT &= ~((1<<0) | (1<<1) | (1<<2));
	}
	if (state == 0b00000001){
		P1->OUT &= ~(1<<0);
		P2->OUT &= ((1<<0) | (1<<1) | (1<<2));
	}
	else{
		P1->OUT &= ~(1<<0);
		P2->OUT &= ~((1<<0) | (1<<1) | (1<<2));
	}
}
//Sends the current State to the BUFFER for UART.
void sendState(){
	EUSCI_A0->TXBUF = state;
}

		
		

//When BUTTONA or USB instructs it to.
void incrementState(){
	if (state != 0b11){
		state = state +1;
		updateLED();
		sendState();
	}
	else{
		state = 0b00;
		updateLED();
		sendState();
	}
}

//When BUTTONB or USB instructs
void decrementState(){
	if(state != 0b00000000){
		state = state -1;
		updateLED();
		sendState();
	}
	else{
		state = 0b00000011;
		updateLED();
		sendState();
	}
}









//Configuring ButtonA(1.1) and ButtonB(1.4) Interrupts
void PORT1_IRQHandler(void)
{
    if (P1->IFG & (1<<1)) {
        // Select LED Button
        for (uint32_t i = 0; i < DEBOUNCE_TIME; i++);
        if (!(P1->IN & (1<<1))) {
            incrementState();
        }
        P1->IFG &= ~(1<<1);
    }
    
    if (P1->IFG & (1<<4)) {
        // Select Mode Button
        for (uint32_t i = 0; i < DEBOUNCE_TIME; i++);
        if (!(P1->IN & (1<<4))) {
            decrementState();
        }
        P1->IFG &= ~(1<<4);
    }
}


// UART interrupt service routine
void EUSCIA0_IRQHandler(void)
{
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)
    {
        // Check if the TX buffer is empty first
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));

				//State is set to the value stored in the buffer.
        if ((EUSCI_A0->RXBUF == '+') || (EUSCI_A0->RXBUF == '-')){
					if(EUSCI_AO->RXBUF == '+'){
						incrementState();
					}
					if(EUSCI_AO->RXBUF == '-'){
						decrementState();
					}
				}
    }
}







