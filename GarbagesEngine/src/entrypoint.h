#pragma once
#ifdef GE_PLATFORM_WINDOWS

extern GarbagesEngine::Application* GarbagesEngine::CreateApplication();

int main(int argc, char** argv)
{
	
	auto app = GarbagesEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif