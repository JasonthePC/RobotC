#pragma config(Sensor, in1,    firstjointsensor, sensorPotentiometer)
#pragma config(Motor,  port2,           basemotor,     tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           firstjointmotor, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           secondjointmotor, tmotorVex393_MC29, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/*
Description:
Proportional Integral Derivative control for robotic arm joints.
Built to function with potentiometers and VEX 393 motors.
*/

#define PID_DRIVE_MAX       127
#define PID_DRIVE_MIN     (-127)
#define PID_INTEGRAL_LIMIT  50

//defines how quickly the microcontroller will update values
float refreshrate = 60.0;
int period = (1/refreshrate) * 1000; //computes period in milliseconds

//computed with Zeigler-Nichols method
float Ku = 40.0;
float Pu = 1.7;
//PID constants
float Kc = Ku/1.7;
float Ti = Pu/2.0;
float Td = Pu/8;

//temporary variable for setting the target value through the debugger
int targetValue = 10;

//defines the jointData struct, the data structure that holds information about a joint
typedef struct
{
	int errorIntegral; //used exclusively for PID (the sum of error over a period of time)
	int prevError; //the error between the target and the sensor the pervious time it was checked
	int curError; //the error between the target and the sensor
	tMotor motorname; //the name of a joint's motor
	tSensors sensorname; // the name of a joint's potentiometer
	int target; //the target potentiometer value
}jointData;

//updates the motors of a joint using its data and
void updateMotorPower(struct jointData*p_input, int power){
	setMotor(p_input->motorname, power);
}

//puts the old current error value into the previous error value and calculates the new error
void errorCalculator(struct jointData*p_input)
{
	p_input->prevError = p_input->curError;
	p_input->curError = p_input->target - (SensorValue[p_input->sensorname]/30);

}

//gives starting values to a jointData struct
void jointDataSetup(struct jointData*p_input, tSensors insensorname, tMotor inmotorname)
{
	p_input->sensorname = insensorname;
	p_input->motorname = inmotorname;

	p_input->errorIntegral = 0;
	p_input->curError = 0;
	p_input->prevError = 0;
}

//the PID algorithm
int PID(struct jointData*p_input)
{

	//integrates the error if the jont is within the integral range
  if( abs(p_input->curError) < PID_INTEGRAL_LIMIT )
  	p_input->errorIntegral += (p_input->curError*period)/1000;
  else
  	p_input->errorIntegral = 0;

  int derivative = (1000*(p_input->curError - p_input->prevError)/period);

  //calculates the suggest power
	int suggestedPower = Kc*(p_input->curError + (1/Ti * p_input->errorIntegral) + (Td * derivative));

	//makes sure power is with in [-127, 127]
	if(suggestedPower > PID_DRIVE_MAX)
		suggestedPower = PID_DRIVE_MAX;
	if(suggestedPower < PID_DRIVE_MIN)
		suggestedPower = PID_DRIVE_MIN;

  return suggestedPower;
}

//takes the global targetValue variable and puts it into the joint data
//can be later used to take data from VEX remote
void updateTarget(struct jointData*p_input){
	p_input->target = targetValue;
}


task main()
{
	//declare and initialize jointData structs
	struct jointData firstJoint;
	jointDataSetup(&firstJoint, firstjointsensor, firstjointmotor);

	//update values continuously
	while(true){

	  clearTimer(T1); //resets timer

		//move the first joint
		updateTarget(&firstJoint);
		errorCalculator(&firstJoint);
		updateMotorPower( &firstJoint, PID(&firstJoint) );

		//makes sure the microcontroller updates the joints at the specified refresh rate
		while(time1[T1]<period){}
 }//close while
}//close task main()
