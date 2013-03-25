#include "PixelCosm.hpp"

namespace PixelCosm
{

void SaveHandler::save(const std::string& filename)
{
	std::ofstream file(saveDir_ + '/' + filename + SAVE_EXTENSION, std::ios::binary);

	char revision = SED_MAJOR_REVISION;
	file.write(&revision, sizeof(char));
	revision = SED_MINOR_REVISION;
	file.write(&revision, sizeof(char));

	file.write(&app_.field_.borderFlags, sizeof(char));
	file.write(reinterpret_cast<char*>(&app_.field_.gravity), sizeof(bool));

	auto& particles = app_.field_.particles_;

	particles.sort(
		[](const Particle& a, const Particle& b) {return a.elem->id < b.elem->id;});

	auto first = particles.begin();
	char size;
	while (first != particles.end()) {
		int id = first->elem->id;

		auto last = std::upper_bound(first, particles.end(), id,
			[](int id, const Particle& part) {return id < part.elem->id;});

		const std::string& name = first->elem->name;
		size = name.size();
		int count = std::distance(first, last);

		file.write(&size, sizeof(char));
		file.write(name.c_str(), size * sizeof(char));
		file.write(reinterpret_cast<char*>(&count), sizeof(int));

		while (first != last) {
			ParticleParams::Physical phys{first->pos, first->veloc, first->life};
			file.write(reinterpret_cast<char*>(&phys), sizeof(ParticleParams::Physical));
			++first;
		}
	}

	size = 0;
	file.write(&size, sizeof(char)); // Null-terminate

	file.close();
}

void SaveHandler::load(const std::string& filename)
{
	std::ifstream file(saveDir_ + '/' + filename + SAVE_EXTENSION, std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error(std::string("Failed to open file '") +
			saveDir_ + '/' + filename + SAVE_EXTENSION + "'");

	char revision;
	file.read(&revision, sizeof(char));
	if (revision != SED_MAJOR_REVISION)
		throw std::runtime_error(std::string("Save file format is outdated and no longer supported"));
	file.read(&revision, sizeof(char));
	if (revision != SED_MINOR_REVISION)
		/* TODO: warn */;

	file.read(&app_.field_.borderFlags, sizeof(char));
	file.read(reinterpret_cast<char*>(&app_.field_.gravity), sizeof(bool));

	ParticleParams params;
	app_.field_.reset();
	char size;
	char name[MAX_ELEM_NAME + 1];

	while (true) {
		file.read(&size, sizeof(char));
		if (size == 0) // Reached the null byte; done
			break;
		else if (file.eof())
			throw std::runtime_error("Error reading file: unexpected end-of file");

		file.read(name, size * sizeof(char));
		*(name + size) = '\0'; // Null-terminate the name string

		try {
			params.elemid = Element::withName(name).id;
		} catch (const std::runtime_error& e) {
			params.elemid = 0;
		}

		int count;
		file.read(reinterpret_cast<char*>(&count), sizeof(int));

		for (int i = 0; i < count; ++i) {
			file.read(reinterpret_cast<char*>(&params.phys),
				sizeof(ParticleParams::Physical));
			app_.field_.add_particle(params);
		}
	}

	file.close();
}

void SaveHandler::saveCounts()
{
	std::ofstream file(userDir_ + "/counts", std::ios::binary);

	char size;
	for (int i = 1; i < Group::getEndId(); ++i) {
		for (auto& member : Group::withId(i).members) {
			const std::string& name = Element::withId(member.elemid).name;
			size = name.size();
			file.write(&size, sizeof(char)); // Write the size of the name of the element
			file.write(name.c_str(), size * sizeof(char)); // Write the name of the element
			file.write(reinterpret_cast<char*>(&member.count),
				sizeof(unsigned char)); // Write the count for that element
		}
	}

	size = 0;
	file.write(&size, sizeof(char)); // Null-terminate

	file.close();
}

void SaveHandler::loadCounts()
{
	std::ifstream file(userDir_ + "/counts", std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("Failed to open file 'usr/counts'");

	char name[MAX_ELEM_NAME + 1];
	char size;

	while (true) {
		file.read(&size, sizeof(char));
		if (size == 0) // Reached the null byte; done
			break;
		else if (file.eof())
			throw std::runtime_error("Error reading file: unexpected end-of file");

		file.read(name, size * sizeof(char));
		*(name + size) = '\0'; // Null-terminate the name string
		try {
			const Element& elem = Element::withName(name);

			unsigned char count;
			file.read(reinterpret_cast<char*>(&count),
				sizeof(unsigned char));
			Group::withId(elem.groupid).memberWithId(elem.id).count = count;
		} catch (const std::runtime_error& e) {continue;}
	}

	file.close();
}

void SaveHandler::saveSettings()
{
	std::ofstream file(userDir_ + "/settings");
	file << "volume\t" << app_.musicGen_.getVolume() << std::endl;
	file << "fullscreen\t" << app_.getFullscreen() << std::endl;
	file << "pen\t" << Element::withId(app_.curElemId).name << std::endl;
	file << "pensize\t" << app_.penSize << std::endl;
	file << "quickbar\t";

	auto quickElemRange = app_.menu_.getQuickElems();
	for (auto it = quickElemRange.first; it != quickElemRange.second; ++it)
		file << static_cast<ElementButton*>(it->get())->getElem().name << ' ';
	file << std::endl;

	file.close();
}

void SaveHandler::loadSettings()
{
	std::ifstream file(userDir_ + "/settings");
	if (!file.is_open())
		throw std::runtime_error(std::string("Failed to open file '") + userDir_ + "/settings");

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

		if (setting == "volume") {
			try {
				app_.musicGen_.setVolume(std::stoi(value));
			} catch (const std::invalid_argument& e) {}
			catch (const std::out_of_range& e) {}
		} else if (setting == "fullscreen") {
#ifndef FORCE_FULLSCREEN
			try {
				if (app_.getFullscreen() != std::stoi(value))
					app_.toggleFullscreen();
			} catch (const std::invalid_argument& e) {}
#endif // FORCE_FULLSCREEN
		} else if (setting == "pen") {
			try {
				app_.curElemId = Element::withName(value).id;
			} catch (const std::runtime_error& e) {}
		} else if (setting == "pensize") {
			try {
				app_.penSize = std::stoi(value);
			} catch (const std::invalid_argument& e) {}
			catch (const std::out_of_range& e) {}
		} else if (setting == "quickbar") {
			bool cont = true;
			do {
				std::size_t spacePos = value.find(' ');
				if (spacePos == std::string::npos)
					cont = false;

				try {
					app_.menu_.addQuickElement(Element::withName(value.substr(0, spacePos)));
				} catch (const std::runtime_error& e) {}

				value = value.substr(spacePos + 1);
			} while (cont && value.size());
		}
	} while (!file.eof());

	file.close();
}

}
