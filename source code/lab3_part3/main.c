//*****************************************************************************
//
// Application Name     - int_sw
// Application Overview - The objective of this application is to demonstrate
//                          GPIO interrupts using SW2 and SW3.
//                          NOTE: the switches are not debounced!
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup int_sw
//! @{
//
//****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdint.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "systick.h"

// Common interface includes
#include "uart_if.h"

// OLED Display includes
#include "spi.h"
#include "oled/Adafruit_GFX.h"
#include "oled/Adafruit_SSD1351.h"

#include "pin_mux_config.h"


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
extern void (* const g_pfnVectors[])(void);

uint32_t dataReceived = 0;
uint32_t lastCmd = 0;

unsigned int charCounter;

char currentChar;

char output[100];

unsigned charX = 0;
unsigned charY = 0;

#define DELAY_TIME              8000000
#define COMMIT_TIME             80000000
#define ZERO_DELAY_THRESHOLD    1600
#define WHITE                   0xFFFFFF

uint32_t button_one =   0x02FD807F;
uint32_t button_two =   0x02FD40BF;
uint32_t button_three = 0x02FDC03F;
uint32_t button_four =  0x02FD20DF;
uint32_t button_five =  0x02FDA05F;
uint32_t button_six =   0x02FD609F;
uint32_t button_seven = 0x02FDE01F;
uint32_t button_eight = 0x02FD10EF;
uint32_t button_nine =  0x02FD906F;
uint32_t button_zero =  0x02FD00FF;
uint32_t button_last =  0x02FD02FD;
uint32_t button_mute =  0x02FD08F7;

#define SPI_IF_BIT_RATE  100000

#define SYSCLKFREQ 80000000ULL

// macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
    ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
    ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\

// macro to convert microseconds to ticks
#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL
#define 	UART_INT_RT   0x040
#define 	UART_INT_RX   0x010
#define UART_INT_TX   0x020


// track systick counter periods elapsed
// if it is not 0, we know the transmission ended
volatile int systick_cnt = 1;

extern void (* const g_pfnVectors[])(void);


//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

// an example of how you can use structs to organize your pin settings for easier maintenance
typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

char storedOutput[100]; // Buffer to store the last committed string


//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
static void BoardInit(void);

//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//*****************************************************************************

void CommitCharacter() {
    if(currentChar != '\0') {

        Report("%s + %c = ", output, currentChar);
        strncat(output, &currentChar, 1);
        Report("%s\n\r", output);
        drawChar(charX * 7, charY * 7, currentChar, WHITE ,0,1);
        currentChar = '\0';
        charCounter = 0;

        if(charX > 21) {
            charY++;
            charX = 0;
        } else {
            charX++;
        }
   }
}
void PrintSentMessages(void) {
    unsigned char printX = 0;
    // Start printY at half the height of the screen to print in the bottom half
    unsigned char printY = (64 / 2) / 7;  // This divides the screen height by 14 (each char height is 7 pixels)

    // Iterate through each character in storedOutput and print it
    int i;
    for (i = 0; i < strlen(storedOutput); i++) {
        // Draw the character on the screen at the adjusted Y position (printY * 7 + half screen height)
        drawChar(printX * 7, (printY * 7) + (64 / 2), storedOutput[i], WHITE, 0, 1);

        // Move the cursor to the next position
        if (++printX > 21) {
            printX = 0;
            if (++printY >= 128 / 7) {  // Ensure the bottom limit respects the full screen height divided by character height
                break; // Stop if we run out of screen space in the bottom half
            }
        }
    }
}

volatile int txIndex = 0; // Transmit buffer index
char txBuffer[100]; // Transmit buffer

void UARTIntHandler(void) {
    unsigned long ulInts = UARTIntStatus(UARTA1_BASE, true);
    UARTIntClear(UARTA1_BASE, ulInts);

    // Handle received characters
    while (UARTCharsAvail(UARTA1_BASE)) {
        char c = UARTCharGetNonBlocking(UARTA1_BASE);
        // Process the received character
        output[charX++] = c;
        if (charX >= sizeof(output)) {
            charX = 0; // Reset buffer index if overflow
        }
    }

    // Handle transmit buffer empty interrupt
    if (UARTIntStatus(UARTA1_BASE, false) & UART_INT_TX) {
        if (txIndex < strlen(txBuffer)) {
            UARTCharPutNonBlocking(UARTA1_BASE, txBuffer[txIndex++]);
        }
        if (txIndex >= strlen(txBuffer)) {
            txIndex = 0;  // Reset index
        }
    }
}


void ConfigureUARTInterrupts(void) {
    // Register the interrupt handler
    UARTIntRegister(UARTA1_BASE, UARTIntHandler);
    // Enable UART receive interrupts
    UARTIntEnable(UARTA1_BASE,  0x010 | 0x040);
}



void parseForCharacter(const char possibleChars[], const int numChars) {
    if(lastCmd != dataReceived) {
        CommitCharacter();
    }

    currentChar = possibleChars[0] + charCounter;
    charCounter = (charCounter + 1) % numChars;

    lastCmd = dataReceived;

    Report("Current character: %c\n\r", currentChar);
}

static void GPIOA0IntHandler(void) { // SW3 handler
    unsigned long ulStatus;
    uint64_t delta = systick_cnt*SYSTICK_RELOAD_VAL - SysTickValueGet();
    uint64_t delta_us = TICKS_TO_US(delta);

   dataReceived = dataReceived << 1;
   if(delta_us > ZERO_DELAY_THRESHOLD) {
       dataReceived += 1;
   }

   if(dataReceived == button_one){
       Report("button pressed: 1\n\r");
   }
   if(dataReceived == button_two){
       Report("button pressed: 2\n\r");

       //Cycle between three letters: A, B, C.
       parseForCharacter("ABC", 3);
   }
   if(dataReceived == button_three){
       Report("button pressed: 3\n\r");

       //Cycle between three letters: D, E, F.
       parseForCharacter("DEF", 3);
          }
   if(dataReceived == button_four){
       Report("button pressed: 4\n\r");

       //Cycle between three letters: G, H, I.
       parseForCharacter("GHI", 3);
          }
   if(dataReceived == button_five){
       Report("button pressed: 5\n\r");

       //Cycle between three letters: J, K, L.
       parseForCharacter("JKL", 3);

          }
   if(dataReceived == button_six){
       Report("button pressed: 6\n\r");

       //Cycle between three letters: M, N, O.
       parseForCharacter("MNO", 3);

          }
   if(dataReceived == button_seven){
       Report("button pressed: 7\n\r");

       //Cycle between four letters: P, Q, R, S.
       parseForCharacter("PQRS", 4);
          }
   if(dataReceived == button_eight){
       Report("button pressed: 8\n\r");

        //Cycle between three letters: T, U, V.
       parseForCharacter("TUV", 3);
          }
   if(dataReceived == button_nine){
       Report("button pressed: 9\n\r");

       //Cycle between four letters: W, X, Y, Z.
       parseForCharacter("WXYZ", 4);
          }
   if(dataReceived == button_zero){
       Report("button pressed: 0\n\r");

       currentChar = ' ';
       CommitCharacter();
  }
   if(dataReceived == button_mute){
       Report("button pressed: mute\n\r");
       strcpy(storedOutput, output);
       fillRect(0, 0, 128, 128 / 2, 0x0000);
       fillRect(0, 64 / 2 - 7, 128, 128 / 2, 0x0000);
       PrintSentMessages();
       Report(">>> Committing string: \"%s\"\n\r", output);
       charX = 0;
       charY = 0;
   }
   if(dataReceived == button_last){
       Report("button pressed: LAST (delete)\n\r");

       //CommitCharacter();

       //Erase the most recent character.
       currentChar = 0;
       fillRect(charX * 7, charY * 7, 5, 7, 0x000);

       unsigned int messageLength = strlen(output);

       if(messageLength >= 1) {
           Report("Size of current message: %d\n\r", messageLength);

           output[messageLength - 1] = '\0';

           Report("Position of the character: [%d, %d]\n\r", charX, charY);

           if(charX <= 0) {
               charY = charY > 0 ? charY - 1 : 0;
               charX = 22;
           } else {
               charX--;
           }
       } else {
           strcpy(output, "");
           charX = 0;
           charY = 0;
       }

       Report("Current message: \"%s\"\n\r", output);
  }


    SysTickReset();
    ulStatus = MAP_GPIOIntStatus (GPIOA0_BASE, true);
    MAP_GPIOIntClear(GPIOA0_BASE, ulStatus);
}

static inline void SysTickReset(void) {
    HWREG(NVIC_ST_CURRENT) = 1;
    systick_cnt = 1;
}


static void SysTickHandler(void) {
    systick_cnt++;
}


void InitSPI() {
    PRCMPeripheralClkEnable(PRCM_GSPI,PRCM_RUN_MODE_CLK);
    PRCMPeripheralReset(PRCM_GSPI);
    SPIReset(GSPI_BASE);
    SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));
    SPIEnable(GSPI_BASE);
}


static void
BoardInit(void) {
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


static void SysTickInit(void) {
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);
    MAP_SysTickIntRegister(SysTickHandler);
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();
}


int main() {
    unsigned long ulStatus;

    BoardInit();
    PinMuxConfig();
    SysTickInit();
    InitTerm();
    ClearTerm();
    ConfigureUARTInterrupts();

    charCounter = 0;
    lastCmd = 0;

    MAP_GPIOIntRegister(GPIOA0_BASE, GPIOA0IntHandler);
    MAP_GPIOIntTypeSet(GPIOA0_BASE, 0x80, GPIO_RISING_EDGE);

    ulStatus = MAP_GPIOIntStatus(GPIOA0_BASE, true);
    MAP_GPIOIntClear(GPIOA0_BASE, ulStatus);
    MAP_GPIOIntEnable(GPIOA0_BASE, 0x80);
    SysTickReset();

    InitSPI();
    Adafruit_Init();
    fillScreen(0x0000);

    output[0] = '\0';
    charX = 0;
    charY = 0;

    unsigned accumulatedTicks = 0;

    //Primary loop
    while(1){
        MAP_UtilsDelay(DELAY_TIME);

        accumulatedTicks++;
        if(accumulatedTicks * DELAY_TIME >= (COMMIT_TIME / 2)) {
            CommitCharacter();
            PrintSentMessages();
            accumulatedTicks = 0;
        }

        if((accumulatedTicks % 2 == 0) || currentChar == '\0') {
            fillRect(charX * 7, charY * 7, 5, 7, (accumulatedTicks % 2 == 1 ? 0x000 : WHITE));
        } else {
            drawChar(charX * 7, charY * 7, currentChar, WHITE,0,1);
        }
    }
}