"Resource/UI/TextWindow.res"
{
	"info"
	{
		"ControlName"		"CTextWindow"
		"fieldName"		"TextWindow"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"f0"
		"tall"			"480"
		"bgcolor_override"    "0 0 0 0"
	}

	"FolderBackground"
	{
		"ControlName"   "PanelTexture"
		"fieldName"     "FolderBackground"
		"xpos"          "c-320"
		"ypos"          "0"
		"wide"          "640"
		"tall"          "480"
		"zpos"          "-100"
		"image"         "folder_background"
	}

	"TopSecret"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"TopSecret"
		"xpos"			"c-255"
		"ypos"			"50"
		"wide"			"230"
		"tall"			"30"
		"labelText"		"#DA_MOTD_TopSecret"
		"textAlignment" "north"
		"font"			"FolderMedium"
		//"bgcolor_override" "0 0 0 100"
	}

	"MessageTitle"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"MessageTitle"
		"xpos"			"c5"
		"ypos"			"50"
		"wide"			"230"
		"tall"			"30"
		"labelText"		"Message Title"
		"textAlignment" "north"
		"font"			"FolderMedium"
		//"bgcolor_override" "0 0 0 100"
	}

	"HTMLMessage"
	{
		"ControlName"		"HTML"
		"fieldName"		"HTMLMessage"
		"xpos"			"c-255"
		"ypos"			"90"
		"wide"			"490"
		"tall"			"320"
	}

	"ok"
	{
		"ControlName"	"Button"
		"fieldName"		"ok"
		"xpos"			"c5"
		"ypos"			"418"
		"wide"			"230"
		"tall"			"20"
		"labelText"		"#PropertyDialog_OK"
		"textAlignment" "center"
		"command"		"okay"
		"font"			"FolderSmall"
	}
}
