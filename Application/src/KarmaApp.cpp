#include "Karma.h"

class KarmaApp : public Karma::Application
{

};

Karma::Application* Karma::CreateApplication()
{ 
	return new KarmaApp();
}