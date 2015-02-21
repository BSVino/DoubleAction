//The following include files are necessary to allow your MyPanel.cpp to compile.
#include "cbase.h"
#include "IDARadioPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>

//CMyPanel class: Tutorial example class
class CDARadioMenu : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CDARadioMenu, vgui::Frame);
	//CMyPanel : This Class / vgui::Frame : BaseClass

	CDARadioMenu(vgui::VPANEL parent); 	// Constructor
	~CDARadioMenu(){};				// Destructor

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);

private:
	//Other used VGUI control Elements:
	Button *m_pCloseButton;



};
// Constuctor: Initializes the Panel
CDARadioMenu::CDARadioMenu(vgui::VPANEL parent)	: BaseClass(NULL, "MyPanel")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetVisible(true);

	/*
	m_pCloseButton = new Button(this, "Button", "Close", this, "turnoff");
	m_pCloseButton->SetPos(433, 472);
	m_pCloseButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pCloseButton->SetReleasedSound("ui/buttonclick.wav");
	*/

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/radiomenu.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	DevMsg("MyPanel has been constructed\n");
}

//Class: CMyPanelInterface Class. Used for construction.
class CDARadioMenuInterface : public IDARadioPanel
{
private:
	CDARadioMenu *MyPanel;
public:
	CDARadioMenuInterface()
	{
		MyPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		MyPanel = new CDARadioMenu(parent);
	}
	void Destroy()
	{
		if (MyPanel)
		{
			MyPanel->SetParent( (vgui::Panel *)NULL);
			delete MyPanel;
		}
	}
	void Activate(void)
	{
		if (mypanel)
		{
			mypanel->Activate();
		}
	}

};
static CDARadioMenuInterface g_MyPanel;
IDARadioPanel* mypanel = (IDARadioPanel*)&g_MyPanel;

ConVar cl_showmypanel("cl_showmypanel", "0", FCVAR_CLIENTDLL, "Sets the state of myPanel <state>");

void CDARadioMenu::OnTick()
{
	BaseClass::OnTick();
	SetVisible(cl_showmypanel.GetBool()); //CL_SHOWMYPANEL / 1 BY DEFAULT
}
 
void CDARadioMenu::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showmypanel.SetValue(0);

	if (!Q_stricmp(pcCommand, "option1"))
	{
		//play a sound
		//print text to the screen
	}
}

CON_COMMAND(ToggleMyPanel, "Toggles myPanel on or off")
{
	cl_showmypanel.SetValue(!cl_showmypanel.GetBool());
	mypanel->Activate();
};


