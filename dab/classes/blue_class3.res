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
		"labelText"		"#class_blue_class3"
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
		"text"			"#classinfo_class3"
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
			"modelname"	"models/weapons/w_smg_mp5.mdl"
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

	"grenade"
	{
		"ControlName"		"CModelPanel"
		"fieldName"		"grenade"
		"xpos"			"325"
		"ypos"			"36"
		"zpos"			"-5"	
		"wide"			"64"
		"tall"			"64"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"50"

		"model"
		{
			"angles_y"	"90"
			"origin_x"	"20"
			"spotlight"	"1"
			"modelname"	"models/weapons/w_eq_fraggrenade_thrown.mdl"
		}
	}
	//Tony; set it up so the second grenade shows up really close to the first.
	"grenade2"
	{
		"ControlName"		"CModelPanel"
		"fieldName"		"grenade2"
		"xpos"			"345"
		"ypos"			"36"
		"zpos"			"-5"	
		"wide"			"64"
		"tall"			"64"
		"visible"		"1"
		"enabled"		"1"
		"fov"			"50"

		"model"
		{
			"angles_y"	"90"
			"origin_x"	"20"
			"spotlight"	"1"
			"modelname"	"models/weapons/w_eq_fraggrenade_thrown.mdl"
		}
	}
}