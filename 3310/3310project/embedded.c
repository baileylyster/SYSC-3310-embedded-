#include <msp.h>

//MARK: Constants
#define DEBOUNCE_TIME 1500
#define LED_RED 0
#define LED_RGB 1
#define RGB_LED_MASK 0x07
#define RGB_LED_OFFSET 0

// MARK: Function prototypes
static void incrementState(void);
static void decrementState(void);

// MARK: ISR Prototypes
void PORT1_IRQHandler(void);
void EUSCIA0_IRQHandler(void);

// MARK: Variable Definitions
uint8_t state = 0;




/**
 *  Initialize the pins as required for the application.
 */
static void setup (void)
{
    // Set direction to input
    P1->DIR &= ~((1<<1) | (1<<4));
    // Enable pull resistor
    P1->REN |= ((1<<1) | (1<<4));
    // Set pull direction to up
    P1->OUT |= ((1<<1) | (1<<4));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1<<1) | (1<<4));
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
    P1->IE &= ~((1<<0) | (1<<1) | (1<<2));
		
		//UART CONFIG FROM ECHO EXAMPLE
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
    EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) | EUSCI_A_MCTLW_OS16;

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        // Enable USCI_A0 RX interrupt

    // Enable sleep on exit from ISR
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;

    // Enable global interrupt
    __enable_irq();

    // Enable eUSCIA0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);
		
		//END UART CONFIG FROM ECHO EXAMPLE
		
		
}

int main(void)
{
    /* Stop watchdog timer */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    
    /* Configure GPIO */
    setup();
    
    /* Configure Interrupts */
    // Trigger interrupts on falling edge
    P1->IES |= ((1<<1) | (1<<4));
    // Clear interrupt flags
    P1->IFG &= ~((1<<1) | (1<<4));
    // Enable pin interrupts
    P1->IE |= ((1<<1) | (1<<4));
    
    NVIC_SetPriority(PORT1_IRQn, 2);
    NVIC->ICPR[PORT1_IRQn > 31] |= ((PORT1_IRQn % 32) << 1);
    NVIC_EnableIRQ(PORT1_IRQn);
    
    /* Main Loop */
    for (;;) {
        __WFI();
    }
    
	return 0;
}




void incrementState(void)
{
				if (state == 0) {
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 1;
        }
        else if (state == 1) {
            P1->OUT ^= ((1 << 0));
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 2;
        }
        else if (state == 2) {
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 3;
        }
        else if (state == 3) {
            P1->OUT ^= ((1 << 0));
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 0;
        }
				EUSCI_A0->TXBUF = state; //Sends the current State to the BUFFER for UART.
}


void decrementState(void)
{
				if (state == 2) {
            P1->OUT ^= ((1 << 0));
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 1;
        }
        else if (state == 3) {
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 2;
        }
        else if (state == 0) {
            P1->OUT ^= ((1 << 0));
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 3;
        }
        else if (state == 1) {
            P2->OUT ^= ((1 << 0) | (1 << 1) | (1 << 2));
            state = 0;
        }
				EUSCI_A0->TXBUF = state; //Sends the current State to the BUFFER for UART.
}



/* Interrupt Service Routines */
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
       
				//Check RXBUFFER for commands from Python
        if ((EUSCI_A0->RXBUF == '+') || (EUSCI_A0->RXBUF == '-')){
					
						if(EUSCI_A0->RXBUF == '+'){
								incrementState();
						}
						if(EUSCI_A0->RXBUF == '-'){
								decrementState();
						}
						//reset flag
            EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;
						
							
						}
				}
    
	}



