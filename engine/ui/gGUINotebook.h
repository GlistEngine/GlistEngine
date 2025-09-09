/*
 * gGUINotebook.h
 *
 *  Created on: 30 Apr 2024
 *      Author: Metehan Gezer
 */

#ifndef UI_GGUINOTEBOOK_H_
#define UI_GGUINOTEBOOK_H_

#include "gGUIContainer.h"
#include "gGUIPanel.h"
#include "gRenderer.h"
#include "gFont.h"
#include <deque>
#include <utility>

class gGUINotebook : public gGUIContainer {
public:
	enum class TabPosition {
		TOP,
		BOTTOM,
		LEFT,
		RIGHT
	};

	gGUINotebook();
	virtual ~gGUINotebook();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) override;

	void draw() override;


	/**
	 * @brief Sets the position of the tab bar.
	 *
	 * @param position The desired tab position. ex: gGUINotebook::TabPosition::LEFT
	 * @see getTabPosition
	 */
	void setTabPosition(TabPosition position);

	/**
	 * @brief Get the current tab position.
	 *
	 * Tab position determines where the tab bar is
	 * displayed relative to the tab contents.
	 *
	 * @return The current tab position as TabPosition enum.
	 */
	TabPosition getTabPosition() const;

	/**
	 * Sets the visibility of the tab header.
	 *
	 * @param visible Whether the tab header should be visible or not.
	 */
	void setTabVisibility(bool visible);

	/**
	 * @brief Sets the active tab of the notebook.
	 *
	 * This function sets the active tab of the notebook to the tab at the given index.
	 * If the index is out of range, the active tab is set to -1 and no tab will be selected.
	 *
	 * @param index The index of the tab to set as active.
	 */
	void setActiveTab(int index);

	/**
	 * @brief Gets the index of the active tab.
	 *
	 * This function returns the index of the active tab in the notebook.
	 *
	 * @return The index of the active tab. -1 if none is selected
	 */
	int getActiveTab() const;

	/**
	 * @brief Sets whether a tab is closable or not.
	 *
	 * @param index The index of the tab.
	 * @param isClosable True if the tab should be closable, false otherwise.
	 */
	void setTabClosable(int index, bool isClosable);


	/**
	 * @brief Returns if a tab is closable or not.
	 *
	 * @param index The index of the tab to get.
	 * @return True if the tab is closable, false otherwise.
	 *
	 * @note If the index is out of range or if there are no tabs in the notebook, false will be returned.
	 */
	bool isTabClosable(int index);


	/**
	 * @brief Sets whether a tab is automatically closed when switched out of it.
	 *
	 * @param index The index of the tab.
	 * @param autoClose True if the tab should be automatically closed, false otherwise.
	 */
	void setTabAutoClose(int index, bool autoClose);

	/**
	 * @brief Returns if a tab is set to automatically close when switched out of it.
	 *
	 * @param index The index of the tab.
	 * @return True if the tab is set to automatically close, false otherwise.
	 */
	bool isTabAutoClose(int index);


	/**
	 * @brief Add a tab to a GUI sizer.
	 *
	 * @param sizer The GUI sizer to which the tab is added.
	 * @param title The title of the tab.
	 * @param closable Whether the tab should be closable. Default is true.
	 *
	 * @return Index that the tab is added, do not store this return value as it can change when a tab before this has closed!
	 *
	 */
	int addTab(gGUISizer* sizer, std::string title, bool closable = true);

	/**
	 * @brief Closes the tab at the specified index.
	 *
	 * If the index is out of range, or if there are no tabs in the notebook, the function does nothing.
	 * If the active tab is closed, previous tab will be active (if available).
	 *
	 * @param index The index of the tab to be closed.
	 */
	void closeTab(int index);

	/**
	 * @brief Gets the tab sizer of the tab at the given index.
	 *
	 * @param index The index of the tab sizer to retrieve.
	 * @return The tab sizer object. It will return nullptr if index is out of bounds.
	 *
	 * @warning It is the responsibility of the caller to handle null pointers returned by this function!
	 */
	gGUISizer* getTabSizer(int index);

	/**
	 * @brief Gets the tab sizer by its title.
	 *
	 * @param title Title of the tab sizer to get.
	 * @return gGUISizer object of the matching tab title, nullptr if none matches.
	 *
	 * @note This function assumes that all tab sizes have unique titles. If not, first matching will be returned.
	 * @warning It is the responsibility of the caller to handle null pointers returned by this function!
	 */
	gGUISizer* getTabSizerByTitle(const std::string& title);

private:
	struct Box {
		int ox = 0, oy = 0, x, y, w, h;

		bool enabled = false;

		bool isPointInside(int tx, int ty) {
			if (!enabled) {
				return false;
			}
			return tx >= ox + x && tx <= ox + x + w && ty >= oy + y && ty <= oy + y + h;
		}

		void flipXY() {
			int t = x;
			x = y,
			y = t;

			t = h;
			h = w;
			w = t;
		}

		Box rotate(TabPosition pos, int boxw, int boxh) const {
			switch (pos) {
			case TabPosition::TOP: {
				return *this;
			}
			case TabPosition::LEFT: {
				Box box = *this;
				box.flipXY();
				return box;
			}
			case TabPosition::RIGHT: {
				Box box = *this;
				box.flipXY();
				box.x = boxw - box.w;
				return box;
			}
			case TabPosition::BOTTOM: {
				Box box = *this;
				box.y = boxh - box.h;
				return box;
			}
			default:
				return *this;
			}
		}

		void render() const {
			gDrawRectangle(x + ox, y + oy, w, h);
		}

	};
	struct Tab {
		gGUISizer* sizer;
		std::string title;
		bool closable;
		bool autoclose;

		// text size of the title tab
		int titlewidth;
		int titleheight;

		// total size of the title tab
		int tabwidth;

		Box tabbox;
		Box closebox;

		Tab(gGUISizer* sizer, std::string title,  bool closable, int titlewidth, int titleheight)
			: sizer(sizer), title(std::move(title)), closable(closable), autoclose(false),
			  titlewidth(titlewidth), titleheight(titleheight), tabwidth(0) {

		}
	};


	void drawHeader();
	void drawHeaderBackground();

	void mousePressed(int x, int y, int button) override;
	void mouseMoved(int x, int y) override;
	void mouseScrolled(int x, int y) override;

	void updateSizer();

	Tab* getTab(int index);
	int findIndexByTitle(const std::string& title);

private:
	std::vector<Tab> tabs;
	int tabscroll;
	int activetab;
	int headerheight;

	int titlepadding;
	int tabgap; // gap between each tab
	int scrollbuttonwidth;
	int closebuttonsize;

	int lastmousex = 0;
	int lastmousey = 0;

	bool tabvisibility;
	Box scrollpreviousbutton;
	Box scrollnextbutton;
	Box headerbox;
	Box notebookbox;

	TabPosition tabposition;
	gGUISizer notebooksizer;
	gFont* titlefont;
	gFbo fbo;
};

#endif /* UI_GGUINOTEBOOK_H_ */
