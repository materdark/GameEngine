// GarbagesEngine.cpp: ����Ӧ�ó������ڵ㡣
//
#include"Pch.h"
#include "Application.h"

namespace GarbagesEngine {

	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		GarbagesEngine::Log::Init();
		GE_CORE_WARN("Initialized Log!");
		GE_INFO("Initialized Log!");
		std::cout << "Hello SandBox." << std::endl;
		while (true)
		{
			// Add your code here
		}
	}
}