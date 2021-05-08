class CustomStepper {
  public:
    CustomStepper(int in1, int in2, int in3, int in4) {
      IN1 = in1;
      IN2 = in2;
      IN3 = in3;
      IN4 = in4;
      pinMode(IN1, OUTPUT);
      pinMode(IN2, OUTPUT);
      pinMode(IN3, OUTPUT);
      pinMode(IN4, OUTPUT);
    };
    void move() {
      stepper(this->step);
    };
    void stop() {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
    }
    void stepper(int xw) {
      for (int x = 0; x < xw; x++) {
        switch (steps) {
          case 0:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, HIGH);
            break;
          case 1:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, HIGH);
            digitalWrite(IN4, HIGH);
            break;
          case 2:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, HIGH);
            digitalWrite(IN4, LOW);
            break;
          case 3:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, HIGH);
            digitalWrite(IN3, HIGH);
            digitalWrite(IN4, LOW);
            break;
          case 4:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, HIGH);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, LOW);
            break;
          case 5:
            digitalWrite(IN1, HIGH);
            digitalWrite(IN2, HIGH);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, LOW);
            break;
          case 6:
            digitalWrite(IN1, HIGH);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, LOW);
            break;
          case 7:
            digitalWrite(IN1, HIGH);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, HIGH);
            break;
          default:
            digitalWrite(IN1, LOW);
            digitalWrite(IN2, LOW);
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, LOW);
            break;
        }
        updateSteps();
        delay(1);
      }
    }
    void updateSteps() {
      if (direction == 1) {
        steps++;
      }
      if (direction == -1) {
        steps--;
      }
      if (direction == 0) {
        stop();
      }
      if (steps > 7) {
        steps = 0;
      }
      if (steps < 0) {
        steps = 7;
      }
    }
    int direction = 0;
    boolean debug = false;
    int step = 20;
  private:
    int IN1, IN2, IN3, IN4;
    int steps = 0;
};
