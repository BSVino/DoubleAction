#base "SourceSchemeBase.res"

Scheme
{
	BaseSettings
	{
		// scheme-specific colors
		MainMenu.TextColor			"TanLight"			[$WIN32]
		MainMenu.ArmedTextColor			"117 107 94 255"		[$WIN32]
		MainMenu.Inset				"32"	

		CheckButton.TextColor          "0 0 0 255"
		CheckButton.BgColor            "255 255 255 0"
		CheckButton.Border1            "0 0 0 255"
		CheckButton.Border2            "100 100 100 255"
		CheckButton.Check              "0 0 0 255"
		CheckButton.SelectedTextColor  "0 0 0 255"

		Button.TextColor              "0 0 0 255"
		Button.BgColor                "255 255 255 0"
		Button.ArmedTextColor         "0 0 0 255"
		Button.ArmedBgColor           "0 0 0 100"
		Button.DepressedTextColor     "0 0 0 255"
		Button.DepressedBgColor       "255 255 255 0"

		Label.TextColor               "0 0 0 255"
		Label.DisabledFgColor1        "200 200 200 255"
		Label.DisabledFgColor2        "150 150 150 255"
		Label.BgColor                 "255 255 255 0"

		Panel.BgColor                 "255 255 255 0"
	}

	CustomFontFiles
	{
		"1"  "resource/franchise_da.ttf"
	}
}
