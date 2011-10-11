#include "object.hpp"

namespace GameEngine
{
	class DummySerializable : public Serializable
	{
	public:
		virtual void
		Serialize(std::ostream *s) {
		}

		virtual void
		Deserialize(std::istream *s) {
		}
	};

	class DummyDrawable : public Drawable
	{
	public:
		virtual void Draw(tick_t tick, void *scene) { }
	};

	class DummyEvent : public Event
	{
	public:
		virtual void Activate(void) { }
	};

	static DummySerializable dummySerializable;
	static DummyDrawable   dummyDrawable;
	static DummyEvent      dummyEvent;

	Serializable * const Serializable::sDummy = &dummySerializable;
	Drawable     * const Drawable::sDummy = &dummyDrawable;
	Event        * const Event::sDummy = &dummyEvent;
	
	Serializable *
	Object::GetSerializable(void)
	{
		return Serializable::sDummy;
	};

	Drawable *
	Object::GetDrawable(void)
	{
		return Drawable::sDummy;
	}
}
