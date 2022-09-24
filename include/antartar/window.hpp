#pragma once

#include <stdexcept>
#include <fmt/format.h>
#include <antartar/log.hpp>
#include <antartar/vk.hpp>

namespace antartar
{
	class scoped_glfw3
	{
	public:
		inline scoped_glfw3()
		{
			glfwInit();
			//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			//glfwWindowHint()
		}
		inline ~scoped_glfw3()
		{
			glfwTerminate();
		}
	};

	class scoped_glfw3_window 
	{
	private:
		GLFWwindow* window_ = nullptr;
	public:
		scoped_glfw3_window(auto ... args)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // make glfw skip creating default openGL context
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			window_ = glfwCreateWindow(args..., nullptr, nullptr);
			if (nullptr == window_)
			{
				throw std::runtime_error(log_message("Failed to create glfw window."sv));
			}
		}

		~scoped_glfw3_window()
		{
			if (nullptr != window_)
			{
				glfwDestroyWindow(window_);
			}
		}
	};
	class window
	{
	private:
		scoped_glfw3 glfw_;
		scoped_glfw3_window glfw_window_;
		vk::vk vulkan_;
	public:
		inline window(auto width, auto height, const std::string& title) : glfw_window_(width, height, title.c_str())
		{
			
		}

	};
}
