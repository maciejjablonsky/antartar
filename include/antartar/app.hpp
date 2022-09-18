#pragma once

#include <antartar/window.hpp>

namespace antartar
{
	class app
	{
	public:
		static constexpr auto WINDOW_WIDTH = 800;
		static constexpr auto WINDOW_HEIGHT = 600;
	private:
		window window_{WINDOW_WIDTH, WINDOW_HEIGHT, "antartar"};
	public:
		void run();
		app() = default;
	};
}