"Resource/UI/FolderMenu.res"
{
	"team"
	{
		"ControlName"	"CClassMenu"
		"fieldName"		"team"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"f0"
		"tall"			"480"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"	"0"

		"frank_pose"
		{
			"sequence"		"john_pose"
			"body_yaw"		"0"
			"body_pitch"	"0"
			"weaponmodel"	"models/weapons/mossberg590.mdl"
			"zpos"			"-5000"
		}

		"wish_pose"
		{
			"sequence"		"wish_pose"
			"body_yaw"		"15"
			"body_pitch"	"-40"
			"weaponmodel"	"models/weapons/m1911.mdl"
			"zpos"			"-5000"
		}

		"bomber_pose"
		{
			"sequence"		"bomber_pose"
			"body_yaw"		"15"
			"body_pitch"	"-40"
			"weaponmodel"	"models/weapons/dualberetta.mdl"
			"zpos"			"-5000"
		}
	}

	"ChooseTeamLabel"
	{
		"ControlName"	"Label"
		"fieldName"		"ChooseTeamLabel"
		"xpos"			"c-255"
		"ypos"			"50"
		"wide"			"220"
		"tall"			"20"
		"font"			"FolderMedium"
		"textAlignment"	"center"
		"underline"     "1"
		"labelText"		"#DA_TeamMenu_JoinTeam"
	}

	"blue"
	{
		"ControlName"	"TeamButton"
		"fieldName"		"blue"
		"xpos"			"c-255"
		"ypos"			"80"
		"wide"			"230"
		"tall"			"300"
		"labelText"		"#DA_CharacterMenu_Button_Blue"
		"textAlignment"	"south"
		"Command"		"jointeam 2"
		"font"			"FolderSmall"

		"skin"          "1"
	}

	"red"
	{
		"ControlName"	"TeamButton"
		"fieldName"		"red"
		"xpos"			"c5"
		"ypos"			"80"
		"wide"			"230"
		"tall"			"300"
		"labelText"		"#DA_CharacterMenu_Button_Red"
		"textAlignment"	"south"
		"Command"		"jointeam 3"
		"font"			"FolderSmall"

		"skin"          "2"
	}

	"blueteaminfo"
	{
		"ControlName"	"Label"
		"fieldName"		"blueteaminfo"
		"xpos"			"c-255"
		"ypos"			"390"
		"wide"			"230"
		"tall"			"15"
		"labelText"		"#DA_TeamMenu_Info"
		"textAlignment"	"center"
		"font"			"FolderSmall"
	}

	"redteaminfo"
	{
		"ControlName"	"Label"
		"fieldName"		"redteaminfo"
		"xpos"			"c5"
		"ypos"			"390"
		"wide"			"230"
		"tall"			"15"
		"labelText"		"#DA_TeamMenu_Info"
		"textAlignment"	"center"
		"font"			"FolderSmall"
	}

	"CharacterImageRed"
	{
		"ControlName"	"CModelPanel"
		"fieldName"		"CharacterImageRed"
		"xpos"			"c5"
		"ypos"			"80"
		"wide"			"230"
		"tall"			"300"
		"zpos"			"-5"
		"autoResize"	"0"
		"pinCorner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"27"

		"model"
		{
			"spotlight"	"1"
			"modelname"	"models/player/wish.mdl"
			"origin_x"	"130"
			"origin_y"	"0"
			"origin_z"	"-35"
			"angles_y"	"180"

			"animation"
			{
				"sequence"		"wish_pose"
				"pose_parameters"
				{
					"body_yaw" "0.0"
					"body_pitch" "0.0"
				}
			}
			
			"attached_model"
			{
				"modelname" "models/weapons/m1911.mdl"
			}
		}
	}

	"CharacterImageBlue"
	{
		"ControlName"	"CModelPanel"
		"fieldName"		"CharacterImageBlue"
		"xpos"			"c-255"
		"ypos"			"80"
		"wide"			"230"
		"tall"			"300"
		"zpos"			"-5"
		"autoResize"	"0"
		"pinCorner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"27"

		"model"
		{
			"spotlight"	"1"
			"modelname"	"models/player/wish.mdl"
			"origin_x"	"130"
			"origin_y"	"0"
			"origin_z"	"-35"
			"angles_y"	"180"

			"animation"
			{
				"sequence"		"wish_pose"
				"pose_parameters"
				{
					"body_yaw" "0.0"
					"body_pitch" "0.0"
				}
			}
			
			"attached_model"
			{
				"modelname" "models/weapons/m1911.mdl"
			}
		}
	}

	"team_random"
	{
		"ControlName"	"TeamButton"
		"fieldName"		"team_random"
		"xpos"			"c158"
		"ypos"			"50"
		"wide"			"54"
		"tall"			"15"
		"labelText"		"#DA_Team_AutoAssign"
		"textAlignment"	"center"
		"font"			"FolderTiny"
		"Command"		"jointeam 0"
		"default"       "1"
		"zpos"			"5"
	}
}
