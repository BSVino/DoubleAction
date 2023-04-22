//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//=============================================================================

// No spaces in event names, max length 32
// All strings are case sensitive
//
// valid data key types are:
//   string : a zero terminated string
//   bool   : unsigned int, 1 bit
//   byte   : unsigned int, 8 bit
//   short  : signed int, 16 bit
//   long   : signed int, 32 bit
//   float  : float, 32 bit
//   local  : any data, but not networked to clients
//
// following key names are reserved:
//   local      : if set to 1, event is not networked to clients
//   unreliable : networked, but unreliable
//   suppress   : never fire this event
//   time	: firing server time
//   eventid	: holds the event ID

"sdkevents"
{
	"player_death"
	{
		"userid"	"short"   	// user ID who died				
		"attacker"	"short"	 	// user ID who killed
		"weapon"	"string" 	// weapon name killed used
		"brawl"		"bool"   	// was the player brawled to death
		"grenade"	"bool"   	// was a grenade involved
	}
	
	"player_hurt"
	{
		"userid"	"short"   	// user ID who was hurt			
		"attacker"	"short"	 	// user ID who attacked
		"weapon"	"string" 	// weapon name attacker used
	}
	
	"player_changeclass"
	{
		"userid"	"short"		// user ID who changed class
		"class"		"short"		// class that they changed to
	}

	"spec_target_updated"
	{
	}

	"vote_ended"
	{
	}

	"vote_started"
	{
		"issue"     "string"
		"param1"    "string"
		"team"      "byte"
		"initiator" "long" // entity id of the player who initiated the vote
	}

	"vote_changed"
	{
		"vote_option1"   "byte"
		"vote_option2"   "byte"
		"vote_option3"   "byte"
		"vote_option4"   "byte"
		"vote_option5"   "byte"
		"potentialVotes" "byte"
	}

	"vote_passed"
	{
		"details" "string"
		"param1"  "string"
		"team"    "byte"
	}

	"vote_failed"
	{
		"team" "byte"
	}

	"vote_cast"
	{
		"vote_option" "byte" // which option the player voted on
		"team"        "short"
		"entityid"    "long" // entity id of the voter
	}

	"vote_options"
	{
		"count"   "byte" // Number of options - up to MAX_VOTE_OPTIONS
		"option1" "string"
		"option2" "string"
		"option3" "string"
		"option4" "string"
		"option5" "string"
	}
	
	// achievements
	
	"DIVEPUNCHKILL"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"DIVEAWAYFROMEXPLOSION"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"DIVEAWAYFROMEXPLOSION_250"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"DODGETHIS"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"SKYPUNCH"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"BETRAYED"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"PENGUIN"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"MAC_DADDYD"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"VINDICATED"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"HORSE_WHISPERER"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"VIGILANT"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"PERSUADED"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"BLACK_MAGICKED"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"UNDERTAKEN"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"FALL_GUY"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"SUPERFALL_SHARPSHOOTER"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"SUPERFALL_KING"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"STEADY_EDDIE"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"POTSHOTTER"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"SLOWPRO"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"SPECIALDELIVERY"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"NO_YOU_DONT"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"GET_BANNED"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"SOMEBODY_STOP_ME"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"DISCOSTU"
	{
		"userid"	"short"   	// user ID who died
	}
	
	"HARDBOILED"
	{
		"userid"	"short"   	// user ID who died
	}
	
}
