#ifndef APP_H
#define APP_H

namespace sf
{
	class RenderWindow;
};

class App
{


private:
	sf::RenderWindow* _window = nullptr;

public:
	static const int DEFAULT_WIDTH;
	static const int DEFAULT_HEIGHT;
	static const double DEFAULT_RESX;
	static const double DEFAULT_RESY;
	App();
	virtual ~App();
	void setWindow(sf::RenderWindow*);
	void run();
	void display();
};
#endif // !APP_H
