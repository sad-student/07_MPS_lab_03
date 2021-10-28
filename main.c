#include <msp430.h>

#define TAxCCR_05Hz 0xffff /* timer upper bound count value */

short unsigned int state = 0;
short unsigned int timer = 0;
unsigned int counter = 0;

#pragma vector = PORT1_VECTOR
__interrupt void S1_handler(void){
	if(P1IFG & BIT7){
        state = ~state;
        if (state) {
            TA1CCR1 = TA1R + (unsigned int)(TAxCCR_05Hz / 2 * 1.5);
        } else {
            TA1CCR1 = TA1R + (unsigned int)(TAxCCR_05Hz / 2 * 0.9);
        }
		TA1CCTL1 = (TA1CCTL1 & (~0x08)) | CCIE;

        // if (timer) {
        //     WDTCTL = (WDTCTL & (~0x0ff80)) | (WDTPW | (~WDTHOLD & (0x080)));
        // } else {
        //
        // }
		P1IFG &= ~BIT7;
	}
}

#pragma vector = PORT2_VECTOR
__interrupt void S2_handler(void){
	if(P2IFG & BIT2){
		//
        timer = ~timer;
        counter = 0;
        if (timer) {
            TA1CCTL1 = (TA1CCTL1 & (~0x08)) | (~CCIE & (0x08));
        } else {
            WDTCTL = WDTPW | WDTHOLD;
        }
		P2IFG &= ~BIT2;
	}
}

#prgma vector = WDT_VECTOR
__interrupt void WDT_handler(void){
    if (state) {
        if(counter < 9){
            ++counter;
            return;
        }
        counter = 0;
        if (~P1OUT & BIT3){
            P1OUT ^= BIT3;
        } else if (~P1OUT & BIT4){
            P1OUT ^= BIT4;
        } else if (~P1OUT & BIT5){
            P1OUT ^= BIT5;
            WDTCTL = WDTPW | WDTHOLD;
        }
        WDTCTL = WDTPW | WDTCNTCL;
    } else {
        if(counter < 14){
            ++counter;
            return;
        }
        counter = 0;
        if (P1OUT & BIT5){
            P1OUT ^= BIT5;
        } else if (P1OUT & BIT4){
            P1OUT ^= BIT4;
        } else if (P1OUT & BIT3){
            P1OUT ^= BIT3;
            WDTCTL = WDTPW | WDTHOLD;
        }
        WDTCTL = WDTPW | WDTCNTCL;
    }
}

// #pragma vector = TIMER1_A0_VECTOR
// __interrupt void TA1_0_handler(void){
// 	P1OUT ^= BIT2;
// }

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_handler(void){
	// if (state){
	// 	switch(TA1IV){
	// 		case 0x02:
	// 			if (~P1OUT & BIT5){
	// 				P1OUT ^= BIT5;
	// 			} else if (~P1OUT & BIT4){
	// 				P1OUT ^= BIT4;
	// 			} else if (~P1OUT & BIT3){
	// 				P1OUT ^= BIT3;
    //                 TA1CCTL1 = (TA1CCTL1 & (~0x08)) | (~CCIE & (0x08));
    //                 break;
	// 			}
	// 			// change comparison value
	// 			TA1CCR1 += (unsigned int)(TAxCCR_05Hz / 2 * 1.5);
	// 			break;
	// 		default:
	// 			break;
	// 	}
	// } else { // S1 pushed
	// 	switch(TA1IV){
	// 		case 0x02:
	// 			if (P1OUT & BIT3){
    //                 P1OUT ^= BIT3;
	// 			} else if (P1OUT & BIT4){
	// 				P1OUT ^= BIT4;
	// 			} else if (P1OUT & BIT5){
    //                 P1OUT ^= BIT5;
    //                 TA1CCTL1 = (TA1CCTL1 & (~0x08)) | (~CCIE & (0x08));
    //                 break;
	// 			}
	// 			// change comparison value
	// 			TA1CCR1 += (unsigned int)(TAxCCR_05Hz / 2 * 0.9);
	// 			break;
	// 		default:
	// 			break;
	// 	}
	// }
    switch(TA1IV){
        case 0x02:
            if (state) {
                if (~P1OUT & BIT3){
                    P1OUT ^= BIT3;
                } else if (~P1OUT & BIT4){
                    P1OUT ^= BIT4;
                } else if (~P1OUT & BIT5){
                    P1OUT ^= BIT5;
                    TA1CCTL1 = (TA1CCTL1 & (~0x08)) | (~CCIE & (0x08));
                    break;
                }
                // change comparison value
                TA1CCR1 += (unsigned int)(TAxCCR_05Hz / 2 * 0.9);
            } else {
                if (P1OUT & BIT5){
					P1OUT ^= BIT5;
				} else if (P1OUT & BIT4){
					P1OUT ^= BIT4;
				} else if (P1OUT & BIT3){
					P1OUT ^= BIT3;
                    TA1CCTL1 = (TA1CCTL1 & (~0x08)) | (~CCIE & (0x08));
                    break;
				}
				// change comparison value
				TA1CCR1 += (unsigned int)(TAxCCR_05Hz / 2 * 1.5);
            }
            break;
        default:
            break;
    }
}

// #pragma vector = TIMER0_A0_VECTOR
// __interrupt void TA0_0_handler(void){
// 	P1OUT ^= BIT2;
// }

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TA0_handler(void){
	switch(TA0IV){
	case 0x04:
		P1OUT ^= BIT2;
		break;
	default:
		break;
	}
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    __bis_SR_register(GIE);

    P1SEL &= ~(BIT2 | BIT3 | BIT4 | BIT5);
    P1DIR |= (BIT2 | BIT3 | BIT4 | BIT5);
    P1OUT &= ~(BIT2 | BIT3 | BIT4 | BIT5);

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
	UCSCTL1 &= ~DISMOD; // enable modulation

	//UCSCTL5 = (UCSCTL5 & (~0x07)) | DIVM__1;
	UCSCTL4 = (UCSCTL4 & (~0x070)) | SELS__DCOCLKDIV;
	UCSCTL5 = (UCSCTL5 & (~0x070)) | DIVS__1;

	TA0CTL = (TA0CTL & (~0x0300)) | TASSEL__SMCLK;
	//TA0CTL = (TA0CTL & (~0x030)) | MC__UP;
	TA0CTL = (TA0CTL & (~0x030)) | MC__UPDOWN;
	TA0CTL = (TA0CTL & (~0x0c0)) | ID__16;
	TA0CTL |= TACLR;
	TA0CCR0 = TAxCCR_05Hz;

	TA0CCR2 = 0x8000; // 0.5s for up + 0.5s for down counting if clocked with 1MHz/8
	TA0CCTL2 = (TA0CCTL2 & (~0x0100)) & ~CAP;
	TA0CCTL2 = (TA0CCTL2 & (~0x0f0)) | OUTMOD_7;
	TA0CCTL2 = (TA0CCTL2 & (~0x08)) | CCIE;

	TA1CTL = (TA1CTL & (~0x0300)) | TASSEL__SMCLK;
	TA1CTL = (TA1CTL & (~0x030)) | MC__CONTINOUS;
	TA1CTL = (TA1CTL & (~0x0c0)) | ID__32;
	TA1CTL |= TACLR;
	TA1CCR0 = TAxCCR_05Hz;

	TA1CCR1 = 0x8000; // 0.5s for up + 0.5s for down counting if clocked with 1MHz/8
	TA1CCTL1 = (TA1CCTL1 & (~0x0100)) & ~CAP;
	TA1CCTL1 = (TA1CCTL1 & (~0x0f0)) | OUTMOD_7;
	// TA1CCTL1 = (TA1CCTL1 & (~0x08)) | CCIE;


    // configure aclk to be clocked with VLO (9.4 KHz)
    // divide aclk by 32(*32)
    // set WDT to be clocked wit 10Hz frequency
    // divide by 9 to gain 1.1Hz (0.9s period)
    // divide by 15 to gain 0.66Hz (1.5s period)

    UCSCTL4 = (UCSCTL4 & (~0x0700)) | SELA__VLOCLK;
    UCACTL5 = (UCSCTL5 & (~0x07000)) | DIVPA__32;

    WDTCTL = (WDTCTL & (~0x0ff7f)) |
        (WDTPW | WDTSSELA | WDTTMSEL | WDTCNTCL | WDTIS_1);
	return 0;
}
