Starship Gold - Made by Macrae Smith Cohort 34

Instructions:
---------------------------------------------
Open up the Starship_Release_x64.exe file in the Run folder to launch the game

or

Open the Starship Solution in Visual Studio 2022 and press ctrl + shift + B to build, and then F5 to play the game. 

(If you are building from VS 2022 for the first time, you will need to right click on the Starship Solution in Editor and click Properties->ConfigurationProperties->Debugging and change the Command field to $(TargetFileName) and the Working Directory filed to $(SolutionDir)Run/)

Keyboard Controls:
---------------------------------------------
W - Move forward
D - Rotate Clockwise
A - Rotate Counter-clockwise
SPACEBAR - fire bullets
N - Respawn

ARROW KEYS - Navigate Menu
SPACEBAR - Select Button
ESC - Go back/Quit Game
P - Pause/Unpause
T - slow down time to 1/10th
O - Run one frame and then pause
N - Respawn after player ship dies
I - Spawn another asteroid
F1 - Turn on/off debug drawings
F8 - Restart Game

Xbox Controller Controls:
---------------------------------------------
Right Stick - Move forward
Left Stick - Rotate
Right Trigger or A button - Fire bullets

DPAD ARROW KEYS - Navigate Menu
A Button - Select Button
B Button - Go Back

Deep Learning:
---------------------------------------------
Throughout this project I was amazed by how much my thought processes changed in approaches to architecture of code. The iterative progress through each assignment (Assignment 1 - 4) made me realize how knowing the all or at least most of the features I will be solving helps me organize my code in a readable and productive manner. However as it is nearly impossible to actually know every problem that will come up, I found that my approach towards architecture is growing in a way that allows me to organize the issues I think I will need to solve, while leaving it easy to add functionality later. Even in the Starship Gold assignment, I found myself beginning the assignment having to constantly go back and rewrite old code, but as I became more aware of the things I was having to change consistently, I began writing all new code in a way that could accommodate for those changes if needed. To that point, I have also began to appreciate the practice of revisiting and refactoring old code more. While it can be frustrating to have to revisit work, I have begun to see the benefit of keeping everything organized as unforeseen changes come up during development. Overall I have seen my understanding of C++ grow immensely in this project, but more importantly, I have seen my understanding of the flow of logic grow as I am managing the connection between systems rather than just tackling every little problem only when it arises.

Known Bugs:
---------------------------------------------
There appears to be some audio issues when playing multiplayer as you get into the higher level enemy waves. This first appeared during the showcase in class, and I have not been able to do enough testing to determine if this was due to the audio setup through HDMI, or would replicate in any setup. The main issue is that all audio cuts out for several seconds and then reappears.
