#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>

#include <u8g2.h>
#include <u8x8_avr.h>

#define SH1106_ADDR                                 0x78

// pin definitions
#define BUTTON_UP                                   PIND7
#define BUTTON_DOWN                                 PIND6

u8g2_t u8g2;

//=================================FUNCTIONS START=================================
void uart_init(void)
{
    // enable uart communication
    /*
        UART specification
        parity, stop bits, data bit length
        this will be used to TRANSMIT

        set baud rate formula:
            UBBR_n = (f_OSC / 16 *BAUD) - 1
                - f_OSC is clock rate
                - BAUD is baud rate (crystal oscillator)

        in this case:
            - using 9600 baud rate
            - at 16MHz

        UBBR_n = (16000000/(16*9600)) - 1
                            = 103.167
                            = 103 (rounded)

        using 8 data bits, since the baud rate registers are 8 bits each
        separate 103 into low and high byte since its less than 255

        no need to change stop bits as this is
        by default and common for many terminals
    */

    // set 9600 baud rate
    // shift to 8 bits my masking with 0xFF
    UBRR0L = (uint8_t)(103 & 0xFF); // set lower byte
    UBRR0H = (uint8_t)(103 >> 8);   // set higher byte

    // enable transmitter
    // | (1 << RXEN1) for receiving
    UCSR0B |= (1 << TXEN0); // set to high reading
}

void uart_check_transmit(uint8_t check)
{
    // UCSR1A is the control register A of the USART
    // UDRE1 means if it is ready to transmit data
    // while UDRE1 is zero, transmitter is busy so don't pass
    // this is a method called pulling
    while (!(UCSR0A & (1 << UDRE0)));

    // sample binary number check
    // recieving end must recieve this number
    // otherwise it won't do task
    UDR0 = check;
}

void uart_send_string(char data[])
{
    for(int i=0; i < strlen(data); i++)
    {
        while ((UCSR0A & (1 << UDRE0)) == 0);
        UDR0 = data[i]; 

    }
}

void init_display(void)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_avr_hw_i2c, u8x8_avr_delay);
    u8g2_InitInterface(&u8g2);

    u8g2_SetI2CAddress(&u8g2, SH1106_ADDR);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    u8g2_SetFont(&u8g2, u8g2_font_smart_patrol_nbp_tr);
    u8g2_SetFontRefHeightText(&u8g2);
    u8g2_SetFontPosTop(&u8g2);
    u8g2_DrawStr(&u8g2, 0, 25, "u8g2 AVR HW I2C");

    u8g2_SendBuffer(&u8g2);
}
//=================================FUNCTIONS END===================================

int main(void)
{
    // outputs
    //  led
    DDRB |= 1 << PINB0;                     // output PB0
    PORTB ^= 1 << PINB0;                    // toggle only PIN 0 on PORTB

    // inputs
    // buttons
    DDRD &= ~(1 << BUTTON_UP);              // input PB1
    DDRD &= ~(1 << BUTTON_DOWN);            // input PB2 (also CS)

    // button status
    PORTD |= 1 << BUTTON_UP;                // set PB1 to high reading
    PORTD |= 1 << BUTTON_DOWN;              // set PB2 to high reading (also CS)

    // variables
    int pressed = 0;                        // for button confirm
    uint8_t res[5];                         // hold responses for sdcard

    // initialize functions
    init_display();                         // oled sh1106 i2c setup
    uart_init();                            // baud rate only at 9600


    while (1)
    {      
        // turn on led
        if (bit_is_clear(PIND, 7))
        {
            if (pressed == 0)
            {
                u8g2_ClearBuffer(&u8g2);
                u8g2_DrawStr(&u8g2, 0, 0, "LED ON");
                u8g2_SendBuffer(&u8g2);

                PORTB ^= 1 << PINB0;
                pressed = 1;
                uart_send_string("ON");

            }
        }
        // turn off led
        else if (bit_is_clear(PIND, 6))
        {
            if (pressed == 1)
            {
                u8g2_ClearBuffer(&u8g2);
                u8g2_DrawStr(&u8g2, 0, 0, "LED OFF");
                u8g2_SendBuffer(&u8g2);

                PORTB |= 1 << PINB0;
                pressed = 0;

                uart_send_string("OFF");

            }
        }
    }
}