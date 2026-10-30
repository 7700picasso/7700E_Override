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
 

motor LeftFront = motor(PORT19, ratio18_1, true);
motor LeftMiddle = motor(PORT7, ratio6_1, true);
motor LeftBack = motor(PORT9, ratio6_1, true);
motor RightFront = motor(PORT18, ratio18_1, false);
motor RightMiddle = motor(PORT11, ratio6_1, false);
motor RightBack = motor(PORT13, ratio6_1, false); 
motor DR4BR = motor(PORT20, ratio6_1, false);
motor DR4BL = motor(PORT2, ratio6_1, true);
digital_out clawP = digital_out(Brain.ThreeWirePort.A);

inertial gyroturn = inertial(PORT21);
// define your global instances of motors and other devices here

double pi = 3.14;
double d = 3.25;
double g = 0.6;
int selectedAuton = 0;
int AutonMin = 0;
int AutonMax = 3;
const double SETUP_DEGREES = 65.0; // Initial pull-in from resting position
const double GRIP_DEGREES  = 100.0; // Degrees inward from ready position to clamp
const double OPEN_DEGREES  = 0.0;   // Ready/open position

bool isSetupDone = false;
bool isGripping  = false;
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
  double left = lspeed * 0.12;
  double right = rspeed * 0.12;
  LeftMiddle.spin(forward, left, volt);
  RightMiddle.spin(forward, right, volt);
  RightFront.spin(forward, right, volt);
  LeftFront.spin(forward, left, volt);
  RightBack.spin(forward, right, volt);
  LeftBack.spin(forward, left, volt);
  wait(wt, msec);
}
void liftBreak(){
	DR4BR.stop(brake);
	DR4BL.stop(brake);
}

void liftUP(){
	DR4BR.spin(forward, -60, pct);
	DR4BL.spin(forward, -60, pct);
}

void liftDOWN(int speed, int wt){
	DR4BR.spin(forward, speed, pct);
	DR4BL.spin(forward, speed, pct);
	wait(wt, msec);
}

void liftDOWN(int speed){
	DR4BR.spin(forward, speed, pct);
	DR4BL.spin(forward, speed, pct);

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
	clawP.set(!clawP.value());
}






// void setupClaw() {
//     // Only allow setup once per match/session
//     if (!isSetupDone) {
//         clawMotor.setVelocity(40, percent);
//         // Pull in from pre-match wide position
//         clawMotor.spinFor(forward, SETUP_DEGREES, degrees, true);
        
//         // Zero out the encoder so all future target angles are relative to this point
//         clawMotor.resetPosition();
//         clawMotor.setStopping(hold);
        
//         isSetupDone = true;
//         isGripping = false;
//     }
// }

// void toggleClaw() {
//     // Prevent toggling before the ready position has been established
//     if (!isSetupDone) return;

//     clawMotor.setVelocity(70, percent);

//     if (isGripping) {
//         // Open to the ready width
//         clawMotor.spinToPosition(OPEN_DEGREES, degrees, false);
//         isGripping = false;
//     } else {
//         // Clamp down to target grip width
//         clawMotor.spinToPosition(GRIP_DEGREES, degrees, false);
//         isGripping = true;
//     }
// }



// ============================================================================
// STEP 2: AUTON SELECTOR FUNCTIONS
// ============================================================================
void drawAutonButtons() {
  const char* autonNames[] = {"RED 1", "RED 2", "BLUE 1", "BLUE 2", "SKILLS"};
  Brain.Screen.setFont(fontType::mono20);

  for (int i = 0; i < 5; i++) {
    int y = 5 + (i * 46); // 42px height + 4px vertical gap

    // Highlight selected button green, others gray
    if (selectedAuton == (i + 1)) {
      Brain.Screen.setPenColor(color::white);
      Brain.Screen.setFillColor(color::green);
    } else {
      Brain.Screen.setPenColor(color::white);
      Brain.Screen.setFillColor(color(60, 60, 60));
    }

    Brain.Screen.drawRectangle(5, y, 225, 42);
    Brain.Screen.setPenColor(color::white);
    Brain.Screen.printAt(20, y + 28, autonNames[i]);
  }
}

void checkAutonTouch() {
  if (Brain.Screen.pressing()) {
    int touchX = Brain.Screen.xPosition();
    int touchY = Brain.Screen.yPosition();

    // Check if the press occurred inside the left column
    if (touchX >= 5 && touchX <= 230) {
      for (int i = 0; i < 5; i++) {
        int buttonY = 5 + (i * 46);

        if (touchY >= buttonY && touchY <= (buttonY + 42)) {
          selectedAuton = i + 1;
          drawAutonButtons(); // Update visual state
          wait(250, msec);    // Debounce to prevent multiple triggers
        }
      }
    }
  }
}

// ============================================================================
// STEP 3: MOTOR MONITOR FUNCTIONS
// ============================================================================
color getTempColor(double tempC) {
  if (tempC <= 30.0) {
    return color::green;
  } else if (tempC < 40.0) {
    return color::yellow;
  } else {
    return color::red;
  }
}

// Helper function to draw an individual motor box with Temp AND Current (Amps)
void drawMotorBox(const char* name, motor &m, int x, int y, int width, int height) {
  Brain.Screen.setPenColor(color::black);

  if (!m.installed()) {
    // Motor disconnected: White box with clear warning
    Brain.Screen.setFillColor(color::white);
    Brain.Screen.drawRectangle(x, y, width, height);

    Brain.Screen.setFont(fontType::mono15);
    Brain.Screen.printAt(x + 6, y + 20, name);
    Brain.Screen.setFont(fontType::mono12);
    Brain.Screen.printAt(x + 6, y + 40, "UNPLUGGED");
  } else {
    // Read live data from the smart motor
    double tempC = m.temperature(temperatureUnits::celsius);
    double amps  = m.current(currentUnits::amp);

    // Color code the background by temperature
    Brain.Screen.setFillColor(getTempColor(tempC));
    Brain.Screen.drawRectangle(x, y, width, height);

    // Line 1: Motor Name
    Brain.Screen.setFont(fontType::mono15);
    Brain.Screen.printAt(x + 6, y + 20, name);

    // Line 2: Temperature & Current (e.g., "31C 1.8A")
    // Using mono12 ensures both fit neatly across the 115px box width
    Brain.Screen.setFont(fontType::mono12);
    Brain.Screen.printAt(x + 6, y + 42, "%.0fC | %.1fA", tempC, amps);
  }
}

void updateMotorMonitor() {
  int col1X = 240;
  int col2X = 360;
  int boxW  = 115;
  int boxH  = 52;

  // Row 1
  drawMotorBox("Left F",  LeftFront,  col1X, 5,   boxW, boxH);
  drawMotorBox("Right F", RightFront, col2X, 5,   boxW, boxH);

  // Row 2
  drawMotorBox("Left M",  LeftMiddle,  col1X, 63,  boxW, boxH);
  drawMotorBox("Right M", RightMiddle, col2X, 63,  boxW, boxH);

  // Row 3
  drawMotorBox("Left B",  LeftBack,  col1X, 121, boxW, boxH);
  drawMotorBox("Right B", RightBack, col2X, 121, boxW, boxH);

  // Row 4
//   drawMotorBox("Claw",  clawMotor, col1X, 179, boxW, boxH);
  //drawMotorBox("Arm",     Arm,    col2X, 179, boxW, boxH);
}




// double YOFFSET = 20; //offset for the display
// //Writes a line for the diagnostics of a motor on the Brain
// void MotorDisplay(double y, double curr, double temp)
// {
// 	Brain.Screen.setFillColor(transparent);
// 	Brain.Screen.printAt(5, YOFFSET + y, "Current: %.1fA", curr); 
	
// 	if (curr < 1){
// 		Brain.Screen.setFillColor(green);
// 	} else if(curr >= 1 && curr  <= 2.5) {
// 		Brain.Screen.setFillColor(yellow);
// 	} else {
// 		Brain.Screen.setFillColor(red);
// 		Brain.Screen.drawRectangle(140, YOFFSET + y - 15, 15, 15);
// 	}
																								   
	
// 	Brain.Screen.setFillColor(transparent);
// 	Brain.Screen.printAt(160, YOFFSET + y, "Temp: %.1fC", temp);  
	
// 	if (temp < 45){
// 		Brain.Screen.setFillColor(green);
// 	} else if(temp <= 50 && temp  >= 45){
// 		// TRUE and TRUE --> True
// 		// TRUE and FALSE --> False
// 		// FALSE and FALSE --> False
// 		Brain.Screen.setFillColor(yellow);
// 	} else {
// 		Brain.Screen.setFillColor(red);
// 		Brain.Screen.drawRectangle(275, YOFFSET + y - 15, 15, 15);
// 		Brain.Screen.setFillColor(transparent);
// 	}
// }


// //Displays information on the brain
// void Display()
// {
// 	double leftFrontCurr = LeftFront.current(amp);
// 	double leftFrontTemp = LeftFront.temperature(celsius);
// 	double leftBackCurr = LeftBack.current(amp);
// 	double leftBackTemp = LeftBack.temperature(celsius);
// 	double rightFrontCurr = RightFront.current(amp);
// 	double rightFrontTemp = RightFront.temperature(celsius);
// 	double rightBackCurr = RightBack.current(amp);
// 	double rightBackTemp = RightBack.temperature(celsius);
//   	double rightMiddleCurr = RightMiddle.temperature(celsius);
//   	double rightMiddleTemp = RightMiddle.current(amp);
//   	double leftMiddleCurr = LeftMiddle.temperature(celsius);
//   	double leftMiddleTemp = LeftMiddle.current(amp);


// 	if (LeftFront.installed()){
// 		MotorDisplay(1, leftFrontCurr, leftFrontTemp);
// 		Brain.Screen.printAt(300, YOFFSET + 1, "LeftFront");
// 	} else {
// 		Brain.Screen.printAt(5, YOFFSET + 1, "LeftFront Problem");
// 	}
	
// 	if (LeftBack.installed()){
// 		MotorDisplay(31, leftBackCurr, leftBackTemp);
// 		Brain.Screen.printAt(300, YOFFSET + 31, "LeftBack");
// 	} else {
// 		Brain.Screen.printAt(5, YOFFSET + 31, "LeftBack Problem");
// 	}

// 	if (RightFront.installed()) {
// 		MotorDisplay(61, rightFrontCurr, rightFrontTemp);
// 		Brain.Screen.printAt(300, YOFFSET + 61, "RightFront");
// 	} else {
// 		Brain.Screen.printAt(5, YOFFSET + 61, "RightFront Problem");
// 	}
	
// 	if (RightBack.installed()) {
// 		MotorDisplay(91, rightBackCurr, rightBackTemp);
// 		Brain.Screen.printAt(300, YOFFSET + 91, "RightBack");
// 	} else {
// 		Brain.Screen.printAt(5, YOFFSET + 91, "RightBack Problem");
// 	}

// 	if (LeftMiddle.installed()) {
// 		MotorDisplay(121, leftMiddleCurr, leftMiddleTemp);
// 		Brain.Screen.printAt(300, YOFFSET + 121, "LeftMiddle");
// 	} else {
// 		Brain.Screen.printAt(5, YOFFSET + 121, "LeftMiddle Problem");
// 	}

// 	if (RightMiddle.installed()) {
// 		MotorDisplay(151, rightMiddleCurr, rightMiddleTemp);
// 		Brain.Screen.printAt(300, YOFFSET + 151, "RightMiddle");
// 	} else {
// 		Brain.Screen.printAt(5, YOFFSET + 151, "RightMiddle Problem");
// 	}
// }

void selectAuton() {
		bool selectingAuton = true;
		
		int x = Brain.Screen.xPosition(); // get the x position of last touch of the screen
		int y = Brain.Screen.yPosition(); // get the y position of last touch of the screen
		
		// check to see if buttons were pressed
		if (x >= 20 && x <= 120 && y >= 50 && y <= 150){ // select button pressed
				selectedAuton++;
				if (selectedAuton > AutonMax){
						selectedAuton = AutonMin; // rollover
				}
				if(selectedAuton == 0){
					Brain.Screen.printAt(1, 200, "Auton Selected =  %d   Left Side", selectedAuton);
				}
				if(selectedAuton == 1){
					Brain.Screen.printAt(1, 200, "Auton Selected =  %d   Right Side", selectedAuton);
				}
				if(selectedAuton == 2){
					Brain.Screen.printAt(1, 200, "Auton Selected =  %d   1 min auton", selectedAuton);
				}																																													   
				
		}
		
		
		if (x >= 170 && x <= 270 && y >= 50 && y <= 150) {
				selectingAuton = false; // GO button pressed
				Brain.Screen.printAt(1, 200, "Auton  =  %d   GO           ", selectedAuton);
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
	Brain.Screen.printAt(1, 200, "Auton Selected =  %d   ", selectedAuton
	);

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
	double kP = 5;
	double speed = kP * error;
	double accuracy = 0.5;
	LeftFront.setPosition(0.0, rev);

	while(fabs(error) >= accuracy){
		drive(speed, speed, 10);
		position = LeftFront.position(rev)*3.14*3.25*1.8;
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
	double accuracy = 0.5;
	gyroturn.setRotation(0.0, deg);

	while(fabs(error) >= accuracy){
		drive(speed, -speed, 10);
		rotation = gyroturn.rotation(deg);
		error = target - rotation;
		speed = kP * error;
	}
	driveBrake();
	
}

int sign(float a ){
	return (a < 0 ? -1 : 1);
}

void turnPID(double target){
	double error = target - gyroturn.rotation(deg);
	double kP = 0.8;
	double kI = 0.03;
	double kD = 0;//1.5;
	double preverror = error;
	double totalerror = 0;
	double accuracy = 0.5;
	gyroturn.setRotation(0.0, deg);

	int debugCounter = 0;
	while(fabs(error) >= accuracy){
		error = target - gyroturn.rotation(deg);
		// while (fabs(error) > 180) {
		// 	error -= 360 * sign(error);
		// }
		
		if (fabs(error) < 20) {
			totalerror +=error;
		}else if ((error > 0 && preverror < 0) || (error < 0 && preverror > 0)){
			totalerror = 0;
		}

		double speed = (kP * error ) + (kI * totalerror ) + kD * (error - preverror);
		if (debugCounter % 10 == -1) {
			printf("error=%.2f gyro=%.2f speed=%.2f p=%.2f i=%.2f d=%.2f actual speed=%.2f\n", error, gyroturn.rotation(deg), speed, kP*error, kI*totalerror, kD*(error - preverror), gyroturn.gyroRate(zaxis, dps));			
		}
		
		drive(speed, -speed, 10);
		
	}
	driveBrake();
	error = target - gyroturn.rotation(deg);
	printf("error=%.2f gyro=%.2f actual speed=%.2f\n", error, gyroturn.rotation(deg), gyroturn.gyroRate(zaxis, dps));			
	
}


// ============================================================================
// STEP 4: COMPETITION THREADS
// ============================================================================
void pre_auton(void) {
  Brain.Screen.clearScreen();
  selectAuton();
  drawGUI();

  // Run selection and health monitor until the match starts
//   while (!Competition.isEnabled()) {
//     checkAutonTouch();
//     updateMotorMonitor();
//     wait(50, msec);
//   }
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
	switch (selectedAuton) {
		case 0:
			//code 0: 15sec LEFT SIDE

			Brain.Screen.printAt(1, 220, "auton 0 is running");
			// //clamp();
			// clawMotor.setStopping(hold);
			// clawMotor.spin(fwd, 70, pct);
			Brain.Screen.clearScreen();

			//toggle
			drive(-100, -100, 250);
			driveBrake();
			drive(100, 100, 250);
			driveBrake();
			wait(500, msec);
			drive(-100, -100, 250);
			driveBrake();
			drive(100, 100, 250);
			driveBrake();
			wait(1000, msec);
			drive(-100, -100, 250);
			driveBrake();
			drive(100, 100, 250);
			driveBrake();
			//drive to alliance goal
			liftUP();
			wait(500, msec);
			liftBreak();
			inchDrive(9);
			driveBrake();
			turnPID(-90);
			driveBrake();
			inchDrive(15);
			driveBrake();
			//place down pre-load(yellow facing up)
			liftDOWN(45, 500);
			clamp();
			//TESTING
			//drive to yellow pin and cup
			wait(250, msec);
			inchDrive(-15);
			driveBrake();
			turnPID(45);
			driveBrake();
			inchDrive(22);
			//grab
			driveBrake();
			clamp();
			liftUP();
			wait(500, msec);
			liftBreak();
			driveBrake();
			//drive to neutral goal
			inchDrive(-22);
			driveBrake();
			turnPID(125);
			driveBrake();
			inchDrive(17);
			driveBrake();
			//place down cup+pin
			liftDOWN(45, 500);
			clamp();
			break;
		
		case 1:
			//code 1: 15sec RIGHT SIDE

			//toggle
			drive(-100, -100, 250);
			driveBrake();
			drive(100, 100, 250);
			driveBrake();
			wait(1000, msec);
			drive(-100, -100, 500);
			driveBrake();
			drive(100, 100, 250);
			driveBrake();
			//drive to alliance goal
			liftUP();
			wait(500, msec);
			liftBreak();
			inchDrive(14);
			driveBrake();
			turn(77);
			driveBrake();
			inchDrive(15);
			driveBrake();
			//place down pre-load(yellow facing up)
			liftDOWN(45, 500);
			clamp();
			//TESTING
			//drive to yellow pin and cup
			wait(250, msec);
			inchDrive(-15);
			driveBrake();
			turnPID(45);
			driveBrake();
			inchDrive(34);
			//grab
			driveBrake();
			clamp();
			liftUP();
			wait(500, msec);
			driveBrake();
			//drive to neutral goal
			inchDrive(-34);
			driveBrake();
			turnPID(-135);
			driveBrake();
			inchDrive(15);
			driveBrake();
			//place down cup+pin
			liftDOWN(45, 500);
			wait(500, msec);
			liftBreak();
			clamp();
			break;

		case 2: 
			//code 2: SINGLE ROBOT AUTON - Both side
			Brain.Screen.printAt(1, 220, "auton 2 is running");
			// liftUP();
			// wait(1000, msec);
			// liftBreak();
			// inchDrive(5);
			// liftDOWN();
			// wait(1000, msec);
			// liftBreak();
			//clamp();
			break;
		
		case 3:
			//code 3: 1 min AUTON SKILLS
			Brain.Screen.printAt(1, 220, "auton 3 is running");
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
 // Brain.Screen.clearScreen(); 
	int lspeed = 0;
	int rspeed = 0;
	// clawMotor.setStopping(hold);

    // Register single-press button callbacks
    Controller.ButtonA.pressed(clamp);
    // Controller.ButtonL1.pressed(toggleClaw);

  while (1) {
	  updateMotorMonitor();
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.
	lspeed = Controller.Axis3.position(pct);
	rspeed = Controller.Axis2.position(pct);
	drive(lspeed, rspeed, 30);

	
	if (Controller.ButtonR2.pressing()){
		liftDOWN(45);
	}else if (Controller.ButtonR1.pressing()){
		liftUP();
	}else{
		DR4BR.stop(brake);
		DR4BL.stop(brake);
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
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																			   