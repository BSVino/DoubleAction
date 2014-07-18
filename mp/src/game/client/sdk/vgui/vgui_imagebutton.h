#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H
 
#ifdef _WIN32
#pragma once
#endif
 
#include <vgui_controls/ImagePanel.h>
 
namespace vgui
{
 
class ImageButton : public vgui::ImagePanel
{
	DECLARE_CLASS_SIMPLE( ImageButton, vgui::ImagePanel );
 
public:
	ImageButton( Panel *parent, const char *panelName, const char *normalImage, const char *mouseOverImage = NULL, const char *mouseClickImage = NULL, const char *pCmd=NULL );
 
	virtual void OnCursorEntered(); // When the mouse hovers over this panel, change images
	virtual void OnCursorExited(); // When the mouse leaves this panel, change back
 
	virtual void OnMouseReleased( vgui::MouseCode code );
 
	 virtual void OnMousePressed( vgui::MouseCode code );
 
	void SetNormalImage( void );
	void SetMouseOverImage( void );
	void SetMouseClickImage( void );
 
private:
 
	char command[32]; // The command when it is clicked on
	vgui::IImage *i_normalImage; // The image when the mouse isn't over it, and its not being clicked
	vgui::IImage *i_mouseOverImage; // The image that appears as when the mouse is hovering over it
	vgui::IImage *i_mouseClickImage; // The image that appears while the mouse is clicking
	char m_normalImage[32];
	char m_mouseOverImage[32];
	char m_mouseClickImage[32];
	Panel* m_pParent;
 
	bool hasCommand; // If this is to act as a button
	bool hasMouseOverImage; // If this changes images when the mouse is hovering over it
	bool hasMouseClickImage; // If this changes images when the mouse is clicking it
 
	virtual void SetImage( vgui::IImage *image ); //Private because this really shouldnt be changed
};
 
} //namespace vgui
 
#endif //IMAGEBUTTON_H
