#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define SCREEN_W 128
#define SCREEN_H 64

#define OLED_DC   9
#define OLED_RST  8
#define OLED_CS   10

Adafruit_SSD1306 oled(
  SCREEN_W, SCREEN_H, &SPI,
  OLED_DC, OLED_RST, OLED_CS
);

const byte JOY_X      = A0;
const byte JOY_Y      = A1;
const byte JOY_SW     = 2;
const byte LED_PIN    = 3;
const byte BUZZER_PIN = 4;

const bool INVERT_X = false;
const bool INVERT_Y = false;

const byte     SPEED_TBL[3] = {2, 3, 4};
const uint16_t DELAY_TBL[3] = {950, 800, 650};
const byte     LIVES_TBL[3] = {3, 3, 2};

const int ADDR_BEST = 0;
const int ADDR_DIFF = 6;

const byte laneX[3] = {27, 64, 101};
const byte PLAYER_Y = 48;
const byte MAX_ENEMIES = 4;
const byte MAX_COINS = 3;

enum GameState {
  MENU_MAIN,
  MENU_OPTIONS,
  MENU_CREDITS,
  PLAYING,
  CRASHING,
  GAMEOVER
};

GameState state = MENU_MAIN;

struct Object {
  int16_t y;
  byte lane;
  bool active;
};

Object enemies[MAX_ENEMIES];
Object coins[MAX_COINS];

byte playerLane = 1;
byte lives = 3;
byte speedLevel = 3;
byte difficulty = 1;

uint16_t score = 0;
uint16_t bestScore[3] = {0, 0, 0};
bool newRecord = false;

int16_t roadOffset = 0;

unsigned long lastFrame = 0;
unsigned long lastEnemy = 0;
unsigned long lastCoin = 0;
unsigned long lastScore = 0;
unsigned long lastButton = 0;
unsigned long lastDraw = 0;
unsigned long invulnUntil = 0;
unsigned long ledOffAt = 0;
unsigned long crashNext = 0;

byte flashFrames = 0;
byte crashStep = 0;

byte mainSel = 0;
byte optSel = 0;
byte goSel = 0;
byte creditPage = 0;

bool previousButton = HIGH;

int joyCX = 512;
int joyCY = 512;
bool armedX = false;
bool armedY = false;
const int JOY_HI = 250;
const int JOY_LO = 130;

int readAvg(byte pin) {
  long s = 0;
  for (byte i = 0; i < 8; i++) s += analogRead(pin);
  return (int)(s / 8);
}

void calibrateJoystick() {
  delay(50);
  joyCX = readAvg(JOY_X);
  joyCY = readAvg(JOY_Y);
  armedX = true;
  armedY = true;
}

int8_t axisEdge(byte pin, int center, bool invert, bool &armed) {
  int d = readAvg(pin) - center;
  if (invert) d = -d;

  if (armed) {
    if (d < -JOY_HI) { armed = false; return -1; }
    if (d >  JOY_HI) { armed = false; return  1; }
  } else if (abs(d) < JOY_LO) {
    armed = true;
  }
  return 0;
}

bool pressed() {
  bool current = digitalRead(JOY_SW);
  bool result = false;

  if (previousButton == HIGH &&
      current == LOW &&
      millis() - lastButton > 150) {
    result = true;
    lastButton = millis();
  }

  previousButton = current;
  return result;
}

void beepMove() {
  tone(BUZZER_PIN, 1400, 25);
}

void beepSelect() {
  tone(BUZZER_PIN, 1800, 40);
  delay(70);
  tone(BUZZER_PIN, 2400, 60);
  delay(60);
}

void beepBack() {
  tone(BUZZER_PIN, 1200, 40);
  delay(70);
  tone(BUZZER_PIN, 800, 60);
  delay(60);
}

void loadSettings() {
  for (byte i = 0; i < 3; i++) {
    EEPROM.get(ADDR_BEST + i * 2, bestScore[i]);
    if (bestScore[i] > 60000) bestScore[i] = 0;
  }

  difficulty = EEPROM.read(ADDR_DIFF);
  if (difficulty > 2) difficulty = 1;
}

void saveDifficulty() {
  EEPROM.update(ADDR_DIFF, difficulty);
}

void saveRecord() {
  if (score > bestScore[difficulty]) {
    bestScore[difficulty] = score;
    EEPROM.put(ADDR_BEST + difficulty * 2, bestScore[difficulty]);
    newRecord = true;
  }
}

const uint8_t PROGMEM heartBmp[] = {
  0x6C, 0xFE, 0xFE, 0x7C, 0x38, 0x10
};

void centerText(const __FlashStringHelper *s, int y, byte size) {
  oled.setTextSize(size);
  oled.setTextColor(SSD1306_WHITE);

  int16_t x1, y1;
  uint16_t w, h;

  oled.getTextBounds(s, 0, y, &x1, &y1, &w, &h);
  oled.setCursor((SCREEN_W - w) / 2, y);
  oled.print(s);
}

void drawItem(const __FlashStringHelper *label, int y, bool selected) {
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);

  if (selected) {

    oled.setCursor(30 + ((millis() / 220) % 2) * 2, y);
    oled.print(F(">"));
  }

  oled.setCursor(44, y);
  oled.print(label);
}

void drawCar(int x, int y, bool enemy) {

  oled.fillRect(x - 7, y + 2, 2, 4, SSD1306_WHITE);
  oled.fillRect(x + 5, y + 2, 2, 4, SSD1306_WHITE);
  oled.fillRect(x - 7, y + 8, 2, 4, SSD1306_WHITE);
  oled.fillRect(x + 5, y + 8, 2, 4, SSD1306_WHITE);

  oled.fillRoundRect(x - 5, y, 10, 13, 2, SSD1306_WHITE);

  if (enemy) {
    oled.fillRect(x - 3, y + 2, 6, 3, SSD1306_BLACK);
    oled.drawFastHLine(x - 3, y + 8, 6, SSD1306_BLACK);
    oled.drawPixel(x - 3, y + 11, SSD1306_BLACK);
    oled.drawPixel(x + 3, y + 11, SSD1306_BLACK);
  } else {
    oled.fillRect(x - 3, y + 3, 6, 3, SSD1306_BLACK);
    oled.drawFastHLine(x - 3, y + 8, 6, SSD1306_BLACK);
    oled.drawPixel(x - 2, y + 1, SSD1306_BLACK);
    oled.drawPixel(x + 2, y + 1, SSD1306_BLACK);
  }
}

void drawCoin(int x, int y) {
  oled.drawCircle(x, y, 4, SSD1306_WHITE);
  oled.drawCircle(x, y, 2, SSD1306_WHITE);
  oled.drawPixel(x, y, SSD1306_WHITE);
}

void drawRoad() {
  oled.drawFastVLine(8, 12, 52, SSD1306_WHITE);
  oled.drawFastVLine(119, 12, 52, SSD1306_WHITE);

  for (int y = -12; y < 64; y += 14) {
    int y0 = y + roadOffset;
    int y1 = y0 + 7;

    if (y0 < 12) y0 = 12;
    if (y1 > 64) y1 = 64;

    if (y1 > y0) {
      oled.drawFastVLine(45, y0, y1 - y0, SSD1306_WHITE);
      oled.drawFastVLine(82, y0, y1 - y0, SSD1306_WHITE);
    }
  }
}

void drawHUD() {
  oled.fillRect(0, 0, 128, 11, SSD1306_BLACK);

  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(1, 2);
  oled.print(F("SCORE "));
  oled.print(score);

  for (byte i = 0; i < lives; i++) {
    oled.drawBitmap(100 + i * 9, 2, heartBmp, 7, 6, SSD1306_WHITE);
  }

  oled.drawFastHLine(0, 11, 128, SSD1306_WHITE);
}

void drawMainMenu() {
  oled.clearDisplay();
  oled.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);

  centerText(F("Ous Rush"), 5, 2);
  oled.drawFastHLine(10, 23, 108, SSD1306_WHITE);

  drawItem(F("START"),   28, mainSel == 0);
  drawItem(F("OPTIONS"), 39, mainSel == 1);
  drawItem(F("CREDITS"), 50, mainSel == 2);

  oled.display();
}

void drawOptions() {
  oled.clearDisplay();
  oled.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);

  centerText(F("DIFFICULTY"), 4, 1);
  oled.drawFastHLine(10, 14, 108, SSD1306_WHITE);

  drawItem(F("EASY"),   18, optSel == 0);
  drawItem(F("MEDIUM"), 29, optSel == 1);
  drawItem(F("HARD"),   40, optSel == 2);
  drawItem(F("BACK"),   51, optSel == 3);

  oled.fillCircle(104, 18 + difficulty * 11 + 3, 2, SSD1306_WHITE);

  oled.display();
}

void drawCredits() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);

  centerText(F("CREDITS"), 2, 1);
  oled.drawFastHLine(0, 12, 128, SSD1306_WHITE);

  if (creditPage == 0) {
    oled.setCursor(4, 17);
    oled.print(F("Made by Ous Hesam"));

    oled.setCursor(4, 29);
    oled.print(F("REPO:"));
    oled.setCursor(4, 38);
    oled.print(F("github.com/oushesam/"));
    oled.setCursor(4, 47);
    oled.print(F("arduino-racer"));

    oled.setCursor(86, 56);
    oled.print(F("NEXT >"));
  } else {
    oled.setCursor(4, 17);
    oled.print(F("PROFILE:"));
    oled.setCursor(4, 28);
    oled.print(F("github.com/oushesam"));

    oled.setCursor(4, 44);
    oled.print(F("Thanks for playing!"));

    oled.setCursor(4, 56);
    oled.print(F("CLICK: BACK"));
  }

  oled.display();
}

void drawGameOver() {
  oled.clearDisplay();
  oled.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);

  centerText(F("GAME OVER"), 5, 2);
  oled.drawFastHLine(10, 23, 108, SSD1306_WHITE);

  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 28);

  if (newRecord) {
    oled.print(F("NEW BEST! "));
    oled.print(score);
  } else {
    oled.print(F("SCORE "));
    oled.print(score);
    oled.print(F("  BEST "));
    oled.print(bestScore[difficulty]);
  }

  drawItem(F("RETRY"), 40, goSel == 0);
  drawItem(F("MENU"),  51, goSel == 1);

  oled.display();
}

void clearObjects() {
  for (byte i = 0; i < MAX_ENEMIES; i++) {
    enemies[i].active = false;
    enemies[i].y = -20;
  }
  for (byte i = 0; i < MAX_COINS; i++) {
    coins[i].active = false;
    coins[i].y = -20;
  }
}

bool spawnEnemy() {
  bool blocked[3] = {false, false, false};
  byte blockedCount = 0;

  for (byte j = 0; j < MAX_ENEMIES; j++) {
    if (enemies[j].active && enemies[j].y < 30) {
      if (!blocked[enemies[j].lane]) {
        blocked[enemies[j].lane] = true;
        blockedCount++;
      }
    }
  }

  if (blockedCount >= 2) return false;

  bool avoid[3] = {blocked[0], blocked[1], blocked[2]};
  for (byte j = 0; j < MAX_COINS; j++) {
    if (coins[j].active && coins[j].y < 22) {
      avoid[coins[j].lane] = true;
    }
  }

  byte candidates[3];
  byte n = 0;
  for (byte l = 0; l < 3; l++) {
    if (!avoid[l]) candidates[n++] = l;
  }
  if (n == 0) return false;

  byte lane = candidates[random(0, n)];

  for (byte i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].lane = lane;
      enemies[i].y = -14;
      enemies[i].active = true;
      return true;
    }
  }
  return false;
}

bool spawnCoin() {
  for (byte i = 0; i < MAX_COINS; i++) {
    if (!coins[i].active) {
      byte lane = random(0, 3);

      for (byte j = 0; j < MAX_ENEMIES; j++) {
        if (enemies[j].active &&
            enemies[j].lane == lane &&
            enemies[j].y < 24) {
          return false;
        }
      }

      coins[i].lane = lane;
      coins[i].y = -8;
      coins[i].active = true;
      return true;
    }
  }
  return false;
}

bool hitEnemy(const Object &e) {
  if (!e.active || e.lane != playerLane) return false;

  return (e.y + 11 > PLAYER_Y + 2) &&
         (e.y + 2  < PLAYER_Y + 11);
}

void startGame() {
  randomSeed(micros() + analogRead(A2));

  score = 0;
  newRecord = false;
  playerLane = 1;
  roadOffset = 0;
  flashFrames = 0;

  speedLevel = SPEED_TBL[difficulty];
  lives = LIVES_TBL[difficulty];

  clearObjects();

  unsigned long now = millis();
  lastFrame = now;
  lastEnemy = now;
  lastCoin = now;
  lastScore = now;
  invulnUntil = now + 800;

  digitalWrite(LED_PIN, LOW);
  ledOffAt = 0;

  state = PLAYING;
}

void enterCrash() {
  saveRecord();
  crashStep = 0;
  crashNext = millis();
  state = CRASHING;
}

void loseLife() {
  if (lives > 0) lives--;

  if (lives == 0) {
    enterCrash();
    return;
  }

  invulnUntil = millis() + 1500;
  flashFrames = 2;
  digitalWrite(LED_PIN, HIGH);
  ledOffAt = millis() + 250;
  tone(BUZZER_PIN, 300, 160);
}

void updateGame(int8_t dx) {
  unsigned long now = millis();

  if (dx < 0 && playerLane > 0) playerLane--;
  else if (dx > 0 && playerLane < 2) playerLane++;

  if (ledOffAt && now >= ledOffAt) {
    digitalWrite(LED_PIN, LOW);
    ledOffAt = 0;
  }

  if (now - lastEnemy >= DELAY_TBL[difficulty]) {
    if (spawnEnemy()) lastEnemy = now;
  }

  if (now - lastCoin >= 1150) {
    if (spawnCoin()) lastCoin = now;
  }

  if (now - lastFrame < 35) return;
  lastFrame = now;

  roadOffset += speedLevel;
  if (roadOffset >= 14) roadOffset -= 14;

  for (byte i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;

    enemies[i].y += speedLevel;

    if (now >= invulnUntil && hitEnemy(enemies[i])) {
      enemies[i].active = false;
      loseLife();
      if (state != PLAYING) return;
      continue;
    }

    if (enemies[i].y > 64) {
      enemies[i].active = false;
      score += 2;
    }
  }

  for (byte i = 0; i < MAX_COINS; i++) {
    if (!coins[i].active) continue;

    coins[i].y += speedLevel;

    if (coins[i].lane == playerLane &&
        abs((coins[i].y + 4) - (PLAYER_Y + 6)) <= 9) {
      coins[i].active = false;
      score += 5;
      tone(BUZZER_PIN, 1900, 30);
      continue;
    }

    if (coins[i].y > 64) coins[i].active = false;
  }

  if (now - lastScore >= 500) {
    score++;
    lastScore = now;
  }

  oled.clearDisplay();

  drawRoad();

  for (byte i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      drawCar(laneX[enemies[i].lane], enemies[i].y, true);
    }
  }

  for (byte i = 0; i < MAX_COINS; i++) {
    if (coins[i].active) {
      drawCoin(laneX[coins[i].lane], coins[i].y + 4);
    }
  }

  bool showPlayer = !(now < invulnUntil && ((now / 90) % 2));
  if (showPlayer) drawCar(laneX[playerLane], PLAYER_Y, false);

  drawHUD();

  if (flashFrames > 0) {
    oled.fillRect(0, 0, 128, 64, SSD1306_INVERSE);
    flashFrames--;
  }

  oled.display();
}

void updateCrash() {
  unsigned long now = millis();
  if (now < crashNext) return;

  const int notes[6] = {880, 660, 440, 330, 260, 200};

  if (crashStep >= 6) {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
    goSel = 0;
    state = GAMEOVER;
    lastDraw = 0;
    return;
  }

  oled.fillRect(0, 0, 128, 64, SSD1306_INVERSE);
  oled.display();

  digitalWrite(LED_PIN, (crashStep % 2 == 0) ? HIGH : LOW);
  tone(BUZZER_PIN, notes[crashStep], 100);

  crashStep++;
  crashNext = now + 110;
}

bool drawDue(bool force) {
  unsigned long now = millis();
  if (force || now - lastDraw >= 90) {
    lastDraw = now;
    return true;
  }
  return false;
}

void handleMainMenu(bool click, int8_t dy) {
  bool changed = false;

  if (dy != 0) {
    mainSel = (mainSel + 3 + dy) % 3;
    beepMove();
    changed = true;
  }

  if (click) {
    beepSelect();
    if (mainSel == 0) {
      startGame();
      return;
    } else if (mainSel == 1) {
      optSel = difficulty;
      state = MENU_OPTIONS;
    } else {
      creditPage = 0;
      state = MENU_CREDITS;
    }
    lastDraw = 0;
    return;
  }

  if (drawDue(changed)) drawMainMenu();
}

void handleOptions(bool click, int8_t dy) {
  bool changed = false;

  if (dy != 0) {
    optSel = (optSel + 4 + dy) % 4;
    beepMove();
    changed = true;
  }

  if (click) {
    if (optSel < 3) {
      difficulty = optSel;
      saveDifficulty();
      beepSelect();
      changed = true;
    } else {
      beepBack();
      mainSel = 1;
      state = MENU_MAIN;
      lastDraw = 0;
      return;
    }
  }

  if (drawDue(changed)) drawOptions();
}

void handleCredits(bool click) {
  bool changed = false;

  if (click) {
    if (creditPage == 0) {
      creditPage = 1;
      beepSelect();
      changed = true;
    } else {
      beepBack();
      mainSel = 2;
      state = MENU_MAIN;
      lastDraw = 0;
      return;
    }
  }

  if (drawDue(changed)) drawCredits();
}

void handleGameOver(bool click, int8_t dy) {
  bool changed = false;

  if (dy != 0) {
    goSel = (goSel + 2 + dy) % 2;
    beepMove();
    changed = true;
  }

  if (click) {
    if (goSel == 0) {
      beepSelect();
      startGame();
    } else {
      beepBack();
      mainSel = 0;
      state = MENU_MAIN;
      lastDraw = 0;
    }
    return;
  }

  if (drawDue(changed)) drawGameOver();
}

void setup() {
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);

  randomSeed(analogRead(A2) + micros());

  loadSettings();

  if (!oled.begin(SSD1306_SWITCHCAPVCC)) {
    while (true) {

    }
  }

  oled.clearDisplay();
  oled.display();

  calibrateJoystick();

  drawMainMenu();
}

void loop() {
  bool click = pressed();
  int8_t dx = axisEdge(JOY_X, joyCX, INVERT_X, armedX);
  int8_t dy = axisEdge(JOY_Y, joyCY, INVERT_Y, armedY);

  switch (state) {
    case MENU_MAIN:    handleMainMenu(click, dy); break;
    case MENU_OPTIONS: handleOptions(click, dy);  break;
    case MENU_CREDITS: handleCredits(click);      break;
    case PLAYING:      updateGame(dx);            break;
    case CRASHING:     updateCrash();             break;
    case GAMEOVER:     handleGameOver(click, dy); break;
  }
}
