#include "Karma.h"

class KarmaApp : public Karma::Application
{
public:
	KarmaApp()
	{
	}

};

Karma::Application* Karma::CreateApplication()
{ 
	return new KarmaApp();
}