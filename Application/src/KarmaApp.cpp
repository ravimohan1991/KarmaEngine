#include "Karma.h"

class KarmaApp : public Karma::Application
{
public:
	KarmaApp()
	{
		PushOverlay(new Karma::ImGuiLayer());
	}

};

Karma::Application* Karma::CreateApplication()
{ 
	return new KarmaApp();
}