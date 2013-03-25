#ifndef PARTICLEGAME_HPP
#define PARTICLEGAME_HPP

#include "Dependencies.hpp"

#include "Group.hpp"
#include "Element.hpp"
#include "GameField.hpp"
#include "Particle.hpp"
#include "Interaction.hpp"

#include "GameMenu.hpp"

#include "MusicGenerator.hpp"

#include "SandParser.hpp"
#include "SaveHandler.hpp"

namespace PixelCosm
{

enum class Tool
{
	Repel,
	Drag,
	Angle,
	Inspect
};

enum class FailCode
{
	None = 0x0,
	Sand = 0xf0,
	Track = 0xf00,
};

class PixelCosmApp : public tetra::Sdl::Application
{
	GameField<WIDTH, HEIGHT> field_;
	GameMenu menu_;
	MusicGenerator musicGen_;
	SaveHandler saveHandler_;

	int failed_ = 0;

	std::stringstream errorStream_;
	Mix_Music* music_;

public:
	int curElemId = 1;
	int penSize = 3;
	static int constexpr maxPenSize = 10;
	float penAngle = 0.0;
	float forceStrength = 1.0;
	static float constexpr maxForceStrength = 5.0;
	static constexpr float forceStep = 0.25;

	Tool tool = Tool::Repel;

	PixelCosmApp(bool fullscreen = false);
	~PixelCosmApp();

	void returnToTitle();

	void save();
	void load();

	bool failed();

	void reset();

	void togglePause();
	bool isPaused();

	void toggleMusic();
	bool musicEnabled();
	void skipSong();

	void toggleGravity();
	bool isGravityOn();

	void nextTool();
	void prevTool();

	void resetSecretElements();

	friend class GameMenu;
	friend class SaveHandler;
	friend class TitleFunctions;
	friend class PlayFunctions;
	friend class InstructionFunctions;
	friend class OptionFunctions;
	friend class UpdateFunctions;
	friend class ErrorScreenFunctions;
};

}

#include "TitleScreen.hpp"
#include "PlayFunctions.hpp"
#include "Instructions.hpp"
#include "Options.hpp"
#include "Update.hpp"
#include "ErrorScreen.hpp"

#endif /* PARTICLEGAME_HPP */
