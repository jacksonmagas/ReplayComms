#pragma once

#include <ranges>
#include <source_location>
#include <iostream>

#define MY_DEBUG true;

template<class C>
struct Invariant {
	std::string description;
	std::function<bool(const C&)> predicate;
};

template<class C>
concept InvariantClass = requires(const C& c)
{
	{ C::invariants() } -> std::ranges::viewable_range;
	requires std::same_as<std::ranges::range_value_t<decltype(C::invariants())>, Invariant<C>>;
};

template<InvariantClass C>
struct InvariantGuard
{
	enum class Location {
		Start, End
	};

	explicit InvariantGuard(const C* c, std::source_location loc = std::source_location::current()) :
		_class(*c),
		_loc(loc)
	{
		assertInvariants(Location::Start);
	};

	~InvariantGuard()
	{
		assertInvariants(Location::End);
	}

	void assertInvariants(Location l) const {
	#if defined(MY_DEBUG)
		bool failed = false;
		for (auto invariant : C::invariants())
		{
            if (!invariant.predicate(_class)) {
                std::cerr << "Invariant failed: " << invariant.description
                          << " at " << _loc.file_name() << ":"
                          << _loc.function_name() << ", ";
				switch (l)
				{
				case Location::Start:
					std::cerr << "precondition";
					break;
				case Location::End:
					std::cerr << "postcondition";
				}
				std::cerr << std::endl;
				failed = true;
            }
		}
		if (failed)
		{
			std::abort();
		}
	#endif
	}

	std::source_location _loc;
	const C& _class;
};
