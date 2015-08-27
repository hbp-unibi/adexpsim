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

#include <ext/stdio_filebuf.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

#include "SurfacePlotIo.hpp"

namespace AdExpSim {

namespace {
/**
 * Class used internally to launch an external process and to send data to it
 * via stdin.
 */
class Process {
private:
	using Filebuf = __gnu_cxx::stdio_filebuf<char>;
	pid_t pid;
	std::unique_ptr<Filebuf> buf;
	std::unique_ptr<std::ostream> os;

	Process() : pid(0), buf(nullptr), os(nullptr) {}

	Process(pid_t pid, int fd)
	    : pid(pid),
	      buf(new Filebuf(fd, std::ios::out)),
	      os(new std::ostream(buf.get()))
	{
	}

public:
	pid_t getPid() { return pid; }

	std::ostream &getStdIn() { return *os; }

	bool valid() const { return pid != 0; }

	static Process create(const std::string &cmd,
	                      const std::vector<std::string> &args)
	{
		// Create a pipe
		int pipefd[2];
		if (pipe(pipefd) < 0) {
			return Process();
		}

		// Fork this process
		pid_t pid = fork();

		// Close the pipe on error and abort
		if (pid < 0) {
			close(pipefd[0]);
			close(pipefd[1]);
			return Process();
		}
		if (pid == 0) {
			// Assemble the argument pointers -- must be an array of pointers
			// terminated by a null pointer
			std::vector<const char *> argsp;
			argsp.reserve(args.size() + 2);
			argsp.push_back(cmd.c_str());
			for (const auto &arg : args) {
				argsp.push_back(arg.c_str());
			}
			argsp.push_back(nullptr);

			// This is the child process, connect STDIN_FILENO to the read end
			// of the pipe, close the write and and execute the given command
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[1]);
			execvp(cmd.c_str(), (char *const *)argsp.data());

			// We'll only get here if execvp failed. Show error message and
			// eat everything from stdin
			std::cout << "Could not execute \"" << cmd << "\"." << std::endl;
			char buf[1024];
			while (read(STDIN_FILENO, buf, 1024) >= 0) {}
			exit(1);
		} else {
			close(pipefd[0]);
			return Process(pid, pipefd[1]);
		}
		return Process();
	}
};
}

void SurfacePlotIo::storeSurfacePlot(std::ostream &os,
                                     const Exploration &exploration,
                                     size_t dim,
                                     bool gnuplot)
{
	const DiscreteRange rX = exploration.rangeX();
	const DiscreteRange rY = exploration.rangeY();
	const ExplorationMemory &mem = exploration.mem();
	for (size_t x = 0; x < rX.steps; x++) {
		for (size_t y = 0; y < rY.steps; y++) {
			os << rX.value(x) << ' ' << rY.value(y) << ' ' << mem(x, y, dim)
			   << std::endl;
		}
		if (gnuplot) {
			os << std::endl;
		}
	}
}

void SurfacePlotIo::runGnuPlot(const Parameters &params,
                               const Exploration &exploration,
                               size_t dim)
{
	Process p = Process::create("gnuplot", {"-p"});
	if (p.valid()) {
		const Range r = exploration.mem().range(dim);
		std::ostream &os = p.getStdIn();
		os << "set nokey; "
			  "set pm3d; "
			  "set hidden3d; "
			  "set cbrange [" << r.max << ":" << r.min << "]; "
			  "set zrange [" << r.max << ":" << r.min << "]; "
			  "set view 0, 0; "
			  "splot '-' with pm3d"
		   << std::endl;
		storeSurfacePlot(os, exploration, dim);
	}
}
}

