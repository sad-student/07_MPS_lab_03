#include <msp430.h> 

#pragma vector = PORT1_VECTOR
__interrupt void S1_handler(void){
	if(P1IFG & BIT7){
		//
		P1IFG &= ~BIT7;
	}
}

#pragma vector = PORT2_VECTOR
__interrupt void S2_handler(void){
	if(P2IFG & BIT2){
		//
		P2IFG &= ~BIT2;
	}
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    __bis_SR_register(GIE);

	P1SEL &= ~BIT7;
	P1DIR &= ~BIT7;
	P1OUT |= BIT7;
	P1REN |= BIT7;
	P1IES |= BIT7;
	P1IFG &= ~BIT7;
	P1IE |= BIT7;


	P2SEL &= ~BIT2;
	P2DIR &= ~BIT2;
	P2OUT |= BIT2;
	P2REN |= BIT2;
	P2IES |= BIT2;
	P2IFG &= ~BIT2;
	P2IE |= BIT2;

	// configure SMCLK for frequency 1MHz

	TA0CTL = (TA0CTL & (~0x0300)) | TASSEL__SMCLK;
	//TA0CTL = (TA0CTL & (~0x030)) | MC__UP;
	TA0CTL = (TA0CTL & (~0x030)) | MC__CONTINOUS;
	TA0CTL = (TA0CTL & (~0x0c0)) | ID__1;
	TA0CTL |= TACLR;

	TA0CCRN0 = 0x8000; // 0.5s for up + 0.5s for down counting if clocked with 1MHz/8
	TA0CCTL2 = (TA0CCTL2 & (~0x0f0)) | OUT_MOD7;
	TA0CCTL2 = (TA0CCTL2 & (~0x08)) | CCIE;


	
	return 0;
}
