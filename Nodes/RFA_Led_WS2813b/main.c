#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
}LED_Color;

LED_Color LED_STRIPE[16];
LED_Color LED_STRIPE_STOP[16];
LED_Color LED_STRIPE_LEFT[16];
LED_Color LED_STRIPE_RIGHT[16];
LED_Color LED_STRIPE_THREE_WAYS[16];

void W0()
{
    PORTF = 0b00000001;
    _delay_us(0.2);
    PORTF = 0b00000000;
    _delay_us(0.6);
}

void W1()
{
    PORTF = 0b00000001;
    _delay_us(0.6);
    PORTF = 0b00000000;
    _delay_us(0.2);
}

uint8_t type = 0;


void processLed(LED_Color LED){
    
    if (LED.G & 128) {W1();}else{W0();}
    if (LED.G & 64) {W1();}else{W0();}
    if (LED.G & 32) {W1();}else{W0();}
    if (LED.G & 16) {W1();}else{W0();}
    if (LED.G & 8) {W1();}else{W0();}
    if (LED.G & 4) {W1();}else{W0();}
    if (LED.G & 2) {W1();}else{W0();}
    if (LED.G & 1) {W1();}else{W0();}
    
    if (LED.R & 128) {W1();}else{W0();}
    if (LED.R & 64) {W1();}else{W0();}
    if (LED.R & 32) {W1();}else{W0();}
    if (LED.R & 16) {W1();}else{W0();}
    if (LED.R & 8) {W1();}else{W0();}
    if (LED.R & 4) {W1();}else{W0();}
    if (LED.R & 2) {W1();}else{W0();}
    if (LED.R & 1) {W1();}else{W0();}
    
    if (LED.B & 128) {W1();}else{W0();}
    if (LED.B & 64) {W1();}else{W0();}
    if (LED.B & 32) {W1();}else{W0();}
    if (LED.B & 16) {W1();}else{W0();}
    if (LED.B & 8) {W1();}else{W0();}
    if (LED.B & 4) {W1();}else{W0();}
    if (LED.B & 2) {W1();}else{W0();}
    if (LED.B & 1) {W1();}else{W0();}
}

int main ()
{
    DDRF  = 0b00000001;
    PORTF = 0b00000000;
    
    _delay_ms(1);
    
    for (uint8_t ledID = 0; ledID < 16; ++ledID){
        LED_Color color;
        color.R = 255;
        color.G = 0;
        color.B = 100;
        
//        LED_STRIPE[ledID] = color;
        
        LED_STRIPE_STOP[ledID] = color;
        
        // if(ledID == 3 || ledID == 4 || ledID == 5){
        //     color.R = 0;
        //     color.G = 10;
        //     color.B = 0;
        // } else {
        //     color.R = 0;
        //     color.G = 0;
        //     color.B = 0;
        // }
        // LED_STRIPE_RIGHT[ledID] = color;
        
        // if(ledID == 11 || ledID == 12 || ledID == 13){
        //     color.R = 0;
        //     color.G = 10;
        //     color.B = 0;
        // } else {
        //     color.R = 0;
        //     color.G = 0;
        //     color.B = 0;
        // }
        // LED_STRIPE_LEFT[ledID] = color;
        
        // if(ledID == 11 || ledID == 12 || ledID == 13 ||
        //    ledID == 7 || ledID == 8 || ledID == 9 ||
        //    ledID == 3 || ledID == 4 || ledID == 5){
        //     color.R = 0;
        //     color.G = 10;
        //     color.B = 0;
        // } else {
        //     color.R = 0;
        //     color.G = 0;
        //     color.B = 0;
        // }
        // LED_STRIPE_THREE_WAYS[ledID] = color;
    }
    
    
    for (uint8_t ledID = 0; ledID < 16; ++ledID)
    {
        for (uint8_t id = 0; id < 24; ++id)
        {
            W0();
        }
    }
    
    _delay_ms(1000);
    
    LED_Color clear;
    clear.R  = 0;
    clear.G  = 0;
    clear.B  = 30;

    LED_Color pink;
    pink.R  = 255;
    pink.G  = 0;
    pink.B  = 100;

    LED_Color minorPink;
    minorPink.R  = 65;
    minorPink.G  = 0;
    minorPink.B  = 25;

    while (1)
    {

        type = rand() % 16;
        int prev = ((type - 1) < 0) ? 15 : (type-1);
        int next = ((type + 1) == 16) ? 0 : (type+1);
        
        LED_Color pink;
        pink.R  = rand() % 178;
        pink.G  = rand() % 178;
        pink.B  = rand() % 178;

        for (int ledID = 0; ledID < 16; ++ledID)
        {

            
            processLed(pink);

            // if(ledID == type){
            //     processLed(pink);
            // } else if(ledID == next) {
            //     processLed(minorPink);
            // }  else if(ledID == prev) {
            //     processLed(minorPink);
            // } else {
            //     processLed(clear);
            // }

            // if(type == 0){
            //     processLed(LED_STRIPE_STOP[ledID]);                
            // } else if(type == 1){
            //     processLed(LED_STRIPE_RIGHT[ledID]);
            // } else if(type == 2){
            //     processLed(LED_STRIPE_LEFT[ledID]);
            // } else if(type == 3){
            //     processLed(LED_STRIPE_THREE_WAYS[ledID]);
            // }
        }
        _delay_ms(300);
        type++;
        type = type % 16;
        // type = type % 4;
        
    }
}
