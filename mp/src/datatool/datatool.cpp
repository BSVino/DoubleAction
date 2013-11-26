#ifdef _WIN32
#include <Windows.h>
#endif

#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <map>

#include "tier0/icommandline.h"
#include "tier0/vcrmode.h"
#include "filesystem.h"
#include "../utils/common/cmdlib.h"

#undef min
#undef max

#include "../datanetworking/math.pb.h"
#include "../datanetworking/data.pb.h"

using std::string;
using std::vector;
using std::map;

int main(int argc, const char** args)
{
	if (argc < 3)
	{
		Msg("Usage: %s [directory] [command]\n", args[0]);
		return 1;
	}

	string sDirectory = args[1];
	string sCommand = args[2];

	if (sDirectory[sDirectory.length()-1] != '\\' && sDirectory[sDirectory.length()-1] != '/')
		sDirectory.push_back('/');

	CommandLine()->CreateCmdLine( Plat_GetCommandLine() );
	CmdLib_InitFileSystem( sDirectory.c_str() );

	string sSearchPath = sDirectory + "*";

	vector<da::protobuf::GameData> apbDatas;

	FileFindHandle_t ffh;
	char const *pszFileName = g_pFullFileSystem->FindFirst( sSearchPath.c_str(), &ffh );
	while (pszFileName)
	{
		if ( pszFileName[0] == '.'  )
		{
			pszFileName = g_pFullFileSystem->FindNext( ffh );
			continue;
		}

		char ext[ 10 ];
		Q_ExtractFileExtension( pszFileName, ext, sizeof( ext ) );

		if ( Q_stricmp( ext, "pb" ) != 0 )
		{
			pszFileName = g_pFullFileSystem->FindNext( ffh );
			continue;
		}

		FileHandle_t fh = g_pFullFileSystem->Open((sDirectory + pszFileName).c_str(), "r");

		if (!fh)
		{
			pszFileName = g_pFullFileSystem->FindNext( ffh );
			continue;
		}

		int iFileSize = g_pFullFileSystem->Size(fh);

		string sBuffer;
		sBuffer.resize(iFileSize);
		g_pFullFileSystem->Read( (void*)sBuffer.data(), iFileSize, fh );

		da::protobuf::GameData pbGameData;
		pbGameData.ParseFromString(sBuffer);

		if (pbGameData.debug() || pbGameData.cheats())
		{
			pszFileName = g_pFullFileSystem->FindNext( ffh );
			continue;
		}

		if (sCommand != "daily_players" && pbGameData.timestamp() <  1383523200) // Nov 4
		{
			pszFileName = g_pFullFileSystem->FindNext( ffh );
			continue;
		}

		apbDatas.push_back(pbGameData);

		pszFileName = g_pFullFileSystem->FindNext( ffh );
	}

	g_pFullFileSystem->FindClose( ffh );

	if (sCommand == "list_maps")
	{
		map<string, int> aMaps;
		for (size_t i = 0; i < apbDatas.size(); i++)
			aMaps[apbDatas[i].map_name()] += apbDatas[i].positions().position_size();

		for (auto it = aMaps.begin(); it != aMaps.end(); it++)
			Msg("%s: %d\n", it->first.c_str(), it->second);
	}
	else if (sCommand == "map_positions")
	{
		if (argc < 4)
		{
			Msg("Usage: %s [directory] map_positions [mapname]\n", args[0]);
			return 1;
		}

		string sMap = args[3];
		vector<Vector> avecPositions;
		for (size_t i = 0; i < apbDatas.size(); i++)
		{
			if (apbDatas[i].map_name() != sMap)
				continue;

			for (size_t j = 0; j < apbDatas[i].positions().position_size(); j++)
			{
				auto& pbPosition = apbDatas[i].positions().position(j);
				avecPositions.push_back(Vector(pbPosition.x(), pbPosition.y(), pbPosition.z()));
			}
		}

		Msg("Positions for %s\n", sMap.c_str());
		for (size_t i = 0; i < avecPositions.size(); i++)
			Msg("%f %f %f\n", avecPositions[i].x, avecPositions[i].y, avecPositions[i].z);
	}
	else if (sCommand == "player_choices")
	{
		vector<int> aiWeaponsChosen;
		aiWeaponsChosen.resize(100);
		memset(aiWeaponsChosen.data(), 0, aiWeaponsChosen.size());

		vector<int> aiSkillsChosen;
		aiSkillsChosen.resize(100);
		memset(aiSkillsChosen.data(), 0, aiSkillsChosen.size());

		map<string, int> asCharactersChosen;

		for (size_t i = 0; i < apbDatas.size(); i++)
		{
			for (size_t j = 0; j < apbDatas[i].weapons_chosen().size(); j++)
				aiWeaponsChosen[apbDatas[i].weapons_chosen(j)]++;

			for (size_t j = 0; j < apbDatas[i].skills_chosen().size(); j++)
				aiSkillsChosen[apbDatas[i].skills_chosen(j)]++;

			for (size_t j = 0; j < apbDatas[i].characters_chosen().size(); j++)
			{
				if (!apbDatas[i].characters_chosen(j).length())
					continue;

				asCharactersChosen[apbDatas[i].characters_chosen(j)]++;
			}
		}

		int iTotal = 0;
		for (size_t i = 0; i < aiWeaponsChosen.size(); i++)
			iTotal += aiWeaponsChosen[i];

		Msg("Weapons:\n");
		for (size_t i = 0; i < aiWeaponsChosen.size(); i++)
		{
			if (aiWeaponsChosen[i])
				Msg(" %i: %i (%.2f%%)\n", i, aiWeaponsChosen[i], (float)((float)aiWeaponsChosen[i]/(float)iTotal)*100);
		}

		iTotal = 0;
		for (size_t i = 0; i < aiSkillsChosen.size(); i++)
			iTotal += aiSkillsChosen[i];

		Msg("Skills:\n");
		for (size_t i = 0; i < aiSkillsChosen.size(); i++)
		{
			if (aiSkillsChosen[i])
				Msg(" %i: %i (%.2f%%)\n", i, aiSkillsChosen[i], (float)((float)aiSkillsChosen[i]/(float)iTotal)*100);
		}

		Msg("Characters:\n");
		for (auto it = asCharactersChosen.begin(); it != asCharactersChosen.end(); it++)
			Msg(" %s: %i\n", it->first.c_str(), it->second);
	}
	else if (sCommand == "daily_players")
	{
		map<long, int> aiDates;

		for (size_t i = 0; i < apbDatas.size(); i++)
		{
			long iDay = apbDatas[i].timestamp() - apbDatas[i].timestamp()%86400;

			// Characters chosen serves as an estimate if we don't have better data
			aiDates[iDay] += apbDatas[i].characters_chosen().size();
		}

		Msg("Days:\n");
		for (auto it = aiDates.begin(); it != aiDates.end(); it++)
			Msg(" %i: %i\n", it->first, it->second);
	}

	return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, char* command_line, int show_command)
{
	int argc;
	char** argv;

	char* arg;
	int index;
	int result;

	// count the arguments

	argc = 1;
	arg = command_line;

	while (arg[0] != 0) {

		while (arg[0] != 0 && arg[0] == ' ') {
			arg++;
		}

		if (arg[0] != 0) {

			argc++;

			while (arg[0] != 0 && arg[0] != ' ') {
				arg++;
			}

		}

	}

	// tokenize the arguments

	argv = (char**)malloc(argc * sizeof(char*));

	arg = command_line;
	index = 1;

	while (arg[0] != 0) {

		while (arg[0] != 0 && arg[0] == ' ') {
			arg++;
		}

		if (arg[0] != 0) {

			argv[index] = arg;
			index++;

			while (arg[0] != 0 && arg[0] != ' ') {
				arg++;
			}

			if (arg[0] != 0) {
				arg[0] = 0;
				arg++;
			}

		}

	}

	// put the program name into argv[0]
	char filename[_MAX_PATH];

	GetModuleFileName(NULL, filename, _MAX_PATH);
	argv[0] = filename;

	// call the user specified main function

	result = main(argc, (const char**)argv);

	free(argv);
	return result;
}
#endif
