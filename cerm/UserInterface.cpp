#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include "UserInterface.h"
#include "Logger.h"
#include "Configuration.h"

using namespace std;
using namespace std::chrono_literals;

UserInterface::UserInterface() : _isSystemPaused(true)
{
	Logger::getInstance()->logInfo("UI Connected!");

	if (!initWindowAndRenderer())
	{
		return;
	}

	if (!initFontLibrary())
	{
		return;
	}

	initBuffers();
}

UserInterface::~UserInterface()
{
	_worker->join();

	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_window);
	SDL_Quit();

	TTF_CloseFont(_font);
	TTF_CloseFont(_fontBig);
	delete _worker;

	Logger::getInstance()->logTrace("UI purged!");
}

bool UserInterface::initWindowAndRenderer()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::stringstream error;
		error << "Failed to init SDL! Error : " << SDL_GetError();
		Logger::getInstance()->logError(error.str());

		return false;
	}

	SDL_GetDesktopDisplayMode(0, &_displayMode);
	SDL_CreateWindowAndRenderer(_displayMode.w, _displayMode.h, NULL, &_window, &_renderer);

	if (!_window || !_renderer)
	{
		std::stringstream error;
		error << "Failed to create SDL context! Error : " << SDL_GetError();
		Logger::getInstance()->logError(error.str());

		return false;
	}

	SDL_SetWindowTitle(_window, "CEMR");
	SDL_SetWindowFullscreen(_window, 1);
	SDL_ShowCursor(0);

	return true;
}

bool UserInterface::initFontLibrary()
{
	if (TTF_Init() != 0)
	{
		std::stringstream error;
		error << "Failed to init TTF! Error : " << SDL_GetError();
		Logger::getInstance()->logError(error.str());

		return false;
	}

	_font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSerif.ttf", 32);
	_fontBig = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSerif.ttf", 120);

	if (!_font || !_fontBig)
	{
		Logger::getInstance()->logError("Failed to load font : /usr/share/fonts/truetype/freefont/FreeSerif.ttf");

		return false;
	}

	return true;
}

void UserInterface::initBuffers()
{
	std::lock_guard<std::mutex> lg(_mutexChannels);

	_bufferCH0 = std::list<uint8_t>(_displayMode.w, 0);
	_bufferCH1 = std::list<uint8_t>(_displayMode.w, 0);
}

bool UserInterface::isSystemPaused()
{
	return _isSystemPaused;
}

void UserInterface::clearScreen()
{
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); //Black
	SDL_RenderClear(_renderer); //Clear screen
}

void UserInterface::drawDetails()
{
	if (Configuration::getInstance()->getConfiguration()->Graph->_showStats)
	{
		drawCPUUsage();
		drawCPUTemp();
	}
}

void UserInterface::drawBuffer()
{
	std::vector<uint8_t> upperBuffer;
	std::vector<uint8_t> lowerBuffer;

	SDL_Point upperPoints[_displayMode.w];
	SDL_Point lowerPoints[_displayMode.w];

	_mutexChannels.lock(); //Single method, block everything and copy in a shoot.
	upperBuffer.assign(_bufferCH0.begin(), _bufferCH0.end()); //Copy
	lowerBuffer.assign(_bufferCH1.begin(), _bufferCH1.end()); //Copy
	_mutexChannels.unlock();

	for (int i = 0; i < _displayMode.w; i++)
	{
		upperPoints[i] = { i , (_displayMode.h - upperBuffer[i] * 3) - _displayMode.h / 2 - _displayMode.h / 5 + 75 };
		lowerPoints[i] = { i , (_displayMode.h - lowerBuffer[i]) - _displayMode.h / 5 + 75 };
	}

	SDL_SetRenderDrawColor(_renderer, 255, 200, 75, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLines(_renderer, upperPoints, _displayMode.w - 1);

	SDL_SetRenderDrawColor(_renderer, 0, 255, 225, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLines(_renderer, lowerPoints, _displayMode.w - 1);
}

void UserInterface::renderToScreen()
{
	SDL_RenderPresent(_renderer); //Swap buffers
}

void UserInterface::tick()
{
	clearScreen();
	drawDetails();

	if (_isSystemPaused)
	{
		drawText(_fontBig, "System is offline!");
		std::this_thread::sleep_for(200ms); // Slower rendering when no graph is present.
	}
	else
	{
		SDL_SetRenderDrawColor(_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); //White
		SDL_RenderDrawLine(_renderer, 0, _displayMode.h / 2, _displayMode.w, _displayMode.h / 2); //Draw a separation plane between upper and lower screen.

		drawText(_font, (char*)"ECG", 20, 20);
		drawText(_font, (char*)"HB", 20, _displayMode.h / 2 + 20);

		drawBuffer();
	}

	renderToScreen();

	while (SDL_PollEvent(&event))
	{

	}

	std::this_thread::sleep_for(40ms); //~25 FPS
}

void UserInterface::drawText(TTF_Font * font, char * msg, short posX, short posY)
{
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, msg, { 255,255,255,0 });

	SDL_Texture* text = SDL_CreateTextureFromSurface(_renderer, textSurface);

	SDL_Rect renderQuad;

	if (font == _fontBig)
	{
		int textureW, textureH;
		SDL_QueryTexture(text, NULL, NULL, &textureW, &textureH);
		renderQuad = { (_displayMode.w - textureW) / 2, (_displayMode.h - textureH) / 2, textSurface->w, textSurface->h };
	}
	else {
		renderQuad = { posX, posY, textSurface->w, textSurface->h };
	}
	SDL_RenderCopy(_renderer, text, NULL, &renderQuad);

	SDL_DestroyTexture(text);
	SDL_FreeSurface(textSurface);
}

void UserInterface::drawCPUUsage()
{
	FILE * avgFile = fopen("/proc/loadavg", "r");
	char fileBuffer[1024];
	float load;

	fread(fileBuffer, sizeof(fileBuffer) - 1, 1, avgFile);
	sscanf(fileBuffer, "%f", &load);

	int val = (int)(load * 100) / 4;

	char str[] = "CPU : ";
	strcat(str, (char*)std::to_string(val).c_str());
	strcat(str, " %");

	drawText(_font, str, _displayMode.w / 8, _displayMode.h - 40);

	fclose(avgFile);
}

void UserInterface::drawCPUTemp()
{
	FILE * tempFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");

	char fileBuffer[1024];
	float load;

	fread(fileBuffer, sizeof(fileBuffer) - 1, 1, tempFile);
	sscanf(fileBuffer, "%f", &load);
	fclose(tempFile);

	load /= 1000;
	int data = load;

	char str[] = "TEMP : ";
	strcat(str, (char*)std::to_string(data).c_str());
	strcat(str, " C");

	drawText(_font, str, _displayMode.w / 3, _displayMode.h - 40);
}

void UserInterface::runAsync()
{
	_worker = new std::thread([=] {
		while (true)
		{
			try {
				this->tick();
			}
			catch (std::exception e) {
				Logger::getInstance()->logError("Tick failed");
				Logger::getInstance()->logError(e.what());
			}
		}
		Logger::getInstance()->logInfo("UI worker exited!");
	});
}

void UserInterface::addToBuffers(uint8_t valueCH0, uint8_t valueCH1)
{
	std::lock_guard<std::mutex> lg(_mutexChannels);

	_bufferCH0.push_back(valueCH0);
	_bufferCH1.push_back(valueCH1);

	_bufferCH0.pop_front();
	_bufferCH1.pop_front();
}

void UserInterface::togglePause()
{
	_isSystemPaused = !_isSystemPaused;
}
