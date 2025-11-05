# NOTE: This program requires the following files from https://github.com/russhughes/st7789py_mpy
# st789py.py
# vga1_8x8.py

import machine
import st7789py as st7789
import vga1_8x8 as font
import utime
import random

# Pin definitions
TFT_SCLK = 36
TFT_MOSI = 35
TFT_MISO = 34
TFT_RST = 48
TFT_CS = 33
TFT_DC = 37
PIN_RIGHT = 13  # Analog button inputs
PIN_LEFT = 14
PIN_DOWN = 15
PIN_UP = 16
PIN_B = 17
PIN_A = 18
PIN_RED = 38
PIN_GREEN = 39
PIN_BLUE = 40
PIN_BUZZER = 42

# Game constants
GRID_WIDTH = 10
GRID_HEIGHT = 18
BLOCK_SIZE = 11
GRID_OFFSET_X = 50
GRID_OFFSET_Y = 15
BUTTON_THRESHOLD = 4095 // 3  # 1/3 of max ADC value
BUTTON_DELAY = 150  # ms
NORMAL_MOVE_DELAY = 500  # ms
FAST_MOVE_DELAY = 50  # ms
PAUSE_DEBOUNCE = 200  # ms
LINE_FLASH_DELAY = 50  # ms
LINE_FLASH_COUNT = 6
POINTS_PER_LINE = 100

# PWM settings for RGB LED
PWM_MAX_DUTY = 30

# Color definitions (RGB565, matching Arduino COLORS)
COLORS = [
    0x07FF,  # Cyan
    0xFFE0,  # Yellow
    0xF81F,  # Magenta
    0x07E0,  # Green
    0xF800,  # Red
    0x001F,  # Blue
    0xFD20   # Orange
]

# RGB LED colors (8-bit, matching Arduino RGB_COLORS)
RGB_COLORS = [
    (0, 255, 255),    # Cyan
    (255, 255, 0),    # Yellow
    (255, 0, 255),    # Magenta
    (0, 255, 0),      # Green
    (255, 0, 0),      # Red
    (0, 0, 255),      # Blue
    (255, 165, 0)     # Orange
]

# Tetromino shapes (matching Arduino SHAPES)
SHAPES = [
    # I
    [[[0,0,0,0], [1,1,1,1], [0,0,0,0], [0,0,0,0]],
     [[0,0,1,0], [0,0,1,0], [0,0,1,0], [0,0,1,0]],
     [[0,0,0,0], [0,0,0,0], [1,1,1,1], [0,0,0,0]],
     [[0,1,0,0], [0,1,0,0], [0,1,0,0], [0,1,0,0]]],
    # O
    [[[0,1,1,0], [0,1,1,0], [0,0,0,0], [0,0,0,0]],
     [[0,1,1,0], [0,1,1,0], [0,0,0,0], [0,0,0,0]],
     [[0,1,1,0], [0,1,1,0], [0,0,0,0], [0,0,0,0]],
     [[0,1,1,0], [0,1,1,0], [0,0,0,0], [0,0,0,0]]],
    # T
    [[[0,1,0,0], [1,1,1,0], [0,0,0,0], [0,0,0,0]],
     [[0,1,0,0], [0,1,1,0], [0,1,0,0], [0,0,0,0]],
     [[0,0,0,0], [1,1,1,0], [0,1,0,0], [0,0,0,0]],
     [[0,1,0,0], [1,1,0,0], [0,1,0,0], [0,0,0,0]]],
    # S
    [[[0,1,1,0], [1,1,0,0], [0,0,0,0], [0,0,0,0]],
     [[0,1,0,0], [0,1,1,0], [0,0,1,0], [0,0,0,0]],
     [[0,0,0,0], [0,1,1,0], [1,1,0,0], [0,0,0,0]],
     [[1,0,0,0], [1,1,0,0], [0,1,0,0], [0,0,0,0]]],
    # Z
    [[[1,1,0,0], [0,1,1,0], [0,0,0,0], [0,0,0,0]],
     [[0,0,1,0], [0,1,1,0], [0,1,0,0], [0,0,0,0]],
     [[0,0,0,0], [1,1,0,0], [0,1,1,0], [0,0,0,0]],
     [[0,1,0,0], [1,1,0,0], [1,0,0,0], [0,0,0,0]]],
    # J
    [[[1,0,0,0], [1,1,1,0], [0,0,0,0], [0,0,0,0]],
     [[0,1,1,0], [0,1,0,0], [0,1,0,0], [0,0,0,0]],
     [[0,0,0,0], [1,1,1,0], [0,0,1,0], [0,0,0,0]],
     [[0,1,0,0], [0,1,0,0], [1,1,0,0], [0,0,0,0]]],
    # L
    [[[0,0,1,0], [1,1,1,0], [0,0,0,0], [0,0,0,0]],
     [[0,1,0,0], [0,1,0,0], [0,1,1,0], [0,0,0,0]],
     [[0,0,0,0], [1,1,1,0], [1,0,0,0], [0,0,0,0]],
     [[1,1,0,0], [0,1,0,0], [0,1,0,0], [0,0,0,0]]]
]

# Initialize SPI
spi = machine.SPI(
    2,
    baudrate=60000000,
    polarity=1,
    phase=0,
    sck=machine.Pin(TFT_SCLK),
    mosi=machine.Pin(TFT_MOSI),
    miso=machine.Pin(TFT_MISO)
)

# Initialize display
tft = st7789.ST7789(
    spi,
    240, 320,
    reset=machine.Pin(TFT_RST, machine.Pin.OUT),
    cs=machine.Pin(TFT_CS, machine.Pin.OUT),
    dc=machine.Pin(TFT_DC, machine.Pin.OUT),
    rotation=3,  # Fixes upside-down text
    color_order=st7789.BGR  # Fix red/blue swap
)

# Initialize analog buttons (no pull-up needed for ADC)
adc_right = machine.ADC(machine.Pin(PIN_RIGHT))
adc_left = machine.ADC(machine.Pin(PIN_LEFT))
adc_down = machine.ADC(machine.Pin(PIN_DOWN))
adc_up = machine.ADC(machine.Pin(PIN_UP))
adc_a = machine.ADC(machine.Pin(PIN_A))
adc_b = machine.ADC(machine.Pin(PIN_B))

# Initialize RGB LED PWM
pwm_red = machine.PWM(machine.Pin(PIN_RED), freq=1000)
pwm_green = machine.PWM(machine.Pin(PIN_GREEN), freq=1000)
pwm_blue = machine.PWM(machine.Pin(PIN_BLUE), freq=1000)

# Initialize buzzer
buzzer = machine.PWM(machine.Pin(PIN_BUZZER), freq=1000, duty=0)

# Game state
grid = [[0] * GRID_WIDTH for _ in range(GRID_HEIGHT)]
game_state = {
    'last_move_time': 0,
    'last_button_time': 0,
    'move_delay': NORMAL_MOVE_DELAY,
    'score': 0,
    'is_over': False,
    'is_paused': False,
    'current_piece': {'type': 0, 'rotation': 0, 'x': 0, 'y': 0, 'color': 0},
    'prev_piece': {'x': 0, 'y': 0, 'rotation': 0}
}

def set_rgb_color(r, g, b):
    """Set RGB LED color (0-255 to PWM duty)."""
    duty_red = int((r / 255) * PWM_MAX_DUTY / 255 * 1023)
    duty_green = int((g / 255) * PWM_MAX_DUTY / 255 * 1023)
    duty_blue = int((b / 255) * PWM_MAX_DUTY / 255 * 1023)
    pwm_red.duty(duty_red)
    pwm_green.duty(duty_green)
    pwm_blue.duty(duty_blue)

def play_tone(freq, duration_ms):
    """Play a tone on the buzzer."""
    if freq > 0:
        buzzer.freq(freq)
        buzzer.duty(512)  # 50% duty cycle
    utime.sleep_ms(duration_ms)
    buzzer.duty(0)

def is_button_pressed(pin):
    """Check if analog button is pressed."""
    return pin.read() >= BUTTON_THRESHOLD

def draw_block(x, y, color_idx):
    """Draw a single block on the display."""
    if color_idx > 0:
        color = COLORS[color_idx - 1]
    else:
        color = st7789.BLACK
    tft.fill_rect(
        GRID_OFFSET_X + x * BLOCK_SIZE,
        GRID_OFFSET_Y + y * BLOCK_SIZE,
        BLOCK_SIZE - 1,
        BLOCK_SIZE - 1,
        color
    )

def draw_grid():
    """Draw the entire game grid."""
    for y in range(GRID_HEIGHT):
        for x in range(GRID_WIDTH):
            draw_block(x, y, grid[y][x])

def draw_current_piece():
    """Draw the current falling piece."""
    piece = SHAPES[game_state['current_piece']['type']][game_state['current_piece']['rotation']]
    x = game_state['current_piece']['x']
    y = game_state['current_piece']['y']
    color_idx = game_state['current_piece']['color'] + 1
    for py in range(4):
        for px in range(4):
            if piece[py][px]:
                grid_x = x + px
                grid_y = y + py
                if grid_y >= 0 and grid_y < GRID_HEIGHT and grid_x >= 0 and grid_x < GRID_WIDTH:
                    draw_block(grid_x, grid_y, color_idx)

def erase_previous_piece():
    """Erase the previous piece position."""
    piece = SHAPES[game_state['current_piece']['type']][game_state['prev_piece']['rotation']]
    x = game_state['prev_piece']['x']
    y = game_state['prev_piece']['y']
    for py in range(4):
        for px in range(4):
            if piece[py][px]:
                grid_x = x + px
                grid_y = y + py
                if grid_y >= 0 and grid_y < GRID_HEIGHT and grid_x >= 0 and grid_x < GRID_WIDTH:
                    draw_block(grid_x, grid_y, grid[grid_y][grid_x])

def draw_score():
    """Draw the score display."""
    tft.fill_rect(200, 10, 110, 30, st7789.BLACK)
    tft.text(font, "Score:", 200, 10, st7789.WHITE, st7789.BLACK)
    tft.text(font, str(game_state['score']), 200, 20, st7789.WHITE, st7789.BLACK)

def draw_controls():
    """Draw the control instructions."""
    tft.text(font, "Controls:", 200, 60, st7789.CYAN, st7789.BLACK)
    tft.text(font, "L/R: Move", 200, 75, st7789.WHITE, st7789.BLACK)
    tft.text(font, "Up: Rotate", 200, 90, st7789.WHITE, st7789.BLACK)
    tft.text(font, "Dn: Drop", 200, 105, st7789.WHITE, st7789.BLACK)
    tft.text(font, "A: Pause", 200, 120, st7789.WHITE, st7789.BLACK)
    tft.text(font, "B: New", 200, 135, st7789.WHITE, st7789.BLACK)

def draw_border():
    """Draw the game grid border."""
    tft.rect(
        GRID_OFFSET_X - 2,
        GRID_OFFSET_Y - 2,
        GRID_WIDTH * BLOCK_SIZE + 2,
        GRID_HEIGHT * BLOCK_SIZE + 2,
        st7789.WHITE
    )

def check_collision(offset_x, offset_y, rotation):
    """Check if piece collides with walls or other pieces."""
    piece = SHAPES[game_state['current_piece']['type']][rotation]
    x = game_state['current_piece']['x'] + offset_x
    y = game_state['current_piece']['y'] + offset_y
    for py in range(4):
        for px in range(4):
            if piece[py][px]:
                grid_x = x + px
                grid_y = y + py
                if grid_x < 0 or grid_x >= GRID_WIDTH or grid_y >= GRID_HEIGHT:
                    return True
                if grid_y >= 0 and grid[grid_y][grid_x]:
                    return True
    return False

def lock_piece():
    """Lock the current piece into the grid."""
    piece = SHAPES[game_state['current_piece']['type']][game_state['current_piece']['rotation']]
    x = game_state['current_piece']['x']
    y = game_state['current_piece']['y']
    color_idx = game_state['current_piece']['color'] + 1
    for py in range(4):
        for px in range(4):
            if piece[py][px]:
                grid_x = x + px
                grid_y = y + py
                if grid_y >= 0 and grid_y < GRID_HEIGHT and grid_x >= 0 and grid_x < GRID_WIDTH:
                    grid[grid_y][grid_x] = color_idx

def clear_lines():
    """Clear completed lines and update score."""
    global grid
    lines_cleared = 0
    y = GRID_HEIGHT - 1
    while y >= 0:
        if all(grid[y][x] > 0 for x in range(GRID_WIDTH)):
            lines_cleared += 1
            # Flash effect
            for flash in range(LINE_FLASH_COUNT):
                flash_color = st7789.GREEN if flash % 2 == 0 else st7789.BLUE
                for x in range(GRID_WIDTH):
                    draw_block(x, y, COLORS.index(flash_color) + 1 if flash_color in COLORS else 0)
                set_rgb_color(0, 255, 0) if flash % 2 == 0 else set_rgb_color(0, 0, 255)
                utime.sleep_ms(LINE_FLASH_DELAY)
            # Move lines down
            for move_y in range(y, 0, -1):
                for x in range(GRID_WIDTH):
                    grid[move_y][x] = grid[move_y - 1][x]
            for x in range(GRID_WIDTH):
                grid[0][x] = 0
            # Stay at the same y to check the new row
        else:
            y -= 1  # Move to next row only if no line was cleared
    if lines_cleared > 0:
        game_state['score'] += lines_cleared * POINTS_PER_LINE
        play_tone(800, 100)
        draw_score()  # Update score display

def spawn_new_piece():
    """Spawn a new random piece."""
    piece_type = random.randint(0, 6)
    game_state['current_piece'] = {
        'type': piece_type,
        'rotation': 0,
        'x': 3,
        'y': 0,
        'color': piece_type
    }
    game_state['prev_piece'] = {
        'x': 3,
        'y': 0,
        'rotation': 0
    }
    set_rgb_color(*RGB_COLORS[piece_type])
    if check_collision(0, 0, 0):
        game_state['is_over'] = True

def reset_game():
    """Reset the game state."""
    global grid
    grid = [[0] * GRID_WIDTH for _ in range(GRID_HEIGHT)]
    game_state['score'] = 0
    game_state['is_over'] = False
    game_state['is_paused'] = False
    game_state['move_delay'] = NORMAL_MOVE_DELAY
    tft.fill(st7789.BLACK)
    draw_border()
    draw_grid()  # Draw initial grid
    spawn_new_piece()
    draw_current_piece()  # Draw initial piece
    draw_score()
    draw_controls()

def handle_game_over():
    """Draw game over screen."""
    tft.fill(st7789.BLACK)
    tft.text(font, "GAME", 80, 100, st7789.RED, st7789.BLACK)
    tft.text(font, "OVER", 80, 120, st7789.RED, st7789.BLACK)
    tft.text(font, "Score: " + str(game_state['score']), 60, 160, st7789.WHITE, st7789.BLACK)
    tft.text(font, "Press B for New Game", 50, 180, st7789.WHITE, st7789.BLACK)
    set_rgb_color(255, 0, 0)
    play_tone(200, 1000)

def toggle_pause():
    """Toggle pause state."""
    game_state['is_paused'] = not game_state['is_paused']
    if game_state['is_paused']:
        tft.fill_rect(80, 100, 120, 40, st7789.BLACK)
        tft.rect(79, 99, 122, 42, st7789.WHITE)
        tft.text(font, "PAUSED", 100, 110, st7789.YELLOW, st7789.BLACK)
        set_rgb_color(255, 255, 0)
    else:
        tft.fill_rect(79, 99, 122, 42, st7789.BLACK)
        draw_border()
        draw_grid()
        draw_current_piece()
        draw_controls()
        set_rgb_color(*RGB_COLORS[game_state['current_piece']['color']])

def setup():
    """MicroPython equivalent of Arduino setup()."""
    print("Starting Tetris...")
    
    # Initialize display
    try:
        tft.init([])  # Use empty list for initialization
        print("Display initialized successfully")
    except TypeError as e:
        print("TypeError in tft.init:", e)
        raise
    
    # Clear the screen
    try:
        tft.fill(st7789.BLACK)
        print("Screen cleared successfully")
    except Exception as e:
        print("Error in tft.fill:", e)
        raise

    # Seed random number generator
    random.seed(utime.ticks_us())
    
    # Show splash screen
    try:
        tft.text(font, "TETRIS", 60, 100, st7789.GREEN, st7789.BLACK)
        tft.text(font, "Arrows: Move/Rotate", 40, 140, st7789.WHITE, st7789.BLACK)
        tft.text(font, "Down: Drop faster", 40, 155, st7789.WHITE, st7789.BLACK)
        tft.text(font, "A: Pause  B: New Game", 40, 170, st7789.WHITE, st7789.BLACK)
        print("Splash screen drawn successfully")
    except TypeError as e:
        print("TypeError in tft.text:", e)
        raise
    
    utime.sleep(3)
    reset_game()
    print("Game initialized!")

def loop():
    """Main game loop."""
    while True:
        if game_state['is_over']:
            handle_game_over()
            while True:
                if is_button_pressed(adc_b):
                    utime.sleep_ms(200)
                    reset_game()
                    break
                utime.sleep_ms(50)
            continue
        
        if game_state['is_paused']:
            if is_button_pressed(adc_a):
                toggle_pause()
                utime.sleep_ms(PAUSE_DEBOUNCE)
            utime.sleep_ms(50)
            continue
        
        current_time = utime.ticks_ms()
        if utime.ticks_diff(current_time, game_state['last_button_time']) >= BUTTON_DELAY:
            piece_moved = False
            if is_button_pressed(adc_left) and not check_collision(-1, 0, game_state['current_piece']['rotation']):
                game_state['prev_piece']['x'] = game_state['current_piece']['x']
                game_state['prev_piece']['y'] = game_state['current_piece']['y']
                game_state['prev_piece']['rotation'] = game_state['current_piece']['rotation']
                game_state['current_piece']['x'] -= 1
                play_tone(400, 50)
                game_state['last_button_time'] = current_time
                piece_moved = True
            if is_button_pressed(adc_right) and not check_collision(1, 0, game_state['current_piece']['rotation']):
                game_state['prev_piece']['x'] = game_state['current_piece']['x']
                game_state['prev_piece']['y'] = game_state['current_piece']['y']
                game_state['prev_piece']['rotation'] = game_state['current_piece']['rotation']
                game_state['current_piece']['x'] += 1
                play_tone(450, 50)
                game_state['last_button_time'] = current_time
                piece_moved = True
            if is_button_pressed(adc_down):
                game_state['move_delay'] = FAST_MOVE_DELAY
            else:
                game_state['move_delay'] = NORMAL_MOVE_DELAY
            if is_button_pressed(adc_up):
                new_rotation = (game_state['current_piece']['rotation'] + 1) % 4
                if not check_collision(0, 0, new_rotation):
                    game_state['prev_piece']['x'] = game_state['current_piece']['x']
                    game_state['prev_piece']['y'] = game_state['current_piece']['y']
                    game_state['prev_piece']['rotation'] = game_state['current_piece']['rotation']
                    game_state['current_piece']['rotation'] = new_rotation
                    play_tone(500, 50)
                    game_state['last_button_time'] = current_time
                    piece_moved = True
            if is_button_pressed(adc_a):
                toggle_pause()
                game_state['last_button_time'] = current_time
                utime.sleep_ms(PAUSE_DEBOUNCE)
            if is_button_pressed(adc_b):
                utime.sleep_ms(200)
                reset_game()
            if piece_moved:
                erase_previous_piece()
                draw_current_piece()
        
        if utime.ticks_diff(current_time, game_state['last_move_time']) >= game_state['move_delay']:
            if not check_collision(0, 1, game_state['current_piece']['rotation']):
                game_state['prev_piece']['x'] = game_state['current_piece']['x']
                game_state['prev_piece']['y'] = game_state['current_piece']['y']
                game_state['prev_piece']['rotation'] = game_state['current_piece']['rotation']
                game_state['current_piece']['y'] += 1
                erase_previous_piece()
                draw_current_piece()
            else:
                lock_piece()
                clear_lines()
                spawn_new_piece()
                draw_grid()  # Redraw grid after piece lock or line clear
                draw_current_piece()  # Draw new piece
            game_state['last_move_time'] = current_time
        
        utime.sleep_ms(10)

try:
    setup()
    loop()
except Exception as e:
    print("Error in main:", e)
    raise