#base "../../hl2/resource/SourceScheme.res"

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

	Fonts
	{
		"FolderLarge"
		{
			"1"
			{
				"name"		"Veteran Typewriter"
				"tall"		"20"
				"weight"	"500"
				"additive"	"0"
				"antialias" "1"
			}
		}

		"FolderMedium"
		{
			"1"
			{
				"name"		"Veteran Typewriter"
				"tall"		"16"
				"weight"	"500"
				"additive"	"0"
				"antialias" "1"
			}
		}

		"FolderSmall"
		{
			"1"
			{
				"name"		"Veteran Typewriter"
				"tall"		"12"
				"weight"	"0"
				"additive"	"0"
				"antialias" "1"
			}
		}

		"FolderTiny"
		{
			"1"
			{
				"name"		"Veteran Typewriter"
				"tall"		"10"
				"weight"	"0"
				"additive"	"0"
				"antialias" "1"
			}
		}

		"MainMenuFont"
		{
			"1"	[$WIN32]
			{
				"name"		"Tahoma"
				"tall"		"18"
				"weight"	"500"
				"additive"	"0"
				"antialias" "1"
			}
		}
		"MenuLarge"
		{
			"1"	[$X360]
			{
				"tall_hidef"		"24"
			}
		}
		"MenuTitle"
		{
			"1"
			{
				"name"		"Verdana Bold"
				"tall"		"20"
				"weight"	"500"
			}
		}

		"MenuTitle2"
		{
			"1"
			{
				"name"		"Verdana Bold"
				"tall"		"18"
				"weight"	"500"
			}
		}

		"MenuTitle2"
		{
			"1"
			{
				"name"		"Verdana Bold"
				"tall"		"14"
				"weight"	"500"
			}
		}

		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		Default
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"700"
				"antialias" "1"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"15"
				"weight"	"900"
				"antialias" "1"
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"18"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1024 1199"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1200 10000"
				"additive"	"1"
			}
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"10"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"16"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"18"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"1200 6000"
				"antialias"	"1"
			}
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"9"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"10"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"18"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1200 6000"
				"antialias"	"1"
			}
		}
		"ClassMenuDefault"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"9"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"700"
				"antialias" "1"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"900"
				"antialias" "1"
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1024 1199"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1200 10000"
				"additive"	"1"
			}
		}
		"ClassMenuDefaultSmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"0"
				"range"		"0x0000 0x017F"
				"yres"	"1200 6000"
				"antialias"	"1"
			}
			"6"
			{
				"name"		"Arial"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
		}
		"ClassMenuDefaultVerySmall"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"480 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"13"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"	"1200 6000"
				"antialias"	"1"
			}
			"6"
			{
				"name"		"Verdana"
				"tall"		"12"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
			"7"
			{
				"name"		"Arial"
				"tall"		"11"
				"range" 		"0x0000 0x00FF"
				"weight"		"0"
			}
		}
		"ClassMenuBodyFont"
		{
			"1"
			{
				"name"		"Verdana"
				"tall"		"9"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"Verdana"
				"tall"		"12"
				"weight"	"700"
				"antialias" "1"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"Verdana"
				"tall"		"14"
				"weight"	"900"
				"antialias" "1"
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"Verdana"
				"tall"		"20"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1024 1199"
			}
			"5"
			{
				"name"		"Verdana"
				"tall"		"24"
				"weight"	"900"
				"antialias" "1"
				"yres"	"1200 10000"
				"additive"	"1"
			}
		}
		"Marlett"
		{
			"1"
			{
				"name"		"Marlett"
				"tall"		"14"
				"weight"	"0"
				"symbol"	"1"
			}
		}
	}

	CustomFontFiles
	{
		"1"  "resource/veteran-typewriter.ttf"
		"2"  "resource/franchise-bold-hinted.ttf"
	}

	Borders
	{
		ButtonBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "-1 0"
				}
				"3"
				{
					"color" "0 0 0 200"
					"offset" "1 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "-1 0"
				}
				"3"
				{
					"color" "0 0 0 200"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "0 1"
				}
				"3"
				{
					"color" "0 0 0 200"
					"offset" "0 -1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "0 1"
				}
				"3"
				{
					"color" "0 0 0 200"
					"offset" "0 -1"
				}
			}
		}

		ButtonDepressedBorder
		{
		}

		FolderButtonArmedBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "-1 0"
				}
				"3"
				{
					"color" "0 0 0 150"
					"offset" "-2 0"
				}
				"4"
				{
					"color" "0 0 0 100"
					"offset" "-3 0"
				}
				"5"
				{
					"color" "0 0 0 50"
					"offset" "-4 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "1 0"
				}
				"3"
				{
					"color" "0 0 0 150"
					"offset" "2 0"
				}
				"4"
				{
					"color" "0 0 0 100"
					"offset" "3 0"
				}
				"5"
				{
					"color" "0 0 0 50"
					"offset" "4 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "0 -1"
				}
				"3"
				{
					"color" "0 0 0 150"
					"offset" "0 -2"
				}
				"4"
				{
					"color" "0 0 0 100"
					"offset" "0 -3"
				}
				"5"
				{
					"color" "0 0 0 50"
					"offset" "0 -4"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "0 0 0 255"
					"offset" "0 0"
				}
				"2"
				{
					"color" "0 0 0 200"
					"offset" "0 1"
				}
				"3"
				{
					"color" "0 0 0 150"
					"offset" "0 2"
				}
				"4"
				{
					"color" "0 0 0 100"
					"offset" "0 3"
				}
				"5"
				{
					"color" "0 0 0 50"
					"offset" "0 4"
				}
			}
		}
	}
}
