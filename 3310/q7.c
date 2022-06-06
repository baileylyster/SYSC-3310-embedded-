/*You're responsible for building an embedded system that controls a very simple vending machine that sells face masks (each costs $1).
 The vending machine accepts a one dollar coin and outputs a disposable face mask. When it outputs a face mask, 
it lights up a light signal saying "you're welcome" during one second. While the light is on, the system shouldn't accept any more coins.
The embedded system is connected to (1) one coin sensor connected to a pin (P1.0): transitions from 0 to 1 when a coin is detected for a short time. 
(2) one coin recognition sensor connected to a pin (P1.1): at the same time as sensor (1) detects coin, this sensor is 1 if the coin is identified as a one dollar coin.
(3) an actuator that outputs a face mask when you write "1" to the pin it's connected to (P2.0).
(4) an actuator that lights the "you're welcome" signal while the pin it's connected to is "1" (P2.1).    
Write the C code that implements this embedded system (all the configurations, main function, and any ISRs, if any). Write down any assumptions as comments, if required.
 If you're unsure about something, write down a reasonable assumption explaining.
*/



// Stop watchdog timer
WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
static void setup(void)
{
    //Inputs
    // Set direction to input
    P1->DIR &= ~((1 << 0) | (1 << 1));
    // Enable pull resistor
    P1->REN |= ((1 << 0) | (1 << 1));
    // Set pull direction to up
    P1->OUT |= ((1 << 0) | (1 << 1));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1 << 0) | (1 << 1));

    //Outputs
    // Set direction to output
    P2->DIR |= ((1 << 0) | (1 << 1));
    // Ensure that high drive strength is disabled
    P2->DS &= ~((1 << 0) | (1 << 1));
    // Initialize to driven low
    P2->OUT &= ~((1 << 0) | (1 << 1));
    // Ensure that interrupts are disabled
    P1->IE &= ~((1 << 0) | (1 << 1));
    TIMER_A0->CTL = (TIMER_A_CTL_TASSEL_1 | TIMER_A_CTL_ID_0 | TIMER_A_CTL_MC_1 | TIMER_A_CTL_IE);  //ACLK, divide mode set to  1, IE
    
     NVIC_SetPriority(TA0_N_IRQn, 2);      //NVIC config
     NVIC_ClearPendingIRQ(TA0_N_IRQn);
     NVIC_EnableIRQ(TA0_N_IRQn);
     __ASM("CPSIE I"); //machine level interrupt
}

static timerON(void){
    TIMER_A0->CCR[0] = 32768; //Max value at 32768, 1 sec at 32768hz
}









int main(void)
{
    /* Configure Interrupts */
    // Trigger interrupts on rising edge
    P1->IES &= ~(1 << 0);
    // Clear interrupt flags
    P1->IFG &= ~(1 << 0);
    // Enable pin interrupts
    P1->IE |= (1 << 0);

    NVIC_SetPriority(PORT1_IRQn, 2);
    NVIC->ICPR[PORT1_IRQn > 31] |= ((PORT1_IRQn % 32) << 0);
    NVIC_EnableIRQ(PORT1_IRQn);
    setup();

    / Main Loop */
    for (;;)
    {
        __WFI();
    }
}
void TA0_N_IRQHandler(void)
{
    if (TIMER_A0->CTL & TIMER_A_CTL_IFG) {
            P2->OUT ^= (uint8_t) (1<<2)
        }
        TIMER_A0->CTL &= ~(TIMER_A_CTL_IFG); //reset
}
}
