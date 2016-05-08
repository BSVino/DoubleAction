"Resource/UI/MapInfo.res"
{
	"intro"
	{
		"ControlName"	"CMapInfo"
		"fieldName"		"intro"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"f0"
		"tall"			"480"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"	"0"
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

	"MapName"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"MapName"
		"xpos"			"c5"
		"ypos"			"50"
		"wide"			"230"
		"tall"			"30"
		"labelText"		""
		"textAlignment" "north"
		"font"			"FolderMedium"
		//"bgcolor_override" "0 0 0 100"
	}

	"MapMessage"
	{
		"ControlName"	"HTML"
		"fieldName"		"MapMessage"
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
