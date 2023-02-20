#include "../timer.h"

std::map<std::string, ImVector<ImVec2>> Kasumi::Timer::bench_marker;
int Kasumi::Timer::offset = 0;
int Kasumi::Timer::max_size = 2000;
float Kasumi::Timer::t = 0;
bool Kasumi::Timer::full = false;

Kasumi::Timer::Timer(std::string unit) : _unit(std::move(unit)) { _starting_point = std::chrono::steady_clock::now(); }
void Kasumi::Timer::record() const
{
	float duration = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _starting_point).count()) / 1000000.f;
	if (!full)
	{
		if (bench_marker[_unit].size() < max_size)
			bench_marker[_unit].push_back(ImVec2(t, duration));
		else
			full = true;
	} else
	{
		bench_marker[_unit][offset] = ImVec2(t, duration);
	}
}
auto Kasumi::Timer::duration() const -> float
{
	return static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _starting_point).count()) / 1000000.f;
}