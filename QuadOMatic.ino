// The quadrature phases to output
const bool waveform1[] = {HIGH, LOW, LOW, HIGH};
const bool waveform2[] = {HIGH, HIGH, LOW, LOW};

const int xGPIOs[] = {6, 5};
const int yGPIOs[] = {4, 3};
const int xAnalogue = 1;
const int yAnalogue = 0;

const int clickOutGPIO = 2;
const int clickInGPIO = 11;

// Two digital outputs set permanently to high and low, respectively, to reference
// the variable resistors with 
const int hiRefGPIO = 13;
const int loRefGPIO = 12;

// the timestamps (in milliseconds) when the axis last was updated
unsigned long xTimestamp = 0;
unsigned long yTimestamp = 0;

// the indexes into the waveforms
int xIdx = 0; 
int yIdx = 0;

int xIsStill() {
  return analogueValueIsStill(analogRead(xAnalogue));
}

int yIsStill() {
  return analogueValueIsStill(analogRead(yAnalogue));
}

long xDelay() {
  return analogueValueToDelay(analogRead(xAnalogue));
}

long yDelay() {
  return analogueValueToDelay(analogRead(yAnalogue));
}

int xDirection() {
  return analogueValueToDirection(analogRead(xAnalogue));
}

int yDirection() {
  return analogueValueToDirection(analogRead(yAnalogue));
}

int xShouldAdvance() {
  unsigned long now = millis();
  // if the joystick is in the "still" region, don't update
  if (xIsStill()) {
    return 0;
  }
  
  if ((xTimestamp + xDelay()) < now) {
    xTimestamp = now;
    return 1;
  } else {
    return 0;
  }
}

int yShouldAdvance() {
  unsigned long now = millis();
  // if the joystick is in the "still" region, don't update
  if (yIsStill()) {
    return 0;
  }
  
  if ((yTimestamp + yDelay()) < now) {
    yTimestamp = now;
    return 1;
  } else {
    return 0;
  }
}

void dealWithButton() {
  int btn = digitalRead(clickInGPIO);
  digitalWrite(clickOutGPIO, btn);
}

int analogueValueIsStill(int value) {
  const int tolerance = 30;

  if (value > (512 + tolerance)) {
    return 0;
  }
  if (value < (512 - tolerance)) {
    return 0;
  }
  
  return 1;
}

int analogueValueToMagnitude(int value) {
  return abs(value - 512);
}

int analogueValueToDirection(int value) {
  if (value < 512) {
    return -1;
  }
  return 1;
}

long analogueValueToDelay(int value) {
  const long minDelay = 4;
  
  long mag = analogueValueToMagnitude(value);

  return ((512 - mag) / 32) + minDelay;
}

void writeQuadrature(int gpios[], int idx) {
  digitalWrite(gpios[0], waveform1[idx % 4]);
  digitalWrite(gpios[1], waveform2[idx % 4]);
}


void setup() {
  Serial.begin(9600);
  
  // set up quadrature outputs
  pinMode(xGPIOs[0], OUTPUT);
  pinMode(xGPIOs[1], OUTPUT);
  pinMode(yGPIOs[0], OUTPUT);
  pinMode(yGPIOs[1], OUTPUT);
  pinMode(clickInGPIO, INPUT_PULLUP);
  pinMode(clickOutGPIO, OUTPUT);

  pinMode(hiRefGPIO, OUTPUT);
  digitalWrite(hiRefGPIO, HIGH);
  pinMode(loRefGPIO, OUTPUT);
  digitalWrite(loRefGPIO, LOW);

  digitalWrite(xGPIOs[0], HIGH);
  digitalWrite(xGPIOs[1], HIGH);
  digitalWrite(yGPIOs[0], HIGH);
  digitalWrite(yGPIOs[1], HIGH);
  digitalWrite(clickOutGPIO, HIGH);

  Serial.print("initialised...\n");
}

void loop() {
  if (xShouldAdvance()) {
    xIdx = (xIdx + xDirection()) % 4;
    if (xIdx < 0) { xIdx += 4; }
    writeQuadrature(xGPIOs, xIdx);
  }

  if (yShouldAdvance()) {
    yIdx = (yIdx + yDirection()) % 4;
    if (yIdx < 0) { yIdx += 4; }
    writeQuadrature(yGPIOs, yIdx);
  }

  dealWithButton();

  // DEBUG:
  /* Serial.print("x: ");
  Serial.print(analogRead(xAnalogue));
  Serial.print("y: ");
  Serial.print(analogRead(yAnalogue));
  Serial.print("button: ");
  Serial.print(digitalRead(clickInGPIO));
  Serial.print("\n"); */
}
