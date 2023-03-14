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
#include <functional>

#include "imgui.h"

namespace Kasumi
{
class Timer final
{
public:
	explicit Timer(std::string name);
	void record() const;
	auto duration() const -> float;

	static std::map<std::string, ImVector <ImVec2>> bench_marker;
	static int offset;
	static int max_size;
	static float t;
	static bool full;

private:
	std::string _name;
	std::chrono::steady_clock::time_point _starting_point;
};
using TimerPtr = std::shared_ptr<Timer>;
}
#define HINA_TRACK(f, name) { Kasumi::Timer timer(name); f; timer.record(); }
#endif //KASUMI_TIMER_H
