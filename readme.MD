#Maze solving robot

This repository contains the source code of a simple maze solving robot. In order to do this, the robot uses an Arduino Mega as well as a sensor that is able to detect the lines which form the maze.

The robot first travels through the maze in search of an exit while keeping track of all the turns that were made during this operation. It then computes a direct path from this series of turns so that it will be able to instantly exit the maze on the second run.

The path shortening algorithm was inspired by the one explained [Here](https://create.arduino.cc/projecthub/mjrobot/maze-solver-robot-using-artificial-intelligence-4318cf), it simply performs a series of search and replaces on the path until no more U turns are found.

This project is made for our electronics class over at [ENSAM](http://www.ensam-casa.ma/).

![Console demo](http://i.imgur.com/Ka6GPJk.gif "Console demo")
