// GarbagesEngine.cpp: 定义应用程序的入口点。
#include<Config.h>
class SandBox :public GarbagesEngine::Application
{
	public:
	SandBox()
	{
	}
	~SandBox()
	{
	}
};
GarbagesEngine::Application* GarbagesEngine::CreateApplication()
{
	return new SandBox();
}