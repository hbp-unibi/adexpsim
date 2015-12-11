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

/**
 * @file SpikeTrain.hpp
 *
 * Contains methods for the parametric generation of spike trains and data
 * structures for holding the parameters.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SPIKE_TRAIN_HPP_
#define _ADEXPSIM_SPIKE_TRAIN_HPP_

#include <random>

#include "Spike.hpp"

namespace AdExpSim {
/**
 * Structure defining global properties of a SpikeTrain. These properties are
 * used for various spike train generators.
 */
struct SpikeTrainEnvironment {
	/**
	 * Number of spikes in a single spike burst.
	 */
	size_t burstSize;

	/**
	 * Length of a spike train group.
	 */
	Time T;

	/**
	 * Offset of individual bursts (standard deviation).
	 */
	Time sigmaTOffs;

	/**
	 * Noise in the individual spike times of a burst (standard deviation).
	 */
	Time sigmaT;

	/**
	 * Inter spike interval of the individual spike times of a burst.
	 */
	Time deltaT;

	/**
	 * Noise in the synapse weights (standard deviation).
	 */
	Val sigmaW;

	/**
	 * Default constructor, allows to set all member variables.
	 */
	SpikeTrainEnvironment(size_t burstSize = 1, Time T = 50e-3_s,
	                      Time sigmaT = 0e-3_s, Time deltaT = 5e-3_s,
	                      Val sigmaW = 0.0, Time sigmaTOffs = 0.0_s)
	    : burstSize(burstSize),
	      T(T),
	      sigmaTOffs(sigmaTOffs),
	      sigmaT(sigmaT),
	      deltaT(deltaT),
	      sigmaW(sigmaW)
	{
	}
};

/**
 * The GenericGroupDescriptor is used to describe a generic spike group with
 * both excitatory and inhibitory spikes with a specific weight and an expected
 * number of output spike bursts.
 */
struct GenericGroupDescriptor {
	size_t nE, nI, nOut;
	Val wE, wI;

	/**
	 * Default constructor, create a new instance of GenericGroupDescriptor with
	 * sane default values.
	 */
	GenericGroupDescriptor() : GenericGroupDescriptor(1) {}

	/**
	 * Constructor, allows to specify the excitatory spike parameters.
	 */
	GenericGroupDescriptor(size_t nE, size_t nOut = 1, Val wE = 1.0)
	    : GenericGroupDescriptor(nE, 0, nOut, wE, 1.0)
	{
	}

	/**
	 * Constructor, allows to set all struct members.
	 *
	 * @param nE is the number of excitatory input bursts.
	 * @param nI is the number of inhibitory input bursts.
	 * @param nOut is the number of expected output bursts.
	 * @param wE is the weight factor of the excitatory input bursts.
	 * @param wI is the weight factor of the inhibitory input bursts.
	 */
	GenericGroupDescriptor(size_t nE, size_t nI, size_t nOut, Val wE, Val wI)
	    : nE(nE), nI(nI), nOut(nOut), wE(wE), wI(wI)
	{
		adjust();
	}

	/**
	 * Makes sure the GenericGroupDescriptor describes at least one spike -- if
	 * not, generates a pseudo excitatory spike with zero weight.
	 */
	void adjust()
	{
		if (nE + nI == 0) {
			nE = 1;
			wE = 0.0;
		}
	}

	/**
	 * Adds a spike group described by this structure to the given spike vector.
	 * The type parameter allows to choose whether the excitatory spikes or the
	 * inhibitory spikes should be generated.
	 */
	SpikeVec &build(SpikeVec &spikes,
	                Spike::Type type = Spike::Type::EXCITATORY,
	                const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
	                bool equidistant = false, Time t0 = Time(),
	                Time *tMin = nullptr, Time *tMax = nullptr,
	                size_t *seed = nullptr) const;

	/**
	 * Returns a list of spikes generated according to the parameters stored in
	 * this structure.
	 */
	SpikeVec build(Spike::Type type = Spike::Type::EXCITATORY,
	               const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
	               bool equidistant = false, Time t0 = Time(),
	               Time *tMin = nullptr, Time *tMax = nullptr,
	               size_t *seed = nullptr) const
	{
		SpikeVec res;
		return build(res, type, env, equidistant, t0, tMin, tMax, seed);
	}
};

/**
 * The SingleGroupSingleOutDescriptor describes an experiment for a single
 * expected output spike (not spike bursts). This output spike should occur
 * for n input spikes, but not for nM1 (read "n minus 1") input spike bursts.
 */
struct SingleGroupSingleOutDescriptor {
	enum class Type { N, NM1 };

	size_t n, nM1;

	/**
	 * Default constructor, creates a new SingleGroupSingleOutDescriptor with
	 * sane default values.
	 */
	SingleGroupSingleOutDescriptor() : SingleGroupSingleOutDescriptor(3) {}

	/**
	 * Creates a new SingleGroupSingleOutDescriptor with the nM1 value being
	 * automatically set to n - 1.
	 */
	SingleGroupSingleOutDescriptor(size_t n)
	    : SingleGroupSingleOutDescriptor(n, n - 1)
	{
	}

	/**
	 * Constructor, allows to initialize all memebers individually.
	 */
	SingleGroupSingleOutDescriptor(size_t n, size_t nM1) : n(n), nM1(nM1) {}

	/**
	 * Adds a spike group described by this structure to the given spike vector.
	 * The type parameter allows to choose whether the excitatory spikes or the
	 * inhibitory spikes should be generated.
	 */
	SpikeVec &build(SpikeVec &spikes, Type type = Type::N,
	                const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
	                Time t0 = Time(), Time *tMin = nullptr,
	                Time *tMax = nullptr) const;

	/**
	 * Returns a list of spikes generated according to the parameters stored in
	 * this structure.
	 */
	SpikeVec build(Type type = Type::N,
	               const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
	               Time t0 = Time(), Time *tMin = nullptr,
	               Time *tMax = nullptr) const
	{
		SpikeVec res;
		return build(res, type, env, t0, tMin, tMax);
	}
};

/**
 * The SingleGroupMultiOutDescriptor describes an experiment, where nOut spike
 * bursts are expected for n input spike bursts, but zero output spikes are
 * expected for nM1 input spike bursts.
 */
struct SingleGroupMultiOutDescriptor : public SingleGroupSingleOutDescriptor {
	size_t nOut;

	/**
	 * Default constructor, creates a new SingleGroupSingleOutDescriptor with
	 * sane default values.
	 */
	SingleGroupMultiOutDescriptor() : SingleGroupMultiOutDescriptor(3, 2, 1) {}

	/**
	 * Constructor, allows to initialize all memebers individually.
	 */
	SingleGroupMultiOutDescriptor(size_t n, size_t nM1, size_t nOut)
	    : SingleGroupSingleOutDescriptor(n, nM1), nOut(nOut)
	{
	}
};

/**
 * The SpikeTrain class is used to construct input spikes as a set of random
 * spike trains which are generated from generic group descriptors. The
 * SpikeTrain class furthermore generates the ranges in which an output is (or
 * is not).
 */
class SpikeTrain {
public:
	/**
	 * The Range descriptor describes how many spikes are expected within a
	 * range starting at the given time point. This structure is needed within
	 * the SpikeTrainEvaluation, which analyses this particular SpikeTrain.
	 */
	struct Range {
		/**
		 * Start time of this range element.
		 */
		Time start;

		/**
		 * Incrementing group index, all ranges with the same value for group
		 * belong to a single "group".
		 */
		size_t group;

		/**
		 * Spike train descriptor index this range belongs to.
		 */
		size_t descrIdx;

		/**
		 * Number of spikes to occur inside this range.
		 */
		size_t nOut;

		Range(Time start = Time(), size_t group = 0, size_t descrIdx = 0,
		      size_t nOut = 0)
		    : start(start), group(group), descrIdx(descrIdx), nOut(nOut)
		{
		}

		friend bool operator<(const Range &r1, const Range &r2)
		{
			return r1.start < r2.start;
		}
	};

private:
	/**
	 * Generated input spikes.
	 */
	SpikeVec spikes;

	/**
	 * Range descriptor.
	 */
	std::vector<Range> ranges;

	/**
	 * Indices of the spikes in the spike list that coincide with the start of a
	 * new range.
	 */
	std::vector<size_t> rangeStartSpikes;

	/**
	 * Descriptors given in the constructor.
	 */
	std::vector<GenericGroupDescriptor> descrs;

	/**
	 * Number of spike train groups as given in the constructor.
	 */
	size_t n;

	/**
	 * Environment parameters such as standard deviations.
	 */
	SpikeTrainEnvironment env;

	/**
	 * Set to true, if the spike train groups were drawn sorted.
	 */
	bool sorted;

	/**
	 * Set to true if equal spacing should be used, just as in the SingleGroup
	 * simulation.
	 */
	bool equidistant;

public:
	/**
	 * Default constructor. Creates an empty spike train.
	 */
	SpikeTrain() : SpikeTrain(std::vector<GenericGroupDescriptor>{}) {}

	/**
	 * Constructor of the SpikeTrain class -- calculates a new set of input
	 * spikes trains.
	 *
	 * @param descrs is the set of spike train descriptors from which to
	 * (randomly) choose.
	 * @param n is the number of spike train groups the should be created. If
	 * set to zero, the number of groups is set to the number of descriptors.
	 * @param env contains all additional parameters which specify the spike
	 * train.
	 * @param sorted if set to true, the descriptors are not chosen randomly,
	 * but repeated in the order in which they were given in the descriptor
	 * list.
	 * @param equidistant if true, creates equidistant input spikes just as in
	 * the SingleGroup simulation.
	 */
	SpikeTrain(const std::vector<GenericGroupDescriptor> &descrs, size_t n = 0,
	           const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
	           bool sorted = true, bool equidistant = false)
	    : descrs(descrs),
	      n(n),
	      env(env),
	      sorted(sorted),
	      equidistant(equidistant)
	{
		rebuild();
	}

	/**
	 * Allows to build a SpikeTrain instance from the given
	 * SingleGroupMultiOutDescriptor. All other parameters as above.
	 */
	SpikeTrain(const SingleGroupMultiOutDescriptor &data, size_t n = 0,
	           const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
	           bool sorted = true, bool equidistant = false)
	    : n(n), env(env), sorted(sorted), equidistant(equidistant)
	{
		fromSingleGroupSpikeData(data);
		rebuild();
	}

	/**
	 * Builds a new spike train using the parameters given in the constructor.
	 */
	void rebuild();

	/**
	 * Returns the spike train group descriptors.
	 */
	const std::vector<GenericGroupDescriptor> &getDescrs() const
	{
		return descrs;
	}

	/**
	 * Sets the spike train group descriptors.
	 */
	void setDescrs(const std::vector<GenericGroupDescriptor> &descrs)
	{
		this->descrs = descrs;
	}

	/**
	 * Returns the number of spike train groups.
	 */
	size_t getN() const { return n; }

	/**
	 * Returns the number of spike train groups.
	 */
	void setN(size_t n) { this->n = n; }

	/**
	 * Returns a reference at the environment parameters.
	 */
	const SpikeTrainEnvironment &getEnvironment() { return env; }

	/**
	 * Sets the SpikeTrainEnvironment instance.
	 */
	void setEnvironment(const SpikeTrainEnvironment &env) { this->env = env; }

	/**
	 * Returns whether the spike train was sorted.
	 */
	bool isSorted() const { return sorted; }

	/**
	 * Sets the "sorted" flag.
	 */
	void setSorted(bool sorted) { this->sorted = sorted; }

	/**
	 * Returns whether the input spikes are drawn equidistant.
	 */
	bool isEquidistant() const { return equidistant; }

	/**
	 * Sets the "equidistant" flag.
	 */
	void setEquidistant(bool equidistant) { this->equidistant = equidistant; }

	/**
	 * Returns the simulation end time, which is set to the end of the last
	 * spike train group, should be n * env.T.
	 */
	Time getMaxT() const
	{
		return ranges.empty() ? Time(0) : ranges.back().start;
	}

	/**
	 * Returns the generated input spikes.
	 */
	const SpikeVec &getSpikes() const { return spikes; }

	/**
	 * Returns the spike ranges, the last element marks the end time.
	 */
	const std::vector<Range> &getRanges() const { return ranges; }

	/**
	 * Returns the indices of the spikes that coincide with the start of a new
	 * range.
	 */
	const std::vector<size_t> &getRangeStartSpikes() const
	{
		return rangeStartSpikes;
	}

	/**
	 * Returns the number of expected output spikes.
	 */
	size_t getExpectedOutputSpikeCount() const;

	/**
	 * Adapts this instance to the parameters found in the
	 * SingleGroupMultiOutSpikeData instance.
	 */
	void fromSingleGroupSpikeData(const SingleGroupMultiOutDescriptor &data);
};

/**
 * Method used to build a set of n input spikes with uniform spacing.
 *
 * @param n is the number of input spikes. If a fractional number is given, an
 * additional input spike is generated which is scaled by the fractional part
 * of the number.
 * @param t is the delay between the spikes.
 * @param t0 is the time at which the first spike should be generated.
 * @param w is the weight factor with which the spike weights are multiplied.
 */
SpikeVec buildInputSpikes(Val n, Time t, Time t0 = Time(0), Val w = 1);

/**
 * The buildSpikeGroup class can be used to build a input spike group with the
 * given properties. Adds the spikes to the given SpikeVec instance and returns
 * a reference to it.
 *
 * @param spikes is a reference at the SpikeVec to which the new spikes should
 * be added. Note that the vector will be sorted by this function in order to
 * maintain the time order of the spikes.
 * @param w is the weight of the newly creates spikes.
 * @param nBursts is the number of spike bursts that should happen at the same
 * time. The size of a burst is stored in the SpikeTrainEnvironment passed in
 * the "env" parameter.
 * @param env contains environment parameters like the inter-spike interval and
 * the noise descriptor.
 * @param equidistant if true, time noise is approximated by distributing the
 * spikes of all bursts equidistantly.
 * @param t0 is the time at which the spike group should be inserted.
 * @param tMin if not nullptr, the time of the first spike is written to this
 * value.
 * @param tMax if not nullptr, the time of the last spike is written to this
 * value.
 * @param seed is a pointer the random number generator seed. If nullptr, the
 * random number generator will initialized with and internal random seed is
 * used. Otherwise the value at the pointer position will be used. Advances the
 * seed value.
 */
SpikeVec &buildSpikeGroup(
    SpikeVec &spikes, Val w, size_t nBursts,
    const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
    bool equidistant = false, Time t0 = Time(), Time *tMin = nullptr,
    Time *tMax = nullptr, size_t *seed = nullptr);

/**
 * The buildSpikeGroup class can be used to build a input spike group with the
 * given properties. Returns a new SpikeVec instance.
 *
 * @param t0 is the time at which the spike group should be inserted.
 * @param w is the weight of the newly creates spikes.
 * @param nBursts is the number of spike bursts that should happen at the same
 * time. The size of a burst is stored in the SpikeTrainEnvironment passed in
 * the "env" parameter.
 * @param env contains environment parameters like the inter-spike interval and
 * the noise descriptor.
 * @param equidistant if true, time noise is approximated by distributing the
 * spikes of all bursts equidistantly.
 * @param tMin if not nullptr, the time of the first spike is written to this
 * value.
 * @param tMax if not nullptr, the time of the last spike is written to this
 * value.
 * @param seed is a pointer the random number generator seed. If nullptr, the
 * random number generator will initialized with and internal random seed is
 * used. Otherwise the value at the pointer position will be used. Advances the
 * seed value.
 */
SpikeVec buildSpikeGroup(
    Val w, size_t nBursts = 1,
    const SpikeTrainEnvironment &env = SpikeTrainEnvironment(),
    bool equidistant = false, Time t0 = Time(), Time *tMin = nullptr,
    Time *tMax = nullptr, size_t *seed = nullptr);
}

#endif /* _ADEXPSIM_SPIKE_TRAIN_HPP_ */
