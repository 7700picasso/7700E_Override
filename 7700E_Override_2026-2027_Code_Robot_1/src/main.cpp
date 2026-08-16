/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Student                                                   */
/*    Created:      8/9/2026, 3:38:53 PM                                      */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include "vex.h"

using namespace vex;

// A global instance of competition
competition Competition;
brain Brain;
controller Controller;

motor LeftFront = motor(PORT8, ratio18_1, true);
motor LeftMiddle = motor(PORT6, ratio6_1, true);
motor LeftBack = motor(PORT4, ratio6_1, true);
motor RightFront = motor(PORT7, ratio18_1, false);
motor RightMiddle = motor(PORT5, ratio6_1, false);
motor RightBack = motor(PORT9, ratio6_1, false); 

// define your global instances of motors and other devices here

/*---------------------------------------------------------------------------*/
/*                          Pre-Autonomous Functions                         */
/*                                                                           */
/*  You may want to perform some actions before the competition starts.      */
/*  Do them in the following function.  You must return from this function   */
/*  or the autonomous and usercontrol tasks will not be started.  This       */
/*  function is only called once after the V5 has been powered on and        */
/*  not every time that the robot is disabled.                               */
/*---------------------------------------------------------------------------*/

void drive(int lspeed, int rspeed, int wt){
  LeftMiddle.spin(forward, lspeed, pct);
  RightMiddle.spin(forward, rspeed, pct);
  RightFront.spin(forward, rspeed, pct);
  LeftFront.spin(forward, lspeed, pct);
  RightBack.spin(forward, rspeed, pct);
  LeftBack.spin(forward, lspeed, pct);
  wait(wt, msec);
}

void driveBrake(){
  LeftMiddle.stop(brake);
  RightMiddle.stop(brake);
  LeftFront.stop(brake);
  RightFront.stop(brake);
  LeftBack.stop(brake);
  RightBack.stop(brake);
}

double YOFFSET = 20; //offset for the display
//Writes a line for the diagnostics of a motor on the Brain
void MotorDisplay(double y, double curr, double temp)
{
	Brain.Screen.setFillColor(transparent);
	Brain.Screen.printAt(5, YOFFSET + y, "Current: %.1fA", curr);
	
	if (curr < 1){
		Brain.Screen.setFillColor(green);
	} else if(curr >= 1 && curr  <= 2.5) {
		Brain.Screen.setFillColor(yellow);
	} else {
		Brain.Screen.setFillColor(red);
		Brain.Screen.drawRectangle(140, YOFFSET + y - 15, 15, 15);
	}

	
	Brain.Screen.setFillColor(transparent);
	Brain.Screen.printAt(160, YOFFSET + y, "Temp: %.1fC", temp);
	
	if (temp < 45){
		Brain.Screen.setFillColor(green);
	} else if(temp <= 50 && temp  >= 45){
		// TRUE and TRUE --> True
		// TRUE and FALSE --> False
		// FALSE and FALSE --> False
		Brain.Screen.setFillColor(yellow);
	} else {
		Brain.Screen.setFillColor(red);
		Brain.Screen.drawRectangle(275, YOFFSET + y - 15, 15, 15);
		Brain.Screen.setFillColor(transparent);
	}
}


//Displays information on the brain
void Display()
{
	double leftFrontCurr = LeftFront.current(amp);
	double leftFrontTemp = LeftFront.temperature(celsius);
	double leftBackCurr = LeftBack.current(amp);
	double leftBackTemp = LeftBack.temperature(celsius);
	double rightFrontCurr = RightFront.current(amp);
	double rightFrontTemp = RightFront.temperature(celsius);
	double rightBackCurr = RightBack.current(amp);
	double rightBackTemp = RightBack.temperature(celsius);
  double rightMiddleCurr = RightMiddle.temperature(celsius);
  double rightMiddleTemp = RightMiddle.current(amp);
  double leftMiddleCurr = LeftMiddle.temperature(celsius);
  double leftMiddleTemp = LeftMiddle.current(amp);


	if (LeftFront.installed()){
		MotorDisplay(1, leftFrontCurr, leftFrontTemp);
		Brain.Screen.printAt(300, YOFFSET + 1, "LeftFront");
	} else {
		Brain.Screen.printAt(5, YOFFSET + 1, "LeftFront Problem");
	}
	
	
	if (LeftBack.installed()){
		MotorDisplay(31, leftBackCurr, leftBackTemp);
		Brain.Screen.printAt(300, YOFFSET + 31, "LeftBack");
	} else {
		Brain.Screen.printAt(5, YOFFSET + 31, "LeftBack Problem");
	}


	if (RightFront.installed()) {
		MotorDisplay(61, rightFrontCurr, rightFrontTemp);
		Brain.Screen.printAt(300, YOFFSET + 61, "RightFront");
	} else {
		Brain.Screen.printAt(5, YOFFSET + 61, "RightFront Problem");
	}
	
	
	if (RightBack.installed()) {
		MotorDisplay(91, rightBackCurr, rightBackTemp);
		Brain.Screen.printAt(300, YOFFSET + 91, "RightBack");
	} else {
		Brain.Screen.printAt(5, YOFFSET + 91, "RightBack Problem");
	}

}

void pre_auton(void) {

  // All activities that occur before the competition starts
  // Example: clearing encoders, setting servo positions, ...
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

void autonomous(void) {
  // ..........................................................................
  // Insert autonomous user code here.
  // ..........................................................................
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              User Control Task                            */
/*                                                                           */
/*  This task is used to control your robot during the user control phase of */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

void usercontrol(void) {
  // User control code here, inside the loop
	int lspeed = 0;
	int rspeed = 0;
  while (1) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.
	lspeed = Controller.Axis3.position(pct);
	rspeed = Controller.Axis2.position(pct);
	drive(lspeed, rspeed, 10);
    // ........................................................................
    // Insert user code here. This is where you use the joystick values to
    // update your motors, etc.
    // ........................................................................

    wait(20, msec); // Sleep the task for a short amount of time to
                    // prevent wasted resources.
  }
}

//
// Main will set up the competition functions and callbacks.
//
int main() {
  // Set up callbacks for autonomous and driver control periods.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);
  Display();
  // Run the pre-autonomous function.
  pre_auton();

  // Prevent main from exiting with an infinite loop.
  while (true) {
    wait(100, msec);
  }
}
