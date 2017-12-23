
#ifndef __CDRICON_H__
#define __CDRICON_H__

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <string>
#include <map>

#include "ResourceManager.h"
#include "cdr_widgets.h"

class CdrIcon
{
public:
	typedef struct __CdrIon_t
	{
		enum ResourceID id;
		const char *name;
	}CdrIcon_t;

	CdrIcon(CdrIcon_t* iconData, XCreateParams* createParam);
	~CdrIcon();

	HWND getHwnd(void);
	std::string getName();
	PBITMAP getImage();
	enum ResourceID getId();
	unsigned int getIconIndex();
	void setIconIndex(unsigned int index);
	void updateIconIndex(void);

	void setBkgroud(int bkgroudId);
	void unsetBkgroud(void);

	int windowProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);
private:
	HWND mHwnd;
	HWND mHparent;
	enum ResourceID mId;
	std::string mName;
	BITMAP mBmp;
	BITMAP mBkgroudBmp;
	unsigned int mIconIndex;

	void updateIcon(void);
};


class CdrIconOps
{
public:
	CdrIconOps();
	~CdrIconOps();

	int createCdrIcon(CdrIcon::CdrIcon_t* iconData, XCreateParams* createParam );
	void hideIcon(enum ResourceID);
	void showIcon(enum ResourceID);
	CdrIcon* getIcon(enum ResourceID);
	size_t size(void);
	void setIconIndex(enum ResourceID id, int value);
	void updateIconIndex(enum ResourceID id);
	unsigned int getIconIndex(enum ResourceID id);

	void setBkgroud(enum ResourceID resId, int bkgroudId);
	void unsetBkgroud(enum ResourceID resId);

protected:
	std::map<ResourceID, CdrIcon*> mPic;
};

#endif
