/*
 *  AdExpSim -- Simulator for the AdExp model
 *  Copyright (C) 2015  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.d.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TERMINAL_HPP_
#define _TERMINAL_HPP_

#include <cstdint>
#include <string>

class Terminal {
private:
	bool useColor;

public:
	/**
	 * ANSI color code for black.
	 */
	static const int BLACK = 30;

	/**
	 * ANSI color code for red.
	 */
	static const int RED = 31;

	/**
	 * ANSI color code for green.
	 */
	static const int GREEN = 32;

	/**
	 * ANSI color code for yellow.
	 */
	static const int YELLOW = 33;

	/**
	 * ANSI color code for blue.
	 */
	static const int BLUE = 34;

	/**
	 * ANSI color code for magenta.
	 */
	static const int MAGENTA = 35;

	/**
	 * ANSI color code for cyan.
	 */
	static const int CYAN = 36;

	/**
	 * ANSI color code for white.
	 */
	static const int WHITE = 37;

	/**
	 * Creates a new instance of the Terminal class.
	 *
	 * @param useColor specifies whether color codes should be generated.
	 */
	Terminal(bool useColor) : useColor(useColor) {}

	/**
	 * Returns a control string for switching to the given color.
	 *
	 * @param color is the color the terminal should switch to.
	 * @param bright specifies whether the terminal should switch to the bright
	 * mode.
	 * @return a control string to be included in the output stream.
	 */
	std::string color(int color, bool bright = true) const;

	/**
	 * Returns a control string for switching the background to the given color.
	 *
	 * @param color is the background color the terminal should switch to.
	 * @return a control string to be included in the output stream.
	 */
	std::string background(int color) const;

	/**
	 * Sets an RGB color.
	 */
	std::string rgb(uint8_t r, uint8_t g, uint8_t b, bool background) const;

	/**
	 * Returns a control string for switching to the bright mode.
	 *
	 * @return a control string to be included in the output stream.
	 */
	std::string bright() const;

	/**
	 * Makes the text italic.
	 *
	 * @return a control string to be included in the output stream.
	 */
	std::string italic() const;

	/**
	 * Underlines the text.
	 *
	 * @return a control string to be included in the output stream.
	 */
	std::string underline() const;

	/**
	 * Returns a control string for switching to the default mode.
	 *
	 * @return a control string to be included in the output stream.
	 */
	std::string reset() const;
};

#endif /* _TERMINAL_HPP_ */

