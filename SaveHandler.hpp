#ifndef PARTICLES_SAVEHANDLER_HPP
#define PARTICLES_SAVEHANDLER_HPP

#include "Dependencies.hpp"

#include <sys/stat.h>

namespace PixelCosm
{

constexpr char SED_MAJOR_REVISION = 1;
constexpr char SED_MINOR_REVISION = 0;

class PixelCosmApp;

class SaveHandler {
public:
	static constexpr const char *saveDirectory = "usr/saves",
								*userDirectory = "usr";

private:
	PixelCosmApp& app_;
	std::string saveDir_, userDir_;

	static void create_directory(const std::string& dirName)
	{
#ifndef __WIN32
		mkdir(dirName.c_str(), 0777);
#else
		mkdir(dirName.c_str());
#endif /* __WIN32 */
	}

public:
	SaveHandler(PixelCosmApp& app)
		: app_(app), saveDir_(saveDirectory), userDir_(userDirectory)
	{
		create_directory(userDir_);
		create_directory(saveDir_);
	}

	void save(const std::string& filename);
	void load(const std::string& filename);

	void saveCounts();
	void loadCounts();

	void saveSettings();
	void loadSettings();
};

}

#endif /* PARTICLES_SAVEHANDLER_HPP */
