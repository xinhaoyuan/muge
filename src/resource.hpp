#ifndef __RESOURCE_HPP__
#define __RESOURCE_HPP__

#include <map>
#include <string>

namespace Game
{
	class Resource
	{
		static std::map<std::string, void *> sMap;
		
	public:

		template<class T>
		static T *Get(const char *name) {
			if (sMap.find(name) == sMap.end())
				sMap[name] = T::Load(name);
			return (T *)sMap[name];
		}
	};
}

#endif
