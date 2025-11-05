/*
 * Tetris Game for ESP32-S3-Mini
 * 
 * Hardware:
 * - ST7789 240x320 TFT Display (rotated to 320x240)
 * - Analog buttons (Up, Down, Left, Right, A, B)
 * - RGB LED for piece color feedback
 * - Piezo buzzer for sound effects
 *
 * Libraries (using the Arduino IDE's Library Manager):
 * - Adafruit GFX Library
 * - Adafruit ST7735 and ST7789 Library
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
const int PIN_X = 2;
const int PIN_Y = 1;
const int PIN_MINIBADGE_CLK = 12;
const int PIN_RIGHT = 13;
const int PIN_LEFT = 14;
const int PIN_DOWN = 15;
const int PIN_UP = 16;
const int PIN_B = 17;
const int PIN_A = 18;
const int PIN_SCL = 21;
const int PIN_RED = 38;
const int PIN_GREEN = 39;
const int PIN_BLUE = 40;
const int PIN_LED = 41;
const int PIN_BUZZER = 42;
const int PIN_TX = 43;
const int PIN_RX = 44;
const int PIN_SDA = 47;

// TFT Display pins
#define TFT_CS   33
#define TFT_RST  48
#define TFT_DC   37
#define TFT_MOSI 35
#define TFT_SCLK 36
#define TFT_MISO 34

// ============================================================================
// CONSTANTS
// ============================================================================
const int BUTTON_THRESHOLD = 4095 / 3;  // 1/3 of max ADC value
const int PWM_RESOLUTION_BITS = 10;
const int PWM_MAX_DUTY = 30;

// Game grid dimensions
const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 18;
const int BLOCK_SIZE = 11;
const int GRID_OFFSET_X = 50;
const int GRID_OFFSET_Y = 15;

// Timing constants
const unsigned long BUTTON_DELAY = 150;
const unsigned long NORMAL_MOVE_DELAY = 500;
const unsigned long FAST_MOVE_DELAY = 50;
const unsigned long PAUSE_DEBOUNCE = 200;
const unsigned long LINE_FLASH_DELAY = 50;
const int LINE_FLASH_COUNT = 6;

// Scoring
const int POINTS_PER_LINE = 100;

// ============================================================================
// TETROMINO SHAPES
// ============================================================================
// Format: [piece_type][rotation][y][x]
const uint8_t SHAPES[7][4][4][4] = {
  // I piece
  {
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
    {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
    {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
  },
  // O piece (square - same for all rotations)
  {
    {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
  },
  // T piece
  {
    {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
    {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
  },
  // S piece
  {
    {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
    {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
  },
  // Z piece
  {
    {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
  },
  // J piece
  {
    {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
    {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
  },
  // L piece
  {
    {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
    {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
  }
};

// ============================================================================
// COLOR DEFINITIONS
// ============================================================================
// Display colors (16-bit RGB565 format)
const uint16_t COLORS[7] = {
  0x07FF,  // Bright Cyan
  0xFFE0,  // Bright Yellow
  0xF81F,  // Bright Magenta
  0x07E0,  // Bright Green
  0xF800,  // Bright Red
  0x001F,  // Bright Blue
  0xFD20   // Bright Orange
};

// RGB LED colors (8-bit per channel)
const uint8_t RGB_COLORS[7][3] = {
  {0, 255, 255},    // Cyan
  {255, 255, 0},    // Yellow
  {255, 0, 255},    // Magenta
  {0, 255, 0},      // Green
  {255, 0, 0},      // Red
  {0, 0, 255},      // Blue
  {255, 165, 0}     // Orange
};

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Game grid (0 = empty, 1-7 = color index + 1)
uint8_t grid[GRID_HEIGHT][GRID_WIDTH];

// Current piece state
struct {
  int type;
  int rotation;
  int x;
  int y;
  int color;
} currentPiece;

// Previous piece position (for smooth redrawing)
struct {
  int x;
  int y;
  int rotation;
} prevPiece;

// Game state
struct {
  unsigned long lastMoveTime;
  unsigned long lastButtonTime;
  unsigned long moveDelay;
  int score;
  bool isOver;
  bool isPaused;
} gameState;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Set RGB LED color
 */
void setRgbColor(int r, int g, int b) {
  int dutyRed = map(r, 0, 255, 0, PWM_MAX_DUTY);
  int dutyGreen = map(g, 0, 255, 0, PWM_MAX_DUTY);
  int dutyBlue = map(b, 0, 255, 0, PWM_MAX_DUTY);
  
  analogWrite(PIN_RED, dutyRed);
  analogWrite(PIN_GREEN, dutyGreen);
  analogWrite(PIN_BLUE, dutyBlue);
}

/**
 * Check if analog button is pressed
 */
bool isButtonPressed(int pin) {
  return analogRead(pin) >= BUTTON_THRESHOLD;
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

/**
 * Draw a single block on the display
 */
void drawBlock(int x, int y, uint16_t color) {
  int screenX = GRID_OFFSET_X + x * BLOCK_SIZE;
  int screenY = GRID_OFFSET_Y + y * BLOCK_SIZE;
  tft.fillRect(screenX, screenY, BLOCK_SIZE - 1, BLOCK_SIZE - 1, color);
}

/**
 * Draw the entire game grid
 */
void drawGrid() {
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] > 0) {
        drawBlock(x, y, COLORS[grid[y][x] - 1]);
      } else {
        drawBlock(x, y, ST77XX_BLACK);
      }
    }
  }
}

/**
 * Draw the current falling piece
 */
void drawCurrentPiece() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (SHAPES[currentPiece.type][currentPiece.rotation][y][x]) {
        int gridX = currentPiece.x + x;
        int gridY = currentPiece.y + y;
        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          drawBlock(gridX, gridY, COLORS[currentPiece.color]);
        }
      }
    }
  }
}

/**
 * Erase the previous piece position
 */
void erasePreviousPiece() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (SHAPES[currentPiece.type][prevPiece.rotation][y][x]) {
        int gridX = prevPiece.x + x;
        int gridY = prevPiece.y + y;
        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          // Redraw the background (either empty or locked block)
          if (grid[gridY][gridX] > 0) {
            drawBlock(gridX, gridY, COLORS[grid[gridY][gridX] - 1]);
          } else {
            drawBlock(gridX, gridY, ST77XX_BLACK);
          }
        }
      }
    }
  }
}

/**
 * Draw the score display
 */
void drawScore() {
  tft.fillRect(200, 10, 110, 30, ST77XX_BLACK);
  tft.setCursor(200, 10);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("Score:");
  tft.setCursor(200, 30);
  tft.print(gameState.score);
}

/**
 * Draw the control key
 */
void drawControls() {
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(200, 60);
  tft.println("Controls:");
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(200, 75);
  tft.println("L/R: Move");
  tft.setCursor(200, 90);
  tft.println("Up: Rotate");
  tft.setCursor(200, 105);
  tft.println("Dn: Drop");
  tft.setCursor(200, 120);
  tft.println("A: Pause");
  tft.setCursor(200, 135);
  tft.println("B: New Game");
}

/**
 * Draw the game border
 */
void drawBorder() {
  tft.drawRect(GRID_OFFSET_X - 2, GRID_OFFSET_Y - 2, 
               GRID_WIDTH * BLOCK_SIZE + 2, GRID_HEIGHT * BLOCK_SIZE + 2, 
               ST77XX_WHITE);
}

// ============================================================================
// GAME LOGIC FUNCTIONS
// ============================================================================

/**
 * Check if current piece collides with walls or other pieces
 */
bool checkCollision(int offsetX, int offsetY, int rotation) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (SHAPES[currentPiece.type][rotation][y][x]) {
        int newX = currentPiece.x + x + offsetX;
        int newY = currentPiece.y + y + offsetY;
        
        // Check boundaries
        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT) {
          return true;
        }
        
        // Check collision with placed blocks
        if (newY >= 0 && grid[newY][newX] > 0) {
          return true;
        }
      }
    }
  }
  return false;
}

/**
 * Lock the current piece into the grid
 */
void lockPiece() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (SHAPES[currentPiece.type][currentPiece.rotation][y][x]) {
        int gridX = currentPiece.x + x;
        int gridY = currentPiece.y + y;
        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          grid[gridY][gridX] = currentPiece.color + 1;
        }
      }
    }
  }
}

/**
 * Clear completed lines and update score
 */
void clearLines() {
  int linesCleared = 0;
  
  for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;
    
    // Check if line is full
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] == 0) {
        fullLine = false;
        break;
      }
    }
    
    if (fullLine) {
      linesCleared++;
      
      // Flash effect for cleared line
      for (int flash = 0; flash < LINE_FLASH_COUNT; flash++) {
        uint16_t flashColor = (flash % 2 == 0) ? 0x07E0 : 0x001F; // Green : Blue
        
        // Flash line on display
        for (int x = 0; x < GRID_WIDTH; x++) {
          drawBlock(x, y, flashColor);
        }
        
        // Flash RGB LED
        if (flash % 2 == 0) {
          setRgbColor(0, 255, 0);  // Green
        } else {
          setRgbColor(0, 0, 255);  // Blue
        }
        
        delay(LINE_FLASH_DELAY);
      }
      
      // Update score
      gameState.score += POINTS_PER_LINE;
      tone(PIN_BUZZER, 800, 100);
      
      // Move lines down
      for (int moveY = y; moveY > 0; moveY--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
          grid[moveY][x] = grid[moveY - 1][x];
        }
      }
      
      // Clear top line
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[0][x] = 0;
      }
      
      y++; // Check this line again after moving down
    }
  }
  
  // Restore LED to current piece color after flashing
  if (linesCleared > 0) {
    setRgbColor(RGB_COLORS[currentPiece.color][0], 
                RGB_COLORS[currentPiece.color][1], 
                RGB_COLORS[currentPiece.color][2]);
  }
}

/**
 * Spawn a new piece at the top
 */
void spawnNewPiece() {
  currentPiece.type = random(7);
  currentPiece.color = currentPiece.type;
  currentPiece.rotation = 0;
  currentPiece.x = 3;
  currentPiece.y = 0;
  
  prevPiece.x = currentPiece.x;
  prevPiece.y = currentPiece.y;
  prevPiece.rotation = currentPiece.rotation;
  
  // Set RGB LED to match piece color
  setRgbColor(RGB_COLORS[currentPiece.color][0], 
              RGB_COLORS[currentPiece.color][1], 
              RGB_COLORS[currentPiece.color][2]);
  
  // Check for game over
  if (checkCollision(0, 0, 0)) {
    gameState.isOver = true;
  }
}

/**
 * Reset game to initial state
 */
void resetGame() {
  memset(grid, 0, sizeof(grid));
  gameState.score = 0;
  gameState.isOver = false;
  gameState.isPaused = false;
  gameState.moveDelay = NORMAL_MOVE_DELAY;
  
  tft.fillScreen(ST77XX_BLACK);
  drawBorder();
  spawnNewPiece();
  drawScore();
  drawControls();
}

/**
 * Handle game over state
 */
void handleGameOver() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(80, 100);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.println("GAME");
  tft.setCursor(80, 130);
  tft.println("OVER");
  tft.setTextSize(2);
  tft.setCursor(60, 170);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Score: ");
  tft.println(gameState.score);
  tft.setTextSize(1);
  tft.setCursor(50, 200);
  tft.println("Press B for New Game");
  
  setRgbColor(255, 0, 0);
  tone(PIN_BUZZER, 200, 1000);
  
  // Wait for B button to restart
  while (true) {
    if (isButtonPressed(PIN_B)) {
      delay(200); // Debounce
      resetGame();
      return;
    }
    delay(50);
  }
}

/**
 * Toggle pause state
 */
void togglePause() {
  gameState.isPaused = !gameState.isPaused;
  
  if (gameState.isPaused) {
    // Show pause overlay
    tft.fillRect(80, 100, 120, 40, ST77XX_BLACK);
    tft.drawRect(79, 99, 122, 42, ST77XX_WHITE);
    tft.setCursor(100, 110);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(2);
    tft.println("PAUSED");
    setRgbColor(255, 255, 0); // Yellow for pause
  } else {
    // Clear pause overlay and restore display
    tft.fillRect(79, 99, 122, 42, ST77XX_BLACK);
    drawBorder();
    drawGrid();
    drawCurrentPiece();
    drawControls();
    setRgbColor(RGB_COLORS[currentPiece.color][0], 
                RGB_COLORS[currentPiece.color][1], 
                RGB_COLORS[currentPiece.color][2]);
  }
}

/**
 * Process player input
 */
void processInput() {
  unsigned long currentTime = millis();
  
  if (currentTime - gameState.lastButtonTime < BUTTON_DELAY) {
    return;
  }
  
  bool leftPressed = isButtonPressed(PIN_LEFT);
  bool rightPressed = isButtonPressed(PIN_RIGHT);
  bool downPressed = isButtonPressed(PIN_DOWN);
  bool upPressed = isButtonPressed(PIN_UP);
  bool aPressed = isButtonPressed(PIN_A);
  bool bPressed = isButtonPressed(PIN_B);
  
  // A button toggles pause
  if (aPressed) {
    togglePause();
    gameState.lastButtonTime = currentTime;
    delay(PAUSE_DEBOUNCE);
    return;
  }
  
  // B button starts new game
  if (bPressed) {
    delay(200);
    resetGame();
    return;
  }
  
  // Don't process other inputs if paused
  if (gameState.isPaused) {
    return;
  }
  
  bool pieceMoved = false;
  
  // Move left
  if (leftPressed && !checkCollision(-1, 0, currentPiece.rotation)) {
    prevPiece.x = currentPiece.x;
    prevPiece.y = currentPiece.y;
    prevPiece.rotation = currentPiece.rotation;
    currentPiece.x--;
    tone(PIN_BUZZER, 400, 50);
    gameState.lastButtonTime = currentTime;
    pieceMoved = true;
  }
  
  // Move right
  if (rightPressed && !checkCollision(1, 0, currentPiece.rotation)) {
    prevPiece.x = currentPiece.x;
    prevPiece.y = currentPiece.y;
    prevPiece.rotation = currentPiece.rotation;
    currentPiece.x++;
    tone(PIN_BUZZER, 450, 50);
    gameState.lastButtonTime = currentTime;
    pieceMoved = true;
  }
  
  // Fast drop
  if (downPressed) {
    gameState.moveDelay = FAST_MOVE_DELAY;
  } else {
    gameState.moveDelay = NORMAL_MOVE_DELAY;
  }
  
  // Rotate
  if (upPressed) {
    int newRotation = (currentPiece.rotation + 1) % 4;
    if (!checkCollision(0, 0, newRotation)) {
      prevPiece.x = currentPiece.x;
      prevPiece.y = currentPiece.y;
      prevPiece.rotation = currentPiece.rotation;
      currentPiece.rotation = newRotation;
      tone(PIN_BUZZER, 500, 50);
      gameState.lastButtonTime = currentTime;
      pieceMoved = true;
    }
  }
  
  // Redraw if piece moved
  if (pieceMoved) {
    erasePreviousPiece();
    drawCurrentPiece();
  }
}

/**
 * Update game state (piece falling)
 */
void updateGame() {
  if (gameState.isPaused) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  if (currentTime - gameState.lastMoveTime > gameState.moveDelay) {
    if (!checkCollision(0, 1, currentPiece.rotation)) {
      // Move piece down
      prevPiece.x = currentPiece.x;
      prevPiece.y = currentPiece.y;
      prevPiece.rotation = currentPiece.rotation;
      currentPiece.y++;
      erasePreviousPiece();
      drawCurrentPiece();
    } else {
      // Piece has landed
      lockPiece();
      clearLines();
      spawnNewPiece();
      drawGrid();
      drawScore();
    }
    gameState.lastMoveTime = currentTime;
  }
}

// ============================================================================
// ARDUINO SETUP & LOOP
// ============================================================================

void setup() {
  Serial.begin(9600);
  Serial.println("Starting Tetris...");
  
  // Initialize display
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  tft.init(240, 320);
  tft.setRotation(1); // 320x240 landscape
  tft.fillScreen(ST77XX_BLACK);
  
  // Initialize RGB LED PWM
  analogWriteResolution(PIN_RED, PWM_RESOLUTION_BITS);
  analogWriteResolution(PIN_GREEN, PWM_RESOLUTION_BITS);
  analogWriteResolution(PIN_BLUE, PWM_RESOLUTION_BITS);
  
  // Initialize input pins
  pinMode(PIN_RIGHT, INPUT);
  pinMode(PIN_LEFT, INPUT);
  pinMode(PIN_DOWN, INPUT);
  pinMode(PIN_UP, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(PIN_A, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  
  // Seed random number generator
  randomSeed(analogRead(PIN_X));
  
  // Show splash screen
  tft.setCursor(60, 100);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("TETRIS");
  tft.setCursor(40, 140);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Arrows: Move/Rotate");
  tft.setCursor(40, 155);
  tft.println("Down: Drop faster");
  tft.setCursor(40, 170);
  tft.println("A: Pause  B: New Game");
  
  delay(3000);
  
  // Initialize game
  tft.fillScreen(ST77XX_BLACK);
  resetGame();
  
  Serial.println("Game initialized!");
}

void loop() {
  if (gameState.isOver) {
    handleGameOver();
  }
  
  processInput();
  updateGame();
  
  delay(10);
}