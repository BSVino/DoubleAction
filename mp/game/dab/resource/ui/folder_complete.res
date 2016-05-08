"Resource/UI/FolderMenu.res"
{
	"folder"
	{
		"ControlName"	"CFolderMenu"
		"fieldName"		"folder"
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

	"AgentName"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"AgentName"
		"xpos"			"c5"
		"ypos"			"50"
		"wide"			"220"
		"tall"			"20"
		"font"			"FolderMedium"
		"textAlignment"	"center"
		"underline"     "1"
		"labelText"		"#DA_Checklist_Heading"
		//"bgcolor_override" "0 0 0 100"
	}

	"player_preview"
	{
		"ControlName"	"CModelPanel"
		"fieldName"		"player_preview"
		"xpos"          "c-5"
		"ypos"          "70"
		"wide"          "120"
		"tall"          "210"
		"zpos"			"-25"
		"autoResize"	"0"
		"pinCorner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"20"
		//"bgcolor_override" "150 150 150 255"

		"model"
		{
			"spotlight"	"1"
			"modelname"	"models/player/frank.mdl"
			"origin_x"	"130"
			"origin_y"	"0"
			"origin_z"	"-35"
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

	"WeaponIconArea"
	{
		"ControlName"   "FolderLabel"
		"fieldName"     "WeaponIconArea"
		"xpos"			"c115"
		"ypos"			"90"
		"wide"			"120"
		"tall"			"200"
		"zpos"          "1"
		"image"         ""
		//"bgcolor_override" "0 0 0 155"
	}

	"WeaponTotalWeight"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"WeaponTotalWeight"
		"xpos"			"c150"
		"ypos"			"270"
		"wide"			"55"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_TotalWeight"
		"textAlignment"	"east"
		"font"			"FolderTiny"
		//"bgcolor_override" "0 0 0 100"
	}

	"WeaponTotalWeightNumber"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"WeaponTotalWeightNumber"
		"xpos"			"c210"
		"ypos"			"270"
		"wide"			"30"
		"tall"			"15"
		"labelText"		"0/30"
		"textAlignment"	"west"
		"font"			"FolderTiny"
		//"bgcolor_override" "0 0 0 100"
	}

	"StyleSkill"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"StyleSkill"
		"xpos"			"c5"
		"ypos"			"300"
		"wide"			"230"
		"tall"			"160"
		"font"			"FolderSmall"
		"textAlignment"	"north-west"
		"wrap"          "1"
		"labelText"		"#DA_BuyMenu_StyleSkill"
		//"bgcolor_override" "0 0 0 100"
	}

	"SkillInfo"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"SkillInfo"
		"xpos"			"c5"
		"ypos"			"320"
		"wide"			"150"
		"tall"			"60"
		"font"			"FolderTiny"
		"textAlignment"	"north-west"
		"wrap"          "1"
		//"bgcolor_override" "0 0 0 100"
	}

	"SkillIcon"
	{
		"ControlName"   "PanelTexture"
		"fieldName"		"SkillIcon"
		"xpos"			"c180"
		"ypos"			"295"
		"wide"			"60"
		"tall"			"60"
		"image"         ""
		"zpos"          "-1"
	}

	"ApproveButton"
	{
		"ControlName"	"Button"
		"fieldName"		"ApproveButton"
		"xpos"			"c5"
		"ypos"			"365"
		"wide"			"230"
		"tall"			"55"
		"labelText"		"#DA_Close"
		"textAlignment"	"center"
		"font"			"FolderLarge"
		"Command"		"respawn"
	}

	"suicide_option"
	{
		"ControlName"	"CheckButton"
		"fieldName"		"suicide_option"
		"xpos"			"c-5"
		"ypos"			"420"
		"zpos"			"5"
		"wide"			"240"
		"tall"			"21"
		"labelText"		"#DA_Checklist_AutoKill"
		"textAlignment"	"west"
		"font"			"FolderTiny"
	}

	"ChangeTeamsButton"
	{
		"ControlName"	"Button"
		"fieldName"		"ChangeTeamsButton"
		"xpos"			"c105"
		"ypos"			"11"
		"zpos"			"5"
		"wide"			"80"
		"tall"			"15"
		"labelText"		"#DA_Menu_ChooseTeam"
		"textAlignment"	"center"
		"font"			"FolderTiny"
		"Command"		"tab team"
	}

	"SpectateButton"
	{
		"ControlName"	"Button"
		"fieldName"		"SpectateButton"
		"xpos"			"c195"
		"ypos"			"11"
		"zpos"			"5"
		"wide"			"50"
		"tall"			"15"
		"labelText"		"#DA_Menu_Spectate"
		"textAlignment"	"center"
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
