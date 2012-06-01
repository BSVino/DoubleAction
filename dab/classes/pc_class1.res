"classes/random.res"
{
	"classNameLabel"
	{
		"ControlName"		"Label"
		"fieldName"		"classNameLabel"
		"xpos"			"10"
		"ypos"			"18"
		"wide"			"175"
		"tall"			"15"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#class_pc_class1"
		"textAlignment"		"north-west"
		"dulltext"		"0"
		"brighttext"		"1"
		"font"			"ClassMenuDefault"
		"wrap"			"0"
	}
	"classInfo"
	{
		"ControlName"		"RichText"
		"fieldName"		"classInfo"
		"xpos"			"7"
		"ypos"			"36"
		"wide"			"170"
		"tall"			"90"
		"autoResize"		"3"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"maxchars"		"-1"
		"text"			"#classinfo_class1"
		"wrap"			"1"
		"textAlignment"		"north-west"
		"font"			"ClassMenuDefaultSmall"
	}
	
	"primaryWeapon"
	{
		"ControlName"		"CModelPanel"
		"fieldName"		"primaryWeapon"
		"xpos"			"187"
		"ypos"			"36"
		"zpos"			"-5"		

		"wide"			"64"
		"tall"			"64"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"100"

		"model"
		{
			"angles_y"	"90"
			"origin_x"	"20"
			"origin_y"	"-10"
			"spotlight"	"1"
			"modelname"	"models/weapons/w_pist_deagle.mdl"
		}

	}

	"crowbar"
	{	
		"ControlName"		"CModelPanel"
		"fieldName"		"crowbar"
		"xpos"			"256"
		"ypos"			"36"
		"zpos"			"-5"		

		"wide"			"64"
		"tall"			"64"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"90"

		"model"
		{
			"angles_y"	"90"
			"origin_x"	"20"
			"spotlight"	"1"
			"modelname"	"models/weapons/w_crowbar.mdl"
		}
	}
}