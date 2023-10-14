----Introduction to source code----
I'm write this source code to make "GAS-CO monitor and warning project" use NODEMCU-32S and Arduino farmework.
It will read analog value from MQ-7 and MQ-6 sensor and Upload to BKYNK. At the board I use a buzzer to warn when the threshold is crossed. In addition, there are RGB LED lights to indicate the concentration of the above two gases. 
This source read analog value from MQ-7 and MQ-6 sensor. You can use library to read value of ppm if you want that uint.
