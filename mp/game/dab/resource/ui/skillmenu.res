"Resource/UI/SkillMenu.res"
{
	"buyequip_ct"
	{
		"ControlName"	"CSkillMenu"
		"fieldName"		"buyequip_ct"
		"xpos"			"0"
		"ypos"			"0"
		"wide"			"f0"
		"tall"			"480"
	}

	"Title"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"Title"
		"xpos"			"c-250"
		"ypos"			"50"
		"wide"			"220"
		"tall"			"20"
		"font"			"FolderMedium"
		"textAlignment"	"center"
		"underline"     "1"
		"labelText"		"#DA_SkillMenu_Title"
	}

	"skill_random"
	{
		"ControlName"	"SkillButton"
		"fieldName"		"skill_random"
		"xpos"			"c-80"
		"ypos"			"70"
		"wide"			"40"
		"tall"			"15"
		"labelText"		"#DA_SkillMenu_Random"
		"textAlignment"	"center"
		"font"			"FolderTiny"
		"Command"		"setskill random"
		"skill"         "random"
		"default"       "1"
	}

	"skill_cancel"
	{
		"ControlName"	"SkillButton"
		"fieldName"		"skill_cancel"
		"xpos"			"c160"
		"ypos"			"70"
		"wide"			"50"
		"tall"			"15"
		"labelText"		"#DA_SkillMenu_Cancel"
		"textAlignment"	"center"
		"font"			"FolderTiny"
		"Command"		"close"
		"default"       "1"
		"visible"       "1"
	}

	"skill_bg_marksman"
	{
		"ControlName"   "PanelTexture"
		"fieldName"     "skill_bg_marksman"
		"xpos"			"c-255"
		"ypos"			"100"
		"wide"			"70"
		"tall"			"70"
		"zpos"          "-10"
		"image"         "marksman"
	}

	"skill_marksman"
	{
		"ControlName"	"SkillButton"
		"fieldName"		"skill_marksman"
		"xpos"			"c-255"
		"ypos"			"100"
		"wide"			"70"
		"tall"			"70"
		"textAlignment"	"center"
		"labelText"		"#DA_Skill_Marksman"
		"Command"		"setskill marksman"
		"skill"         "marksman"
		"font"			"FolderSmall"
	}

	"skill_bg_bouncer"
	{
		"ControlName"   "PanelTexture"
		"fieldName"     "skill_bg_bouncer"
		"xpos"			"c-175"
		"ypos"			"100"
		"wide"			"70"
		"tall"			"70"
		"zpos"          "-10"
		"image"         "bouncer"
	}

	"skill_bouncer"
	{
		"ControlName"	"SkillButton"
		"fieldName"		"skill_bouncer"
		"xpos"			"c-175"
		"ypos"			"100"
		"wide"			"70"
		"tall"			"70"
		"textAlignment"	"center"
		"labelText"		"#DA_Skill_Bouncer"
		"Command"		"setskill bouncer"
		"skill"         "bouncer"
		"font"			"FolderSmall"
	}

	"skill_bg_reflexes"
	{
		"ControlName"   "PanelTexture"
		"fieldName"     "skill_bg_reflexes"
		"xpos"			"c-95"
		"ypos"			"100"
		"wide"			"70"
		"tall"			"70"
		"zpos"          "-10"
		"image"         "reflexes"
	}

	"skill_reflexes"
	{
		"ControlName"	"SkillButton"
		"fieldName"		"skill_reflexes"
		"xpos"			"c-95"
		"ypos"			"100"
		"wide"			"70"
		"tall"			"70"
		"textAlignment"	"center"
		"labelText"		"#DA_Skill_Reflexes"
		"Command"		"setskill reflexes"
		"skill"         "reflexes"
		"font"			"FolderSmall"
	}

	"skill_bg_athletic"
	{
		"ControlName"   "PanelTexture"
		"fieldName"     "skill_bg_athletic"
		"xpos"			"c-255"
		"ypos"			"180"
		"wide"			"70"
		"tall"			"70"
		"zpos"          "-10"
		"image"         "athletic"
	}

	"skill_athletic"
	{
		"ControlName"	"SkillButton"
		"fieldName"		"skill_athletic"
		"xpos"			"c-255"
		"ypos"			"180"
		"wide"			"70"
		"tall"			"70"
		"textAlignment"	"center"
		"labelText"		"#DA_Skill_Athletic"
		"Command"		"setskill athletic"
		"skill"         "athletic"
		"font"			"FolderSmall"
	}

	//"skill_resilient"
	//{
	//	"ControlName"	"SkillButton"
	//	"fieldName"		"skill_resilient"
	//	"xpos"			"c-250"
	//	"ypos"			"250"
	//	"wide"			"230"
	//	"tall"			"15"
	//	"labelText"		"#DA_Skill_Resilient"
	//	"textAlignment"	"west"
	//	"Command"		"setskill resilient"
	//	"skill"         "resilient"
	//	"font"			"FolderSmall"
	//}

	"skill_bg_troll"
	{
		"ControlName"   "PanelTexture"
		"fieldName"     "skill_bg_troll"
		"xpos"			"c-175"
		"ypos"			"180"
		"wide"			"70"
		"tall"			"70"
		"zpos"          "-10"
		"image"         "troll"
	}

	"skill_troll"
	{
		"ControlName"	"SkillButton"
		"fieldName"		"skill_troll"
		"xpos"			"c-175"
		"ypos"			"180"
		"wide"			"70"
		"tall"			"70"
		"textAlignment"	"center"
		"labelText"		"#DA_Skill_Troll"
		"Command"		"setskill troll"
		"skill"         "troll"
		"font"			"FolderSmall"
	}

	"SkillInfo"
	{
		"ControlName"	"FolderLabel"
		"fieldName"		"SkillInfo"
		"xpos"			"c-250"
		"ypos"			"290"
		"wide"			"230"
		"tall"			"160"
		"font"			"FolderTiny"
		"textAlignment"	"north-west"
		"wrap"          "1"
		//"bgcolor_override" "0 0 0 100"
	}

	"SkillIcon"
	{
		"ControlName"   "PanelTexture"
		"fieldName"		"SkillIcon"
		"xpos"			"c-125"
		"ypos"			"290"
		"wide"			"100"
		"tall"			"100"
		"image"         ""
		"zpos"          "-1"
	}
}
