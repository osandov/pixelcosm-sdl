#include "PixelCosm.hpp"
#include "Cursor.hpp"

namespace PixelCosm
{

std::random_device random_device;

std::default_random_engine random_engine(random_device());

std::vector<Group> Group::groups_;

std::vector<ElementGroup> ElementGroup::elementGroups_;

std::vector<Element> Element::elements_;

std::multimap<Interaction::idPair, std::shared_ptr<Interaction>,
		 InteractionHandler::compareIdPairs> InteractionHandler::map_;
MusicGenerator* InteractionHandler::musicGen_ = nullptr;

InterDirGenerator Particle::dirGen_;

constexpr tetra::point<int>
	Direction::up, Direction::right,
	Direction::down, Direction::left;

constexpr const tetra::point<int>* Direction::dirs_[4];

}

using namespace PixelCosm;

bool checkFullscreen()
{
#ifndef FORCE_FULLSCREEN
	std::ifstream file(std::string(SaveHandler::userDirectory) + "/settings");

	if (!file.is_open())
		return false;

	std::string line;
	do {
		getline(file, line);
		if (line.empty())
			continue;

		std::size_t pos = line.find('\t');
		if (pos == std::string::npos)
			continue;

		std::string setting = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		if (setting == "fullscreen") {
			try {
				return std::stoi(value);
			} catch (const std::invalid_argument& e) {
				return false;
			}
		}
	} while (!file.eof());

	file.close();
	return false;
#else
	return true;
#endif // FORCE_FULLSCREEN
}

extern "C" int main(int argc, char* argv[])
{
	PixelCosmApp game(checkFullscreen());
	tetra::Sdl::Cursor cursor(pointCursor);
	game.setCursor(cursor);
	game.appendToCaption(VERSION);

	if (game.failed())
		game.setFunctions<ErrorScreenFunctions>();
	else
		game.setFunctions<TitleFunctions>();

	game.capFrameRate(true, FRAMERATECAP);
	int r = game.main(argc, const_cast<const char**>(argv));
	return r;
}
