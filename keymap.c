#include QMK_KEYBOARD_H
#include "debug.h"
#include "action_layer.h"
#include "version.h"
#include "layers.h"

// Timer to detect tap/hold on NEO_RMOD3 key
static uint16_t neo3_timer;
// State bitmap to track which key(s) enabled NEO_3 layer
static uint8_t neo3_state = 0;
// State bitmap to track key combo for CAPSLOCK
static uint8_t capslock_state = 0;

// bitmasks for modifier keys
#define MODS_NONE   0
#define MODS_SHIFT  (MOD_BIT(KC_LSHIFT)|MOD_BIT(KC_RSHIFT))
#define MODS_CTRL   (MOD_BIT(KC_LCTL)|MOD_BIT(KC_RCTRL))
#define MODS_ALT    (MOD_BIT(KC_LALT)|MOD_BIT(KC_RALT))
#define MODS_GUI    (MOD_BIT(KC_LGUI)|MOD_BIT(KC_RGUI))

// Used to trigger macros / sequences of keypresses
enum custom_keycodes {
  PLACEHOLDER = SAFE_RANGE,     // can always be here
  US_OSX_SMALL_UE,
  US_OSX_SMALL_AE,
  US_OSX_SMALL_OE,
  US_OSX_CAPITAL_UE,
  US_OSX_CAPITAL_AE,
  US_OSX_CAPITAL_OE,
  NEO2_LMOD3,
  NEO2_RMOD3,
  NEO2_1,
  NEO2_2,
  NEO2_3,
  NEO2_4,
  NEO2_5,
  NEO2_6,
  NEO2_7,
  NEO2_8,
  NEO2_9,
  NEO2_0,
  NEO2_MINUS,
  NEO2_UE,
  NEO2_AE,
  NEO2_OE,
  NEO2_COMMA,
  NEO2_DOT,
  NEO2_SHARP_S
};

#define NEO2_LMOD4                  TT(NEO_4)
#define NEO2_RMOD4                  NEO2_LMOD4

// Use _______ to indicate a key that is transparent / falling through to a lower level
#define _______ KC_TRNS

// NEO_3 special characters
#define US_OSX_SUPERSCRIPT_1        KC_NO                       // ¹
#define US_OSX_SUPERSCRIPT_2        KC_NO                       // ²
#define US_OSX_SUPERSCRIPT_3        KC_NO                       // ³
#define US_OSX_RSAQUO               LALT(LSFT(KC_4))            // ›
#define US_OSX_LSAQUO               LALT(LSFT(KC_3))            // ‹
#define US_OSX_CENT                 LALT(KC_4)                  // ¢
#define US_OSX_YEN                  LALT(KC_Y)                  // ¥
#define US_OSX_SBQUO                LALT(LSFT(KC_0))            // ‚
#define US_OSX_LEFT_SINGLE_QUOTE    LALT(KC_RBRACKET)           // ‘
#define US_OSX_RIGHT_SINGLE_QUOTE   LALT(LSFT(KC_RBRACKET))     // ’
#define US_OSX_ELLIPSIS             LALT(KC_SCOLON)             // …
#define US_OSX_UNDERSCORE           LSFT(KC_MINUS)              // _
#define US_OSX_LBRACKET             KC_LBRACKET                 // [
#define US_OSX_RBRACKET             KC_RBRACKET                 // ]
#define US_OSX_CIRCUMFLEX           LSFT(KC_6)                  // ^
#define US_OSX_EXCLAMATION          LSFT(KC_1)                  // !
#define US_OSX_LESSTHAN             LSFT(KC_COMMA)              // <
#define US_OSX_GREATERTHAN          LSFT(KC_DOT)                // >
#define US_OSX_EQUAL                KC_EQUAL                    // =
#define US_OSX_AMPERSAND            LSFT(KC_7)                  // &
#define US_OSX_SMALL_LONG_S         KC_NO                       // ſ
#define US_OSX_BSLASH               KC_BSLASH
#define US_OSX_SLASH                KC_SLASH                    // /
#define US_OSX_CLBRACKET            LSFT(KC_LBRACKET)           // {
#define US_OSX_CRBRACKET            LSFT(KC_RBRACKET)           // }
#define US_OSX_ASTERISK             LSFT(KC_8)                  // *
#define US_OSX_QUESTIONMARK         LSFT(KC_SLASH)              // ?
#define US_OSX_LPARENTHESES         LSFT(KC_9)                  // (
#define US_OSX_RPARENTHESES         LSFT(KC_0)                  // )
#define US_OSX_HYPHEN_MINUS         KC_MINUS                    // -
#define US_OSX_COLON                LSFT(KC_SCOLON)             // :
#define US_OSX_AT                   LSFT(KC_2)                  // @
#define US_OSX_HASH                 LSFT(KC_3)                  // #
#define US_OSX_PIPE                 LSFT(KC_BSLASH)             // |
#define US_OSX_TILDE                LSFT(KC_GRAVE)              // ~
#define US_OSX_BACKTICK             KC_GRAVE                    // `
#define US_OSX_PLUS                 LSFT(KC_EQUAL)              // +
#define US_OSX_PERCENT              LSFT(KC_5)                  // %
#define US_OSX_DOUBLE_QUOTE         LSFT(KC_QUOTE)              // "
#define US_OSX_SINGLE_QUOTE         KC_QUOTE                    // '
#define US_OSX_SEMICOLON            KC_SCOLON                   // ;

// NEO_4 special characters
#define US_OSX_FEMININE_ORDINAL     LALT(KC_9)                  // ª
#define US_OSX_MASCULINE_ORDINAL    LALT(KC_0)                  // º
#define US_OSX_NUMERO_SIGN          KC_NO                       // №
#define US_OSX_MIDDLE_DOT           LALT(LSFT(KC_9))            // ·
#define US_OSX_BRITISH_POUND        LALT(KC_3)                  // £
#define US_OSX_CURRENCY_SIGN        KC_NO                       // ¤
#define US_OSX_INV_EXCLAMATION      LALT(KC_1)                  // ¡
#define US_OSX_INV_QUESTIONMARK     LALT(LSFT(KC_SLASH))        // ¿
#define US_OSX_DOLLAR               KC_DOLLAR                   // $
#define US_OSX_EM_DASH              LALT(LSFT(KC_MINUS))        // —

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  /* NEO_1: Basic layer
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  |  1/° |  2/§ |  3/  |  4/» |  5/« |  ESC |           | US_1 |  6/$ |  7/€ |  8/„ |  9/“ |  0/” |  -/—   |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  TAB   |   X  |   V  |   L  |   C  |   W  | LCTL |           | RCTL |   K  |   H  |   G  |   F  |   Q  |   ß    |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |  NEO_3 |   U  |   I  |   A  |   E  |   O  |------|           |------|   S  |   N  |   R  |   T  |   D  |   Y    |
   * |--------+------+------+------+------+------| LALT |           | RALT |------+------+------+------+------+--------|
   * | LSHIFT |   Ü  |   Ö  |   Ä  |   P  |   Z  |      |           |      |   B  |   M  |  ,/– |  ./• |   J  | RSHIFT |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   | ---- | ---- | LCTL | LALT | LGUI |                                       | RGUI | Left | Down |  Up  | Right|
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        | FKEYS| Home |       | PgUp | FKEYS|
   *                                 ,------|------|------|       |------+------+------.
   *                                 | Back-|      | End  |       | PgDn |      |      |
   *                                 | space|Delete|------|       |------| Enter|Space |
   *                                 |      |      | NEO_4|       | NEO_4|      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_1] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */, NEO2_1,                   NEO2_2,                   NEO2_3,                   NEO2_4,           NEO2_5,           KC_ESCAPE,
    KC_TAB,           KC_X,                     KC_V,                     KC_L,                     KC_C,             KC_W,             KC_LCTRL,
    NEO2_LMOD3,       KC_U,                     KC_I,                     KC_A,                     KC_E,             KC_O,             /* --- */
    KC_LSHIFT,        NEO2_UE,                  NEO2_OE,                  NEO2_AE,                  KC_P,             KC_Z,             KC_LALT,
    KC_NO /* NOOP */, KC_NO /* NOOP */,         KC_LCTRL,                 KC_LALT,                  KC_LGUI,          /* --- */         /* --- */

    // left hand side - thumb cluster
    /* --- */         MO(FKEYS),        KC_HOME,
    /* KC_BSPACE */   /* KC_DELETE */   KC_END,
    KC_BSPACE,        KC_DELETE,        NEO2_LMOD4,

    // right hand side - main
    TO(US_1),         NEO2_6,           NEO2_7,           NEO2_8,           NEO2_9,           NEO2_0,           NEO2_MINUS,
    KC_RCTRL,         KC_K,             KC_H,             KC_G,             KC_F,             KC_Q,             NEO2_SHARP_S,
    /* --- */         KC_S,             KC_N,             KC_R,             KC_T,             KC_D,             NEO2_RMOD3,
    KC_RALT,          KC_B,             KC_M,             NEO2_COMMA,       NEO2_DOT,         KC_J,             KC_RSHIFT,
    /* --- */         /* --- */         KC_RGUI,          KC_LEFT,          KC_DOWN,          KC_UP,            KC_RIGHT,

    // right hand side - thumb cluster
    KC_PGUP,          MO(FKEYS),        /* --- */
    KC_PGDOWN,        /* --- */         /* --- */
    NEO2_RMOD4,       KC_ENTER,         KC_SPACE
  ),

  /* NEO_3: Symbol layer
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  | ---- | ---- | ---- |   ›  |   ‹  |      |           |      |   ¢ 	|   ¥  |   ‚  |   ‘  |   ’  |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  |   …  |   _  |   [  |   ]  |   ^  |      |           |      |   !  |   <  |   >  |   =  |   &  |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        |   \  |   /  |   {  |   }  |   *  |------|           |------|   ?  |   (  |   )  |   -  |   :  |   @    |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        |   #  |   $  |   |  |   ~  |   `  |      |           |      |   +  |   %  |   "  |   '  |   ;  |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_3] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,     KC_NO /* NOOP */,     US_OSX_RSAQUO,            US_OSX_LSAQUO,                _______,
    KC_NO /* NOOP */,   US_OSX_ELLIPSIS,      US_OSX_UNDERSCORE,    US_OSX_LBRACKET,      US_OSX_RBRACKET,          US_OSX_CIRCUMFLEX,            _______,
    _______,            US_OSX_BSLASH,        US_OSX_SLASH,         US_OSX_CLBRACKET,     US_OSX_CRBRACKET,         US_OSX_ASTERISK,              /* --- */
    _______,            US_OSX_HASH,          US_OSX_DOLLAR,        US_OSX_PIPE,          US_OSX_TILDE,             US_OSX_BACKTICK,              _______,
    _______,            _______,              _______,              _______,              _______,                  /* --- */                     /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,              _______,
    /* --- */           /* --- */             _______,
    _______,            _______,              _______,

    // right hand side - main
    _______,            US_OSX_CENT,          US_OSX_YEN,           US_OSX_SBQUO,         US_OSX_LEFT_SINGLE_QUOTE,  US_OSX_RIGHT_SINGLE_QUOTE,   KC_NO,
    _______,            US_OSX_EXCLAMATION,   US_OSX_LESSTHAN,      US_OSX_GREATERTHAN,   US_OSX_EQUAL,              US_OSX_AMPERSAND,            US_OSX_SMALL_LONG_S,
    /* --- */           US_OSX_QUESTIONMARK,  US_OSX_LPARENTHESES,  US_OSX_RPARENTHESES,  US_OSX_HYPHEN_MINUS,       US_OSX_COLON,                NEO2_RMOD3,
    _______,            US_OSX_PLUS,          US_OSX_PERCENT,       US_OSX_DOUBLE_QUOTE,  US_OSX_SINGLE_QUOTE,       US_OSX_SEMICOLON,            _______,
    /* --- */           /* --- */             _______,              _______,              _______,                   _______,                     _______,

    // right hand side - thumb cluster
    _______,            _______,              /* --- */
    _______,            /* --- */             /* --- */
    _______,            _______,              _______
  ),

  /* NEO_4: Cursor & Numpad
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  |   ª  |   º  | ---- |   ·  |   £  |      |           |      | ---- | Tab  |   /  |   *  |   -  |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  | PgUp |   ⌫  |  Up  |   ⌦  | PgDn |      |           |      |   ¡  |   7  |   8  |   9  |   +  |   –    |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | Home | Left | Down | Right| End  |------|           |------|   ¿  |   4  |   5  |   6  |   ,  |   .    |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | Esc  | Tab  | Ins  |Return| ---- |      |           |      |   :  |   1  |   2  |   3  |   ;  |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |   0  |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_4] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   US_OSX_FEMININE_ORDINAL,  US_OSX_MASCULINE_ORDINAL, KC_NO /* NOOP */,     US_OSX_MIDDLE_DOT,  US_OSX_BRITISH_POUND, _______,
    _______,            KC_PGUP,                  KC_BSPACE,                KC_UP,                KC_DELETE,          KC_PGDOWN,            _______,
    _______,            KC_HOME,                  KC_LEFT,                  KC_DOWN,              KC_RIGHT,           KC_END,               /* --- */
    _______,            KC_ESCAPE,                KC_TAB,                   KC_INSERT,            KC_ENTER,           KC_NO /* NOOP */,     _______,
    _______,            _______,                  _______,                  _______,              _______,            /* --- */             /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,                  _______,
    /* --- */           /* --- */                 _______,
    _______,            _______,                  _______,

    // right hand side - main
    _______,            US_OSX_CURRENCY_SIGN,     KC_TAB,                   KC_KP_SLASH,          KC_KP_ASTERISK,     KC_KP_MINUS,          KC_NO /* NOOP */,
    _______,            US_OSX_INV_EXCLAMATION,   KC_KP_7,                  KC_KP_8,              KC_KP_9,            KC_KP_PLUS,           US_OSX_EM_DASH,
    /* --- */           US_OSX_INV_QUESTIONMARK,  KC_KP_4,                  KC_KP_5,              KC_KP_6,            KC_KP_COMMA,          KC_KP_DOT,
    _______,            US_OSX_COLON,             KC_KP_1,                  KC_KP_2,              KC_KP_3,            US_OSX_SEMICOLON,     _______,
    /* --- */           /* --- */                 _______,                  KC_KP_0,              _______,            _______,              _______,

    // right hand side - thumb cluster
    _______,            _______,                  /* --- */
    _______,            /* --- */                 /* --- */
    _______,            _______,                  _______
  ),

  /* NEO_5: Greek
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |------|           |------| ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_5] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   /* --- */
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            _______,            _______,            _______,              _______,            /* --- */           /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,            _______,
    /* --- */           /* --- */           _______,
    _______,            _______,            _______,

    // right hand side - main
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    /* --- */           KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    /* --- */           /* --- */           _______,            _______,              _______,            _______,            _______,

    // right hand side - thumb cluster
    _______,            _______,            /* --- */
    _______,            /* --- */           /* --- */
    _______,            _______,            _______
  ),

  /* NEO_6: Math symbols
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |------|           |------| ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_6] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   /* --- */
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            _______,            _______,            _______,              _______,            /* --- */           /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,            _______,
    /* --- */           /* --- */           _______,
    _______,            _______,            _______,

    // right hand side - main
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    /* --- */           KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    /* --- */           /* --- */           _______,            _______,              _______,            _______,            _______,

    // right hand side - thumb cluster
    _______,            _______,            /* --- */
    _______,            /* --- */           /* --- */
    _______,            _______,            _______
  ),

  /* US_1: US QWERTY
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |   =    |   1  |   2  |   3  |   4  |   5  | ESC  |           | NEO_1|   6  |   7  |   8  |   9  |   0  |    -   |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |   \    |   Q  |   W  |   E  |   R  |   T  | ---- |           |   [  |   Y  |   U  |   I  |   O  |   P  |    ]   |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |  TAB   |   A  |   S  |   D  |   F  |   G  |------|           |------|   H  |   J  |   K  |   L  |   ;  |    '   |
   * |--------+------+------+------+------+------| ---- |           | ---- |------+------+------+------+------+--------|
   * | LSHIFT |   Z  |   X  |   C  |   V  |   B  |      |           |      |   N  |   M  |   ,  |   .  |   /  | RSHIFT |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   | LGUI |   `  | ---- | ---- | FKEYS|                                       | Left | Down |  Up  | Right| RGUI |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        | LCTRL| LALT |       | RALT | RCTRL|
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      | HOME |       | PGUP |      |      |
   *                                 | BKSP | DEL  |------|       |------| ENTR | SPCE |
   *                                 |      |      | END  |       | PGDN |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [US_1] = LAYOUT_ergodox(
    // left hand side - main
    KC_EQUAL,         KC_1,         KC_2,       KC_3,       KC_4,       KC_5,       KC_ESCAPE,
    KC_BSLASH,        KC_Q,         KC_W,       KC_E,       KC_R,       KC_T,       KC_NO /* NOOP */,
    KC_TAB,           KC_A,         KC_S,       KC_D,       KC_F,       KC_G,       /* --- */
    KC_LSHIFT,        KC_Z,         KC_X,       KC_C,       KC_V,       KC_B,       KC_NO /* NOOP */,
    KC_LGUI,          KC_GRAVE,     KC_NO,      KC_NO,      MO(FKEYS),  /* --- */   /* --- */

    // left hand side - thumb cluster
    /* --- */         KC_LCTRL,     KC_LALT,
    /* --- */         /* --- */     KC_HOME,
    KC_BSPACE,        KC_DELETE,    KC_END,

    // right hand side - main
    TO(NEO_1),        KC_6,         KC_7,       KC_8,       KC_9,       KC_0,       KC_MINUS,
    KC_LBRACKET,      KC_Y,         KC_U,       KC_I,       KC_O,       KC_P,       KC_RBRACKET,
    /* --- */         KC_H,         KC_J,       KC_K,       KC_L,       KC_SCOLON,  KC_QUOTE,
    KC_NO /* NOOP */, KC_N,         KC_M,       KC_COMMA,   KC_DOT,     KC_SLASH,   KC_RSHIFT,
    /* --- */         /* --- */     KC_LEFT,    KC_DOWN,    KC_UP,      KC_RIGHT,   KC_RGUI,

    // right hand side - thumb cluster
    KC_RALT,          KC_RCTRL,     /* --- */
    KC_PGUP,          /* --- */     /* --- */
    KC_PGDOWN,        KC_ENTER,     KC_SPACE
  ),

  /* FKEYS: Function keys
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  Prev  |  F1  |  F2  |  F3  |  F4  |  F5  |  F11 |           |  F12 |  F6  |  F7  |  F8  |  F9  |  F10 |  VolUp |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  Play  |      |      |      |      |      |      |           |      |      |      |      |      |      |  VolDn |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |  Next  |      |      |      |      |      |------|           |------|      |      |      |      |      |  Mute  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [FKEYS] = LAYOUT_ergodox(
    // left hand side - main
    KC_MEDIA_REWIND,        KC_F1,              KC_F2,              KC_F3,                KC_F4,              KC_F5,              KC_F11,
    KC_MEDIA_PLAY_PAUSE,    _______,            _______,            _______,              _______,            _______,            _______,
    KC_MEDIA_FAST_FORWARD,  _______,            _______,            _______,              _______,            _______,            /* --- */
    _______,                _______,            _______,            _______,              _______,            _______,            _______,
    _______,                _______,            _______,            _______,              _______,            /* --- */           /* --- */

    // left hand side - thumb cluster
    /* --- */               _______,            _______,
    /* --- */               /* --- */           _______,
    _______,                _______,            _______,

    // right hand side - main
    KC_F12,                 KC_F6,              KC_F7,              KC_F8,                KC_F9,              KC_F10,             KC_AUDIO_VOL_UP,
    _______,                _______,            _______,            _______,              _______,            _______,            KC_AUDIO_VOL_DOWN,
    /* --- */               _______,            _______,            _______,              _______,            _______,            KC_AUDIO_MUTE,
    _______,                _______,            _______,            _______,              _______,            _______,            _______,
    /* --- */               /* --- */           _______,            _______,              _______,            _______,            _______,

    // right hand side - thumb cluster
    _______,                _______,            /* --- */
    _______,                /* --- */           /* --- */
    _______,                _______,            _______
  ),
};

// Send a key tap with a optional set of modifiers.
void tap_with_modifiers(uint16_t keycode, uint8_t force_modifiers) {
  uint8_t active_modifiers = get_mods();

  if ((force_modifiers & MODS_SHIFT) && !(active_modifiers & MODS_SHIFT)) register_code(KC_LSFT);
  if ((force_modifiers & MODS_CTRL) && !(active_modifiers & MODS_CTRL)) register_code(KC_LCTRL);
  if ((force_modifiers & MODS_ALT) && !(active_modifiers & MODS_ALT)) register_code(KC_LALT);
  if ((force_modifiers & MODS_GUI) && !(active_modifiers & MODS_GUI)) register_code(KC_LGUI);

  register_code(keycode);
  unregister_code(keycode);

  if ((force_modifiers & MODS_SHIFT) && !(active_modifiers & MODS_SHIFT)) unregister_code(KC_LSFT);
  if ((force_modifiers & MODS_CTRL) && !(active_modifiers & MODS_CTRL)) unregister_code(KC_LCTRL);
  if ((force_modifiers & MODS_ALT) && !(active_modifiers & MODS_ALT)) unregister_code(KC_LALT);
  if ((force_modifiers & MODS_GUI) && !(active_modifiers & MODS_GUI)) unregister_code(KC_LGUI);
}

// Special remapping for keys with different keycodes/macros when used with shift modifiers.
bool process_record_user_shifted(uint16_t keycode, keyrecord_t *record) {
  uint8_t active_modifiers = get_mods();
  uint8_t shifted = active_modifiers & MODS_SHIFT;

  // Early return on key release
  if(!record->event.pressed) {
    return true;
  }

  if(shifted) {
    clear_mods();

    switch(keycode) {
      case NEO2_1:
        // degree symbol
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_8) SS_UP(X_LSHIFT) SS_UP(X_LALT));
        break;
      case NEO2_2:
        // section symbol
        SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_6) SS_UP(X_LALT));
        break;
      case NEO2_3:
        // There is no OSX key combination for the script small l character
        break;
      case NEO2_4:
        // right angled quote
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_BSLASH) SS_UP(X_LSHIFT) SS_UP(X_LALT));
        break;
      case NEO2_5:
        // left angled quote
        SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_BSLASH) SS_UP(X_LALT));
        break;
      case NEO2_6:
        // dollar sign
        SEND_STRING(SS_DOWN(X_LSHIFT) SS_TAP(X_4) SS_UP(X_LSHIFT));
        break;
      case NEO2_7:
        // euro sign
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_2) SS_UP(X_LSHIFT) SS_UP(X_LALT));
        break;
      case NEO2_8:
        // low9 double quote
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_W) SS_UP(X_LSHIFT) SS_UP(X_LALT));
        break;
      case NEO2_9:
        // left double quote
        SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_LBRACKET) SS_UP(X_LALT));
        break;
      case NEO2_0:
        // right double quote
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_LBRACKET) SS_UP(X_LSHIFT) SS_UP(X_LALT));
        break;
      case NEO2_MINUS:
        // em dash
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_MINUS) SS_UP(X_LSHIFT) SS_UP(X_LALT));
        break;
      case NEO2_COMMA:
        // en dash
        SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_MINUS) SS_UP(X_LALT));
        break;
      case NEO2_DOT:
        // bullet
        SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_8) SS_UP(X_LALT));
        break;
      case NEO2_SHARP_S:
        // german sharp s
        SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_S) SS_UP(X_LALT));
        break;
      case NEO2_UE:
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_U) SS_UP(X_U) SS_UP(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_U) SS_UP(X_LSHIFT));
        break;
      case NEO2_OE:
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_U) SS_UP(X_U) SS_UP(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_O) SS_UP(X_LSHIFT));
        break;
      case NEO2_AE:
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_U) SS_UP(X_U) SS_UP(X_LALT) SS_DOWN(X_LSHIFT) SS_TAP(X_A) SS_UP(X_LSHIFT));
        break;
      default:
        set_mods(active_modifiers);
        return true;
    }

    set_mods(active_modifiers);
    return false;
  } else {
    switch(keycode) {
      case NEO2_1:
        SEND_STRING(SS_TAP(X_1));
        break;
      case NEO2_2:
        SEND_STRING(SS_TAP(X_2));
        break;
      case NEO2_3:
        SEND_STRING(SS_TAP(X_3));
        break;
      case NEO2_4:
        SEND_STRING(SS_TAP(X_4));
        break;
      case NEO2_5:
        SEND_STRING(SS_TAP(X_5));
        break;
      case NEO2_6:
        SEND_STRING(SS_TAP(X_6));
        break;
      case NEO2_7:
        SEND_STRING(SS_TAP(X_7));
        break;
      case NEO2_8:
        SEND_STRING(SS_TAP(X_8));
        break;
      case NEO2_9:
        SEND_STRING(SS_TAP(X_9));
        break;
      case NEO2_0:
        SEND_STRING(SS_TAP(X_0));
        break;
      case NEO2_MINUS:
        SEND_STRING(SS_TAP(X_MINUS));
        break;
      case NEO2_COMMA:
        SEND_STRING(SS_TAP(X_COMMA));
        break;
      case NEO2_DOT:
        SEND_STRING(SS_TAP(X_DOT));
        break;
      case NEO2_SHARP_S:
        // german sharp s
        SEND_STRING(SS_DOWN(X_LALT) SS_TAP(X_S) SS_UP(X_LALT));
        break;
      case NEO2_UE:
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_U) SS_UP(X_U) SS_UP(X_LALT) SS_TAP(X_U));
        break;
      case NEO2_OE:
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_U) SS_UP(X_U) SS_UP(X_LALT) SS_TAP(X_O));
        break;
      case NEO2_AE:
        SEND_STRING(SS_DOWN(X_LALT) SS_DOWN(X_U) SS_UP(X_U) SS_UP(X_LALT) SS_TAP(X_A));
        break;
      default:
        return true;
    }

    return false;
  }
}

// Runs for each key down or up event.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch(keycode) {
    case KC_LSHIFT:
      if (record->event.pressed) {
        capslock_state |= (MOD_BIT(KC_LSHIFT));
      } else {
        capslock_state &= ~(MOD_BIT(KC_LSHIFT));
      }
      break;
    case KC_RSHIFT:
      if (record->event.pressed) {
        capslock_state |= MOD_BIT(KC_RSHIFT);
      } else {
        capslock_state &= ~(MOD_BIT(KC_RSHIFT));
      }
      break;
    case NEO2_LMOD3:
      if (record->event.pressed) {
        layer_on(NEO_3);
        neo3_state |= (1 << 1);
      } else {
        // Turn off NEO_3 layer unless it's enabled through NEO2_RMOD3 as well.
        if ((neo3_state & ~(1 << 1)) == 0) {
          layer_off(NEO_3);
        }
        neo3_state &= ~(1 << 1);
      }
      break;
    case NEO2_RMOD3:
      if (record->event.pressed) {
        neo3_timer = timer_read();
        neo3_state |= (1 << 2);
        layer_on(NEO_3);
      } else {
        // Turn off NEO_3 layer unless it's enabled through NEO2_LMOD3 as well.
        if ((neo3_state & ~(1 << 2)) == 0) {
          layer_off(NEO_3);
        }
        neo3_state &= ~(1 << 2);

        // Was the NEO2_RMOD3 key TAPPED?
        if (timer_elapsed(neo3_timer) <= 150) {
          if (neo3_state > 0) {
            // We are still in NEO_3 layer, send keycode and modifiers for @
            tap_with_modifiers(KC_2, MODS_SHIFT);
            return false;
          } else {
            // Do the normal key processing, send y
            tap_with_modifiers(KC_Y, MODS_NONE);
            return false;
          }
        }
      }
      break;
  }

  if ((capslock_state & MODS_SHIFT) == MODS_SHIFT) {
    // CAPSLOCK is currently active, disable it
    if (host_keyboard_leds() & (1<<USB_LED_CAPS_LOCK)) {
      unregister_code(KC_LOCKING_CAPS);
    } else {
      register_code(KC_LOCKING_CAPS);
    }
    return false;
  }

  return process_record_user_shifted(keycode, record);
};


// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {

};


// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {
    uint8_t layer = biton32(layer_state);

    ergodox_board_led_off();
    ergodox_right_led_1_off();
    ergodox_right_led_2_off();
    ergodox_right_led_3_off();
    switch (layer) {
      // TODO: Make this relevant to the ErgoDox EZ.
        case 1:
            ergodox_right_led_1_on();
            break;
        case 2:
            ergodox_right_led_2_on();
            break;
        default:
            // none
            break;
    }
};
