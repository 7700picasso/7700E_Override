/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Student                                                   */
/*    Created:      8/9/2026, 3:38:53 PM                                      */
/*    Description:  V5 project                                                */
/*    he took a bite of a boneless wing                                       */
/*----------------------------------------------------------------------------*/

#include "vex.h"

using namespace vex;

// A global instance of competition
competition Competition;
brain Brain;
controller Controller;

motor LeftFront = motor(PORT11, ratio18_1, true);
motor LeftMiddle = motor(PORT6, ratio6_1, true);
motor LeftBack = motor(PORT4, ratio6_1, true);
motor RightFront = motor(PORT7, ratio18_1, false);
motor RightMiddle = motor(PORT5, ratio6_1, false);
motor RightBack = motor(PORT9, ratio6_1, false); 
motor DR4B1 = motor(PORT20, ratio6_1, true);
motor DR4B2 = motor(PORT8, ratio6_1, true);

digital_out claw = digital_out(Brain.ThreeWirePort.A);

inertial gyroturn = inertial(PORT10);
// define your global instances of motors and other devices here

double pi = 3.14;
double d = 3.25;
double g = 0.6;
int AutonSelected = 0;
int AutonMin = 0;
int AutonMax = 2;

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

void liftUP(){
	DR4B1.spin(forward, 100, pct);
	DR4B2.spin(reverse, 100, pct);
}

void liftDOWN(){
	DR4B1.spin(forward, -100, pct);
	DR4B2.spin(reverse, -100, pct);
}


void driveBrake(){
  LeftMiddle.stop(brake);
  RightMiddle.stop(brake);
  LeftFront.stop(brake);
  RightFront.stop(brake);
  LeftBack.stop(brake);
  RightBack.stop(brake);
}

void clamp(){
	claw.set(!claw.value());
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

	if (LeftMiddle.installed()) {
		MotorDisplay(121, leftMiddleCurr, leftMiddleTemp);
		Brain.Screen.printAt(300, YOFFSET + 121, "LeftMiddle");
	} else {
		Brain.Screen.printAt(5, YOFFSET + 121, "LeftMiddle Problem");
	}

	if (RightMiddle.installed()) {
		MotorDisplay(151, rightMiddleCurr, rightMiddleTemp);
		Brain.Screen.printAt(300, YOFFSET + 151, "RightMiddle");
	} else {
		Brain.Screen.printAt(5, YOFFSET + 151, "RightMiddle Problem");
	}
}

void selectAuton() {
		bool selectingAuton = true;
		
		int x = Brain.Screen.xPosition(); // get the x position of last touch of the screen
		int y = Brain.Screen.yPosition(); // get the y position of last touch of the screen
		
		// check to see if buttons were pressed
		if (x >= 20 && x <= 120 && y >= 50 && y <= 150){ // select button pressed
				AutonSelected++;
				if (AutonSelected > AutonMax){
						AutonSelected = AutonMin; // rollover
				}
				if(AutonSelected == 0){
					Brain.Screen.printAt(1, 200, "Auton Selected =  %d   Left Side", AutonSelected);
				}
				if(AutonSelected == 1){
					Brain.Screen.printAt(1, 200, "Auton Selected =  %d   Right Side", AutonSelected);
				}
				if(AutonSelected == 2){
					Brain.Screen.printAt(1, 200, "Auton Selected =  %d   1 min auton", AutonSelected);
				}																																													   
				
		}
		
		
		if (x >= 170 && x <= 270 && y >= 50 && y <= 150) {
				selectingAuton = false; // GO button pressed
				Brain.Screen.printAt(1, 200, "Auton  =  %d   GO           ", AutonSelected);
		}
		
		if (!selectingAuton) {
				Brain.Screen.setFillColor(green);
				Brain.Screen.drawCircle(300, 75, 75);
				Brain.Screen.printAt(300, 75, "Ready");
		} else {
				Brain.Screen.setFillColor(red);
				Brain.Screen.drawCircle(300, 75, 25);
		}
		
		wait(10, msec); // slow it down
		Brain.Screen.setFillColor(black);
		
}

void drawGUI() {
	// Draws 2 buttons to be used for selecting auto
	Brain.Screen.clearScreen();
	Brain.Screen.printAt(1, 40, "Select Auton then Press Go");
	Brain.Screen.printAt(1, 200, "Auton Selected =  %d   ", AutonSelected);

	//Draw SELECT button
	Brain.Screen.setFillColor(red);
	Brain.Screen.drawRectangle(20, 50, 100, 100);
	Brain.Screen.drawCircle(300, 75, 25);
	Brain.Screen.printAt(25, 75, "Select");

	//Draw GO button
	Brain.Screen.setFillColor(green);
	Brain.Screen.drawRectangle(170, 50, 100, 100);
	Brain.Screen.printAt(175, 75, "GO");
	Brain.Screen.setFillColor(black);
}


//inch drive
void inchDrive(double target){
	double position = 0;
	double error = target - position;
	double kP = 1.5;
	double speed = kP * error;
	double accuracy = 2;
	LeftFront.setPosition(0.0, rev);

	while(fabs(error) >= accuracy){
		drive(speed, speed, 10);
		position = LeftFront.position(rev);
		error = target - position;
		speed = kP * error;
	}
	driveBrake();
}


//gyro turning

void turn(double target){
	double rotation = 0;
	double error = target - rotation;
	double kP = 1.5;
	double speed = kP * error;
	double accuracy = 2;
	gyroturn.setRotation(0.0, deg);

	while(fabs(error) >= accuracy){
		drive(speed, -speed, 10);
		rotation = gyroturn.rotation(deg);
		error = target - rotation;
		speed = kP * error;
	}
	driveBrake();
	
}


void pre_auton(void) {
  	// Initializing Robot Configuration. DO NOT REMOVE!
	Brain.Screen.printAt(1, 40, "pre auton is running");
	drawGUI();
	Brain.Screen.pressed(selectAuton);
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

	//switch case
	switch (AutonSelected) {
		case 0:
			//code 0: 15sec LEFT SIDE
			Brain.Screen.printAt(1, 220, "auton 0 is running");
			clamp();
			liftUP();
			inchDrive(14);
			turn(-90);
			inchDrive(4);
			liftDOWN();
			break;
		
		case 1:
			//code 1: 15sec RIGHT SIDE
			Brain.Screen.printAt(1, 220, "auton 1 is running");
			clamp();
			liftUP();
			inchDrive(14);
			turn(90);
			inchDrive(4);
			liftDOWN();
			break;
		
		case 2:
			//code 2: 1 min AUTON SKILLS
			Brain.Screen.printAt(1, 220, "auton 2 is running");
			break;
		}
		Brain.Screen.printAt(10, 220, "COMPLETE");
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
	  Display();
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.
	lspeed = Controller.Axis3.position(pct);
	rspeed = Controller.Axis2.position(pct);
	drive(lspeed, rspeed, 10);

	Controller.ButtonA.pressed(clamp);

	if (Controller.ButtonR2.pressing()){
		liftUP();
	}else if (Controller.ButtonR1.pressing()){
		liftDOWN();
	}else{
		DR4B1.stop(brake);
		DR4B2.stop(brake);
	}
	
	
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
  // Run the pre-autonomous function.
  pre_auton();

  // Prevent main from exiting with an infinite loop.
  while (true) {
    wait(100, msec);
  }
}
