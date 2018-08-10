#pragma once
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include<mutex>
#include<list>
#include<thread>
#include<atomic>
class UserInterface
{
public:

	UserInterface();
	~UserInterface();

	void runAsync();
	void addToBuffers(uint8_t valueCH0, uint8_t valueCH1);
	void togglePause();
	bool isSystemPaused();

private:
	SDL_Window* _window = nullptr;
	SDL_Renderer* _renderer = nullptr;
	TTF_Font* _font = nullptr;
	TTF_Font* _fontBig = nullptr;
	SDL_DisplayMode _displayMode;
	SDL_Event event;

	std::atomic<bool> _isSystemPaused;

	std::mutex _mutexChannels;
	std::list<uint8_t> _bufferCH0, _bufferCH1;
	std::thread * _worker;

	void tick();
	void drawText(TTF_Font * font, char * msg, short posX = -1, short posY = -1);

	bool initWindowAndRenderer();
	bool initFontLibrary();
	void initBuffers();

	void clearScreen();
	void drawDetails();
	void drawBuffer();
	void renderToScreen();

	void drawCPUUsage();
	void drawCPUTemp();
};
