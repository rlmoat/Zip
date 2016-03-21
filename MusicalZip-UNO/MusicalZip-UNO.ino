/***********
 MUSICAL ZIP-UNO
 Plays a pitch that changes based on a variable resistor zip input!
 ***********/

 /*Borrows from   http://www.arduino.cc/en/Tutorial/Smoothing 
 by David A. Mellis & Tom Igoe
 */

/* Define the number of samples to track.
High numbers = smoother reading but will take longer to respond to input!
Use a constant rather than a normal variable to allow this
value to determine the size of the readings array.
*/


// Zip Constants - set for v1 - will need calibration!


const int maximumReading = 890; // pick a value slightly lower than the true max to calm!
const int minimumReading = 280; // minimumReading is for the last stable value
const int minimumSwoop = 20; // lower value gives comic sweep at the top!

const int minimumPitch = 440; // 440Hz == A4
const int maximumPitch = 880; // 880Hz == A5


const int numReadings = 20;
const int numAvgReadings = 20;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int lastAverage[numReadings];  // the last readings from the analog input
int avgReadIndex = 0;
int avgTotal = 0;
int avgAvg = 0;               // the average of the average array

int movementThreshold = 4;

bool calibration = true;
int prelimCycles = 0; // keeps track of inital sensor readings during calibration
const int calibrationLength = 30; // 30 is an arbitary number for sensor calming!

const int zipPin = A0;
const int speakerPin = 10;
const int GND_emul = 2; // emulates ground for the voltage divider

void setup() {
  
  pinMode(zipPin, INPUT);
  pinMode(speakerPin, OUTPUT);
  // initialize serial communications (for debugging only):
  Serial.begin(9600);
  arraySet(); // set length of readings[] and avgAvg[]
  calibrate();
 
}

void loop() {

  smoothing();
  averageChecker();
  dataSpewer();
  if (zipMovementDetected()) {
    zippyMusic();
  }
 delay(15);
}

void smoothing()
{
  total = total - readings[readIndex];
  // power saving trick:
  pinMode(GND_emul, OUTPUT);
  // emulate GND:
  digitalWrite(GND_emul, LOW); 
  // read from the sensor:
  readings[readIndex] = analogRead(zipPin);
  // disconnect the emulated GND:
  pinMode(GND_emul, INPUT);
  
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex++;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;


}

void zippyMusic(){
  if ((average < maximumReading) && (average > minimumSwoop)) {
    // if the zip is between min and max (note use of minimumSwoop not minimumPitch
    // for comic effect!)

    // map the analog input range
    // to the output pitch range (440-880Hz)
    int thisPitch = map(average, minimumSwoop, maximumReading, maximumPitch, minimumPitch);

    // play the pitch:
    tone(speakerPin, thisPitch, 20); // (pinNo, frequency, duration)
    delay(1);        // delay in between reads for stability
  }
}

void averageChecker() {
  avgTotal = avgTotal - lastAverage[avgReadIndex];
  // read from the sensor:
  lastAverage[avgReadIndex] = average;
  // add the reading to the total:
  avgTotal = avgTotal + lastAverage[avgReadIndex];
  // advance to the next position in the array:
  avgReadIndex++;

  // if we're at the end of the array...
  if (avgReadIndex >= numAvgReadings) {
    // ...wrap around to the beginning:
    avgReadIndex = 0;
    if (calibration) prelimCycles++;
  }

  // calculate the average:
  avgAvg = avgTotal / numAvgReadings;
}

void dataSpewer(){
  Serial.print(average);
  Serial.print("\t");
  Serial.print(avgAvg);
  Serial.print("\t");
  Serial.println(zipMovementDetected());
}

int zipMovementDetected() {
  int zipMovement;
  if ((abs(average-avgAvg)) > movementThreshold) {
    zipMovement = 1;
  }
  else zipMovement = 0;

  return zipMovement;
}

void calibrate() {
  delay(100);
  while (calibration && prelimCycles < calibrationLength) { 
    smoothing();
    averageChecker();
  }
  calibration = false;
  Serial.println("calibration complete!");
}

void arraySet() {
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  for (int thisReading = 0; thisReading < numAvgReadings; thisReading++) {
    lastAverage[thisReading] = 0;
  }
}


