#pragma once
#include "Core.h"
namespace GarbagesEngine {

	class GARBAGES_ENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLIENT
	Application* CreateApplication();

}
// TODO: 在此处引用程序需要的其他标头。
