---
title: 'Snake (if it was good)'
author: '**Rami Abudamous and Mihika Krishna** (website template by Ryan Tsang)'
date: 'EEC172 SQ24'

subtitle: '<blockquote>The website source is hosted 
<a href="https://github.com/RamiAbudamous/EEC172_Final">on github</a>.
</blockquote>'

toc-title: 'Table of Contents'
abstract-title: '<h2>Description</h2>'
abstract: 'We decided to create a game of snake for our project as early as lab two because we noticed that the moving ball from lab 2 was similar to how a snake moves, and we thought that the accelerometer would be an interesting control mechanism for snake compared to the traditional directional inputs. When it came time to implement our design, we also included directional inputs via the IR remote, as we felt that giving the player the option to choose their method of control would make the game more enjoyable, in addition to meeting the project requirements. The game begins with a title screen that prompts the user to start. The main gameplay loop consists of a snake that can move around the screen, seeking to eat food. Eating  piece of food increases the size of the snake and places a new piece of food on the board. The snake dies when it runs into itself, but can move into one end of the border and appear from the other side. We also implemented power ups to further differentiate our game from the traditional snake game, with a special piece of food that makes the snake much larger for a period, making it easier to get to food, while also increasing the risk of running into oneself. Upon the snake's death, a game over screen is displayed, and the user's score is sent to an email via AWS. The game then returns to the title screen and prompts the user to play again.
<br/><br/>
Zip files of our source code can be found 
<!-- replace this link -->
<a href="https://github.com/RamiAbudamous/EEC172_Final">
  here</a>.

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;padding-top:20px">
  <div style="display: inline-block; vertical-align: bottom;">
    <img src="./media/Image_001.jpg" style="width:2in;height:auto"/>
    <!-- <span class="caption"> </span> -->
  </div>
  <div style="display: inline-block; vertical-align: bottom;">
    <img src="./media/Image_002.jpg" style="width:auto;height:2in" />
    <!-- <span class="caption"> </span> -->
  </div>
</div>

<!-- 560 by 315 originally -->
<h2>Video Demo</h2>
<div style="text-align:center;margin:auto;max-width:560px">
  <div style="padding-bottom:56.25%;position:relative;height:0;">
    <iframe style="left:0;top:0;width:100%;height:100%;position:absolute;" width="auto" height="auto" src="https://youtu.be/_4U2e4Hofy8" title="YouTube video player" frameborder="0" allow="accelerometer; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
  </div>
</div>
'
---

# Design

## System Architecture

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style="display:inline-block;vertical-align:top;flex:1 0 300px;">
  The architecture diagram consists of 5 blocks each representing a component of the system. The main control unit is the CC3200 board which runs the written code and connects all the other components to eachother. All information is displayed on an OLED screen via SPI, which is connected to the MCU by 5 pins, power, and ground. User input is taken from both the CC3200's built in accelerometer through I2C, as well as the IR remote via a receiver that is built on a breadboard (See the implementation section for the circuit). Finally, AWS is called from the CC3200's code in order to email the user their final score.
  </div>
  <div style="display:inline-block;vertical-align:top;flex:0 0 500px">
    <div class="fig">
      <img src="./media/Image_006.jpg" style="width:90%;height:auto;" />
      <span class="caption">State Diagram</span>
    </div>
  </div>
</div>

## Functional Specification

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style="display:inline-block;vertical-align:top;flex:1 0 400px;">
  Our state machine consisted of four main states shown in grey. The idle state is called from main and contains the start screen. It waits until a user input begins the game, at which point it calls the initialization function and moves to the game loop state. The game loop contains one large function that loops every tick. Here, the move snake function polls the accelerometer and IR remote for any user input and handles movement, before moving to the collision detection function. If there is no collision, all changes are rendered to the display and the game loop restarts. If a collision is detected however, the game over state is called, and an ending screen is shown to the user along with their score. After the player acknowledges this, the leaderboard state is called and the user's score is emailed through AWS before returning to the idle screen state.
  </div>
  <div style="display:inline-block;vertical-align:top;flex:0 0 400px;">
    <div class="fig">
      <img src="./media/Image_005.jpg" style="width:90%;height:auto;" />
      <span class="caption">Function Diagram</span>
    </div>
  </div>
</div>

# Implementation
Our implementation consists of 4 main states, as well as containing many other important functions.

The code begins with the main function that calls a function for hardware setup. All necessary initializations for modules such as the pin mux, systick, UART, etc should be done here, before calling the idle state. In addition, the hardware setup function is where our machine connects to the internet and calls Amazon's servers. We decided to do this because the process takes roughly 10 seconds, and we felt that it would be better to wait this out at the start rather than force the user to wait after the game ends where the downtime is more noticeable. 

### Idle State

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 200px'>
    This state consists of the start screen and game initialization. A function is called to show the title screen on the OLED, and waits until a user input before starting the game. Once the user has decided to continue, the game is initialized by resetting all relevant variables of the game to their default state, such as the snake's length, color, size, speed, and position, in addition to all necessary game flags and the tick counter. Once this is done, the game is ready to begin, and the main game loop is called.

  </div>
  <div style='display: inline-block; vertical-align: top;flex:0 0 400px'>
    <div class="fig">
      <img src="./media/Image_007.jpg" style="width:auto;height:2.5in" />
      <span class="caption">Device Shadow JSON</span>
    </div>
  </div>
</div>

### Game Loop State
The main loop handles all game functionality. It begins by polling the accelerometer to adjust the snake's speed for the current tick and resetting the flag for if the snake has eaten.

#### Checking if food is eaten


#### Checking if a power up has been eaten


#### Move the Snake


#### Detect Collisions



#### Render Changes


### Game Over State

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    Once a collision has been detected, the game immediately ends and the user is shown a game over screen containing their score and an input prompt to continue. Once the input has been received, the leaderboard state is called.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:0 0 400px'>
    <div class="fig">
      <img src="./media/Image_008.jpg" style="width:auto;height:2in" />
      <span class="caption">OLED Wiring Diagram</span>
    </div>
  </div>
</div>

### IR Receiver

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    On both the Master and Slave devices, a user can input the TDS
    thresholds using a TV remote. These TV remotes use the NEC code format
    with a carrier frequency of 38KHz. The Vishay IR receiver is connected
    to Pin 62 of the CC3200, which is configured as a GPIO input pin. Each
    positive edge of the signal triggers an interrupt in the main program,
    storing the pulse distances into a buffer, and allowing us to decode the
    inputs (1-9, delete and enter). The IR receiver is connected to VCC
    through a resistor and a capacitor to filter any ripples.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:0 0 400px'>
    <div class="fig">
      <img src="./media/Image_009.jpg" style="width:auto;height:2in" />
      <span class="caption">IR Receiver Wiring Diagram</span>
    </div>
  </div>
</div>

## Functional Blocks: Slave

The slave device contains all the functional blocks from the master
device, plus the following:

### Analog-To-Digital Converter (ADC) Board

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    The outputs from the thermistor and TDS sensor board are
    in the form of analog voltages, which need to be converted to digital
    values to be usable in our program. We chose the AD1015 breakout board
    from Adafruit, which sports 4-channels and 12 bits of precision. We
    ended up using only 2 channels, so there is a potential for even more
    cost savings. The ADC board supports I2C communication, which we can use
    to request and read the two channel voltages. 
    The <a href="https://cdn-shop.adafruit.com/datasheets/ads1015.pdf">
    product datasheet</a> contains the necessary configuration values
    and register addresses for operation.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    <div class="fig">
      <img src="./media/Image_010.jpg" style="width:auto;height:2in" />
      <span class="caption">ADC Wiring Diagram</span>
    </div>
  </div>
</div>

### Thermistor

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 300px;'>
    Conductivity-based TDS measurements are sensitive to temperature. To
    allow accurate TDS measurements in a variety of climates and seasons,
    temperature compensation calculations must be performed. To measure the
    temperature, we use an NTC thermistor connected in a voltage divider
    with a 10k resistor. The voltage across the resistor is read by the ADC
    and converted to temperature using the equation provided by the
    thermistor datasheet.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    <div class="fig">
      <img src="./media/Image_011.jpg" style="width:auto;height:2in" />
      <span class="caption">Thermistor Circuit Diagram</span>
    </div>
  </div>
</div>

### TDS Sensor Board

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 500px'>
    In our first attempt to measure TDS, we used a simple two-probe analog
    setup with a voltage divider. We soon found out that this was a naïve
    approach (see Challenges). Consequently, we acquired a specialty TDS
    sensing board from CQRobot, which generates a sinusoidal pulse and
    measures the voltage drop to give a highly precise voltage to the ADC.
    The MCU can then convert this voltage to a TDS value using the equation
    provided by the device datasheet. We calibrated the TDS readings using a
    standalone TDS sensor pen. After calibration and setting up the curves
    for temperature compensation, we were able to achieve TDS readings
    accurate to within 5% of the TDS sensor pen.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 600px'>
    <div class="fig">
      <img src="./media/Image_012.jpg" style="width:auto;height:2in;padding-top:30px" />
      <span class="caption">TDS Sensor Wiring Diagram</span>
    </div>
  </div>
</div>

### Pumps and Control Circuit

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 500px'>
    The CC3200 is unable to provide sufficient power to drive the pumps,
    which need 100mA of current each. Therefore, we used an external power
    source in the form of 2 AA batteries for each pump motor. To allow the
    CC3200 to turn on/off the motors, we designed a simple amplifier using a
    Common Emitter topology. When the control pin is asserted HIGH, the BJT
    will allow current to flow from 3V to ground through the pump motor.
    Conversely, if the control signal is LOW, the BJT will not allow current
    to flow in the motor. For each motor, there is a reverse-biased diode
    connected across it. This protects our circuit from current generated by
    the motor if it is spun from external force or inertia.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 600px'>
    <div class="fig">
      <img src="./media/Image_013.jpg" style="width:auto;height:2in;padding-top:30px" />
      <span class="caption">Pump Circuit Diagrams</span>
    </div>
  </div>
</div>

# Challenges

The most significant challenge we faced while developing this prototype
was of inaccurate and inconsistent Electrical Conductivity (EC)
measurements. This occurred due to two reasons: probe channel
polarization and current limitations of GPIO pins.

## Probe Channel Polarization

Our first design for the probe was simply two copper rods, which would
add as electrodes. This probe would be connected in series with a
1000-ohm resistor to act as a voltage divider. We would simply connect
the probe to VCC and measure the voltage divider through the ADC,
allowing us to calculate the EC. However, when we used the probe for a
few minutes, we realized that the EC value would continue to rise. This
is because the DC current causes an ionized channel to build up between
the two electrodes in the water. This cause inconsistent EC readings as
time goes on.

## Current Limitation of GPIO Pins

Our next idea was to try using the GPIO pins to power the probe, since
we can turn it off when not needed, preventing excessive polarization.
However, the GPIO pins are current limited, and any control circuit with
a transistor would introduce extra voltage drops. Therefore, the simple
two-probe implementation was not feasible.

## Solution to Challenges

We realized that using DC current to measure EC was not feasible.
Therefore, we purchased a standalone EC measurement board from CQRobot.
This inexpensive solution (\$8) used a low-voltage, low-current AC
signal to prevent polarization. The board would convert the AC voltage
drop across the solution to a DC analog voltage, which would then be
read by our ADC. After calibrating the setup using a commercial TDS pen,
the results were accurate within 3%, and would not drift by more than
0.5% over time.

# Future Work

Given more time, we had the idea of developing a web app to allow users
to control the device from their cell phone. Another idea we wanted to
implement in the future is adding a grow light and pH controller to
maintain a more suitable and stable environment for different plants to
grow.


# Finalized BOM

<!-- you can convert google sheet cells to html for free using a converter
  like https://tabletomarkdown.com/convert-spreadsheet-to-html/ -->

<table style="border-collapse:collapse;">
<thead>
  <tr>
    <th><p>No.</p></th>
    <th><p>PART NAME</p></th>
    <th><p>DESCRIPTION</p></th>
    <th><p>Qty</p></th>
    <th><p>SUPPLIER / MANUFACTURER</p></th>
    <th><p>UNIT COST</p></th>
    <th><p>TOTAL PART COST</p></th>
    <th><p>Purpose</p></th>
  </tr>
</thead>
<tbody>
  <tr>
    <td><p>1</p></td>
    <td><p>CC3200-LAUNCHXL</p></td>
    <td><p>MCU Evaluation Board</p></td>
    <td><p>2</p></td>
    <td><p>Provided by EEC172 Course</p></td>
    <td><p>$66.00</p></td>
    <td><p>$132.00</p></td>
    <td><p>Control Remote and Local Devices</p></td>
  </tr>
  <tr>
    <td><p>2</p></td>
    <td><p>Adafruit 1431 OLED</p></td>
    <td><p>128x128 RGB OLED Display. SPI protocol</p></td>
    <td><p>2</p></td>
    <td><p>Provided by EEC172 Course</p></td>
    <td><p>$39.95</p></td>
    <td><p>$79.90</p></td>
    <td><p>Display PPM, Temperature, Thresholds, Inputs</p></td>
  </tr>
  <tr>
    <td><p>3</p></td>
    <td><p>Adafruit 4547 3VDC Pump</p></td>
    <td><p>Submersible pump. 3V 100mA DC</p></td>
    <td><p>2</p></td>
    <td><p>Adafruit</p></td>
    <td><p>$2.95</p></td>
    <td><p>$5.90</p></td>
    <td><p>For dispensing water and nutrient solution</p></td>
  </tr>
  <tr>
    <td><p>4</p></td>
    <td><p>Adafruit 4545 6mm Tube</p></td>
    <td><p>6mm Silicone Tube: 1 meter length</p></td>
    <td><p>1</p></td>
    <td><p>Adafruit</p></td>
    <td><p>$1.50</p></td>
    <td><p>$1.50</p></td>
    <td><p>For dispensing water and nutrient solution</p></td>
  </tr>
  <tr>
    <td><p>5</p></td>
    <td><p>NTC Thermistor 10k</p></td>
    <td><p>10k ohm nominal resistance, 100cm lead</p></td>
    <td><p>1</p></td>
    <td><p>(Already had one, available on Aliexpress)</p></td>
    <td><p>$0.92</p></td>
    <td><p>$0.92</p></td>
    <td><p>For temperature compensation</p></td>
  </tr>
  <tr>
    <td><p>6</p></td>
    <td><p>Adafruit AD1015 12-bit ADC</p></td>
    <td><p>12-bit resolution, 4 channels, I2C</p></td>
    <td><p>1</p></td>
    <td><p>Adafruit</p></td>
    <td><p>$9.95</p></td>
    <td><p>$9.95</p></td>
    <td><p>To convert Thermistor and TDS sensor reading to digital</p></td>
  </tr>
  <tr>
    <td><p>7</p></td>
    <td><p>PN2222A Transistor</p></td>
    <td><p>NPN BJT (40V, 1000mA)</p></td>
    <td><p>2</p></td>
    <td><p>Digikey (onSemi)</p></td>
    <td><p>$0.40</p></td>
    <td><p>$0.80</p></td>
    <td><p>For digital motor control</p></td>
  </tr>
  <tr>
    <td><p>8</p></td>
    <td><p>1N4001 Rectifier Diode</p></td>
    <td><p>Diffused junction: 50V 1000mA</p></td>
    <td><p>2</p></td>
    <td><p>Digikey (Good-Ark Semi)</p></td>
    <td><p>$0.16</p></td>
    <td><p>$0.32</p></td>
    <td><p>Reverse Current Protection</p></td>
  </tr>
  <tr>
    <td><p>9</p></td>
    <td><p>10k ohm resistor</p></td>
    <td><p>10k ohm , 1% tolerance, 0.25W</p></td>
    <td><p>1</p></td>
    <td><p>Digikey (Stackpole Electronics)</p></td>
    <td><p>$0.10</p></td>
    <td><p>$0.10</p></td>
    <td><p>Voltage divider for Thermistor</p></td>
  </tr>
  <tr>
    <td><p>10</p></td>
    <td><p>Vishay TSOP31130 IR RCVR</p></td>
    <td><p>30kHz carrier frequency</p></td>
    <td><p>2</p></td>
    <td><p>Provided by EEC172 Course</p></td>
    <td><p>$1.41</p></td>
    <td><p>$2.82</p></td>
    <td><p>Decode user inputs</p></td>
  </tr>
  <tr>
    <td><p>11</p></td>
    <td><p>330 ohm resistor</p></td>
    <td><p>330 ohm resistor, &lt;5% tolerance, 3W</p></td>
    <td><p>2</p></td>
    <td><p>Provided by EEC172 Course</p></td>
    <td><p>$0.59</p></td>
    <td><p>$1.18</p></td>
    <td><p>Current Limit for IR Receiv er</p></td>
  </tr>
  <tr>
    <td><p>12</p></td>
    <td><p>ATT-RC1534801 Remote</p></td>
    <td><p>General-purpose TV remote. IR NTC protocol</p></td>
    <td><p>1</p></td>
    <td><p>Provided by EEC172 Course</p></td>
    <td><p>$9.99</p></td>
    <td><p>$9.99</p></td>
    <td><p>Allow user inputs</p></td>
  </tr>
  <tr>
    <td><p>13</p></td>
    <td><p>CQRSENTDS01 TDS Sensor</p></td>
    <td><p>Analog reading 0-2.3V. 0-1000ppm range</p></td>
    <td><p>1</p></td>
    <td><p>CQRobot</p></td>
    <td><p>$7.99</p></td>
    <td><p>$7.99</p></td>
    <td><p>Measure TDS of plant solution</p></td>
  </tr>
  <tr>
    <td><p>14</p></td>
    <td><p>10uF Capacitor</p></td>
    <td><p>Electrolytic Cap 100V</p></td>
    <td><p>2</p></td>
    <td><p>Provided by EEC172 Course</p></td>
    <td><p>$0.18</p></td>
    <td><p>$0.36</p></td>
    <td><p>DC Filtering for IR Receiv er</p></td>
  </tr>
  <tr>
    <td><p>15</p></td>
    <td><p><u>AA Battery (4ct</u>)</p></td>
    <td><p>1.5 Volt, Non-rechargable</p></td>
    <td><p>1</p></td>
    <td><p>Already had, but</p>
      <p>available on Amazon</p></td>
    <td><p>$3.65</p></td>
    <td><p>$3.65</p></td>
    <td><p>Provide Power to Motors</p></td>
  </tr>
  <tr>
    <td><p>16</p></td>
    <td><p>Battery Holder</p></td>
    <td><p>2xAA (3 Volts Total)</p></td>
    <td><p>2</p></td>
    <td><p>Amazon</p></td>
    <td><p>$2.50</p></td>
    <td><p>$4.99</p></td>
    <td><p>Provide Power to Motors</p></td>
  </tr>
  <tr>
    <td colspan="3">
      <p>TOTAL PARTS</p></td>
    <td><p>25</p></td>
    <td colspan="2">
      <p>TOTAL</p></td>
    <td><p>$262.37</p></td>
    <td></td>
  </tr>
  <tr>
    <td colspan="3">
      <p>TOTAL PARTS (Excluding Provided)</p></td>
    <td><p>14</p></td>
    <td colspan="2">
      <p>TOTAL (Exluding Provided)</p></td>
    <td><p>$36.12</p></td>
    <td></td>
  </tr>
</tbody>
</table>
