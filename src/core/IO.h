#ifndef DJINN_INCLUDE_IO_H
#define DJINN_INCLUDE_IO_H

constexpr uint8_t DJINN_KEY_DOWN = 1;
constexpr uint8_t DJINN_KEY_UP = 0;
constexpr uint8_t DJINN_KEY_VALID = 128;

namespace Djinn
{
	struct KeyboardState
	{
		uint8_t a = 0;
		uint8_t b = 0;
		uint8_t c = 0;
		uint8_t d = 0;
		uint8_t e = 0;
		uint8_t f = 0;
		uint8_t g = 0;
		uint8_t h = 0;
		uint8_t i = 0;
		uint8_t j = 0;
		uint8_t k = 0;
		uint8_t l = 0;
		uint8_t m = 0;
		uint8_t n = 0;
		uint8_t o = 0;
		uint8_t p = 0;
		uint8_t q = 0;
		uint8_t r = 0;
		uint8_t s = 0;
		uint8_t t = 0;
		uint8_t u = 0;
		uint8_t v = 0;
		uint8_t w = 0;
		uint8_t x = 0;
		uint8_t y = 0;
		uint8_t z = 0;
		uint8_t space = 0;
		uint8_t backspace = 0;
		uint8_t enter = 0;
		uint8_t tab = 0;
		uint8_t capslock = 0;
		uint8_t l_shift = 0;
		uint8_t r_shift = 0;
		uint8_t l_ctrl = 0;
		uint8_t r_ctrl = 0;
		uint8_t fn = 0;
		uint8_t tilde = 0;
		uint8_t l_alt = 0;
		uint8_t r_alt = 0;
		uint8_t num_1 = 0;
		uint8_t num_2 = 0;
		uint8_t num_3 = 0;
		uint8_t num_4 = 0;
		uint8_t num_5 = 0;
		uint8_t num_6 = 0;
		uint8_t num_7 = 0;
		uint8_t num_8 = 0;
		uint8_t num_9 = 0;
		uint8_t num_0 = 0;
		uint8_t dir_up = 0;
		uint8_t dir_down = 0;
		uint8_t dir_left = 0;
		uint8_t dir_right = 0;
	};

	struct MouseState
	{
		double x_pos = 0.0;
		double y_pos = 0.0;
		double  x_pos_raw = 0.0;
		double y_pos_raw = 0.0;
	};

	struct GamepadState
	{
		uint8_t a = 0;
		uint8_t b = 0;
		uint8_t x = 0;
		uint8_t y = 0;
		uint8_t dir_up = 0;
		uint8_t dir_down = 0;
		uint8_t dir_left = 0;
		uint8_t dir_right = 0;
	};


}

#endif // DJINN_INCLUDE_IO_H