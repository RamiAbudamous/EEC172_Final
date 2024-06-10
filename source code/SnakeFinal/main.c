// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
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
#include "spi.h"
#include "i2c_if.h"
#include "uart_if.h"
#include "uart.h"
#include "pin_mux_config.h"
#include "common.h"

#include "oled_test.h"
#include "Adafruit_SSD1351.h"
#include "Adafruit_GFX.h"
#include "glcdfont.h"

//aws
#include "C:\ti\CC3200SDK_1.5.0\cc3200-sdk\simplelink\include\simplelink.h"
//#include "simplelink_extlib.h"
#include "utils/network_utils.h"

//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************

//for AWS
#define DATE                4    /* Current Date */
#define MONTH               6     /* Month 1-12 */
#define YEAR                2024  /* Current year */
#define HOUR                10    /* Time - hours */
#define MINUTE              55    /* Time - minutes */
#define SECOND              0     /* Time - seconds */

#define APPLICATION_NAME      "SSL"
#define APPLICATION_VERSION   "SQ24"
#define SERVER_NAME           "a1ti8w8ng69utd-ats.iot.us-east-2.amazonaws.com" // CHANGE ME
#define GOOGLE_DST_PORT       8443

#define POSTHEADER "POST /things/Rami_Ohio/shadow HTTP/1.1\r\n"             // CHANGE ME
#define HOSTHEADER "Host: a1ti8w8ng69utd-ats.iot.us-east-2.amazonaws.com\r\n" // CHANGE ME
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"


#define UART_PRINT Report
#define SPI_IF_BIT_RATE 100000
#define FOREVER 1
#define CONSOLE UARTA0_BASE
#define FAILURE -1
#define SUCCESS 0
#define RETERR_IF_TRUE(condition) \
    {                             \
        if (condition)            \
            return FAILURE;       \
    }
#define RET_IF_ERR(Func)        \
    {                           \
        int iRetVal = (Func);   \
        if (SUCCESS != iRetVal) \
            return iRetVal;     \
    }

#define SPI_IF_BIT_RATE 100000
#define TR_BUFF_SIZE 100
#define MAX_SNAKE_LENGTH 100

#define SPI_IF_BIT_RATE  100000
#define SYSCLKFREQ       80000000ULL
#define SYSTICK_RELOAD_VAL 3200000UL
#define NEC_BIT_PULSE_WIDTH 1700

// Macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
    (((ticks / SYSCLKFREQ) * 1000000ULL) + (((ticks % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))

// Macro to convert microseconds to ticks
#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))
#define DELAY_TIME      5000000
#define COMMIT_TIME     30000000
#define WHITE           0xFFFFFF
#define UART_BAUD_RATE  115200

#define GPIO_PORT_SW2_BASE      GPIOA2_BASE  // fix?
#define GPIO_SW2_PIN            0x40

#define DATA1 "{" \
            "\"state\": {\r\n"                                              \
                "\"desired\" : {\r\n"                                       \
                    "\"var\" :\""                                           \
                        "You had a score of %s"                             \
                        "\"\r\n"                                            \
                "}"                                                         \
            "}"                                                             \
        "}\r\n\r\n"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************

#if defined(ccs)
extern void (*const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
 int snake_segment_size = 4;
static int colorInt = 0;
static int colorDelay = 50;
int colorsArr[] = {WHITE, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, 0x8888};
int x_position = 63, y_position = 63;
int x_rand, y_rand, current_size;
int snake[MAX_SNAKE_LENGTH][2]; // Each element holds x and y positions of a segment
int snakeLength = 0;            // Current length of the snake
int x_rand, y_rand;             // Random positions for food
bool justAte = false;
bool started = false;

volatile uint32_t necMessage = 0;
uint32_t prevNecMessage = 0;
unsigned charCount = 0;
char currentChar = '\0';
char output[100] = {0};
unsigned charX = 0, charY = 0;
char uartBuffer[128] = {0};
int uartIndex = 0;

typedef enum {
    ONE = 0x02FD807F, TWO = 0x02FD40BF, THREE = 0x02FDC03F, FOUR = 0x02FD20DF,
    FIVE = 0x02FDA05F, SIX = 0x02FD609F, SEVEN = 0x02FDE01F, EIGHT = 0x02FD10EF,
    NINE = 0x02FD906F, MUTE = 0x02FD08F7, ZERO = 0x02FD00FF, LAST = 0x02FD02FD
} ButtonCodes;

typedef struct {
    uint32_t buttonCode;
    int buttonNumber;
    const char* characters;
} ButtonMapping;

ButtonMapping buttonMappings[] = {
    {ONE, 1, NULL}, {TWO, 2, "ABC"}, {THREE, 3, "DEF"}, {FOUR, 4, "GHI"},
    {FIVE, 5, "JKL"}, {SIX, 6, "MNO"}, {SEVEN, 7, "PQRS"}, {EIGHT, 8, "TUV"},
    {NINE, 9, "WXYZ"}, {ZERO, 0, NULL}, {MUTE, -1, NULL}, {LAST, -2, NULL}
};

typedef enum {
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

Direction snakeDirection = DIRECTION_UP; // Default direction

volatile int systick_cnt = 1;

char storedOutput[100]; // Buffer to store the last committed string
int last_sw = 0;
int sw2_pressed = 0;
long lRetVal = -1;

#define POWER_UP_ACTIVE_TIME 50  // Number of frames the power-up effect lasts

typedef struct {
    int x;
    int y;
    bool active;  // true if the power-up is currently on the field
} PowerUp;

PowerUp speedBoost;  // Power-up for speed boosting
int boostedFramesLeft = 0;  // Counter for the duration of the speed boost



//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

//*****************************************************************************
// Function Prototypes
//*****************************************************************************
void ConfigSPI();
//void BoardInit();
void SysTickInit();
void InitTerm();
void ClearTerm();
void GPIOInit();
void InitUART();
void InitSPI();
void Adafruit_Init();
void setupHardware();
void displayStart();
void displayGameOver();
void idleScreen();
void initializeGame();
void collisionDetect();
void gameOver();
void processGameLoop();

int getRandomPosition(int max);
void addSnakeSegment(int x, int y);
void moveSnake(int x_speed, int y_speed, bool justAte);
void renderGame(bool justAte);

int getRandomPosition(int max);
void updateFoodPosition();
void UARTIntHandler(void);
void InitUART(void);
void SysTickHandler(void);
void GPIOA0IntHandler(void);

//aws
static int set_time();
static void BoardInit(void);
void leaderboard();

//*****************************************************************************
// Main function handling the I2C example
//*****************************************************************************
void main() {
    setupHardware();
    idleScreen();
}

//*****************************************************************************
// Hardware setup
//*****************************************************************************
void setupHardware() {
    BoardInit();
    PinMuxConfig();
    SysTickInit();
    GPIOInit();
    InitUART();
    InitTerm();
    ClearTerm();
    UART_PRINT("My terminal works!\n\r");

    I2C_IF_Open(I2C_MASTER_MODE_FST);
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);
    MAP_SPIReset(GSPI_BASE);
    MAP_PRCMPeripheralReset(PRCM_GSPI);
    ConfigSPI();
    Adafruit_Init();
    fillScreen(BLACK);

    //
    //
    //aws stuff
    // initialize global default app configuration
    g_app_config.host = SERVER_NAME;
    g_app_config.port = GOOGLE_DST_PORT;

    //Connect the CC3200 to the local access point
    lRetVal = connectToAccessPoint();
    //Set time so that encryption can be used
    lRetVal = set_time();
    if(lRetVal < 0) {
        UART_PRINT("Unable to set time in the device");
        LOOP_FOREVER();
    }
    //Connect to the website with TLS encryption
    lRetVal = tls_connect();
    if(lRetVal < 0) {
        ERR_PRINT(lRetVal);
    }
//
//    //idk if these are needed
//    // http_post(lRetVal);
//    // sl_Stop(SL_STOP_TIMEOUT);
//    // LOOP_FOREVER();
}

void displayStart() {
    fillScreen(BLACK);
    unsigned char mainmessage1[] = "SNAKE";
    unsigned char mainmessage2[] ="(if it was good)";
    unsigned char mainmessage3[] ="Hold SW2 to start!";
    int i = 0;
    int j = 0;
    int k = 0;

    for(k=0; k<sizeof(mainmessage1); k++)
        {
            drawChar((i+36), (j+42), mainmessage1[k], BLUE, BLACK, 2);
            i += 12;
            if(i >= 123){
                i = 0;
                j += 8;
            }
        }
        i=0;
        j=0;
        k=0;
        for(k=0; k<sizeof(mainmessage2); k++)
            {
                drawChar((i+18), (j+60), mainmessage2[k], WHITE, BLACK, 1);
                i += 6;
                if(i >= 123){
                    i = 0;
                    j += 8;
                }
            }
        i=0;
        j=0;
        k=0;
        for(k=0; k<sizeof(mainmessage3); k++)
            {
                drawChar((i+12), (j+112), mainmessage3[k], WHITE, BLACK, 1);
                i += 6;
                if(i >= 123){
                    i = 0;
                    j += 8;
                }
            }
}

void displayGameOver() {
    fillScreen(BLACK);
    unsigned char mainmessage1[] = "GAME OVER!";
    unsigned char mainmessage2[] ="You had a score of %s";
    unsigned char mainmessage3[] ="Hold SW2 to continue!";

    char snakeScore[64];
    sprintf(snakeScore, "%d", snakeLength);
    sprintf(mainmessage2, mainmessage2, snakeScore);

    int i = 0;
    int j = 0;
    int k = 0;

    for(k=0; k<sizeof(mainmessage1); k++)
    {
        drawChar((i+6), (j+36), mainmessage1[k], RED, BLACK, 2);
        i += 12;
        if(i >= 123){
            i = 0;
            j += 8;
        }
    }
    i=0;
    j=0;
    k=0;
    for(k=0; k<sizeof(mainmessage2); k++)
        {
            drawChar(i, (j+60), mainmessage2[k], colorsArr[colorInt], BLACK, 1);
            i += 6;
            if(i >= 123){
                i = 0;
                j += 8;
            }
        }
    i=0;
    j=0;
    k=0;
    for(k=0; k<sizeof(mainmessage3); k++)
        {
            drawChar(i, (j+106), mainmessage3[k], WHITE, BLACK, 1);
            i += 6;
            if(i >= 123){
                i = 0;
                j += 8;
            }
        }
}

//*****************************************************************************
// Initialize game variables
//*****************************************************************************
void initializeGame() {
    fillScreen(BLACK);
    snakeLength = 4; // Initial length of the snake
    int i;
    for (i = 0; i < snakeLength; i++) {
        snake[i][0] = 63; // Initial x position
        snake[i][1] = 63 - i * snake_segment_size; // Initial y position
    }
    x_rand = getRandomPosition(127 - snake_segment_size);
    y_rand = getRandomPosition(127 - snake_segment_size);
}

void placePowerUp() {
    if (!speedBoost.active) {
        speedBoost.x = getRandomPosition(127 - snake_segment_size);
        speedBoost.y = getRandomPosition(127 - snake_segment_size);
        speedBoost.active = true;
    }
}

void idleScreen() {
    displayStart();

    while(sw2_pressed == 0)
    {
        sw2_pressed = GPIOPinRead(GPIO_PORT_SW2_BASE, GPIO_SW2_PIN);
        MAP_UtilsDelay(8000000);
    }

    //reset the values to be used for next time
    sw2_pressed = 0;
    colorInt = 0;
    started = false;
    snake_segment_size = 4;
    speedBoost.active = false;

    initializeGame();
    unsigned tickCount = 0;

    while (1) {
        processGameLoop();
        //MAP_UtilsDelay(DELAY_TIME);
        tickCount++;
    }
}

//*****************************************************************************
// Game loop processing
//*****************************************************************************
void processGameLoop() {
    char x_buffer[256] = "readreg 0x18 0x5 1 \n\r";
    char y_buffer[256] = "readreg 0x18 0x3 1 \n\r";
    bool justAte = false;

    int x_speed = ParseNProcessCmd(x_buffer) / 5; // Simplified speed scaling
    int y_speed = ParseNProcessCmd(y_buffer) / 5; // Simplified speed scaling

    // printf("%d %d\n", abs(snake[0][0] - x_rand), abs(snake[0][1] - y_rand));

    if (abs(snake[0][0] - x_rand) <= snake_segment_size+2 && abs(snake[0][1] - y_rand) <= snake_segment_size+2) {
        colorInt += 1;
        if(colorInt == 8)
            colorInt = 0;
        updateFoodPosition();
        placePowerUp();
        addSnakeSegment(snake[snakeLength-1][0], snake[snakeLength-1][1]); // Add segment at the current tail position
        justAte = true; // Snake just ate the food
    }

    // Check for power-up consumption
   if (speedBoost.active && abs(snake[0][0] - speedBoost.x) <= snake_segment_size && abs(snake[0][1] - speedBoost.y) <= snake_segment_size) {
       speedBoost.active = false;
       boostedFramesLeft = POWER_UP_ACTIVE_TIME;  // Set the boost duration
       snake_segment_size = snake_segment_size * 2;
       fillCircle(speedBoost.x, speedBoost.y, snake_segment_size, BLACK);  // Clear the power-up from the screen
   }

   // Decrease the boostedFramesLeft counter
   if (boostedFramesLeft > 0) {
       boostedFramesLeft--;
       if (boostedFramesLeft == 0) {
           int i;
           for ( i = 0; i < snakeLength; i++) {
                       fillCircle(snake[i][0], snake[i][1], snake_segment_size * 2, BLACK);
                   }
           snake_segment_size = snake_segment_size / 2;
       }
   }

    moveSnake(x_speed, y_speed, justAte);
    if (started)
        collisionDetect();
    else
        started = true;
    renderGame(justAte);
}


//*****************************************************************************
// Add segment to snake
//*****************************************************************************
void addSnakeSegment(int x, int y) {
    if (snakeLength < MAX_SNAKE_LENGTH) {
        snake[snakeLength][0] = x;
        snake[snakeLength][1] = y;
        snakeLength++;
    }
}

//*****************************************************************************
// Move snake based on speed
//*****************************************************************************
void moveSnake(int x_speed, int y_speed, bool justAte) {
    switch (snakeDirection) {
        case DIRECTION_UP:
            y_speed = -10;
            break;
        case DIRECTION_DOWN:
            y_speed = 10;
            break;
        case DIRECTION_LEFT:
            x_speed = -10;
            break;
        case DIRECTION_RIGHT:
            x_speed = 10;
            break;
    }

    int i;
    int new_x = snake[0][0] + x_speed;
    int new_y = snake[0][1] + y_speed;

    // Wrapping the snake around the screen instead of bounding it
    if (new_x < snake_segment_size) new_x = 127 - snake_segment_size;
    else if (new_x > 127 - snake_segment_size) new_x = snake_segment_size;

    if (new_y < snake_segment_size) new_y = 127 - snake_segment_size;
    else if (new_y > 127 - snake_segment_size) new_y = snake_segment_size;

    // Update the positions of the snake segments
    for (i = snakeLength - 1; i > 0; i--) {
        snake[i][0] = snake[i-1][0];
        snake[i][1] = snake[i-1][1];
    }
    snake[0][0] = new_x;
    snake[0][1] = new_y;
}


//*****************************************************************************
// Get random position for food
//*****************************************************************************
int getRandomPosition(int max) {
    return rand() % max;
}

//*****************************************************************************
// Update position of the food
//*****************************************************************************
void updateFoodPosition() {
    fillCircle(x_rand, y_rand, snake_segment_size, BLACK);
    x_rand = getRandomPosition(127 - snake_segment_size);
    y_rand = getRandomPosition(127 - snake_segment_size);
}

//*****************************************************************************
// Detect if the snake is colliding with itself
//*****************************************************************************
void collisionDetect(void) {
    int i = 3;
    for(i; i<snakeLength; i++) {
        //if position of body part is the same as the position of snake head, gameOver()
//        if ((snake[i][0]==snake[0][0])&&(snake[i][1]==snake[0][1]))
        if (abs(snake[0][0] - snake[i][0]) <= snake_segment_size*2 && abs(snake[0][1] - snake[i][1]) <= snake_segment_size*2) {
            gameOver();
        }
    }
}

//*****************************************************************************
// End the game
//*****************************************************************************
void gameOver(void) {
    displayGameOver();
    while(sw2_pressed == 0)
        {
            sw2_pressed = GPIOPinRead(GPIO_PORT_SW2_BASE, GPIO_SW2_PIN);
            MAP_UtilsDelay(8000000);
        }

        //reset the value to be used for next time
    sw2_pressed = 0;
    leaderboard(); //replace with leaderboard later.
}

//*****************************************************************************
// Send the user's score through email and return to idle
//*****************************************************************************
void leaderboard(void) {
    //email user's score
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal2 = 0;
    char tmp[256];
    char snakeScore[64];
    sprintf(snakeScore, "%d", snakeLength);
//    char snakeScore = snakeLength + '0'; //convert int score to char

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, POSTHEADER);
    pcBufHeaders += strlen(POSTHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    //sprintf
    sprintf(tmp, DATA1, snakeScore);
    int dataLength = strlen(tmp);
    //sprintf

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);

    strcpy(pcBufHeaders, tmp);
    pcBufHeaders += strlen(tmp);

    int testDataLength = strlen(pcBufHeaders);

    lRetVal2 = sl_Send(lRetVal, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal2 < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(lRetVal);
//        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        // return lRetVal2;
        idleScreen();
    }
    lRetVal2 = sl_Recv(lRetVal, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal2 < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal2);
        //sl_Close(lRetVal);
//        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        //    return lRetVal2;
        idleScreen();
    }
    else {
        acRecvbuff[lRetVal2+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    //return to idle
    idleScreen();
}

//*****************************************************************************
// Render the game elements on display
//*****************************************************************************
void renderGame(bool justAte) {
    // Draw food
    fillCircle(x_rand, y_rand, 2, RED);

    if (speedBoost.active) {
        fillCircle(speedBoost.x, speedBoost.y, snake_segment_size, MAGENTA);  // Use a distinct color for the power-up
    }

    // Draw the entire snake for each frame
    int i;
    for (i = 0; i < snakeLength; i++) {
        fillCircle(snake[i][0], snake[i][1], snake_segment_size, colorsArr[colorInt]);
    }

    // Clear the old position of the last segment unless the snake just ate
    if (snakeLength > 1) {
        int tailIndex = snakeLength - 1;
        int old_tail_x = snake[tailIndex][0];
        int old_tail_y = snake[tailIndex][1];

        // Only move tail if it hasn't just grown
        fillCircle(old_tail_x, old_tail_y, snake_segment_size, BLACK);
    }
}

//*****************************************************************************
//
//! SPI Master mode main loop
//!
//! This function configures SPI modelue as master and enables the channel for
//! communication
//!
//! \return None.
//
//*****************************************************************************
void ConfigSPI()
{
    MAP_SPIReset(GSPI_BASE);
    MAP_SPIConfigSetExpClk(GSPI_BASE, MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                           SPI_IF_BIT_RATE, SPI_MODE_MASTER, SPI_SUB_MODE_0,
                           (SPI_SW_CTRL_CS |
                            SPI_4PIN_MODE |
                            SPI_TURBO_OFF |
                            SPI_CS_ACTIVEHIGH |
                            SPI_WL_8));
    MAP_SPIEnable(GSPI_BASE);
    Adafruit_Init();
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//*****************************************************************************
//
//! This function updates the date and time of CC3200.
//!
//! \param None
//!
//! \return
//!     0 for success, negative otherwise
//!
//*****************************************************************************

static int set_time() {
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;

    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}

//*****************************************************************************
//
//! Main function for spi demo application
//!
//! \param none
//!
//! \return None.
//

//*****************************************************************************
//
//! Display the buffer contents over I2C
//!
//! \param  pucDataBuf is the pointer to the data store to be displayed
//! \param  ucLen is the length of the data to be displayed
//!
//! \return none
//!
//*****************************************************************************
int DisplayBuffer(unsigned char *pucDataBuf, unsigned char ucLen)
{
    unsigned char ucBufIndx = 0;
    int buffer;
    while (ucBufIndx < ucLen)
    {
        buffer = (int)pucDataBuf[ucBufIndx];
        if (buffer & 0x80)
        {
            buffer = buffer | 0xffffff00;
        }
        ucBufIndx++;
    }
    return buffer;
}
//****************************************************************************
//
//! Parses the read command parameters and invokes the I2C APIs
//!
//! \param pcInpString pointer to the user command parameters
//!
//! This function
//!    1. Parses the read command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessReadCommand(char *pcInpString)
{
    unsigned char ucDevAddr, ucLen;
    unsigned char aucDataBuf[256];
    char *pcErrPtr;
    int iRetVal;

    //
    // Get the device address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucDevAddr = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);
    //
    // Get the length of data to be read
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucLen = (unsigned char)strtoul(pcInpString, &pcErrPtr, 10);
    // RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

    //
    // Read the specified length of data
    //
    iRetVal = I2C_IF_Read(ucDevAddr, aucDataBuf, ucLen);

    if (iRetVal == SUCCESS)
    {
        printf("I2C Read complete\n\r");

        //
        // Display the buffer over UART on successful write
        //
        DisplayBuffer(aucDataBuf, ucLen);
    }
    else
    {
        printf("I2C Read failed\n\r");
        return FAILURE;
    }

    return SUCCESS;
}

//****************************************************************************
//
//! Parses the readreg command parameters and invokes the I2C APIs
//! i2c readreg 0x<dev_addr> 0x<reg_offset> <rdlen>
//!
//! \param pcInpString pointer to the readreg command parameters
//!
//! This function
//!    1. Parses the readreg command parameters.
//!    2. Invokes the corresponding I2C APIs
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
int ProcessReadRegCommand(char *pcInpString)
{
    unsigned char ucDevAddr, ucRegOffset, ucRdLen;
    unsigned char aucRdDataBuf[256];
    char *pcErrPtr;

    //
    // Get the device address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucDevAddr = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);
    //
    // Get the register offset address
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucRegOffset = (unsigned char)strtoul(pcInpString + 2, &pcErrPtr, 16);

    //
    // Get the length of data to be read
    //
    pcInpString = strtok(NULL, " ");
    RETERR_IF_TRUE(pcInpString == NULL);
    ucRdLen = (unsigned char)strtoul(pcInpString, &pcErrPtr, 10);
    // RETERR_IF_TRUE(ucLen > sizeof(aucDataBuf));

    //
    // Write the register address to be read from.
    // Stop bit implicitly assumed to be 0.
    //
    RET_IF_ERR(I2C_IF_Write(ucDevAddr, &ucRegOffset, 1, 0));

    //
    // Read the specified length of data
    //
    RET_IF_ERR(I2C_IF_Read(ucDevAddr, &aucRdDataBuf[0], ucRdLen));

    //
    // Display the buffer over UART on successful readreg
    //
    return DisplayBuffer(aucRdDataBuf, ucRdLen);
}

// code to parse accelerometer
int ParseNProcessCmd(char *pcCmdBuffer)
{
    char *pcInpString;
    int iRetVal = FAILURE;

    pcInpString = strtok(pcCmdBuffer, " \n\r");
    if (pcInpString != NULL)

    {

        if (!strcmp(pcInpString, "readreg"))
        {
            //
            // Invoke the readreg command handler
            //
            iRetVal = ProcessReadRegCommand(pcInpString);
        }
    }

    return iRetVal;
}

//*****************************************************************************
// Function Definitions
//*****************************************************************************
void InitUART(void) {
    // Configure UART0
    MAP_UARTConfigSetExpClk(CONSOLE, MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH), UART_BAUD_RATE,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    // Clear UART0 Terminal
    ClearTerm();

    // Configure UART1
    MAP_UARTConfigSetExpClk(UARTA1_BASE, MAP_PRCMPeripheralClockGet(PRCM_UARTA1), UART_BAUD_RATE,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    // Configure interrupts for UART1_RX
    MAP_UARTFIFODisable(UARTA1_BASE);
    MAP_UARTIntRegister(UARTA1_BASE, UARTIntHandler);
    MAP_UARTFIFOLevelSet(UARTA1_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    MAP_UARTIntEnable(UARTA1_BASE, UART_INT_RX);
}

void UARTIntHandler(void) {
    unsigned long ulstatus = MAP_UARTIntStatus(UARTA1_BASE, true);
    MAP_UARTIntClear(UARTA1_BASE, ulstatus); // Clear the interrupt status

    // Loop while there are characters in the receive FIFO
    while (MAP_UARTCharsAvail(UARTA1_BASE)) {
        char character = MAP_UARTCharGetNonBlocking(UARTA1_BASE);
        if (character != -1) { // Check if the read was successful
            appendCharAndUpdateOutput(uartBuffer, character);
        }
    }

    // Re-enable UART RX interrupts
    MAP_UARTIntEnable(UARTA1_BASE, UART_INT_RX);
}

//*****************************************************************************
// Utility Functions
//*****************************************************************************
void UARTSendString(const char *str) {
    while (*str != '\0') {
        MAP_UARTCharPut(UARTA1_BASE, *str++);
    }
}

void appendCharAndUpdateOutput(char *output, char currentChar) {
    Report("%s + %c = ", output, currentChar);
    strncat(output, &currentChar, 1);
    Report("%s\n\r", output);
}

void sendChar() {
    if (currentChar != '\0') {
        appendCharAndUpdateOutput(output, currentChar);
        drawCharacterOnScreen(charX, charY, currentChar);
        UARTSendChar(currentChar);
        currentChar = '\0';
        charCount = 0;
        updateCursorPosition(&charX, &charY);
    }
}

void drawCharacterOnScreen(int x, int y, char character) {
    drawChar(x * 7, y * 7, character, WHITE, 0, 1);
}

void updateCursorPosition(int *x, int *y) {
    if (*x > 21) {
        (*y)++;
        *x = 0;
    } else {
        (*x)++;
    }
}

void printIncoming(void) {
    unsigned char printX = 0;
    unsigned char printY = (64 / 2) / 7; // Start at half the screen height
    unsigned char maxHeight = 128 / 7; // Maximum y position based on screen height

    int i;
    for (i = 0; i < strlen(storedOutput); i++) {
        drawCharacterOnScreen(printX, printY, storedOutput[i]);
        updatePrintPosition(&printX, &printY, maxHeight);
    }
}

void updatePrintPosition(unsigned char *x, unsigned char *y, unsigned char maxY) {
    if (++(*x) > 21) {
        *x = 0;
        if (++(*y) >= maxY) *y = maxY;
    }
}

void wrapCharacters(const char chars[], int numChars) {
    if(prevNecMessage != necMessage) {
        sendChar();
    }
    currentChar = chars[(charCount++) % numChars];
    prevNecMessage = necMessage;
}

void deleteChar(void) {
    if (strlen(output) > 0) {
        output[strlen(output) - 1] = '\0';
        charX = (charX == 0) ? (charX = 21, charY > 0 ? --charY : 0) : --charX;
        fillRect(charX * 7, charY * 7, 5, 7, 0);
    }
}

void enterIncoming(void) {
    // Clears the entire screen before printing incoming messages
    fillScreen(0);

    strcpy(storedOutput, output); // Store current output in storedOutput
    UARTSendString(storedOutput);
    printIncoming();

    // Clear the current output buffer and reset cursor positions
    output[0] = '\0';
    charX = 0;
    charY = 0;
}


void DecodeNECMessage(uint32_t message) {
    int i;
    for (i = 0; i < sizeof(buttonMappings) / sizeof(ButtonMapping); ++i) {
        if (message == buttonMappings[i].buttonCode) {
            Report("Button pressed: %s\n\r", buttonMappings[i].characters ? buttonMappings[i].characters : "Special");
            if (buttonMappings[i].buttonNumber == -1) {
                enterIncoming();
            } else if (buttonMappings[i].buttonNumber == -2) {
                deleteChar();
            } else if (buttonMappings[i].buttonNumber == 0) {
                currentChar = ' ';
                sendChar();
            } else {
                switch (buttonMappings[i].buttonNumber) {
                    case 2:
//                        if (lastCase!=8) {
                            snakeDirection = DIRECTION_UP;
//                            lastCase=2;
//                        }
                        break;
                    case 4:
                        snakeDirection = DIRECTION_LEFT;
                        break;
                    case 6:
                        snakeDirection = DIRECTION_RIGHT;
                        break;
                    case 8:
                        snakeDirection = DIRECTION_DOWN;
                        break;
                }
            }
            break;
        }
    }
}


//*****************************************************************************
// Hardware Initialization Functions
//*****************************************************************************
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
}

static void GPIOA0IntHandler(void) {
    // read the countdown register and compute elapsed cycles
    uint64_t delta = systick_cnt * SYSTICK_RELOAD_VAL - MAP_SysTickValueGet();

    // convert elapsed cycles to microseconds
    uint64_t delta_us = TICKS_TO_US(delta);

    if (delta_us > NEC_BIT_PULSE_WIDTH) {
        // Append a "1" bit to the end of necMessage
        necMessage = necMessage * 2 + 1;
    } else {
        // Append a "0" bit to the end of necMessage
        necMessage = necMessage * 2;
    }

    DecodeNECMessage(necMessage);
    UART_PRINT("%d\n", snakeDirection);
    SysTickReset();
    MAP_GPIOIntClear(GPIOA0_BASE, 0x80);
}

void InitSPI() {
    PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);
    PRCMPeripheralReset(PRCM_GSPI);
    SPIConfigSetExpClk(GSPI_BASE, MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                       SPI_IF_BIT_RATE, SPI_MODE_MASTER, SPI_SUB_MODE_0,
                       (SPI_SW_CTRL_CS | SPI_4PIN_MODE | SPI_TURBO_OFF |
                        SPI_CS_ACTIVEHIGH | SPI_WL_8));
    SPIEnable(GSPI_BASE);
}

void SysTickInit(void) {
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);
    MAP_SysTickIntRegister(SysTickHandler);
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();
}

void GPIOInit(void) {
    MAP_GPIOIntRegister(GPIOA0_BASE, GPIOA0IntHandler);
    MAP_GPIOIntTypeSet(GPIOA0_BASE, 0x80, GPIO_RISING_EDGE);
    MAP_GPIOIntClear(GPIOA0_BASE, MAP_GPIOIntStatus(GPIOA0_BASE, true));
    MAP_GPIOIntEnable(GPIOA0_BASE, 0x80);
    SysTickReset();
}

void SysTickReset(void) {
    HWREG(NVIC_ST_CURRENT) = 0; // Proper reset by writing 0
    systick_cnt = 1;
}

void UARTSendChar(char c) {
    MAP_UARTCharPut(UARTA1_BASE, c);
}
