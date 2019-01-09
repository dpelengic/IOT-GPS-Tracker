#include <LGSM.h>
#include <LGPS.h>
#include <LCheckSIM.h>
#include <LBattery.h>
#include <LSensorHub.h>

char mnum[13] = "+11122333444"; // master, control number
int FENCE = 0;
long xbase, ybase, zbase;

int check_battery_level();
int check_sim_state();
int check_gps_state();
int check_acc_state();

void sms_commands(char *rnum);
void send_sms(char *rnum, char *text);
void fetch_location (char *buffer);
void fetch_acc_data(long *x, long *y, long *z);
int verify_fence(long x, long y, long z);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Tracking device\r\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  long x, y, z = 0;
  int failratio = 0;
  int alarm_sent = 0;

  // Check for new sms every 10 seconds
  while (!LSMS.available()) {
    Serial.print("No sms\r\n");
    for ( int i = 0 ; i < 10 ; i++) {
      // if fencing is enabled, check if accelometer values changed from initial ones every second
      delay(1000);
      if (FENCE == 1) {
        fetch_acc_data(&x, &y, &z);

        // trigger alarm if accelometer value varies from initially set values at least 50% of time
        failratio = failratio + verify_fence(x, y, z);
        if (failratio > 5) {
          Serial.print("ALARM!\r\n");

          if (alarm_sent == 0) {
            char text[30] = {0};
            snprintf ( text, 30, "ALARM! MOVEMENT DETECTED.");
            send_sms(mnum, text);
            alarm_sent = 1;
          }
        }
      }
    }
  }

  // get remote number
  char rnum[20];
  LSMS.remoteNumber(rnum, 20);
  Serial.print(rnum);
  Serial.print("\r\n");

  if (strstr(rnum, mnum) != NULL) {
    Serial.print("Master number, proceed\r\n");
    sms_commands(rnum);
    LSMS.flush();
  }
  else {
    Serial.print("Not Master number, abort\r\n");
    LSMS.flush();
  }
}

// Check battery %
int check_battery_level() {
  return LBattery.level();
}

// Check SIM state
int check_sim_state() {
  return LCheckSIM.isCheck();
}

// Check GPS
int check_gps_state() {
  return LGPS.check_online();
}

// Check Accelometer
int check_acc_state() {
  return LSensorHub.check_on_line();
}

// Validate and execute SMS commands
void sms_commands(char *rnum) {
  char text[154] = {0}; // to-send sms text
  char buf_contex[154] = {0}; // received sms text
  LSMS.remoteContent(buf_contex, 154);
  Serial.println(buf_contex);
  Serial.println(strstr("Location", buf_contex));

  // responses to command
  if (strstr("Location", buf_contex) != NULL) {
    Serial.println("Location invoked\r\n");
    char buffer[154] = {0};

    // check GPS status
    Serial.println("GPS state");
    int count = 1;
    while ( !check_gps_state() && count < 4 ) {
      delay(10000); // delay getting location
      count++;
      Serial.println("Retry getting location\r\n");
      Serial.println(count);
    }
    if (check_gps_state()) {
      fetch_location(buffer);
      snprintf ( text, 154, "%s.", buffer );
    }
    else {
      snprintf ( text, 25, "Cannot fetch location.");
    }
  }

  else if (strstr("Status", buf_contex) != NULL) {
    Serial.println("Battery status invoked");
    int blevel = check_battery_level();
    snprintf ( text, 20, "Battery Level %d.", blevel );
  }

  else if (strstr("Fence", buf_contex) != NULL) {
    Serial.println("Fence functionality invoked");
    int count = 1;
    while ( !check_acc_state() && count < 4 ) {
      delay(5000); // delay setting fence
      count++;
      Serial.println("Retry setting fence\r\n");
      Serial.println(count);
    }
    if (check_acc_state()) {
      FENCE = !FENCE;
      xbase, ybase, zbase = 0;
      fetch_acc_data(&xbase, &ybase, &zbase);
      snprintf ( text, 20, "Fence %d.", FENCE );
    }
    else {
      snprintf ( text, 20, "Fence set ERROR.");
    }
  }
  else {
    snprintf ( text, 20, "Invalid command.");
  }
  send_sms(rnum, text);
  //LSMS.flush();
}


// Send SMS
void send_sms(char *rnum, char *text) {
  int retry = 1;

  while (retry < 4 && !LSMS.ready()) {
    delay(1000);
    retry++;
  }

  if ( LSMS.ready() ) {
    Serial.println("SIM card ready");
    LSMS.beginSMS(rnum);
    LSMS.print(text);

    if (LSMS.endSMS()) {
      Serial.println("SMS sent");
      //LSMS.flush();
    }
  }
}

void fetch_location (char *buffer) {
  unsigned char *utc_date_time = 0;
  utc_date_time = LGPS.get_utc_date_time();
  sprintf(buffer, "UTC DateTime:%d-%d-%d  %d:%d:%d\r\nLocation https://maps.google.com/maps?q=%f,%f ",
          utc_date_time[0], utc_date_time[1], utc_date_time[2], utc_date_time[3], utc_date_time[4], utc_date_time[5], LGPS.get_latitude(), LGPS.get_longitude());
}

void fetch_acc_data(long *x, long *y, long *z) {
  Serial.println("fetching acc data ...\r\n");
  LSensorHub.GetAccData(x, y, z);
}

int verify_fence(long x, long y, long z) {
  // x,y,z "mismatch" allowed
  long margin = 200;
  if (x < (xbase - margin) || x > (xbase + margin)) {
    Serial.println("x changed");
    return 1;
  }
  if (y < (ybase - margin) || y > (ybase + margin)) {
    Serial.println("y changed");
    return 1;
  }
  if (z < (zbase - margin) || z > (zbase + margin)) {
    Serial.println("z changed");
    return 1;
  }
  // value within safe bounds
  return 0;
}
