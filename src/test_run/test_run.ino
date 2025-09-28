#include "Arduino_BMI270_BMM150.h"
#include <Arduino_LPS22HB.h> 
#include <cmath>
#include <math.h>
#include <imuFilter.h>

const int COEFFICIENT_LENGTH = 150;

float unfilteredData[COEFFICIENT_LENGTH]; // stores data using ring buffer
float filteredData[COEFFICIENT_LENGTH];
float times[COEFFICIENT_LENGTH]; // stores data using ring buffer
int currentIndex = 0; // keeps track of index
bool filled = false; // keeps track of whether the array is full of data or not

// necessary for ensuring a specific ssample rate
float samplePeriod = 16.667; // ~ 60 Hz to match filter we made
float previous = 0;
float current = 0;

// just for double checking after the fact
float averageSamplePeriod = 0;
int totalSamples = 0; 

float coefficients[COEFFICIENT_LENGTH] = {0.0001413449037592738, 0.00020398015454728578, 0.000273317013217923, 0.0003479703924559791, 0.0004261177806571362, 0.0005052984210727668, 0.0005822531788412448, 0.0006528328313434523, 0.0007119982374842783, 0.0007539291417544126, 0.0007722496594072448, 0.0007603683543208318, 0.0007119199843026495, 0.0006212852647827851, 0.000484155242877159, 0.0002980989085702341, 6.308724364405459e-05, -0.0002180753733830064, -0.000539460212621155, -0.00089190647710339, -0.001263130924691315, -0.0016380768973192933, -0.0019995100471737175, -0.0023288513344082357, -0.0026072202233439526, -0.002816643555050245, -0.0029413695160413014, -0.0029692126337230915, -0.0028928458954621907, -0.0027109508286505173, -0.0024291363772493257, -0.0020605430573722235, -0.0016260602293836348, -0.0011541010916827983, -0.0006799015325161054, -0.0002443342889722146, 0.00010774231442769872, 0.00032955293912992237, 0.00037481480559270345, 0.00020038608198860326, -0.0002309709780791961, -0.0009478525964152769, -0.001967651783524058, -0.0032943243425750763, -0.004916628112336124, -0.006806966225376657, -0.008920942365534791, -0.011197706026156296, -0.013561129868206904, -0.01592182111082454, -0.018179926419067446, -0.02022864715586606, -0.021958341403778208, -0.02326105307585837, -0.024035278810497798, -0.024190761997464085, -0.023653091646983253, -0.022367882856757466, -0.0203043257842839, -0.017457911151439392, -0.013852171671999248, -0.009539319121050797, -0.004599704277306177, 0.0008599205334175753, 0.006709301591078135, 0.012798657168357189, 0.018963582879562534, 0.02503070149603765, 0.030823831493030248, 0.03617042036578756, 0.040907972086887706, 0.044890193955033915, 0.047992596866382846, 0.05011730443297992, 0.051196859450023974, 0.051196859450023974, 0.05011730443297992, 0.047992596866382846, 0.044890193955033915, 0.040907972086887706, 0.03617042036578756, 0.030823831493030248, 0.02503070149603765, 0.01896358287956253, 0.012798657168357189, 0.006709301591078135, 0.0008599205334175752, -0.004599704277306177, -0.009539319121050797, -0.013852171671999248, -0.017457911151439392, -0.0203043257842839, -0.022367882856757466, -0.023653091646983253, -0.024190761997464085, -0.02403527881049779, -0.02326105307585837, -0.021958341403778205, -0.02022864715586606, -0.018179926419067446, -0.015921821110824536, -0.0135611298682069, -0.011197706026156296, -0.008920942365534791, -0.006806966225376655, -0.004916628112336124, -0.003294324342575076, -0.0019676517835240576, -0.0009478525964152771, -0.00023097097807919605, 0.00020038608198860318, 0.0003748148055927035, 0.0003295529391299223, 0.00010774231442769867, -0.00024433428897221467, -0.0006799015325161054, -0.001154101091682798, -0.0016260602293836338, -0.0020605430573722235, -0.0024291363772493244, -0.0027109508286505147, -0.0028928458954621907, -0.0029692126337230915, -0.0029413695160412975, -0.002816643555050245, -0.0026072202233439526, -0.002328851334408234, -0.0019995100471737175, -0.0016380768973192933, -0.0012631309246913139, -0.00089190647710339, -0.000539460212621155, -0.00021807537338300632, 6.308724364405459e-05, 0.0002980989085702341, 0.00048415524287715865, 0.0006212852647827851, 0.0007119199843026495, 0.0007603683543208311, 0.0007722496594072448, 0.0007539291417544126, 0.0007119982374842779, 0.0006528328313434512, 0.0005822531788412448, 0.0005052984210727664, 0.00042611778065713594, 0.0003479703924559791, 0.000273317013217923, 0.00020398015454728578, 0.0001413449037592738};

// all variables regarding detecting steps
float risingTime = 0;
float fallingTime = 0;
float previousRisingTime = 0;
float previousStepTime = 0;

float currentAmplitude = 0;
float previousAmplitude = 0;
float minValue = 0;
float maxValue = 0;

// for cadence measurements

struct Step {
  float spm; // steps per minute
  float stepTime; // at what time did it occur in ms after program's been running
  bool isRunning; // true if running step
  bool isWalking; // true if walking step
  // both false if neither (since we only count it as runnning/walking at floor contact)
};

imuFilter filter;
// 
int stepLength = 10;
int currentStepIndex;
bool filledSteps;
Step steps[10];

void setup() {
  Serial.begin(9600);
  while (!Serial); // waits

  filter.begin(1.0/60.0, 0.1f);

  if (!IMU.begin()){ // from arduino doc, ensures IMU initializes
    Serial.println("Failued to initialize IMU");
    while (1);
  }

  if (!BARO.begin()){
    while (1);
  }
}

double FIRFilter(float magnitudeAcceleration[], int currentIndex, float coefficients[], bool filled, int coefficient_length){
  double output = 0;

  for (int i = 0; i < coefficient_length; i++){
    output += magnitudeAcceleration[currentIndex] * coefficients[i];

    //if (isinf(output)){
    //  //Serial.print("Infinity at: ");
    //  //Serial.println(i);
   // }

    if (currentIndex == 0){
      if (!filled){
        break;
      }

      currentIndex = coefficient_length - 1;
    }

    currentIndex --;
  }

  
  return output;
}

bool rising(float current, float previous, float tolerance=0.0001){
  //Serial.println(current - previous);
  //Serial.println(((current - previous) > tolerance));
  return ((current - previous) > tolerance);
}

bool falling(float current, float previous, float tolerance = 0.001){
  
  return ((previous - current) > tolerance);
}

float spm(float currentStepTime, float previousStepTime){
  float stepPeriod = (currentStepTime - previousStepTime)/1000;
  
  return 120/stepPeriod; // 60/stepPeriod * 2 (times two since this is calculated from only one foot)
}

bool walking(float currentAmplitude, float previousAmplitude, float stepsPerMinute, float lowerAmp = 0.50, float higherAmp = 0.75, float lowerSPM=50, float higherSPM=120){
  // since walking is typically a pattern of higher then lower peaks we can detect it by simply comparing
  // the current peak to the previous peak ignoring the second peak
  return ((currentAmplitude * 0.5 < previousAmplitude) && (currentAmplitude * 0.75 > previousAmplitude) && (stepsPerMinute > lowerSPM) && (stepsPerMinute < higherSPM));
}

bool running(float currentAmplitude, float previousAmplitude, float stepsPerMinute, float tolerance=0.20, float lowerSPM = 140, float higherSPM = 220){
  return (((fabs(currentAmplitude - previousAmplitude)/currentAmplitude) < tolerance) && (stepsPerMinute > lowerSPM) && (stepsPerMinute < higherSPM));
}

Step detect_step(float accelerationData[], float times[], int currentIndex, int samples, float &risingTime, float &fallingTime, float &previousRisingTime, float &previousStepTime, float samplePeriod, float &currentAmplitude, float &previousAmplitude, float &minValue, float &maxValue){
  int previousIndex = currentIndex - 1;

  if (previousIndex == -1){
    previousIndex = samples - 1; // last index of acceleration data, assumes its been filled completely 
  }

  float currentMagnitude = accelerationData[currentIndex];
  float previousMagnitude = accelerationData[previousIndex];
  
  float currentTime = times[currentIndex];
  float previousTime = times[previousIndex];
  float stepTime = currentTime - previousTime;

  // tracks if only comparing the current and previous data point
  // we are falling, may be susceptible to very sudden spike
  // therefore further checks are added to make sure its "actually"
  // falling rather than a small bump
  bool currentlyRising = rising(currentMagnitude, previousMagnitude);
  bool currentlyFalling = falling(currentMagnitude, previousMagnitude);

  if (currentlyRising){
    risingTime += stepTime;
  } else if (currentlyFalling){
    fallingTime += stepTime;
  }

  // if we've been rising for more than 3 samples, AND we're still currently rising
  // then it can safely be assumed acceleration is heading upwards on a curve
  // and isn't just a small spike 
  if (risingTime >= (samplePeriod * 3) && currentlyRising){
    maxValue = currentMagnitude;

    // we're transitioning from falling to rising (end of a step)
    if (fallingTime >= (samplePeriod * 3)) {
    // checks if current amplitude is between 50% and 75% of previous amplitude
    // which is in par with walking (a high peak then slightly lower peak)
      float runningStepTime = fallingTime + previousRisingTime;
      float walkingStepTime = runningStepTime + previousStepTime;

      float walkingSPM = (120000/walkingStepTime); // 60s -> 60,000 ms, multiplied by 2 because steps account for both feet
      float runningSPM = (120000/runningStepTime); // same logic

      //Serial.print("Walking SPM: ");
      //Serial.println(walkingSPM);
      //Serial.print("Running SPM: ");
      //Serial.println(runningSPM);
      
      fallingTime = 0;
      previousStepTime = runningStepTime;

      
      if (walking(currentAmplitude, previousAmplitude, walkingSPM)){ // change to detect higher peak
        Serial.println("walking");
        Step walkingStep = {walkingSPM, currentTime, false, true};
        return walkingStep;
      } else if (running(currentAmplitude, previousAmplitude, runningSPM)){
        Serial.println("running");
        Step runningStep = {runningSPM, currentTime, true, false};
        return runningStep;
      }       
    }

    fallingTime = 0;
    
    // indicates we're transitioning from falling to rising
    
  } else if (fallingTime >= (samplePeriod * 3) && currentlyFalling){
    ///Serial.println("FALLING!");
    // samplePeriod * 3 is the same check as prior
    // meaning its likely that it was *actually* rising prior to this
    // and is now reaching the point at which its going down
    if (risingTime >= (samplePeriod * 3)){ 
      previousAmplitude = currentAmplitude;
      currentAmplitude = maxValue - minValue;

      previousRisingTime = risingTime; // since we're about to set it to zero
    }

    risingTime = 0;
    minValue = currentMagnitude;
  }

  Step idleStep = {0,currentTime, false, false};
  return idleStep;
}

float calculateCadence(Step steps[], int currentStepIndex, bool filledSteps, int stepLength){
  float cadenceSums = 0;
  int totalValidSteps = 0;
  Step recentStep = steps[currentStepIndex]; // we will only be checking types which align with most recent step
  // e.g if the array is [W, W, W, W, W, R, R, R];
  // where W = walking, R = running and we say the last index is the most recent
  // then we stop the loop once we get to W which is different than R
  // as its drastically different and should not be considered since it jacks
  // up the spm when transitioning from running to walking and undermines
  // running when going from walking to running
  float currentTime = millis();

  for (int i = 0; i < stepLength; i++){
    if ((recentStep.isRunning == steps[currentStepIndex].isRunning) && ((currentTime - steps[currentStepIndex].stepTime) < 5000)){
      cadenceSums += steps[currentStepIndex].spm; 
      totalValidSteps++;
    } else { // current one doesn't match most recent one
      break;
    }

    currentStepIndex --;

    if (currentStepIndex == -1){
      if (!filledSteps){
        break;
      } else {
        currentStepIndex = stepLength - 1;
      }
    }
  }

  if (totalValidSteps == 0){
    return 0;
  } else {
    return cadenceSums/totalValidSteps;
  }
}

float magnitude(float ax, float ay, float az){
  return sqrt(ax*ax + ay*ay + az*az);
}

void loop() {

  current = millis();

  if (current - previous < samplePeriod){
    return;
  }

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {

    // to ensure that sample rate sticks around 60 Hz
    //if (previous != 0){
    //  averageSamplePeriod += (current - previous);
    //  totalSamples++;
    //  Serial.print("Average Sample Rate: ");
    //  Serial.println(1000/(averageSamplePeriod/totalSamples));
    //}

    float ax, ay, az; // accelerometer data
    float gx, gy, gz; // gyroscopic data


    IMU.readAcceleration(ax, ay, az); 
    IMU.readGyroscope(gx, gy, gz);

    // degrees -> radians
    gx = radians(gx);
    gy = radians(gy);
    gz = radians(gz);

    // updates data stored
    unfilteredData[currentIndex] = magnitude(ax, ay, az);
    filteredData[currentIndex] = FIRFilter(unfilteredData, currentIndex, coefficients, filled, COEFFICIENT_LENGTH);
    times[currentIndex] = current;

    filter.update(gx, gy, gz, ax, ay, az); // for sensor fusion


    if (currentIndex == (COEFFICIENT_LENGTH - 1)){
      filled = true;
    } else if (!filled){ // not enough data until we have filled the FIR data
      currentIndex++; // needs to add here since we end up adding later
      // mainly because we need this current index but since we're not doing anything
      // and just skipping we can continue
      return;
    }

    int actualCurrentStepIndex = currentStepIndex - 1;
    
    if (actualCurrentStepIndex < 0 && filledSteps){
      actualCurrentStepIndex = stepLength - 1;
    }

    if (actualCurrentStepIndex >= 0){
      float cadence;

      if ((current - steps[actualCurrentStepIndex].stepTime) < 2000){
        cadence = calculateCadence(steps, actualCurrentStepIndex, filledSteps, stepLength);
        
      } else { // if no steps for a second we assume the rest of the data is likely junk
        cadence = 0;
      }

      Serial.print("Cadence: ");
      Serial.println(cadence);

    }

    float roll = filter.getRoll();
    float pitch = filter.getPitch();
    float yaw = filter.getYaw();

    //Step currentStep = detect_step(filteredData, times, currentIndex, COEFFICIENT_LENGTH, risingTime, fallingTime, previousRisingTime, previousStepTime, samplePeriod, currentAmplitude, previousAmplitude, minValue, maxValue);

    if (currentStep.isRunning || currentStep.isWalking){
      //Serial.println("WALK OR RUN");
      steps[currentStepIndex] = currentStep;
      currentStepIndex++;
    }

    // can safely assume its filled here since if it isnt we don't actually
    // have enough data to really determine if a step has been made therefore 
    // we don't do anything

    previous = current;
    currentIndex = ((currentIndex + 1) % COEFFICIENT_LENGTH);
    
    if (currentStepIndex == stepLength){
      filledSteps = true;
      currentStepIndex = 0;
    }
  }

  delay(5);

}


