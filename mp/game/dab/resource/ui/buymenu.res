"Resource/UI/BuyMenu.res"
{
	"buy"
	{
		"ControlName"	"CClassMenu"
		"fieldName"		"buy"
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

	"Requisitions"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"Requisitions"
		"xpos"			"c-250"
		"ypos"			"50"
		"wide"			"220"
		"tall"			"20"
		"font"			"FolderMedium"
		"textAlignment"	"center"
		"underline"     "1"
		"labelText"		"#DA_BuyMenu_Requisitions"
		//"bgcolor_override" "0 0 0 100"
	}

	"WeaponInfo"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"WeaponInfo"
		"xpos"			"c-250"
		"ypos"			"285"
		"wide"			"230"
		"tall"			"120"
		"font"			"FolderTiny"
		"textAlignment"	"north-west"
		"wrap"          "1"
		//"bgcolor_override" "0 0 0 100"
	}

	"WeaponImage"
	{
		"ControlName"	"CModelPanel"
		"fieldName"		"WeaponImage"
		"xpos"			"c-175"
		"ypos"			"265"
		"wide"			"160"
		"tall"			"100"
		"zpos"			"5"
		"autoResize"	"0"
		"pinCorner"		"2"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"25"
		//"start_framed"  "1"
		//"bgcolor_override" "0 0 0 100"

		"model"
		{
			"spotlight"	"1"
			"modelname"	"models/weapons/beretta.mdl"
			"origin_z"	"0"
			"origin_y"	"5"
			"origin_x"	"70"
			"angles_y"	"200"
		}
	}

	"WeaponModel"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"WeaponModel"
		"xpos"			"c-210"
		"ypos"			"120"
		"wide"			"280"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Model"
		"textAlignment"	"west"
		"font"			"FolderSmall"
	}

	"WeaponStyle"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"WeaponStyle"
		"xpos"			"c-110"
		"ypos"			"120"
		"wide"			"280"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Style"
		"textAlignment"	"west"
		"font"			"FolderSmall"
	}

	"WeaponWeight"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"WeaponWeight"
		"xpos"			"c-70"
		"ypos"			"120"
		"wide"			"280"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Weight"
		"textAlignment"	"west"
		"font"			"FolderSmall"
	}

	"weapon_beretta"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_beretta"
		"xpos"			"c-210"
		"ypos"			"135"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_beretta"
		"textAlignment"	"west"
		"Command"		"buy beretta"
		"font"			"FolderSmall"
		"weaponid"      "beretta"
		"info_model"    "models/weapons/beretta.mdl"
	}

	"weapon_m1911"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_m1911"
		"xpos"			"c-210"
		"ypos"			"150"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_M1911"
		"textAlignment"	"west"
		"Command"		"buy m1911"
		"font"			"FolderSmall"
		"weaponid"      "m1911"
		"info_model"    "models/weapons/m1911.mdl"
	}

	"weapon_mossberg"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_mossberg"
		"xpos"			"c-210"
		"ypos"			"165"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_Mossberg"
		"textAlignment"	"west"
		"Command"		"buy mossberg"
		"font"			"FolderSmall"
		"weaponid"      "mossberg"
		"info_model"    "models/weapons/mossberg590.mdl"
	}

	"weapon_sawnoff"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_sawnoff"
		"xpos"			"c-210"
		"ypos"			"180"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_Sawnoff"
		"textAlignment"	"west"
		"Command"		"buy sawnoff"
		"font"			"FolderSmall"
		"weaponid"      "sawnoff"
		"info_model"    "models/weapons/w_stakeout.mdl"
	}

	"weapon_mp5k"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_mp5k"
		"xpos"			"c-210"
		"ypos"			"195"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_MP5K"
		"textAlignment"	"west"
		"Command"		"buy mp5k"
		"font"			"FolderSmall"
		"weaponid"      "mp5k"
		"info_model"    "models/weapons/w_smg_mp5.mdl"
	}

	"weapon_mac10"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_mac10"
		"xpos"			"c-210"
		"ypos"			"210"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_MAC10"
		"textAlignment"	"west"
		"Command"		"buy mac10"
		"font"			"FolderSmall"
		"weaponid"      "mac10"
		"info_model"    "models/weapons/mac10.mdl"
	}

	"weapon_fal"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_fal"
		"xpos"			"c-210"
		"ypos"			"225"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_FAL"
		"textAlignment"	"west"
		"Command"		"buy fal"
		"font"			"FolderSmall"
		"weaponid"      "fal"
		"info_model"    "models/weapons/fal.mdl"
	}

	"weapon_m16"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_m16"
		"xpos"			"c-210"
		"ypos"			"240"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_M16"
		"textAlignment"	"west"
		"Command"		"buy m16"
		"font"			"FolderSmall"
		"weaponid"      "m16"
		"info_model"    "models/weapons/w_rif_m4a1.mdl"
	}

	"weapon_grenade"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_grenade"
		"xpos"			"c-210"
		"ypos"			"255"
		"wide"			"170"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Button_Grenade"
		"textAlignment"	"west"
		"Command"		"buy grenade"
		"font"			"FolderSmall"
		"weaponid"      "grenade"
		"info_model"    "models/weapons/w_eq_fraggrenade.mdl"
	}

	"weapon_random"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_random"
		"xpos"			"c-80"
		"ypos"			"70"
		"wide"			"40"
		"tall"			"15"
		"labelText"		"#DA_BuyMenu_Random"
		"textAlignment"	"center"
		"font"			"FolderTiny"
		"Command"		"buy random"
		"info_string"   "#weaponinfo_random"
		"default"       "1"
	}

	"weapon_clear"
	{
		"ControlName"	"WeaponButton"
		"fieldName"		"weapon_clear"
		"xpos"			"c-80"
		"ypos"			"90"
		"wide"			"40"
		"tall"			"15"
		"labelText"		"#DA_Clear"
		"textAlignment"	"center"
		"font"			"FolderTiny"
		"Command"		"buy clear"
	}
}
