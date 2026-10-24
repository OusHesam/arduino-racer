<div align="center">

# 🏎️ OUS RUSH

### A retro 3-lane arcade racing game for Arduino Uno

**SSD1306 OLED · Analog Joystick · Buzzer · LED · EEPROM High Scores**

![Platform](https://img.shields.io/badge/platform-Arduino%20Uno-00979D?logo=arduino&logoColor=white)
![Display](https://img.shields.io/badge/display-SSD1306%20128x64%20SPI-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B%20(Arduino)-orange)
![License](https://img.shields.io/badge/license-MIT-green)
![Made by](https://img.shields.io/badge/made%20by-Ous%20Hesam-purple)

</div>

---

## 📖 Table of Contents

1. [About](#-about)
2. [Features](#-features)
3. [Gameplay](#-gameplay)
4. [Controls](#-controls)
5. [Menus](#-menus)
6. [Difficulty Levels](#-difficulty-levels)
7. [Scoring](#-scoring)
8. [Hardware Required](#-hardware-required)
9. [Wiring](#-wiring)
10. [Software Setup](#-software-setup)
11. [Uploading the Game](#-uploading-the-game)
12. [How It Works](#-how-it-works)
13. [Code Structure](#-code-structure)
14. [Customization](#-customization)
15. [EEPROM Layout](#-eeprom-layout)
16. [Troubleshooting](#-troubleshooting)
17. [Roadmap](#-roadmap)
18. [Contributing](#-contributing)
19. [License](#-license)
20. [Author](#-author)
21. [خلاصه به فارسی](#-خلاصه-به-فارسی)

---

## 🎮 About

**OUS RUSH** is a complete arcade-style racing game that runs on an Arduino Uno and a tiny 128×64 OLED screen. You drive a car down a three-lane highway, dodge oncoming traffic, grab coins, and try to beat your own high score.

The project was built to be a fun, self-contained, and easy-to-build electronics project. It needs only a handful of cheap parts, uses no exotic libraries, and the whole game lives in a single `.ino` file.

It includes a proper menu system, three difficulty levels, a lives system with temporary invincibility, sound effects through a buzzer, LED feedback, and persistent high scores saved in EEPROM, so they survive power-off.

---

## ✨ Features

- 🛣️ **3-lane highway** with an animated, scrolling road
- 🚗 **Detailed pixel cars**, with different designs for the player and the enemy traffic
- 🪙 **Collectible coins** for bonus points
- ❤️ **Lives system** shown as hearts in the HUD
- 🛡️ **Invincibility frames** after a hit (the car blinks) so you never lose multiple lives in one crash
- 📋 **Full menu system**: Start, Options, Credits
- 🎚️ **Three difficulty levels**: Easy, Medium, Hard, each changing speed, traffic density, and lives
- 💾 **Persistent storage**: difficulty setting and a separate high score for every difficulty are saved in EEPROM
- 🔊 **Sound effects**: menu beeps, coin pickups, life-lost sound, and a multi-tone crash jingle
- 💡 **LED feedback** on hits and crashes
- 💥 **Crash animation** with screen flash, LED blinking and descending tones
- 🎯 **Fair collision detection** with a slightly forgiving hitbox
- 🧠 **Smart traffic spawning** that always leaves at least one free lane, so you can never be trapped by a wall of cars
- 🕹️ **Joystick calibration** at startup, with edge-triggered input so a single push means a single move
- 🧩 **Single file**, around 750 lines, with no comments, clean and easy to read

---

## 🕹️ Gameplay

You drive the bottom car. Enemy cars come down from the top of the screen in one of three lanes, and your job is to survive as long as possible.

- **Dodge** enemy cars by switching lanes.
- **Collect** coins to earn bonus points.
- Every hit costs **one life**. When you run out of lives, the game ends.
- After a hit you get about **1.5 seconds of invincibility**, shown by the blinking car.
- At the start of every run you also get a short grace period (about 0.8 s) before traffic can hurt you.
- Your **score** is shown at the top of the screen with your remaining **lives** as hearts.
- The game speed is **constant** within a run. Difficulty is chosen from the menu and does not creep up during play, which keeps the game fair.

---

## 🎛️ Controls

| Input | In menus | In game |
|---|---|---|
| Joystick **up / down** | Move the selection arrow | not used |
| Joystick **left / right** | not used | Change lane |
| Joystick **button (press)** | Select / confirm | not used |

Every joystick push registers **once**. To make another move, return the stick to the center first. This removes accidental double moves and "ghost" lane changes caused by analog noise.

> **Tip:** Do not touch the joystick while the Arduino is powering on. It calibrates its center position during startup.

---

## 📋 Menus

### Main Menu

The title screen shows the game name and three options. A small arrow `>` bounces next to the currently selected option.

```
 ┌──────────────────────────┐
 │        OUS  RUSH         │
 │  ──────────────────────  │
 │       > START            │
 │         OPTIONS          │
 │         CREDITS          │
 └──────────────────────────┘
```

Moving the selection or pressing a button plays a beep. A single short beep means navigation, and a double beep means select or back.

### Options Menu

```
 ┌──────────────────────────┐
 │       DIFFICULTY         │
 │  ──────────────────────  │
 │       > EASY       ●     │
 │         MEDIUM           │
 │         HARD             │
 │         BACK             │
 └──────────────────────────┘
```

- Press on **EASY / MEDIUM / HARD** to select the difficulty. A dot marks the active one, and the choice is saved to EEPROM.
- Press on **BACK** to return to the main menu.

### Credits

Two pages with the author name, the repository link, and the GitHub profile link. Press the joystick button to go to the next page, and press again on the last page to go back.

### Game Over Screen

Shows your score (or **NEW BEST!** when you beat the record) and two choices: **RETRY** and **MENU**.

---

## 🎚️ Difficulty Levels

| Difficulty | Road speed | Traffic interval | Lives |
|:---:|:---:|:---:|:---:|
| 🟢 **Easy** | 2 | 950 ms | 3 |
| 🟡 **Medium** | 3 | 800 ms | 3 |
| 🔴 **Hard** | 4 | 650 ms | 2 |

Road speed is measured in pixels moved per frame. The game logic runs at about 28 frames per second (one frame every 35 ms).

---

## 🏆 Scoring

| Event | Points |
|---|:---:|
| Enemy car passes safely off the bottom of the screen | **+2** |
| Coin collected | **+5** |
| Survival bonus, every 0.5 seconds | **+1** |

A **separate high score is stored for each difficulty**, so playing on Easy never overwrites your Hard record.

---

## 🧰 Hardware Required

| # | Component | Quantity | Notes |
|:-:|---|:-:|---|
| 1 | Arduino Uno (or compatible, ATmega328P) | 1 | Nano also works with the same pin numbers |
| 2 | SSD1306 OLED, 128×64, **SPI** version | 1 | 7-pin module (GND, VCC, D0, D1, RES, DC, CS) |
| 3 | Analog joystick module (KY-023 or similar) | 1 | With a push button |
| 4 | Passive buzzer | 1 | A passive buzzer is required for different tones |
| 5 | LED (any color) | 1 | |
| 6 | Resistor 220 Ω | 1–2 | One for the LED, optionally one in series with the buzzer |
| 7 | Capacitor 1 µF | 1 | Optional, recommended near the OLED for stable power |
| 8 | Breadboard and jumper wires | – | |

> An **active** buzzer will still make a sound, but every tone will sound the same, so the menu and crash melodies will not be audible as different notes. Use a **passive** buzzer.

---

## 🔌 Wiring

### OLED display (hardware SPI)

| OLED pin | Arduino Uno |
|---|---|
| GND | GND |
| VCC | 5V |
| D0 (SCK / CLK) | **D13** |
| D1 (MOSI / SDA) | **D11** |
| RES (Reset) | **D8** |
| DC | **D9** |
| CS | **D10** |

### Joystick

| Joystick pin | Arduino Uno |
|---|---|
| GND | GND |
| +5V | 5V |
| VRx | **A0** |
| VRy | **A1** |
| SW (button) | **D2** |

### LED and buzzer

| Part | Connection |
|---|---|
| LED long leg (+) | **D3** through a 220 Ω resistor |
| LED short leg (−) | GND |
| Buzzer (+) | **D4** (optionally through 220 Ω) |
| Buzzer (−) | GND |

### Summary of pins used

```
D2   ── Joystick button (INPUT_PULLUP)
D3   ── LED (through 220 Ω)
D4   ── Buzzer
D8   ── OLED RES
D9   ── OLED DC
D10  ── OLED CS
D11  ── OLED MOSI (hardware SPI)
D13  ── OLED SCK  (hardware SPI)
A0   ── Joystick X
A1   ── Joystick Y
A2   ── Left unconnected (used as a random seed source)
```

> **Power tip:** Place a 1 µF capacitor between 5V and GND close to the OLED module. Buzzers and LEDs can create noise on the 5V rail, and a small capacitor helps keep the display stable.

---

## 💻 Software Setup

### Requirements

- **Arduino IDE** 1.8.x or 2.x
- The following libraries (install them from **Sketch → Include Library → Manage Libraries**):

| Library | Author |
|---|---|
| `Adafruit SSD1306` | Adafruit |
| `Adafruit GFX Library` | Adafruit |
| `Adafruit BusIO` | Adafruit (installed automatically as a dependency) |

`SPI.h` and `EEPROM.h` are built into the Arduino core, so there is nothing else to install.

---

## 🚀 Uploading the Game

1. Clone or download this repository:
   ```bash
   git clone https://github.com/oushesam/arduino-racer.git
   ```
2. Open the `.ino` file in the Arduino IDE. The IDE may ask to place it in a folder with the same name. Accept that.
3. Select your board: **Tools → Board → Arduino Uno**.
4. Select the right port: **Tools → Port**.
5. Click **Upload**.
6. When the title screen appears, you are ready to play. Remember not to touch the joystick during power-up.

---

## ⚙️ How It Works

### State machine

The game is built around a simple state machine, and `loop()` dispatches to the handler of the current state:

```
MENU_MAIN ──► MENU_OPTIONS ──► (back) ──► MENU_MAIN
    │
    ├──────► MENU_CREDITS ──► (back) ──► MENU_MAIN
    │
    └──────► PLAYING ──► CRASHING ──► GAMEOVER ──► PLAYING (retry)
                                          │
                                          └──────► MENU_MAIN
```

### Non-blocking design

The game never freezes the main loop with long `delay()` calls during play. Timing is handled with `millis()`, the crash animation is a small step-by-step sequence driven by timers, and `tone()` is called with a duration so it runs in the background.

### Joystick input

- At startup, the center position of both axes is measured and stored.
- Each reading is an average of 8 samples to reduce noise.
- Input is **edge-triggered**: the stick must go past a high threshold to register a move, and it must come back below a low threshold (hysteresis) before it can register again.

### Fair gameplay

- **Smaller hitbox:** collisions use a hitbox slightly smaller than the drawn sprites, so edge-touching does not feel unfair.
- **Lives and invincibility:** a hit removes one life and gives the player a short window of safety.
- **Guaranteed escape route:** the spawner never allows cars to occupy all three lanes near the top of the screen.
- **No coin-on-car overlap:** coins are not spawned in a lane that has a car just entering it.

### Rendering

- The HUD is drawn **last**, on top of everything else, so cars never overwrite the score text.
- The screen flash on hit and crash is done by drawing an inverted rectangle in the frame buffer, not by sending the display invert command, which keeps the display stable.
- The scrolling road dashes are clipped so they slide smoothly under the HUD.

---

## 🗂️ Code Structure

| Section | What it does |
|---|---|
| Pin and display definitions | OLED SPI setup and pin constants |
| Difficulty tables | `SPEED_TBL`, `DELAY_TBL`, `LIVES_TBL` |
| Data structures | `Object` struct for enemies and coins, and the `GameState` enum |
| Joystick | `calibrateJoystick()`, `axisEdge()`, `readAvg()` |
| Button | `pressed()` with debounce |
| Sound | `beepMove()`, `beepSelect()`, `beepBack()` |
| EEPROM | `loadSettings()`, `saveDifficulty()`, `saveRecord()` |
| Graphics | `drawCar()`, `drawCoin()`, `drawRoad()`, `drawHUD()`, `drawItem()`, `centerText()` |
| Screens | `drawMainMenu()`, `drawOptions()`, `drawCredits()`, `drawGameOver()` |
| Objects | `spawnEnemy()`, `spawnCoin()`, `clearObjects()`, `hitEnemy()` |
| Game flow | `startGame()`, `loseLife()`, `enterCrash()`, `updateGame()`, `updateCrash()` |
| Menu handlers | `handleMainMenu()`, `handleOptions()`, `handleCredits()`, `handleGameOver()` |
| Entry points | `setup()` and `loop()` |

---

## 🛠️ Customization

You can tweak the game without touching the main logic.

### Change the difficulty values

```cpp
const byte     SPEED_TBL[3] = {2, 3, 4};
const uint16_t DELAY_TBL[3] = {950, 800, 650};
const byte     LIVES_TBL[3] = {3, 3, 2};
```

The order is Easy, Medium, Hard. Keep lives at **3 or fewer**, since the HUD has room for three hearts.

### Flip joystick directions

If your joystick moves the wrong way (for example, up moves the arrow down), change these constants:

```cpp
const bool INVERT_X = false;
const bool INVERT_Y = false;
```

### Change pins

All pins are defined as constants near the top of the file (`JOY_X`, `JOY_Y`, `JOY_SW`, `LED_PIN`, `BUZZER_PIN`, and the `OLED_*` defines).

### Change the game title

Find the line in `drawMainMenu()` that draws the title with `centerText(...)` and replace the text. At text size 2, up to about 10 characters fit on the 128-pixel-wide screen.

### Tune joystick sensitivity

```cpp
const int JOY_HI = 250;
const int JOY_LO = 130;
```

Lower `JOY_HI` makes the stick more sensitive. Raise it if you get accidental moves.

### Change the sounds

The beep functions and the `notes[]` array in `updateCrash()` control all frequencies and durations.

---

## 💾 EEPROM Layout

| Address | Size | Content |
|:-:|:-:|---|
| 0–1 | 2 bytes | High score, Easy |
| 2–3 | 2 bytes | High score, Medium |
| 4–5 | 2 bytes | High score, Hard |
| 6 | 1 byte | Selected difficulty (0 = Easy, 1 = Medium, 2 = Hard) |

Invalid or uninitialized values are automatically reset to safe defaults at boot. To reset all scores manually, upload any sketch that writes `0` to addresses 0–5.

---

## 🩺 Troubleshooting

| Problem | Likely cause and fix |
|---|---|
| **Blank screen** | Check the OLED wiring, especially CS (D10), DC (D9) and RES (D8). Make sure you have the **SPI** version of the module, not I²C. |
| **Display freezes at boot** | `oled.begin()` failed. Re-check wiring and library installation. |
| **Screen glitches or flickers** | Add a 1 µF capacitor across the OLED power pins. Use short wires, and keep the buzzer and LED wires away from the display wires. |
| **Menu moves the wrong way** | Set `INVERT_Y` to `true`. |
| **Car moves the wrong way** | Set `INVERT_X` to `true`. |
| **Car or menu moves by itself** | The joystick was touched during power-up, so calibration was off. Reset the board and keep your hands off the stick. |
| **Button does not respond** | Check that the joystick SW pin is on D2 and the board has the pull-up enabled (the code does this with `INPUT_PULLUP`). |
| **All beeps sound the same** | You are using an active buzzer. Replace it with a passive one. |
| **No sound at all** | Check polarity and that the buzzer is on D4 and GND. |
| **LED does not light** | Check LED direction (long leg to the resistor and D3) and the resistor. |
| **Sketch does not compile** | Make sure the Adafruit SSD1306, Adafruit GFX, and Adafruit BusIO libraries are installed. |
| **Memory warnings** | The 1 KB OLED frame buffer takes up half of the Uno's RAM. The code uses `F()` for strings to save the rest. Avoid adding many extra global variables. |

---

## 🗺️ Roadmap

Ideas for future versions:

- [ ] Pause function
- [ ] Top-3 leaderboard with 3-letter names
- [ ] Power-ups (shield, slow motion)
- [ ] Relay-based external effects on crash
- [ ] Tilt-sensor controls
- [ ] Microphone-based "horn" that clears traffic
- [ ] More car and obstacle types
- [ ] Day and night road themes
- [ ] Settings for sound on/off

---

## 🤝 Contributing

Contributions, bug reports, and ideas are welcome.

1. Fork the repository
2. Create a branch: `git checkout -b feature/my-feature`
3. Commit your changes: `git commit -m "Add my feature"`
4. Push the branch: `git push origin feature/my-feature`
5. Open a Pull Request

Please keep the code style consistent and test your changes on real hardware when possible. For bugs, open an issue and include your board type, display model, and what you saw on the screen.

---

## 📄 License

This project is released under the **MIT License**. You are free to use, modify, and share it, including for commercial purposes, as long as the original copyright notice is kept. Add a `LICENSE` file to the repository with the MIT license text.

---

## 👤 Author

**Ous Hesam**

- GitHub: [github.com/oushesam](https://github.com/oushesam)
- Repository: [github.com/oushesam/arduino-racer](https://github.com/oushesam/arduino-racer)

If you like this project, please give it a ⭐ on GitHub. It really helps!

---

## 🇮🇷 خلاصه به فارسی

**OUS RUSH** یک بازی ماشین‌سواری سه‌لاینه برای آردوینو یونو است که روی نمایشگر OLED مدل SSD1306 (نسخهٔ SPI) اجرا می‌شود.

**امکانات:**
- جادهٔ سه‌لاینه با انیمیشن حرکت
- ماشین‌های دشمن و سکه‌های امتیازی
- سیستم جان با قلب‌های نمایشی و مصونیت کوتاه بعد از برخورد
- منوی کامل: Start، Options، Credits
- سه سطح سختی: آسان، متوسط، سخت
- ذخیرهٔ سختی و رکورد جداگانهٔ هر سطح در EEPROM
- صدای بازر برای حرکت در منو، جمع کردن سکه و تصادف
- چراغ LED و افکت چشمک‌زدن صفحه هنگام تصادف

**کنترل:** جوی‌استیک بالا/پایین برای منو، چپ/راست برای تعویض لاین و فشردن دکمهٔ جوی‌استیک برای انتخاب.

**قطعات لازم:** آردوینو یونو، OLED مدل SSD1306 نسخهٔ SPI، ماژول جوی‌استیک، بازر پسیو، LED، مقاومت ۲۲۰ اهم و (اختیاری) خازن ۱ میکروفاراد.

**نکتهٔ مهم:** هنگام روشن شدن دستگاه به جوی‌استیک دست نزنید، چون در همان لحظه مرکز آن کالیبره می‌شود.

**سیم‌بندی و نصب:** جدول‌های سیم‌بندی و مراحل آپلود در بخش‌های انگلیسی بالا آمده است. کتابخانه‌های `Adafruit SSD1306` و `Adafruit GFX` را نصب کنید، برد را روی Arduino Uno بگذارید و فایل `.ino` را آپلود کنید.

ساخته‌شده توسط **Ous Hesam** 💜
