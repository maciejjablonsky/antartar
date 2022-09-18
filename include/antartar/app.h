#ifndef __ANTARTART_APP_H_
#define __ANTARTART_APP_H_

namespace antartar
{
	class App
	{
	public:
		static constexpr auto WINDOW_WIDTH = 800;
		static constexpr auto WINDOW_HEIGHT = 600;
		void run();
		App() = default;
	};
}
#endif __ANTARTART_APP_H_