AdExpSim
========

AdExpSim is a framework for high-performance single neuron simulation of
the AdEx and LIF spiking neuron models written in C++14. It furthermore
provides code for neuron parameter evaluation regarding the BiNAM
associative memory and a GUI for interactive design space exploration.

How to use
----------

You need a recent, C++14 capable compiler as well as a development version
of Qt5 installed. Furthermore, compilation requires a recent version of
CMake.

You can install and build the project with the following set of commands:
````bash
git clone https://github.com/hbp-sanncs/adexpsim
cd adexpsim && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
````

Authors
-------

This project has been initiated by Andreas Stöckel in 2015 as part of his Masters Thesis
at Bielefeld University in the [Cognitronics and Sensor Systems Group](http://www.ks.cit-ec.uni-bielefeld.de/) which is
part of the [Human Brain Project, SP 9](https://www.humanbrainproject.eu/neuromorphic-computing-platform).

License
-------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

