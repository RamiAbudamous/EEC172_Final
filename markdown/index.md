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
The code checks if the snake has eaten by comparing the position of the food to the position of the snake's head. This is done by checking if the difference in the X and Y positions between the snake's head and the food is less than the size of the snake's head. If this is true, that means that the snake is touching the food, and should eat it. When this happens, the snake's color is updated and the food is moved to a new random position. A power up is also placed if none are on the board. Finally, the snake grows in size.
The snake's color is updated by cycling through an array of predefined colors. The current color is stored as a global variable and is used any time the snake is printed onto the OLED screen. When it is time to update the color, the variable is simply changed to the next entry in the array.
The food is moved to a new position by filling the current space with the background color, and randomly selecting a new space for the next piece of food. The new piece is not drawn until the game renders at the end of the tick.
Power ups are placed very similarly to food, by selecting a random position and waiting for the game to render.
Adding a snake segment is done by moving to the tail of the current snake and adding a segment, as well as updating the counter so that the new segment is not removed at the end of the tick.
Finally, a flag is set stating that the snake has eaten on this tick.

#### Checking if a power up has been eaten
This is done very similarly to the check for food. It compared the position of the power up to the position of the snake's head, and if they overlap then the size increase power up is activated. A flag is set noting that the power up is active, and the size of each snake segment is doubled. The duration of the power up is set to a predetermined constant, and the power up is removed from the screen by filling in the zone with the background color. This does not affect the snake as the renderer at the end of the tick will ensure that the snake is displayed properly.

If a power up is active, then the number of remaining ticks is decremented. When it hits 0, the snake's size returns to normal and the larger circles are filled with the background color by looping through every segment of the snake and filling them one by one. Once again, this does not affect the snake as it will be rendered again at the end of the tick.

#### Move the Snake
This function polls the IR remote and changes the direction if necessary. Otherwise, it updates the speed of the snake's turn based on information from the accelerometer. The snake's position is updated by adding this speed value for both the X and Y axis to the position of the head. If the snake moves out of any of the borders, it is moved to the other side. Finally, a for loop is used to move all segments to the space occupied by the previous segment by iterating backwards from the tail of the snake through the front. The head of the snake is then set to the position calculated earlier when the speed was added to the head.

#### Detect Collisions
Collisions are detected by looping through all snake segments after the third and checking if that segment overlaps with the head, in the same way that food and power ups were checked. Because the second segment touches the head often, it is excluded from collision checks. The third segment is also omitted because the size power up can cause it to touch the head while turning. This does not affect gameplay because it is impossible to get the snake's head to touch the third segment legitimately. When a collision is detected, the game over state is called.

#### Render Changes
The renderer draws all changes on screen. It begins by drawing the food and power up at their current positions. If either is already on the board, this is unnoticeable, but if either one was eaten, then a new one is drawn at this time.
Next, the snake is drawn by looping through each segment and drawing it, as the positions have been recently updated by the moveSnake() function. Despite most of the segments being in the same position as the former segment was on the previous tick, they must all be redrawn so that a color change can happen when needed. After the segments have been moved, the previous tail segment must be filled in with the background color in order to prevent a visual artifact from showing.

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

### AWS Email

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

### Hardware Implementation

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 500px'>
  A breadboard is required to implement the necessary circuit for the IR receiver, as well as to connect the OLED to the CC3200 board. This is how we built it:
  </div>
  <div style='display: inline-block; vertical-align: top;flex:1 0 600px'>
    <div class="fig">
      <img src="./media/Image_012.jpg" style="width:auto;height:2in;padding-top:30px" />
      <span class="caption">Circuit on Breadboard</span>
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

We faced a few major challenges during the implementation of our project. Notably, our code has a dependency on Part 3 from Lab 3 that we were unable to resolve, and we needed to change our AWS implementation from a leaderboard to an email of the player's game score.

## Dependency on Lab 3 Code for OLED Module Initialization

We faced a puzzling issue where we needed to run our code from Lab 3 in order to activate and work with the OLED display. Only after running this code would our OLED turn on for this final project. Depsite using the exact same code from lab 3 in our final code for this project, we could not debug this issue. After hours of debugging the dependency, our attempts to isolate it were unsuccessful and ultimately prevented us from being able to flash the final program onto our CC3200s. Hence, this dependency was unexpected and led to a limitation where our project could not meet the standalone operational requirement.

## Current Limitation of GPIO Pins

Our next idea was to try using the GPIO pins to power the probe, since
we can turn it off when not needed, preventing excessive polarization.
However, the GPIO pins are current limited, and any control circuit with
a transistor would introduce extra voltage drops. Therefore, the simple
two-probe implementation was not feasible.

## Modification of Leaderboard Implementation

Originally, we aimed to implement an AWS-based leaderboard that would dynamically display user scores using server-side JSON. However, we faced issues in integrating and manipulating JSON within our code, despite attempting to use numerous libraries such as cJSON, json-c, jsmn, and tiny-json. Given these technical obstacles and time constraints, we instead chose to modify the leaderboard implementation, and opted to email the final score to the player instead. This change ultimately worked out, but moved away from the interactive and real-time competition aspect that we initially aimed for.

# Future Work

Despite completing all our basic and target goals with some modifications, there were some features we had in mind that we would have liked to implement.

## Advanced Leaderboard Integration

One of our target goals was to successfully implement an AWS-based real-time leaderboard. If we had more time, we would iron out our issues with one of the JSON libraries and ensure its integration with our game. Because more common libraries such as cJSON were too big for us to use, the go-to choice would likely be tiny-json if we had the time to learn how to use it.

## Incorporation of Obstacles (Walls)

Currently, our snake can freely traverse the screen without any obstacles besides its own tail. To add complexity to our gameplay, we would have liked to implement walls that the snake would have to maneuver around to avoid collision. Some ideas for implementing this would be adding wall segments as a possible outcome from a power up, or by adding a wall segment whenever food is eaten.

## More Power Ups

Given more time, we would have implemented more power-ups. Some particular power ups we planned on implementing are a speed boost that would temporarily increase the snake's speed, and a point multiplier that would add two segments to the snake when it eats food, instead of one.

## More Game Modes

Given more time, we would have implemented game modes other than classic snake, such as time trial mode where players must score as many points as possible within the time limit and survival mode where the game introduces more obstacles over time which the player must avoid to survive as long as possible.

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
