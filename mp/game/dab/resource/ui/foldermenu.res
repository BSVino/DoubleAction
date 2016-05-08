"Resource/UI/FolderMenu.res"
{
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

	"Stain"
	{
		"ControlName"   "PanelTexture"
		"fieldName"     "Stain"
		"xpos"          "c-265"
		"ypos"          "355"
		"wide"          "110"
		"tall"          "90"
		"zpos"          "-50"
		"image"         "folder_stain"
	}

	"AgentDeploymentChecklist"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"AgentDeploymentChecklist"
		"xpos"			"c-250"
		"ypos"			"45"
		"wide"			"220"
		"tall"			"35"
		"font"			"FolderMedium"
		"textAlignment"	"center"
		"labelText"		"#DA_Checklist_Heading"
		//"bgcolor_override" "0 0 0 100"
	}

	"player_preview_background"
	{
		"ControlName"   "ImagePanel"
		"fillcolor"     "0 0 0 255"
		"xpos"          "c-257"
		"ypos"          "85"
		"wide"          "100"
		"tall"          "120"
		"zpos"          "-10"
	}

	"player_preview"
	{
		"ControlName"	"CModelPanel"
		"fieldName"		"player_preview"
		"xpos"          "c-257"
		"ypos"          "85"
		"wide"          "100"
		"tall"          "120"
		"zpos"			"-5"		
		"autoResize"	"0"
		"pinCorner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"20"
		"bgcolor_override" "150 150 150 255"

		"model"
		{
			"spotlight"	"1"
			"modelname"	"models/player/frank.mdl"
			"origin_z"	"-35"
			"origin_y"	"0"
			"origin_x"	"130"
			"angles_y"	"180"

			"animation"
			{
				"sequence"		"m1911_idle"
				"pose_parameters"
				{
					"body_yaw" "25.0"
					"body_pitch" "-30.0"
				}
			}
			
			"attached_model"
			{
				"modelname" "models/weapons/m1911.mdl"
			}
		}
	}

	"ProfileHeader"
	{
		"ControlName"	"PanelTexture"
		"fieldName"		"ProfileHeader"
		"xpos"			"c-157"
		"ypos"			"85"
		"wide"			"137"
		"tall"			"26"
		"zpos"          "-10"
		"image"         "folder_header_small"
	}

	"ProfileTitle"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"ProfileTitle"
		"xpos"			"c-152"
		"ypos"			"85"
		"wide"			"130"
		"tall"			"26"
		"labelText"		"#DA_Checklist_Profile"
		"textAlignment"	"west"
		"font"			"FolderTiny"
		"fgcolor_override" "255 255 255 255"
	}

	"ProfileInfo"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"ProfileInfo"
		"xpos"			"c-152"
		"ypos"			"115"
		"wide"			"130"
		"tall"			"100"
		"labelText"		"#DA_Checklist_Profile"
		"textAlignment"	"north-west"
		"font"			"FolderTiny"
	}

	"RequestedArmamentHeader"
	{
		"ControlName"	"PanelTexture"
		"fieldName"		"RequestedArmamentHeader"
		"xpos"			"c-260"
		"ypos"			"205"
		"wide"			"240"
		"tall"			"26"
		"image"         "folder_header_large"
		"zpos"          "-10"
	}

	"RequestedArmamentTitle"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"RequestedArmamentTitle"
		"xpos"			"c-255"
		"ypos"			"205"
		"wide"			"220"
		"tall"			"26"
		"labelText"		"#DA_Checklist_Requisitions"
		"textAlignment"	"west"
		"font"			"FolderTiny"
		"fgcolor_override" "255 255 255 255"
	}

	//"SlotsRemaining"
	//{
	//	"ControlName"	"FolderLabel"
	//	"fieldName"		"SlotsRemaining"
	//	"xpos"			"c-145"
	//	"ypos"			"205"
	//	"wide"			"120"
	//	"tall"			"26"
	//	"labelText"		"#DA_SlotsRemaining"
	//	"textAlignment"	"east"
	//	"font"			"FolderTiny"
	//	"fgcolor_override" "255 255 255 255"
	//}

	"RequestedArmament1"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"RequestedArmament1"
		"xpos"			"c-260"
		"ypos"			"230"
		"wide"			"115"
		"tall"			"85"
		"font"			"FolderTiny"
		"textAlignment"	"north-west"
		"labelText"		""
		"wrap"          "1"
		//"bgcolor_override" "0 0 0 100"
	}

	"RequestedArmament2"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"RequestedArmament2"
		"xpos"			"c-140"
		"ypos"			"230"
		"wide"			"115"
		"tall"			"85"
		"font"			"FolderTiny"
		"textAlignment"	"north-west"
		"labelText"		""
		"wrap"          "1"
		//"bgcolor_override" "0 0 0 100"
	}

	"CharacteristicsHeader"
	{
		"ControlName"	"PanelTexture"
		"fieldName"		"CharacteristicsHeader"
		"xpos"			"c-260"
		"ypos"			"315"
		"wide"			"240"
		"tall"			"26"
		"image"         "folder_header_large"
		"zpos"          "-10"
	}

	"CharacteristicsTitle"
	{	
		"ControlName"	"FolderLabel"
		"fieldName"		"CharacteristicsTitle"
		"xpos"			"c-255"
		"ypos"			"315"
		"wide"			"240"
		"tall"			"26"
		"labelText"		"#DA_Checklist_Characteristics"
		"textAlignment"	"west"
		"font"			"FolderTiny"
		"fgcolor_override" "255 255 255 255"
	}

	"CharacteristicsInfo"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"CharacteristicsInfo"
		"xpos"			"c-255"
		"ypos"			"340"
		"wide"			"240"
		"tall"			"100"
		"labelText"		""
		"textAlignment"	"north-west"
		"font"			"FolderTiny"
		"wrap"          "1"
	}

	"suicide_option"
	{
		"ControlName"	"CheckButton"
		"fieldName"		"suicide_option"
		"xpos"			"c-260"
		"ypos"			"420"
		"zpos"			"5"
		"wide"			"240"
		"tall"			"21"
		"labelText"		"#DA_Checklist_AutoKill"
		"textAlignment"	"west"
		"font"			"FolderTiny"
	}

	"SpectateButton"
	{
		"ControlName"	"Button"
		"fieldName"		"SpectateButton"
		"xpos"			"c-260"
		"ypos"			"450"
		"zpos"			"5"
		"wide"			"50"
		"tall"			"15"
		"labelText"		"#DA_Menu_Spectate"
		"textAlignment"	"west"
		"font"			"FolderTiny"
		"Command"		"spectate"
	}

	"AgentsTab"
	{
		"ControlName"	"ImageButton"
		"fieldName"		"AgentsTab"
		"xpos"			"c235"
		"ypos"			"50"
		"zpos"			"5"
		"wide"			"56"
		"tall"			"28"
		"labelText"		""
		"textAlignment"	"west"
		"font"			"FolderTiny"
		"Command"		"tab characters"
		"image"         "folder_tab_agents"
	}

	"WeaponsTab"
	{
		"ControlName"	"ImageButton"
		"fieldName"		"WeaponsTab"
		"xpos"			"c235"
		"ypos"			"80"
		"zpos"			"5"
		"wide"			"56"
		"tall"			"28"
		"labelText"		""
		"textAlignment"	"west"
		"font"			"FolderTiny"
		"Command"		"tab weapons"
		"image"         "folder_tab_weapons"
	}

	"SkillsTab"
	{
		"ControlName"	"ImageButton"
		"fieldName"		"SkillsTab"
		"xpos"			"c235"
		"ypos"			"110"
		"zpos"			"5"
		"wide"			"56"
		"tall"			"28"
		"labelText"		""
		"textAlignment"	"west"
		"font"			"FolderTiny"
		"Command"		"tab skills"
		"image"         "folder_tab_skills"
	}
}
