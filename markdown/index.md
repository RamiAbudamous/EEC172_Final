---
title: 'Snake (if it was good)'
author: '**Rami Abudamous and Mihika Krishna** (website template by Ryan Tsang)'
date: 'EEC172 SQ24'

subtitle: '<blockquote>The website source is hosted 
<a href="https://github.com/RamiAbudamous/EEC172_Final">on github</a>.
</blockquote>'

toc-title: 'Table of Contents'
abstract-title: '<h2>Description</h2>'
abstract: 'We decided to create a game of snake for our project as early as lab two because we noticed that the moving ball from lab 2 was similar to how a snake moves, and we thought that the accelerometer would be an interesting control mechanism for snake compared to the traditional directional inputs. When it came time to implement our design, we also included directional inputs via the IR remote, as we felt that giving the player the option to choose their method of control would make the game more enjoyable, in addition to meeting the project requirements. The game begins with a title screen that prompts the user to start. The main gameplay loop consists of a snake that can move around the screen, seeking to eat food. Eating  piece of food increases the size of the snake and places a new piece of food on the board. The snake dies when it runs into itself, but can move into one end of the border and appear from the other side. We also implemented power ups to further differentiate our game from the traditional snake game, with a special piece of food that makes the snake much larger for a period, making it easier to get to food, while also increasing the risk of running into oneself. Upon the snake''s death, a game over screen is displayed, and the user''s score is sent to an email via AWS. The game then returns to the title screen and prompts the user to play again.
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
</div>

<!-- 560 by 315 originally -->
<h2>Video Demo</h2>
<div style="text-align:center;margin:auto;max-width:560px">
  <div style="padding-bottom:56.25%;position:relative;height:0;">
    <iframe style="left:0;top:0;width:100%;height:100%;position:absolute;" width="auto" height="auto" src="https://www.youtube.com/embed/_4U2e4Hofy8" title="YouTube video player" frameborder="0" allow="accelerometer; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
  </div>
</div>
'
---

# Design

## System Architecture

<div style="display:flex;flex-wrap:wrap;justify-content:space-evenly;">
  <div style="display:inline-block;vertical-align:top;flex:1 0 400px;">
  The architecture diagram consists of 5 blocks each representing a component of the system. The main control unit is the CC3200 board which runs the written code and connects all the other components to eachother. All information is displayed on an OLED screen via SPI, which is connected to the MCU by 5 pins, power, and ground. User input is taken from both the CC3200's built in accelerometer through I2C, as well as the IR remote via a receiver that is built on a breadboard (See the implementation section for the circuit). Finally, AWS is called from the CC3200's code in order to email the user their final score.
  </div>
  <div style="display:inline-block;vertical-align:top;flex:0 0 400px">
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
      <img src="./media/Image_008.jpg" style="width:auto;height:3in" />
      <span class="caption">Game Over Screen</span>
    </div>
  </div>
</div>

### AWS Email (Leaderboard) State

<div style="display:flex;flex-wrap:wrap;justify-content:space-between;">
  <div style='display: inline-block; vertical-align: top;flex:1 0 400px'>
    While this state is called the leaderboard state, this is simply an artifact from earlier in the project before changes were made due to limitations of the board and C that make it extremely difficult to properly parse a JSON, which is necessary to store a leaderboard through AWS. As such, the functionality of this state was changed to instead email the user their score via AWS. This is done by editing the user's score into a predefined JSON and sending a POST request through SimpleLink. Ensure that the device is already registered in AWS and that the proper credentials have been flashed onto the CC3200 board, otherwise this will not work.
  </div>
  <div style='display: inline-block; vertical-align: top;flex:0 0 400px'>
    <div class="fig">
      <img src="./media/Image_009.jpg" style="width:auto;height:2in" />
      <span class="caption">AWS email</span>
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
      <img src="./media/Image_012.jpg" style="width:auto;height:6in;padding-top:30px" />
      <span class="caption">Circuit on Breadboard</span>
    </div>
  </div>
</div>

# Challenges

We faced a few major challenges during the implementation of our project. Notably, our code has a dependency on Part 3 from Lab 3 that we were unable to resolve, and we needed to change our AWS implementation from a leaderboard to an email of the player's game score.

## Dependency on Lab 3 Code for OLED Module Initialization

We faced a puzzling issue where we needed to run our code from Lab 3 in order to activate and work with the OLED display. Only after running this code would our OLED turn on for this final project. Depsite using the exact same code from lab 3 in our final code for this project, we could not debug this issue. After hours of debugging the dependency, our attempts to isolate it were unsuccessful and ultimately prevented us from being able to flash the final program onto our CC3200s. Hence, this dependency was unexpected and led to a limitation where our project could not meet the standalone operational requirement.

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

<table cellspacing="0" cellpadding="0" dir="ltr" border="1" style="" data-sheets-root="1">
  <thead>
    <tr style="height:21px;">
      <th>Component</th>
      <th>Description</th>
      <th>Quantity</th>
      <th>Purpose</th>
      <th>Total Price</th>
      <th>Supplier</th>
    </tr>
  </thead><colgroup><col width="100"><col width="100"><col width="100"><col width="100"><col width="100"><col width="100"></colgroup>
  <tbody>
    <tr style="height:21px;">
      <td style="border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;CC3200-LAUNCHXL&quot;}">CC3200-LAUNCHXL</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;MCU Board&quot;}">MCU Board</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:1}">1</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Main board&quot;}">Main board</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:66}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$66.00</td>
      <td style="border-right:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Provided by EEC172 Course &quot;}">Provided by EEC172 Course </td>
    </tr>
    <tr style="height:21px;">
      <td style="border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Adafruit 1431 OLED&quot;}">Adafruit 1431 OLED</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;128x128 RGB OLED Display, SPI protocol&quot;}">128x128 RGB OLED Display, SPI protocol</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:1}">1</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Display game&quot;}">Display game</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:39.95}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$39.95</td>
      <td style="border-right:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Provided by EEC172 Course &quot;}">Provided by EEC172 Course </td>
    </tr>
    <tr style="height:21px;">
      <td style="border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;ATT-RC1534801 Remote&quot;}">ATT-RC1534801 Remote</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Universal TV remote, IR NTC protocol&quot;}">Universal TV remote, IR NTC protocol</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:1}">1</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Controlling the game&quot;}">Controlling the game</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:9.99}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$9.99</td>
      <td style="border-right:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Provided by EEC172 Course &quot;}">Provided by EEC172 Course </td>
    </tr>
    <tr style="height:21px;">
      <td style="border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;10uF Capacitor&quot;}">10uF Capacitor</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Capacitor&quot;}">Capacitor</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:1}">1</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Part of the circuit for the IR receiver&quot;}">Part of the circuit for the IR receiver</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:0.18}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$0.18</td>
      <td style="border-right:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Provided by EEC172 Course &quot;}">Provided by EEC172 Course </td>
    </tr>
    <tr style="height:21px;">
      <td style="border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;100 ohm resistor&quot;}">100 ohm resistor</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Resistor&quot;}">Resistor</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:1}">1</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Part of the circuit for the IR receiver&quot;}">Part of the circuit for the IR receiver</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:0.1}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$0.10</td>
      <td style="border-right:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Provided by EEC172 Course &quot;}">Provided by EEC172 Course </td>
    </tr>
    <tr style="height:21px;">
      <td style="border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Vishay TSOP31130 IR RCVR&quot;}">Vishay TSOP31130 IR RCVR</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;IR Receiver&quot;}">IR Receiver</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:1}">1</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Part of the circuit for the IR receiver&quot;}">Part of the circuit for the IR receiver</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:1.41}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$1.41</td>
      <td style="border-right:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Provided by EEC172 Course &quot;}">Provided by EEC172 Course </td>
    </tr>
    <tr style="height:21px;">
      <td style="border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" rowspan="1" colspan="2" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Total Parts:&quot;}">Total Parts:</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:6}">6</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Total Price:&quot;}">Total Price:</td>
      <td style="overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:117.63}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$117.63</td>
      <td style="border-right:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;"></td>
    </tr>
    <tr style="height:21px;">
      <td style="border-bottom:2px solid #000000;border-left:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" rowspan="1" colspan="2" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Total Parts (excluding provided):&quot;}">Total Parts (excluding provided):</td>
      <td style="border-bottom:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:0}">0</td>
      <td style="border-bottom:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;" data-sheets-value="{&quot;1&quot;:2,&quot;2&quot;:&quot;Total Price (exclusing provided):&quot;}">Total Price (exclusing provided):</td>
      <td style="border-bottom:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;wrap-strategy:4;white-space:normal;word-wrap:break-word;text-align:center;" data-sheets-value="{&quot;1&quot;:3,&quot;3&quot;:0}" data-sheets-numberformat="{&quot;1&quot;:4,&quot;2&quot;:&quot;\&quot;$\&quot;#,##0.00&quot;,&quot;3&quot;:1}">$0.00</td>
      <td style="border-right:2px solid #000000;border-bottom:2px solid #000000;overflow:hidden;padding:2px 3px 2px 3px;vertical-align:middle;"></td>
    </tr>
  </tbody>
</table>
