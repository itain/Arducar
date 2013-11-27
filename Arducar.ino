#include <Ultrasonic.h>
#define LED		13

// Right motor
#define DIR1A		3	// Forward
#define DIR1B		4	// Reverse
#define ENA1		5	// speed

// Left motor
#define DIR2A		7	// Forward
#define DIR2B		8	// Reverse
#define ENA2		6	// Speed

// Sonar
#define PULSE		9	// Send Pulse
#define ECHO		10	// Return echo

#define MAXFWD		255	// Maximum forward speed (each wheel)
#define MAXREV		-255	// Maximum reverse speed (each wheel)
#define FWDACCEL	15	// Forward acceleration
#define REVACCEL	60	// Reversecceleration

#define UPDATE		40	// 25Hz!

int16_t rspeed;			// right speed
int16_t lspeed;			// left speed
int16_t wantspeed;		// wanted speed - average right and left
int16_t wantturn;		// wanted turn - 2X the difference of right and left

// sonar
Ultrasonic hcsr(PULSE, ECHO);

void setup() {
	Serial.begin(115200);

	pinMode(DIR1A, OUTPUT);
	pinMode(DIR1B, OUTPUT);
	pinMode(ENA1,  OUTPUT);
	pinMode(DIR2A, OUTPUT);
	pinMode(DIR2B, OUTPUT);
	pinMode(ENA2,  OUTPUT);
	pinMode(PULSE, OUTPUT);
	pinMode(ECHO,  INPUT);

	randomSeed(analogRead(0));

	Serial.println(F("ArduDCar initialized"));
}

uint32_t speedupdate;
uint32_t sonarupdate;

uint32_t distance;	// actually the timing!

void loop() {
	uint32_t now = millis();

	if((now - speedupdate) >= UPDATE) {
		Serial.print(F("Wantspeed: "));
		Serial.print(wantspeed);
		Serial.print(F("; Wantturn: "));
		Serial.println(wantturn);

		int16_t r = wantspeed + wantturn;
		int16_t l = wantspeed - wantturn;
		rspeed = constrain(r, rspeed-REVACCEL, rspeed+FWDACCEL);
		rspeed = constrain(rspeed, MAXREV, MAXFWD);
		lspeed = constrain(l, lspeed-REVACCEL, lspeed+FWDACCEL);
		lspeed = constrain(lspeed, MAXREV, MAXFWD);

		// Now update motors!
		// Right
		digitalWrite(DIR1A, rspeed>0);
		digitalWrite(DIR1B, rspeed<=0);
		analogWrite(ENA1, abs(rspeed));
		// Left
		digitalWrite(DIR2A, lspeed>0);
		digitalWrite(DIR2B, lspeed<=0);
		analogWrite(ENA2, abs(lspeed));

		distance = hcsr.Timing();
		Serial.print(F("Distance: "));
		Serial.println(distance);

		int8_t t;
		if(wantturn > 0)
			t = 1;
		else if(wantturn < 0)
			t = -1;
		else {
			t = random(2) ? -1 : 1;
			Serial.print(F("Random: "));
			Serial.println(t);
		}

		digitalWrite(LED, distance < 1200);

		if(distance > 1000) {
			wantturn = 0;
			wantspeed = MAXFWD;
 		}
		else if(distance < 200) {
			wantspeed = MAXREV;
			wantturn = t*64;
		}
		else if(distance < 600) {
			wantspeed = 0;
			wantturn = t * 128;
		}
		else if(distance < 900) {
			wantspeed = MAXFWD / 4;
			wantturn = t * 64;
		}

		// delay till next time
		speedupdate = now;
	}
}
