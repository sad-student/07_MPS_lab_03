#include <msp430.h>

#define TAxCCR_05Hz 0xffff /* timer upper bound count value */
#define BUTTON_DELAY 0x0500

short unsigned int state = 0;
short unsigned int timer = 0;
unsigned int counter = 0;

unsigned int glitch_counters[] = { 0, 0 };

#pragma vector = PORT1_VECTOR
__interrupt void S1_handler(void){
	if(P1IFG & BIT7){
		if (glitch_counters[0] == 0){
			if(P1IES & BIT7){
				state = ~state;

				if(timer){
					//WDT delays
					counter = 0;
					WDTCTL = (WDTCTL & (~0x0ff88)) | (WDTPW | (~WDTHOLD & (0x080)) | WDTCNTCL);
					SFRIE1 = (SFRIE1 & (~0x01)) | WDTIE;
				} else {
					// TA delays
					TA1CCTL1 = (TA1CCTL1 & (~0x01)) & ~CCIFG;
					if (state) {
						TA1CCR1 = TA1R + (unsigned int)(TAxCCR_05Hz / 2 * 0.9);
					} else {
						TA1CCR1 = TA1R + (unsigned int)(TAxCCR_05Hz / 2 * 1.5);
					}
					TA1CCTL1 = (TA1CCTL1 & (~0x010)) | CCIE;
				}
			}
			P1IES ^= BIT7;
		}

		++glitch_counters[0];
		TA2CCR1 = TA2R + BUTTON_DELAY;
		TA2CCTL1 = (TA2CCTL1 & (~0x010)) | CCIE;

		P1IFG &= ~BIT7;
	}
}

#pragma vector = PORT2_VECTOR
__interrupt void S2_handler(void){
	if(P2IFG & BIT2){
		if(glitch_counters[1] == 0){
			if(P2IES & BIT2){
				timer = ~timer;
				counter = 0;
				if (timer) {
					TA1CCTL1 = (TA1CCTL1 & (~0x010)) | (~CCIE & (0x010));
				} else {
					WDTCTL = (WDTCTL & (~0x0ff80)) | (WDTPW | WDTHOLD);
					SFRIE1 = (SFRIE1 & (~0x01)) | (~WDTIE & (0x01));
				}
			}
			P2IES ^= BIT2;
		}

		++glitch_counters[1];
		TA2CCR2 = TA2R + BUTTON_DELAY;
		TA2CCTL2 = (TA2CCTL2 & (~0x010)) | CCIE;

		P2IFG &= ~BIT2;
	}
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_handler(void){
    switch(TA1IV){
        case TA1IV_TACCR1:
            if (state) {
                if (~P1OUT & BIT2){
                    P1OUT ^= BIT2;
                } else if (~P1OUT & BIT4){
                    P1OUT ^= BIT4;
                } else if (~P1OUT & BIT5){
                    P1OUT ^= BIT5;
                    TA1CCTL1 = (TA1CCTL1 & (~0x010)) | (~CCIE & (0x010)); // & ~CCIE;
                }
                // change comparison value
                TA1CCR1 += (unsigned int)(TAxCCR_05Hz / 2 * 0.9);
            } else {
                if (P1OUT & BIT5){
					P1OUT ^= BIT5;
				} else if (P1OUT & BIT4){
					P1OUT ^= BIT4;
				} else if (P1OUT & BIT2){
					P1OUT ^= BIT2;
                    TA1CCTL1 = (TA1CCTL1 & (~0x010)) | (~CCIE & (0x010)); // & ~CCIE;
				}
				// change comparison value
				TA1CCR1 += (unsigned int)(TAxCCR_05Hz / 2 * 1.5);
            }
            break;
        default:
            break;
    }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TA0_handler(void){
	switch(TA0IV){
		case TA0IV_TACCR2:
			//P1OUT ^= BIT2;
			break;
		default:
			break;
	}
}

#pragma vector = TIMER2_A1_VECTOR
__interrupt void TA2_handler(void){
	switch(TA2IV){
		case TA2IV_TACCR1:
			// S1
			glitch_counters[0] = 0;
			TA2CCTL1 = (TA2CCTL1 & (~0x010)) | (~CCIE & (0x010)); // & ~CCIE;
			break;
		case TA2IV_TACCR2:
			// S2
			glitch_counters[1] = 0;
			TA2CCTL2 = (TA2CCTL2 & (~0x010)) | (~CCIE & (0x010)); // & ~CCIE;
			break;
		default:
			break;
	}
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_handler(void){
    if (state) {
        if(counter < 12){
            ++counter;
            return;
        }
        counter = 0;
        if (~P1OUT & BIT2){
            P1OUT ^= BIT2;
        } else if (~P1OUT & BIT4){
            P1OUT ^= BIT4;
        } else if (~P1OUT & BIT5){
            P1OUT ^= BIT5;
        	SFRIE1 = (SFRIE1 & (~0x01)) | (~WDTIE & (0x01));
        }
    	WDTCTL = (WDTCTL & (~0x0ff08)) | (WDTPW | WDTCNTCL);
    } else {
        if(counter < 19){
            ++counter;
            return;
        }
        counter = 0;
        if (P1OUT & BIT5){
            P1OUT ^= BIT5;
        } else if (P1OUT & BIT4){
            P1OUT ^= BIT4;
        } else if (P1OUT & BIT2){
            P1OUT ^= BIT2;
        	SFRIE1 = (SFRIE1 & (~0x01)) | (~WDTIE & (0x01));
        }
    	WDTCTL = (WDTCTL & (~0x0ff08)) | (WDTPW | WDTCNTCL);
    }
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    __bis_SR_register(GIE);

//    P1SEL &= ~(BIT2 | BIT3 | BIT4 | BIT5);
//    P1DIR |= (BIT2 | BIT3 | BIT4 | BIT5);
//    P1OUT &= ~(BIT2 | BIT3 | BIT4 | BIT5);


    P1SEL &= ~(BIT2 | BIT4 | BIT5);
    P1DIR |= (BIT2 | BIT4 | BIT5);
    P1OUT &= ~(BIT2 | BIT4 | BIT5);

    P1SEL |= BIT3;
    P1DIR |= BIT3;

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
//	P5SEL &= ~(BIT4 | BIT5);
//	P5DIR &= ~BIT4;
//	P5DIR |= BIT5;
//	UCSCTL6 &= ~XT1BYPASS;
//
	UCSCTL3 = (UCSCTL3 & (~0x070)) | SELREF__XT1CLK;
	UCSCTL3 = (UCSCTL3 & (~0x07)) | FLLREFDIV__2;
	UCSCTL2 = (UCSCTL2 & (~0x0cff)) | ((8 - 1) & (0x0cff)); // FLLN multiplier
	UCSCTL2 = (UCSCTL2 & (~0x07000)) | FLLD__8; // FLLD divider
	UCSCTL1 = (UCSCTL1 & (~0x070)) | DCORSEL_1; // frequency range setting

	UCSCTL4 = (UCSCTL4 & (~0x070)) | SELS__DCOCLKDIV;
	UCSCTL5 = (UCSCTL5 & (~0x070)) | DIVS__1;

	TA0CTL = (TA0CTL & (~0x0300)) | TASSEL__SMCLK;
	//TA0CTL = (TA0CTL & (~0x030)) | MC__UP;
	TA0CTL = (TA0CTL & (~0x030)) | MC__UPDOWN;
	TA0CTL = (TA0CTL & (~0x0c0)) | ID__1;
	TA0CTL |= TACLR;
	TA0CCR0 = TAxCCR_05Hz;

	TA0CCR2 = 0x8000; // 0.5s for up + 0.5s for down counting if clocked with 1MHz/8
	TA0CCTL2 = (TA0CCTL2 & (~0x0100)) & ~CAP;
	TA0CCTL2 = (TA0CCTL2 & (~0x034e4)) | (CCIS_3 | SCCI | OUTMOD_7 | OUT);
	TA0CCTL2 = (TA0CCTL2 & (~0x010)) | CCIE;


	TA1CTL = (TA1CTL & (~0x0300)) | TASSEL__SMCLK;
	TA1CTL = (TA1CTL & (~0x030)) | MC__CONTINOUS;
	TA1CTL = (TA1CTL & (~0x0c0)) | ID__4;
	TA1CTL |= TACLR;
	TA1CCTL1 = (TA1CCTL1 & (~0x0100)) & ~CAP;
	TA1CCTL1 = (TA1CCTL1 & (~0x010)) & ~CCIE;
	
	TA2CTL = (TA2CTL & (~0x0300)) | TASSEL__SMCLK;
	TA2CTL = (TA2CTL & (~0x030)) | MC__CONTINOUS;
	TA2CTL = (TA2CTL & (~0x0c0)) | ID__4;
	TA2CTL |= TACLR;
	// for button noise handling
	TA2CCTL1 = (TA2CCTL1 & (~0x0100)) & ~CAP;
	TA2CCTL2 = (TA2CCTL2 & (~0x0100)) & ~CAP;
	TA2CCTL1 = (TA2CCTL1 & (~0x010)) & ~CCIE;
	TA2CCTL2 = (TA2CCTL2 & (~0x010)) & ~CCIE;

//	TA1CCR1 = 0x8000; // 0.5s for up + 0.5s for down counting if clocked with 1MHz/8
//	TA1CCTL1 = (TA1CCTL1 & (~0x0100)) & ~CAP;
//	TA1CCTL1 = (TA1CCTL1 & (~0x0f0)) | OUTMOD_7;
	// TA1CCTL1 = (TA1CCTL1 & (~0x08)) | CCIE;


    // configure aclk to be clocked with VLO (9.4 KHz)
    // divide aclk by 32(*32)
    // set WDT to be clocked wit 10Hz frequency
    // divide by 9 to gain 1.1Hz (0.9s period)
    // divide by 15 to gain 0.66Hz (1.5s period)


	WDTCTL = (WDTCTL & (~0x0ff7f)) | (WDTPW | WDTSSEL__SMCLK | WDTTMSEL | WDTCNTCL | WDTIS__8192);
	return 0;
}
