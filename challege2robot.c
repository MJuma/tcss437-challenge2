
#pragma config(Sensor, S1,     LeftColor,      sensorEV3_Color, modeEV3Color_Reflected)
#pragma config(Sensor, S3,     Sonar,          sensorEV3_Ultrasonic)
#pragma config(Sensor, S4,     RightColor,     sensorEV3_Color, modeEV3Color_Reflected)
#pragma config(Motor,  motorB,          LeftServo,     tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorC,          RightServo,    tmotorEV3_Large, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#define WANDER_SPEED 50

int RobotState = 0;

/*
* The main wander function.
* This function will implement the
* random biased walk.
*/
void wander() {
	clearTimer(T1);

	int num = random(10) - 5; //num from -5 to 5
	int length = random(100)+150; //num from 150-250;

	while(true) {
		if(RobotState == 0) {
			if (time1[T1] > length) {
				length = random(200)+100;
				num = random(10) - 5;
				clearTimer(T1);
			}

			if (num < 0) {
				setMotorSpeed(LeftServo, WANDER_SPEED + (num*5));
				setMotorSpeed(RightServo, WANDER_SPEED);
			} else {
				setMotorSpeed(LeftServo, WANDER_SPEED);
				setMotorSpeed(RightServo, WANDER_SPEED - (num*5));
			}
			sleep(10);
		} else {
			sleep(10);
		}
	}
}

void populate (int *buffer, int pointer, int color) {
	buffer[pointer] = color;
}

int average(int *buffer, int size) {
	int i = 0, avg;
	for (i = 0; i < size; i++) {
		avg += buffer[i];
	}
	avg /= size;
	return avg;
}

task LineFollow() {
	int nMotorSpeedSetting = 20;
	float nPfactor = 1;
	int lowest = 0; //Black
	int highest = 60; //White
	int grey = (highest - lowest) / 2;
	float error = 0, oldError = 25;
	int size = 0;
	int pointer = 0;
	int frontColorBuffer[10];
	int backColorBuffer[10];

	while(true)	{
		int rightColor = SensorValue[RightColor];
		int leftColor = SensorValue[LeftColor];
		displayCenteredBigTextLine(7, "r light: %d", rightColor);
		displayCenteredBigTextLine(10, "l light: %d", leftColor);
		switch(RobotState) {
		case 0: //Wandering - Try to detect a line
			if(grey / 2 > rightColor) {
				RobotState = 1;
			}
			break;
		case 1: //Found Line - Follow it
			oldError = error;
			populate(frontColorBuffer, pointer, rightColor);
			populate(backColorBuffer, pointer, leftColor);
			if (grey / 2 > average(frontColorBuffer, size)
				&& grey / 2 > average(backColorBuffer, size)) {
				RobotState = 0;
			}
			pointer = (pointer + 1) % size;
			error = SensorValue[RightColor] - grey;
			// approach grey,
//			float expFunction = nPfactor * (1 - error/grey) * error; // approaching grey
//			error = round(expFunction);

			displayCenteredBigTextLine(1, "r light: %f", error);

			motor[LeftServo] = nMotorSpeedSetting - (error * nPfactor);
			motor[RightServo] = nMotorSpeedSetting + (error * nPfactor);
			sleep(10);
			break;
		default:
			break;
		}
	}

}

void runAway() {
	//stop
	setMotorSpeed(LeftServo, 0);
	setMotorSpeed(RightServo, 0);
	sleep(2500);

	//reverse
	setMotorSpeed(LeftServo, -WANDER_SPEED);
	setMotorSpeed(RightServo, -WANDER_SPEED);
	sleep(1000);

	//turn around
	setMotorSpeed(LeftServo, -WANDER_SPEED);
	setMotorSpeed(RightServo, WANDER_SPEED);
	sleep(1000);

	//resume wandering
	setMotorSpeed(LeftServo, 0);
	setMotorSpeed(RightServo, 0);
	RobotState = 0;
}

task SonarDetect() {
	int distance = 0;
	while(true)	{
		distance = SensorValue[Sonar];
		displayCenteredBigTextLine(4, "Dist: %3d cm", distance);
		if(RobotState == 2) { //Found object - approach it
			if (distance <= 96) {
				setMotorSpeed(LeftServo, distance);
				setMotorSpeed(RightServo, distance);
				if(distance < 15) {
					runAway();
				}
			} else {
				RobotState = 0;
			}
		} else { //Try to detect an object
			if (distance <= 96) {
				RobotState = 2;
			}
		}
	}
}

//Runs the robot
task main() {
	setSoundVolume(100);
	srand(random(100000));
	startTask(LineFollow);
	startTask(SonarDetect);
	wander();
}
