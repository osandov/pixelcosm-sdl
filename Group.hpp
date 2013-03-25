#ifndef PARTICLES_GROUP_HPP
#define PARTICLES_GROUP_HPP

#include "Dependencies.hpp"

namespace PixelCosm
{

class Subgroup
{
public:
	class param_type
	{
	public:
		std::string name;
		SDL_Surface* image;
	};

	/** The internally generated identification number of the Subgroup */
	const int id;

	/** The name of the Subgroup */
	const std::string name;

	/** The image used to render the Subgroup */
	SDL_Surface* const image;

	Subgroup(const param_type& params, int id) :
		id(id), name(params.name), image(params.image) {}
};

class Group {
	static std::vector<Group> groups_;
	bool newAvailable_ = false;

public:
	class param_type
	{
	public:
		std::string name;
		SDL_Surface* image;
	};

	/** The internally generated identification number of the Group */
	const int id;

	/** The text name of the Group */
	const std::string name;

	/** The image used to display the Group */
	SDL_Surface* const image;

	/** Subgroups of the Group */
	std::vector<Subgroup> subgroups;

	/** @class Descriptor for member Element */
	class Member
	{
	public:
		int subid, elemid;
		bool secret;
		unsigned char count;

		Member(int subid, int elemid, bool secret) :
			subid(subid), elemid(elemid), secret(secret),
			count(secret ? 0 : REQUIRED_PARTICLES) {}

		bool operator<(const Member& rhs) const
		{
			return (subid == rhs.subid) ? (elemid < rhs.elemid) : (subid < rhs.subid);
		}
	};

	/** The member Elements of the Group */
	std::vector<Member> members;

	Group(const param_type& params);

	static Group& withId(int id);
	static Group& withName(const std::string& name);

	/** Returns the identification number following the last generated
	 * identification number
	 */
	static int getEndId();

	static void create(const param_type& params);

	void addSubgroup(const Subgroup::param_type& params);
	Subgroup& subWithName(const std::string& name);

	void addMember(int subid, int elemid, bool secret);
	Member& memberWithId(int elemid);

	std::vector<Member>::difference_type countAvailable() const;
	void newAvailable(bool set);
	bool newAvailable() const;
};

}

#endif /* PARTICLES_GROUP_HPP */

