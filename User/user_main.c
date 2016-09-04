//
// Created by MightyPork on 2016/9/2
//

#include <common.h>
#include <string.h>
#include "init.h"
#include "utils/ws2812.h"
#include "utils/timebase.h"
#include "utils/debug.h"
#include "user_main.h"

//region Colors

#define C_DARK rgb(0,0,0)
#define C_DIMWHITE rgb(15,15,15)
#define C_OKGREEN rgb(5,40,0)
#define C_CRIMSON rgb(140,0,3)

#define C_DIMRED rgb(20,0,0)
#define C_DIMGREEN rgb(0,20,0)
#define C_DIMBLUE rgb(0,0,35)
#define C_DIMYELLOW rgb(15,10,0)

#define C_BRTRED rgb(120,0,0)
#define C_BRTGREEN rgb(2,80,0)
#define C_BRTBLUE rgb(0,0,180)
#define C_BRTYELLOW rgb(70,60,0)

// assign to positions

#define C_DIM1 C_DIMRED
#define C_DIM2 C_DIMGREEN
#define C_DIM3 C_DIMBLUE
#define C_DIM4 C_DIMYELLOW

#define C_BRT1 C_BRTRED
#define C_BRT2 C_BRTGREEN
#define C_BRT3 C_BRTBLUE
#define C_BRT4 C_BRTYELLOW

//endregion

/** Current game state */
enum GameState_enum {
	STATE_NEW_GAME, // new game, waiting for key
	STATE_REPLAY, // showing sequence
	STATE_USER_INPUT, // waiting for user input of repeated sequence
	STATE_SUCCESS_EFFECT, // entered OK, show some fireworks
	STATE_FAIL_EFFECT, // entered wrong, show FAIL animation, then reset.
} GameState = STATE_NEW_GAME;

/** Screen colors */
uint32_t screen[4] = {0, 0, 0, 0};
const uint32_t brt[4] = {C_BRT1, C_BRT2, C_BRT3, C_BRT4};
const uint32_t dim[4] = {C_DIM1, C_DIM2, C_DIM3, C_DIM4};
const uint32_t dark[4] = {C_DARK, C_DARK, C_DARK, C_DARK};

#define REPLAY_INTERVAL 400
#define REPLAY_INTERVAL_GAP 75
#define SUC_EFF_TIME 500
#define FAIL_EFF_TIME 1000

/** Sequence of colors to show. Seed is constant thorough a game.
 * rng_state is used by rand_r() for building the sequence. */
uint32_t game_seed;
unsigned int game_rng_state;

/** Nr of revealed colors in sequence */
uint32_t game_revealed_n;
/** Nr of next color to replay/input */
uint32_t game_replay_n;
/** Nr of succ repeated colors */
uint32_t game_repeat_n;

void enter_state(enum GameState_enum state);

/** Show current screen colors */
void show_screen()
{
	ws2812_send(WSDATA_GPIO_Port, WSDATA_Pin, screen, 4);
}

/** Prepare rng sequence for replay / test */
void reset_sequence()
{
	game_rng_state = game_seed;
}

/** Get next item in the sequence */
uint32_t get_next_item()
{
	return (uint32_t) rand_r(&game_rng_state) & 0x03;
}

/** Enter state - callback for delayed state change */
void deferred_enter_state(void *state)
{
	enter_state((enum GameState_enum) state);
}

/** Future task CB in replay seq */
void replay_callback(void *onOff)
{
	bool on = (bool) onOff;

	screen[0] = C_DARK;
	screen[1] = C_DARK;
	screen[2] = C_DARK;
	screen[3] = C_DARK;

	if (on) {
		uint32_t color = get_next_item();
		game_replay_n++;
		screen[color] = brt[color];
		show_screen();
		schedule_task(replay_callback, (void *) 0, REPLAY_INTERVAL, false);
	} else {
		// turning off
		show_screen();

		// Schedule next turning ON
		if (game_replay_n < game_revealed_n) {
			schedule_task(replay_callback, (void *) 1, REPLAY_INTERVAL_GAP, false);
		} else {
			enter_state(STATE_USER_INPUT);
			//schedule_task(deferred_enter_state, (void *) STATE_USER_INPUT, 50, false);
		}
	}
}

/** SUCCESS effect */
void suc_eff_callback(void *onOff)
{
	bool on = (bool) onOff;

	if (on) {
		for (int i = 0; i < 4; i++) screen[i] = C_OKGREEN;
		schedule_task(suc_eff_callback, 0, SUC_EFF_TIME, false);
	} else {
		for (int i = 0; i < 4; i++) screen[i] = C_DARK;

		schedule_task(deferred_enter_state, (void *) STATE_REPLAY, 200, false);
	}

	show_screen();
}

/** ERROR effect */
void fail_eff_callback(void *onOff)
{
	bool on = (bool) onOff;

	if (on) {
		for (int i = 0; i < 4; i++) screen[i] = C_CRIMSON;
		schedule_task(fail_eff_callback, 0, FAIL_EFF_TIME, false);
	} else {
		for (int i = 0; i < 4; i++) screen[i] = C_DARK;

		schedule_task(deferred_enter_state, (void *) STATE_NEW_GAME, 200, false);
	}

	show_screen();
}

/**
 * @brief Enter a game state
 * @param state
 */
void enter_state(enum GameState_enum state)
{
	GameState = state;

	switch (state) {
		case STATE_NEW_GAME:
			// new game - idle state before new game is started

			// all dimly lit
			for (int i = 0; i < 4; i++) screen[i] = C_DIMWHITE;

			break;

		case STATE_REPLAY:
			game_replay_n = 0;
			reset_sequence();

			// Start replay
			replay_callback((void *) 1);
			break;

		case STATE_USER_INPUT:
			memcpy(screen, dim, sizeof(screen));

			// Start entering & checking
			game_repeat_n = 0;
			reset_sequence();
			break;

		case STATE_SUCCESS_EFFECT:
			memcpy(screen, dim, sizeof(screen));
			//suc_eff_callback((void *) 1);
			schedule_task(suc_eff_callback, (void *) 1, 250, false);
			break;

		case STATE_FAIL_EFFECT:
			memcpy(screen, dim, sizeof(screen));
			//fail_eff_callback((void *) 1);
			schedule_task(fail_eff_callback, (void *) 1, 250, false);
			break;
	}

	show_screen();
}

/** Prepare new sequence, using time for seed. */
void prepare_sequence()
{
	game_seed = ms_now();
	game_rng_state = game_seed;
}

/** Main function, called from MX-generated main.c */
void user_main()
{
	banner("== USER CODE STARTING ==");

	user_init();

	enter_state(STATE_NEW_GAME);
	// we'll init the sequence when user first presses a button - the time is used as a seed

	ms_time_t counter1 = 0;
	while (1) {
		if (ms_loop_elapsed(&counter1, 1000)) {
			// Blink
			HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		}
	}
}

/**
 * @brief Handle a button press. Callback for debouncer.
 * @param button: button identifier
 * @param press: press state (1 = just pressed, 0 = just released)
 */
void ButtonHandler(uint32_t button, bool press)
{
	dbg("Button %d, state %d", button, press);

	switch (GameState) {
		case STATE_NEW_GAME:
			if (!press) { // released
				// user wants to start playing
				prepare_sequence();
				game_revealed_n = 1; // start with 1 revealed

				// darken
				memcpy(screen, dark, sizeof(screen));
				show_screen();

				// start playback with a delay
				// this makes it obvious the playback is not a feedback to the pressed button
				schedule_task(deferred_enter_state, (void *) STATE_REPLAY, 250, false);
				//enter_state(STATE_REPLAY);
			}
			break;
		case STATE_USER_INPUT:
			// user is entering a color
			memcpy(screen, dim, sizeof(screen));

			if (press) {
				// Button is down
				screen[button] = brt[button];
			} else {
				// Button is released
				// Verify correctness
				uint32_t expected = get_next_item();
				if (expected == button) {
					// good!
					game_repeat_n++;
					if (game_repeat_n == game_revealed_n) {
						// repeated all, good work!
						game_revealed_n++;
						enter_state(STATE_SUCCESS_EFFECT);
					}
				} else {
					enter_state(STATE_FAIL_EFFECT);
				}
			}

			show_screen();

			break;

		default:
			// discard button press, not expecting input now
			break;

	}
}
