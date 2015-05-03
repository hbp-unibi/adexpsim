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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iomanip>
#include <sstream>
#include <unistd.h>

#include "Timer.hpp"

static inline double microtime() {
	struct timespec time;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time);
	return time.tv_sec * 1.0e6 + time.tv_nsec * 1.0e-3;
}

Timer::Timer()
{
	value = 0.0;
	active = true;
	start = microtime();
}

void Timer::pause()
{
	// Only continue if the timer is not already paused
	if (active) {
		value += microtime() - start;
		active = false;
	}
}

void Timer::cont()
{
	// Only continue if the timer is paused
	if (!(active)) {
		start = microtime();
		active = false;
	}
}

double Timer::time() const
{
	if (active) {
		return (value + microtime() - start) / 1000.0;
	}
	return (value) / 1000.0;
}

std::ostream& operator<<(std::ostream &os, const Timer &t)
{
	double time = t.time();
	std::stringstream ss;
	ss << "Elapsed time: " << std::setprecision(4) << time << std::endl;
	return os << ss.str();
}

