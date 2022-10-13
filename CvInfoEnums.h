#pragma once

#ifndef CV_INFO_ENUMS_H
#define CV_INFO_ENUMS_H

/*	f1rpo: Macros for letting the pre-processor generate some
	boilerplate code for CvInfo classes and their associated enum types.
	Simplified version of the setup I use in AdvCiv. */

/*	I've copied the type lists from AdvCiv, so any types added by AND
	aren't covered (but could easily be added). */

// Dynamic - number of instances not known at compile time
#define DO_FOR_EACH_DYN_INFO_TYPE(DO) \
	DO(Route, ROUTE) \
	DO(Climate, CLIMATE) \
	DO(SeaLevel, SEALEVEL) \
	DO(Terrain, TERRAIN) \
	DO(Feature, FEATURE) \
	DO(Improvement, IMPROVEMENT) \
	DO(TurnTimer, TURNTIMER) \
	DO(Handicap, HANDICAP) \
	DO(GameSpeed, GAMESPEED) \
	DO(Era, ERA) \
	DO(Victory, VICTORY) \
	/*DO(Camera, CAMERAANIMATION)*/ /* unused */ \
	DO(Advisor, ADVISOR) \
	DO(Emphasize, EMPHASIZE) \
	DO(BonusClass, BONUSCLASS) \
	/*DO(River, RIVER)*/ /* unused */ \
	DO(Goody, GOODY) \
	DO(Trait, TRAIT) \
	DO(Process, PROCESS) \
	DO(Season, SEASON) \
	DO(Month, MONTH) \
	DO(UnitCombat, UNITCOMBAT) \
	DO(Invisible, INVISIBLE) \
	DO(VoteSource, VOTESOURCE) \
	DO(Specialist, SPECIALIST) \
	DO(Religion, RELIGION) \
	DO(Corporation, CORPORATION) \
	DO(Hurry, HURRY) \
	DO(Upkeep, UPKEEP) \
	DO(CultureLevel, CULTURELEVEL) \
	DO(CivicOption, CIVICOPTION) \
	DO(Color, COLOR) \
	DO(AnimationCategory, ANIMCAT) \
	DO(EntityEvent, ENTITYEVENT) \
	DO(Effect, EFFECT) \
	DO(Attachable, ATTACHABLE) \
	DO(Build, BUILD) \
	DO(PlayerColor, PLAYERCOLOR) \
	DO(Bonus, BONUS) \
	DO(Civilization, CIVILIZATION) \
	DO(LeaderHead, LEADER) \
	DO(Cursor, CURSOR) \
	DO(BuildingClass, BUILDINGCLASS) \
	DO(Building, BUILDING) \
	DO(SpecialBuilding, SPECIALBUILDING) \
	DO(Project, PROJECT) \
	DO(Vote, VOTE) \
	DO(Concept, CONCEPT) \
	DO(NewConcept, NEW_CONCEPT) \
	DO(UnitClass, UNITCLASS) \
	DO(Unit, UNIT) \
	DO(SpecialUnit, SPECIALUNIT) \
	DO(Promotion, PROMOTION) \
	DO(Tech, TECH) \
	DO(Civic, CIVIC) \
	DO(Event, EVENT) \
	DO(EventTrigger, EVENTTRIGGER) \
	DO(EspionageMission, ESPIONAGEMISSION) \
	DO(UnitArtStyle, UNIT_ARTSTYLE)

#ifdef DOMAIN // defined in math.h
	#undef DOMAIN
#endif
// Number of instances hardcoded in CvEnums.h
#define DO_FOR_EACH_STATIC_INFO_TYPE(DO) \
	DO(AnimationPath, ANIMATIONPATH) \
	DO(InterfaceMode, INTERFACEMODE) \
	DO(GameOption, GAMEOPTION) \
	DO(MPOption, MPOPTION) \
	DO(PlayerOption, PLAYEROPTION) \
	DO(GraphicOption, GRAPHICOPTION) \
	DO(ForceControl, FORCECONTROL) \
	DO(Mission, MISSION) \
	DO(Yield, YIELD) \
	DO(Commerce, COMMERCE) \
	DO(Control, CONTROL) \
	DO(Command, COMMAND) \
	DO(Automate, AUTOMATE) \
	DO(Domain, DOMAIN) \
	DO(Attitude, ATTITUDE) \
	DO(Memory, MEMORY) \
	DO(CityTab, CITYTAB) \
	DO(Calendar, CALENDAR) \
	DO(UnitAI, UNITAI) \
	DO(Denial, DENIAL)

#define DO_FOR_EACH_INFO_TYPE(DO) \
	DO_FOR_EACH_DYN_INFO_TYPE(DO) \
	DO_FOR_EACH_STATIC_INFO_TYPE(DO)

/*  For the MAKE_INFO_ACCESSOR macro - which will have to be defined in CvGlobals.h.
	These enum types don't have a dedicated CvInfo class.
	(typedef wouldn't be as flexible: not usable in forward declarations.) */
#define CvConceptInfo CvInfoBase
#define CvNewConceptInfo CvInfoBase
#define CvSeasonInfo CvInfoBase
#define CvMonthInfo CvInfoBase
#define CvInvisibleInfo CvInfoBase
#define CvDomainInfo CvInfoBase
#define CvAttitudeInfo CvInfoBase
#define CvMemoryInfo CvInfoBase
#define CvCityTabInfo CvInfoBase
#define CvCalendarInfo CvInfoBase
#define CvUnitAIInfo CvInfoBase
#define CvDenialInfo CvInfoBase
// These two just have irregular names
#define CvUnitArtStyleInfo CvUnitArtStyleTypeInfo
#define MPOptionTypes MultiplayerOptionTypes

#define NUM_ENUM_TYPES(INFIX) NUM_##INFIX##_TYPES
#define SET_ENUM_LENGTH_STATIC(Name, INFIX) \
	__forceinline Name##Types getEnumLength(Name##Types) \
	{ \
		return NUM_ENUM_TYPES(INFIX); \
	}
/*	More non-info types should be added here. Would be better though to wrap the
	enum type definitions in CvEnums.h into macros and to set the enum length there.
	(As it's done in AdvCiv.) */
SET_ENUM_LENGTH_STATIC(Contact, CONTACT);
#define SET_ENUM_LENGTH(Name, PREFIX) \
	__forceinline Name##Types getEnumLength(Name##Types) \
	{ \
		return static_cast<Name##Types>(GC.getNum##Name##Infos()); \
	}
#define SET_ENUM_LENGTH_GLOBAL(Name, PREFIX) \
	__forceinline Name##Types getEnumLength(Name##Types) \
	{ \
		return static_cast<Name##Types>(GC.getNum##Name##Types()); \
	}
// ^Will call these in CvGlobals.h (GC needs to be defined)

#define FOR_EACH_ENUM_HELPER(eVar, EnumNamePrefix) \
	for (EnumNamePrefix##Types eVar = (EnumNamePrefix##Types)0; \
		eVar < getEnumLength(eVar); \
		eVar = (EnumNamePrefix##Types)(eVar + 1))
#define FOR_EACH_ENUM(EnumNamePrefix) \
	FOR_EACH_ENUM_HELPER(eLoop##EnumNamePrefix, EnumNamePrefix)
#define FOR_EACH_ENUM2(EnumNamePrefix, eVar) \
	FOR_EACH_ENUM_HELPER(eVar, EnumNamePrefix)

#endif
