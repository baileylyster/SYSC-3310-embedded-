static void setup(void)
{
    //Inputs
    P1->DIR &= ~((1 << 0) | (1 << 1));
    P1->REN |= ((1 << 0) | (1 << 1));
    P1->OUT |= ((1 << 0) | (1 << 1));
    P1->IE &= ~((1 << 0) | (1 << 1));

    //Outputs
    P2->DIR |= ((1 << 0) | (1 << 1));
    P2->DS &= ~((1 << 0) | (1 << 1));
    P2->OUT &= ~((1 << 0) | (1 << 1));
    P1->IE &= ~((1 << 0) | (1 << 1));
}



int main(void)
{
    // Stop watchdog timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
    setup();

    /* Configure and Start Timer */
    
    TIMER_A0->CTL = (TIMER_A_CTL_TASSEL_1 | TIMER_A_CTL_ID_0 | TIMER_A_CTL_MC_1 | TIMER_A_CTL_IE);
    // Configure Interrupts 
    P1->IES &= ~(1 << 0);
    P1->IFG &= ~(1 << 0);
    P1->IE |= (1 << 0);

    NVIC_SetPriority(PORT1_IRQn, 2);
    NVIC->ICPR[PORT1_IRQn > 31] |= ((PORT1_IRQn % 32) << 0);
    NVIC_EnableIRQ(PORT1_IRQn);

    /* Main Loop */
    for (;;)
    {
        __WFI();
    }
}

static timerON(void){
    P2OUT |= (uint8_t)(1<<1);  //turn on LED
    TIMER_A0->CCR[0] = 32768; //Max value at 32768, 1 sec at 32768hz
    
}

void PORT1_IRQHandler(void)
{
    if ((P1IFG & (uint8_t)(1 << 0)) != 0 && (P1IFG & (uint8_t)(1 << 1)) != 0) // If p1.1 and p1.0 occur simultaneously, dispose mask
    {
        P1IFG &= (uint8_t)(~((1 << 0) | (1 << 1)));
        if (!(P1IN & (uint8_t)(1 << 0)) && !(P1IN & (uint8_t)(1 << 1)))
        {           
            P2OUT |= (uint8_t)(1 << 0); // Give mask
            timerOn(); //turns on timer, which 
            P2OUT |= (uint8_t)(1<<1);  //Reset Pin
        }
    }
}

void TA0_N_IRQHandler(void)
{
    if (TIMER_A0->CTL & TIMER_A_CTL_IFG) {
            P2->OUT ^= (uint8_t) (1<<2)
        }
        TIMER_A0->CTL &= ~(TIMER_A_CTL_IFG); //reset
        P2OUT &= (uint8_t)(~(1<<1)); //turns off led

}

}