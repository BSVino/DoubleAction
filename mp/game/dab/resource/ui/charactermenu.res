"Resource/UI/Character.res"
{
	"class"
	{
		"ControlName"	"CClassMenu"
		"fieldName"		"class"
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

	"Agents"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"Agents"
		"xpos"			"c-250"
		"ypos"			"50"
		"wide"			"220"
		"tall"			"20"
		"font"			"FolderMedium"
		"textAlignment"	"center"
		"underline"     "1"
		"labelText"		"#DA_BuyMenu_Agents"
		//"bgcolor_override" "0 0 0 100"
	}

	"frank"
	{
		"ControlName"	"CharacterButton"
		"fieldName"		"frank"
		"xpos"			"c-250"
		"ypos"			"100"
		"wide"			"100"
		"tall"			"100"
		"labelText"		"#DA_CharacterMenu_Button_Frank"
		"textAlignment"	"center"
		"Command"		"character frank"
		"font"			"FolderSmall"

		"character"     "frank"
		"sequence"		"john_pose"
		"body_yaw"		"0"
		"body_pitch"	"0"
		"weaponmodel"	"models/weapons/mossberg590.mdl"
	}

	"wish"
	{
		"ControlName"	"CharacterButton"
		"fieldName"		"wish"
		"xpos"			"c-140"
		"ypos"			"100"
		"wide"			"100"
		"tall"			"100"
		"labelText"		"#DA_CharacterMenu_Button_Wish"
		"textAlignment"	"center"
		"Command"		"character wish"
		"font"			"FolderSmall"

		"character"     "wish"
		"sequence"		"wish_pose"
		"body_yaw"		"15"
		"body_pitch"	"-40"
		"weaponmodel"	"models/weapons/m1911.mdl"
	}

//	"bomber"
//	{
//		"ControlName"	"CharacterButton"
//		"fieldName"		"Bomber"
//		"xpos"			"c-250"
//		"ypos"			"210"
//		"wide"			"100"
//		"tall"			"100"
//		"labelText"		"#DA_CharacterMenu_Button_Bomber"
//		"textAlignment"	"center"
//		"Command"		"character bomber"
//		"font"			"FolderSmall"

//		"character"     "bomber"
//		"sequence"		"bomber_pose"
//		"body_yaw"		"15"
//		"body_pitch"	"-40"
//		"weaponmodel"	"models/weapons/fal.mdl"
//	}

	"eightball"
	{
		"ControlName"	"CharacterButton"
		"fieldName"		"Eightball"
		"xpos"			"c-250"
		"ypos"			"210"
		"wide"			"100"
		"tall"			"100"
		"labelText"		"#DA_CharacterMenu_Button_Eightball"
		"textAlignment"	"center"
		"Command"		"character eightball"
		"font"			"FolderSmall"

		"character"     "eightball"
		"sequence"		"eightball_pose"
		"body_yaw"		"15"
		"body_pitch"	"-40"
		"weaponmodel"	"models/weapons/dualberetta.mdl"
	}

	"character_random"
	{
		"ControlName"	"CharacterButton"
		"fieldName"		"character_random"
		"xpos"			"c-80"
		"ypos"			"70"
		"wide"			"40"
		"tall"			"15"
		"labelText"		"#DA_CharacterMenu_Random"
		"textAlignment"	"center"
		"font"			"FolderTiny"
		"Command"		"character random"
		"default"       "1"

		"character"     ""
	}
}
