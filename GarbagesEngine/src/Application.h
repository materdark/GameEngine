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
// TODO: �ڴ˴����ó�����Ҫ��������ͷ��
