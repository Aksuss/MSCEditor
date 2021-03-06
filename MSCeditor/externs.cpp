#include "externs.h"
#include "version.h"

WNDPROC DefaultListCtrlProc, DefaultListViewProc;
int iItem;

HWND hDialog, hEdit, hCEdit, hReport;
HINSTANCE hInst;
std::vector<std::wstring> entries;												// The "groups" of variables displayed on the left list
std::vector<Variable> variables;												// All variables read in from file, e.g. carjacktransform
std::vector<std::pair<uint32_t, uint32_t>> indextable;							// Holds indices of the currently selected group
std::vector<std::pair<std::pair<std::wstring, bool>, std::string>> locations;	// Holds all predefined locations, used by the teleport dialog and map. <<Name, IsMapRelevant>, Bin>
std::vector<CarPart> carparts;													// This vector is filled when bolt report is opened. Contains all the carparts found
std::vector<Item> itemTypes;													// Types of items in the game, e.g. sausage, beer etc
std::vector<ItemAttribute> itemAttributes;										// Attributes items can have, e.g. transform, consumed etc
std::vector<SpecialCase> partSCs;												// Bolt report special cases
std::vector<std::wstring> partIdentifiers;										// Properties of car parts. Used by the bolt report to detect parts properly
std::vector<CarProperty> carproperties;											// Car properties with min and max values such as wear, fuel and other liquids
std::vector<TimetableEntry> timetableEntries;									// Entry for the timetable inside the time and weather dialog
std::wstring filepath;															// Full file path of currently opened file
std::wstring filename;															// Filename of currently opened file
std::wstring appfolderpath;														// Path to the writable apps folder
HANDLE hTempFile = INVALID_HANDLE_VALUE;										// Handle of the tempfile. Invalidated upon exiting
SYSTEMTIME filedate;
HFONT hListFont;
DebugOutput *dbglog;
#ifdef _MAP
class MapDialog* EditorMap = NULL;
#endif /*_MAP*/

bool bFiledateinit = FALSE, bMakeBackup = TRUE, bEulerAngles = FALSE, bCheckForUpdate = TRUE, bBackupChangeNotified = FALSE, bFirstStartup = TRUE, bAllowScale = FALSE, bDisplayRawNames = FALSE, bCheckIssues = FALSE, bStartWithMap = FALSE;
const std::wstring settings[] = { L"make_backup", L"backup_change_notified", L"check_updates", L"first_startup", L"allow_scale", L"use_euler", L"raw_names", L"check_issues", L"start_with_map"};
PVOID pResizeState = NULL;

const float kindasmall = 1.0e-4f;
const float pi = std::atan(1.f) * 4.f;
const float rad2deg = 180.f / pi;
const float deg2Rad = pi / 180.f;

const std::wstring posInfinity(1, wchar_t(0x221E));
const std::wstring negInfinity = L"-" + posInfinity;
const std::wstring Version = std::to_wstring(VERSION_MAJOR) + L"." + std::to_wstring(VERSION_MINOR);
const std::wstring bools[2] = { L"false", L"true" };
const std::wstring Title = L"MSCEditor " + Version;
const std::wstring IniFile = L"msce.ini";
const std::wstring ErrorTitle = L"Perkele!";
const std::wstring HtmlHeader = L"<!DOCTYPE html>\n<html>\n<head>\n<style>\n#q{\nfont-family:\"Consolas\";\nborder-collapse:collapse;\ntransform:translateX(15px);\n}\n#q td, #q th{\nborder: 2px solid #fff;\npadding: 1px 10px;\n}\n#q tr:nth-child(even){background-color:#f2f2f2;}\n#q tr:nth-child(odd){background-color:#e0e0e0;}\n#q tr:hover{background-color:#fff;}\n#q th{\npadding-top:12px;\npadding-bottom:12px;\ntext-align:left;\nbackground-color:#fc4979;\ncolor:white;\n}\n</style>\n</head>\n<body style=\"background-color:#262633;\">\n";
const std::wstring HtmlTableHeader = L"<h1 style=\"color:#fff\">%s</h1>\n<table id = \"q\">\n<tr>\n<th>Variable Name</th>\n<th>Value Current File</th>\n<th>Value Other File</th>\n</tr>\n";
const std::wstring HtmlTableEntry = L"<tr>\n<td>%s</td>\n<td>%s</td>\n<td>%s</td>\n</tr>\n";
const std::wstring HtmlEnd = L"</body>\n</html>";


const std::wstring GLOB_STRS[] =
{
	L"Input isn't a valid float!", //0
	L"Input isn't a valid array!", //1
	L"Array contains invalid floats!", //2
	L"Input can't be negative!", //3
	L"Array can only hold values between 0 and 1!", //4
	L"Array can only hold values between -1 and 1!", //5
	L"Could not retrieve filedate. Runtime polling disabled. Savefile might corrupt when modified by external source at runtime!", //6
	L"\"%s\" was modified.\n\nSave Changes?", //7
	L"Input has to be of type FLOAT, e.g. '1.25'", //8
	L"\nCouldn't find start of entry. Expected symbol: " + std::wstring(1, HX_STARTENTRY), //9   
	L"Loaded %d entries in %d groups.", //10
	L"%d unsaved change%s", //11
	L"Save failure: ", //12
	L"\nifstream fail!", //13
	L"Could not rename file!", //14
	L"Could not delete file!", //15
	L"ofstream fail!", //16
	L"\"%s\"\n\nThis file has been modified by another program - most likely by the game. Do you wish to load external changes?\n\nClicking yes will discard all unsaved changes made in MSCeditor and reload (recommended).", //17
	L"Total Bolts: %d", //18
	L"Fastened Bolts: %d", //19
	L"Loose Bolts: %d", //20
	L"Installed Parts: %d / %d", //21
	L"Fixed Parts: %d", //22
	L"Loose Parts: %d", //23
	L"Damaged Parts: %d", //24
	L"Stuck Parts: %d", //25
	L"Could not load identifiers. Make sure" + IniFile + L"is in the same folder as executable.", //26
	L"Rotation Quaternions (x , y , z , w)", //27
	L"Rotation Angles (Pitch, Yaw, Roll)", //28
	L"Input contains invalid angles", //29
	L"Something went wrong when trying to write dumpfile.", //30
	L"Load failure at Offset: ", //31
	L"\nExpected integer, but got nothing!", //32
	L"\nUnexpected end of entry. Expected symbol: " + std::wstring(1, HX_ENDENTRY), //33    
	L"\nNo entries!", //34
	L"Could not locate file \"" + IniFile + L"\"!\nStarting program with reduced functionality.\n\n\nPossible solutions:\n\n - Make sure the file is in the same folder as this program.\n - Extract the compressed archive before starting.\n - If file is missing, redownload the program.\n - Run program as administrator", //35
	L"Update available! Update now?\n(Will start download in browser)\n\nChangelog:\n", //36
	L"Could not write to app folder at path:\n\"%s\"\nPlease report this issue.", //37
	L"There are %d issues with your save that could lead to bugs in the game. Check them out now?\n (You can review the changes before saving)", //38
	L"File could not be reloaded because it was renamed or deleted!", //39
	L"Successfully wrote dumpfile!", //40
	L"Just a heads up, but as of version 1.04,\nsettings no longer reset on launch!\nKeep that in mind.", //41
	L"Hey buddy! Looks like you're new.", //42
	L"\nUnexpected EOF!", //43
	L"You are about to open a backup file. Is this intentional?", //44
	L"\nClick a value to modify, then press set to apply the change or press fix to set it to a recommended value.\n\n\nProgrammers Notes:\nThe values for tuning parts are just my suggestions.\nFor instance, the suggested air/fuel ratio of 14:7 is the stoichiometric mixture that provides the best balance between power and fuel economy. To further decrease fuel consumption, you could make the mixture even leaner. For maximum power, you could set it to something like 13.1. \n\nThe same applies to the spark timing on the distributor. There is no best value for this, as with a racing carburator with N2O, the timing needs to be much higher (~13) than with the twin or stock carburator (~14.8).\n\nThe final gear ratio should be between 3.7 - 4.625. Lower values provide higher top speed but less acceleration.\n\n\nI highly recommend tuning it in the game though, as this is what the game is about ;)\nHave fun!", //45
	L"Could not complete action.\n", //46
	L"Could not find item ID entry!\n", //47
	L"This will remove \"%s\" and can not be undone. Are you sure?\n", //48
	L"Container is corrupted and cannot be opened.\nCaught exception:\n", //49
	L"%d issue%s found!", //50
	L"s were", //51
	L" was", //52
	L"LMB: Drag to navigate map.\nWHEEL: Zoom map.\nRMB: Actions.", //53
	L"Select:", //54
	L"Couldn't open map because app folder wasn't writable!", //55
	L"Copy coordinates to clipboard", //56
	L"Teleport an object here", // 57
	L"Measure distance", // 58
	L"Distance to here", // 59
	L"Clear measurement", // 60
	L"NOW LOADING YEAR 1995...", // 61
	L"Map failed: %s\nPlease report this issue!", // 62
	L"Resulting file saved at \"%s\"\nOpen in browser now?", // 63
	L"Flagged %d consumed items for deletion.\nFixed up %d item identifiers.\nEdited %d entries in total.\n(Gray entries will be removed upon saving)", // 64
	L"Identifier variable for item \"%s\" could not be found!\nPlease report this issue.", // 65
	L"Installed wiring for %d parts.\nYou can review the changes on the main dialog.\n\n%s", // 66
	L"Could not install wiring because it requires \"%s\" to be installed." // 66
};

const std::wstring BListSymbols[] =
{
	{ 0x002D },
	{ 0x2713 },
	{ 0x003F }
};

// Keep this alphabetical
const std::vector<std::pair<std::wstring, std::wstring>> NameTable =
{
	{ L"batteryterminal", L"wiringbattery" },
	{ L"crankbearing", L"mainbearing" },
	{ L"crankwheel", L"crankshaftpulley" },
	{ L"cd1transform", L"cd1" },
	{ L"coffeecuppos", L"coffeecup" },
	{ L"coffeepanpos", L"coffeepan" },
	{ L"computerorderpos", L"computerorder" },
	{ L"fenderflarerf", L"fenderflarerl" },
	{ L"fireextinguisherholder", L"extinguisherholder" },
	{ L"fishtraptransform", L"fishtrap" },
	{ L"floppy1pos", L"floppy1" },
	{ L"floppy2pos", L"floppy2" },
	{ L"floppy3pos", L"floppy3" },
	{ L"gaugeclock", L"clockgauge" },
	{ L"gaugerpm", L"rpmgauge" },
	{ L"headlightleft1", L"headlightleft" },
	{ L"headlightright1", L"headlightright" },
	{ L"ikwishbone", L"wishbone" },
	{ L"kiljubuckettransform", L"kiljubucket" },
	{ L"motorhoisttransform", L"motorhoist" },
	{ L"playertransform", L"player" },
	{ L"rallywheel", L"rallysteeringwheel" },
	{ L"repairshoporder", L"repairshop" },
	{ L"shockrallyfl", L"strutrallyfl" },
	{ L"shockrallyfr", L"strutrallyfr" },
	{ L"shockrallyrl", L"shockabsorberrallyrl" },
	{ L"shockrallyrr", L"shockabsorberrallyrr" },
	{ L"shockfl", L"strutfl" },
	{ L"shockfr", L"strutfr" },
	{ L"shockrl", L"shockabsorberrl" },
	{ L"shockrr", L"shockabsorberrr" },
	{ L"sportwheel", L"sportsteeringwheel" },
	{ L"stocksteeringwheel", L"steeringwheel" },
	{ L"strutflrally", L"strutrallyfl" },
	{ L"strutfrrally", L"strutrallyfr" },
	{ L"valvecover", L"rockercover" },

	{ L"wheelgt1transform", L"wheelgt1" },
	{ L"wheelgt2transform", L"wheelgt2" },
	{ L"wheelgt3transform", L"wheelgt3" },
	{ L"wheelgt4transform", L"wheelgt4" },

	{ L"wheelhayosiko1transform", L"wheelhayosiko1" },
	{ L"wheelhayosiko2transform", L"wheelhayosiko2" },
	{ L"wheelhayosiko3transform", L"wheelhayosiko3" },
	{ L"wheelhayosiko4transform", L"wheelhayosiko4" },

	{ L"wheelocto1transform", L"wheelocto1" },
	{ L"wheelocto2transform", L"wheelocto2" },
	{ L"wheelocto3transform", L"wheelocto3" },
	{ L"wheelocto4transform", L"wheelocto4" },

	{ L"wheelracing1transform", L"wheelracing1" },
	{ L"wheelracing2transform", L"wheelracing2" },
	{ L"wheelracing3transform", L"wheelracing3" },
	{ L"wheelracing4transform", L"wheelracing4" },

	{ L"wheelrally1transform", L"wheelrally1" },
	{ L"wheelrally2transform", L"wheelrally2" },
	{ L"wheelrally3transform", L"wheelrally3" },
	{ L"wheelrally4transform", L"wheelrally4" },

	{ L"wheelslot1transform", L"wheelslot1" },
	{ L"wheelslot2transform", L"wheelslot2" },
	{ L"wheelslot3transform", L"wheelslot3" },
	{ L"wheelslot4transform", L"wheelslot4" },

	{ L"wheelspoke1transform", L"wheelspoke1" },
	{ L"wheelspoke2transform", L"wheelspoke2" },
	{ L"wheelspoke3transform", L"wheelspoke3" },
	{ L"wheelspoke4transform", L"wheelspoke4" },

	{ L"wheelsteel1transform", L"wheelsteel1" },
	{ L"wheelsteel2transform", L"wheelsteel2" },
	{ L"wheelsteel3transform", L"wheelsteel3" },
	{ L"wheelsteel4transform", L"wheelsteel4" },
	{ L"wheelsteel5transform", L"wheelsteel5" },

	{ L"wheelsteelwide1transform", L"wheelsteelwide1" },
	{ L"wheelsteelwide2transform", L"wheelsteelwide2" },
	{ L"wheelsteelwide3transform", L"wheelsteelwide3" },
	{ L"wheelsteelwide4transform", L"wheelsteelwide4" },

	{ L"wheelturbine1transform", L"wheelturbine1" },
	{ L"wheelturbine2transform", L"wheelturbine2" },
	{ L"wheelturbine3transform", L"wheelturbine3" },
	{ L"wheelturbine4transform", L"wheelturbine4" },

	{ L"wiringmesstransform", L"wiring" },
};