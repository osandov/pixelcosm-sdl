#include "PixelCosm.hpp"

namespace PixelCosm
{

PixelCosmApp::PixelCosmApp(bool fullscreen) :
	tetra::Sdl::Application(
		{DISPLAY_WIDTH + GUI_WIDTH, DISPLAY_HEIGHT + GUI_HEIGHT,
		32, SDL_INIT_VIDEO | SDL_INIT_AUDIO,
		SDL_SWSURFACE | (fullscreen ? SDL_FULLSCREEN : 0x0), GAME_NAME}),
		menu_(*this), saveHandler_(*this)
{
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 1024) == -1)
		throw tetra::Sdl::Exception(Mix_GetError());

	music_ = Mix_LoadMUS("resources/title/music.wav");

	Group::create({"None", nullptr});
	ElementGroup::create({"any"});
	Element::create({"none", 0, {0, 0, 0},
					InterPattern(0), 0, std::vector<int>(), 0.0,
					noneMass, 0.0, 0.0, true, 0, {0.0, 0.0}});

	try {
		musicGen_.setup("resources/tracklist");
	} catch (const MusicError& e) {
		std::cerr << "In tracks: " << std::endl;
		errorStream_ << "In tracks: " << std::endl;
		std::cerr << e.what() << std::endl;
		errorStream_ << e.what() << std::endl;
		failed_ |= int(FailCode::Track);
	}

	InteractionHandler::musicGen(&musicGen_);

	{
		ParentParser parser(*this, "std.sand");
		const ParentParser::ErrorLists& allErrors = parser.parse();

		for (auto& errs : allErrors) {
			bool failed = false;
			if (!errs.empty()) {
				std::cerr << "In file " << errs.getFileName() << ":" << std::endl;
				errorStream_ << "In file " << errs.getFileName() << ":" << std::endl;
				for (auto e : errs) {
					failed |= !e.isWarning();
					std::cerr << e.what() << std::endl;
					errorStream_ << e.what() << std::endl;
				}
			}
			failed_ |= (failed) ? int(FailCode::Sand) : 0;
		}
	}

	writer_.openFont("resources/font.ttf", (!(failed_ & int(FailCode::Sand))) ? FONT_SIZE : ERROR_FONT_SIZE);
	if (!(failed_ & int(FailCode::Sand))) {
		menu_.setupMenu();
		Element::mapColors(drawer_);

		try {
			saveHandler_.load("auto");
			saveHandler_.loadCounts();
			saveHandler_.loadSettings();
		} catch (const std::runtime_error& e) {}

		menu_.setupGroups();
	}

	Particle::setNoneElement(&Element::withId(0));
}

PixelCosmApp::~PixelCosmApp()
{
	SDL_FillRect(screen_, nullptr, 0);
	writer_.openFont("resources/font2.ttf", 16 * DISPLAY_FACTOR);
	writer_.writeLineCentered({screenWidth_ / 2, screenHeight_ / 2 - FONT_SIZE * 2}, "Exiting...");
	SDL_Flip(screen_);

	functions_.reset();
	Mix_FreeMusic(music_);

	Mix_CloseAudio();
	if (!(failed_ & int(FailCode::Sand))) {
		saveHandler_.save("auto");
		saveHandler_.saveCounts();
		saveHandler_.saveSettings();
	}

	for (int i = 1; i < Group::getEndId(); ++i) {
		SDL_FreeSurface(Group::withId(i).image);
		for (auto& subgroup : Group::withId(i).subgroups)
			SDL_FreeSurface(subgroup.image);
	}
}

void PixelCosmApp::returnToTitle()
{
	setFunctions<TitleFunctions>();
}

void PixelCosmApp::save()
{
	saveHandler_.save("sed");
}

void PixelCosmApp::load()
{
	try {
		saveHandler_.load("sed");
	} catch (const std::runtime_error& e) {}
}

bool PixelCosmApp::failed()
{
	return failed_;
}

void PixelCosmApp::reset()
{
	field_.reset();
}

void PixelCosmApp::togglePause()
{
	dynamic_cast<PlayFunctions*>(functions_.get())->togglePause();
}

bool PixelCosmApp::isPaused()
{
	return dynamic_cast<PlayFunctions*>(functions_.get())->isPaused();
}

void PixelCosmApp::toggleMusic()
{
	musicGen_.toggleEnabled();
}

bool PixelCosmApp::musicEnabled()
{
	return musicGen_.enabled();
}

void PixelCosmApp::skipSong()
{
	musicGen_.skipSong();
}

void PixelCosmApp::toggleGravity()
{
	field_.gravity ^= true;
}

bool PixelCosmApp::isGravityOn()
{
	return field_.gravity;
}

void PixelCosmApp::nextTool()
{
	if (tool == Tool::Repel)
		tool = Tool::Drag;
	else if (tool == Tool::Drag)
		tool = Tool::Angle;
	else if (tool == Tool::Angle)
		tool = Tool::Inspect;
	else if (tool == Tool::Inspect)
		tool = Tool::Repel;
}

void PixelCosmApp::prevTool()
{
	if (tool == Tool::Repel)
		tool = Tool::Inspect;
	else if (tool == Tool::Drag)
		tool = Tool::Repel;
	else if (tool == Tool::Angle)
		tool = Tool::Drag;
	else if (tool == Tool::Inspect)
		tool = Tool::Angle;
}

void PixelCosmApp::resetSecretElements()
{
	for (int i = 0; i < Group::getEndId(); ++i) {
		for (auto& member : Group::withId(i).members) {
			if (member.secret) {
				member.count = 0;
				Group::withId(i).newAvailable(true);
			}
		}
	}
}

}
