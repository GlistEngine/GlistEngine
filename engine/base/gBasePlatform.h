/*
 * gBasePlatform.h
 *
 *  Created on: Aug 14, 2026
 */

#ifndef ENGINE_BASE_GBASEPLATFORM_H_
#define ENGINE_BASE_GBASEPLATFORM_H_

class gBaseWindow;

/**
 * Supplies the platform objects the engine needs, so a platform can be added
 * from a plugin without the engine knowing that plugin exists.
 *
 * A backend registers itself by being instantiated, the same way plugins do.
 * The instance has to outlive the app, so plugins define it as a static object
 * in their own translation unit:
 *
 *     static gipMyPlatform platform;
 *
 * Nothing in the engine refers to that object, and engine sources end up in a
 * static library, where the linker drops members nothing needs. The file
 * defining the backend therefore belongs in the app rather than the library,
 * which a plugin arranges by appending it to PLUGIN_EXECUTABLE_SRCS in its
 * CMakeLists instead of PLUGIN_SRCS. The engine passes that list on to whoever
 * links it, so apps build platform plugins without changing their CMakeLists.
 *
 * Platforms the engine ships with (desktop, Android, iOS) do not use this and
 * are chosen directly, so getCurrent() is null for them.
 */
class gBasePlatform {
public:
	gBasePlatform();
	virtual ~gBasePlatform();

	/**
	 * Creates the window for this platform. Called once at startup, and the
	 * engine takes ownership of the returned window.
	 */
	virtual gBaseWindow* createWindow() = 0;

	/**
	 * The registered backend, or nullptr when the engine picks the platform
	 * itself.
	 */
	static gBasePlatform* getCurrent();

private:
	static gBasePlatform* current;
};

#endif /* ENGINE_BASE_GBASEPLATFORM_H_ */
