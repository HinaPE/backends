#ifndef KASUMI_TIMER_H
#define KASUMI_TIMER_H

// Copyright (c) 2023 Xayah Hina
// MPL-2.0 license

#include <iostream>
#include <chrono>
#include <memory>
#include <utility>
#include <map>
#include <vector>

#include "imgui.h"

namespace Kasumi
{
class Timer final
{
public:
	explicit Timer(std::string unit);
	void record() const;
	auto duration() const -> float;

public:
	std::string _unit;
	std::chrono::steady_clock::time_point _starting_point;

	static std::map<std::string, ImVector <ImVec2>> bench_marker;
	static int offset;
	static int max_size;
	static float t;
};
using TimerPtr = std::shared_ptr<Timer>;
}
#endif //KASUMI_TIMER_H
