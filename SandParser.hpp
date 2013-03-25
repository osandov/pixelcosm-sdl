#ifndef PARTICLEGAME_SANDPARSER_HPP
#define PARTICLEGAME_SANDPARSER_HPP

#include "Dependencies.hpp"

#include "Group.hpp"
#include "Element.hpp"
#include "Interaction.hpp"

namespace PixelCosm
{

class PixelCosmApp;
class ParentParser;

class ElementGroup {
	static std::vector<ElementGroup> elementGroups_;

public:
	class param_type {
	public:
		std::string name;
	};

	/** The text name of the ElementGroup */
	std::string name;

	/** The identification numbers of the Elements in the
	 * ElementGroup
	 */
	std::vector<int> memberIds;

	ElementGroup(const param_type& params) :
		name(params.name) {}

	void addId(int id) {
		memberIds.push_back(id);
	}

	static ElementGroup& withName(const std::string& name) {
		auto found = std::find_if(elementGroups_.begin(), elementGroups_.end(),
			[&name](const ElementGroup& g) {return g.name == name;});
		if (found == elementGroups_.end())
			throw std::runtime_error(
				std::string("No ElementGroup with name ' ") + name + " '");
		return *found;
	}

	static void create(const param_type& params) {
		auto found = std::find_if(elementGroups_.begin(), elementGroups_.end(),
			[&params](const ElementGroup& g) {return g.name == params.name;});
		if (found != elementGroups_.end())
			throw std::runtime_error(
				std::string("ElementGroup with name ' ") + params.name + " ' already exists");
		elementGroups_.emplace_back(params);
	}
};

class GroupInterExclusion
{
public:
	std::string keyToken;
	int excludeId;

	bool operator==(const GroupInterExclusion& rhs)
	{
		return excludeId == rhs.excludeId && keyToken == rhs.keyToken;
	}
};

class InterCopy
{
public:
	int line;
	std::string from, to;
	int fromId, toId;

	InterCopy(int line, const std::string& from, const std::string& to) :
		line(line), from(from), to(to) {}
};

class TowardsList
{
public:
	std::string elemName;
	std::vector<std::string> names;

	TowardsList(const std::string& elemName) : elemName(elemName) {}
};

class SandParser
{
public:
	class Error : public std::exception {
		int line_;
		std::string what_;
		bool isWarning_;

		void construct(std::ostringstream& stream) {}

		template <typename T, typename... Args>
		void construct(std::ostringstream& stream, T value, Args... args)
		{
			stream << value;
			construct(stream, args...);
		}

	public:
		Error(const Error& rhs) :
			line_(rhs.getLine()), what_(rhs.what()),
			isWarning_(rhs.isWarning()) {}

		template <typename... Args>
		Error(int line, Args... args) :
			line_(line), isWarning_(false)
		{
			std::ostringstream stream;
			stream << "line " << line << ": ";
			construct(stream, args...);
			what_ = stream.str();
		}

		template <typename... Args>
		Error(int line, bool isWarning, Args... args) :
			line_(line), isWarning_(isWarning)
		{
			std::ostringstream stream;
			stream << "line " << line << ": ";
			construct(stream, args...);
			what_ = stream.str();
		}

		bool isWarning() const
		{
			return isWarning_;
		}

		int getLine() const
		{
			return line_;
		}

		const char* what() const throw() {return what_.c_str();}
	};

	class ErrorList {
		std::string filename_;
		std::vector<Error> errors_;

	public:
		ErrorList(const std::string& filename) : filename_(filename) {}

		const std::string& getFileName() const
		{
			return filename_;
		}

		void add(const Error& e)
		{
			errors_.insert(std::find_if(errors_.begin(), errors_.end(),
				[&e](const Error& rhs) {return e.getLine() < rhs.getLine();}), e);
		}

		bool empty() const
		{
			return errors_.empty();
		}

		std::vector<Error>::const_iterator begin() const
		{
			return errors_.begin();
		}

		std::vector<Error>::const_iterator end() const
		{
			return errors_.end();
		}
	};

private:
	PixelCosmApp& app_;
	ParentParser& parent_;
	std::string sandsrcDir_;
	std::string line_;
	int curLine_ = 0;
	int elemLine_ = 0;
	std::ifstream file_;
	std::vector<std::string> tokens_;

	Element::param_type elemparams_;
	int elemSubid_ = 0;
	bool elemIsSecret_ = false;

	bool parsingElem_ = false;
	int wildcardId_;

	std::vector<TowardsList> towardsListQueue_;
	std::vector<GroupInterExclusion> exclusionQueue_;
	std::vector<InterCopy> interCopyQueue_;

	ErrorList errors_;

	int readInt(const std::string& str, const std::string& label) {
		try {
			return std::stoi(str);
		} catch (const std::invalid_argument& e) {
			throw Error{curLine_, label, "must be an integer value"};
		} catch (const std::out_of_range& e) {
			throw Error{curLine_, label, "out of range"};
		}
	}

	float readDouble(const std::string& str, const std::string& label) {
		try {
			return std::stod(str);
		} catch (const std::invalid_argument& e) {
			throw Error{curLine_, label, "must be an integer value"};
		} catch (const std::out_of_range& e) {
			throw Error{curLine_, label, "out of range"};
		}
	}

	void parseTokens();

	void firstPassHandle();
	void secondPassHandle();

	void handleIncludeLine();
	void handleGroupLine();
	void handleSubgroupLine();
	void handleElementGroupLine();
	void handleElemLine();
	void handleElemParam();
	void handleInterLine();
	void handleAddToElemGroupLine();
	void handleExcludeLine();
	void handleCopyLine();
	void checkCopyQueue();

	void createElem();

	void addInteraction(const std::string& name1, const std::string& name2);
	void addInteraction(const std::string& name1, const std::string& name2, const std::string& orig1);

	Interaction* handleElemChangeInter(const Interaction::param_type& baseParams, bool addition, int i);
	Interaction* handleRepelInter(const Interaction::param_type& baseParams, bool addition, int i);
	Interaction* handleAttractInter(const Interaction::param_type& baseParams, bool addition, int i);
	Interaction* handleAngleInter(const Interaction::param_type& baseParams, bool addition, int i);
	Interaction* handleCancelForceInter(const Interaction::param_type& baseParams, bool addition, int i);
	Interaction* handleSwapInter(const Interaction::param_type& baseParams, bool addition, int i);

	void expandInterTowards(const TowardsList& l);

public:
	SandParser(ParentParser& parent, const std::string& filename);

	const ErrorList& parse();
};

class ParentParser
{
public:
	typedef std::vector<SandParser::ErrorList> ErrorLists;

private:
	PixelCosmApp& app_;
	std::deque<std::string> queue_;
	ErrorLists errors_;

public:
	ParentParser(PixelCosmApp& app, const std::string& filename);

	PixelCosmApp& getApp();
	void addToQueue(const std::string& filename);

	ErrorLists& parse();
};

}

#endif /* PARTICLEGAME_SANDPARSER_HPP */
