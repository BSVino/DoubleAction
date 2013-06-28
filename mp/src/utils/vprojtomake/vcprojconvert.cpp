//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

// --------------------------------
// This is an adaption of the original VCPROJ2MAKE from Valve to support
// Visual Studio 2010's new project file format. In doing this, I also
// threw out the ridiculously overkilled use of the COM XML Parser for Win32
// and Xerces for Linux. In its place, TinyXML, which requires absolutely
// no external libraries.
//
// TinyXML 2.6.0
// http://www.grinninglizard.com/tinyxml/
//
// Killermonkey <killermonkey01@gmail.com>
// ----------------------------------


#include "stdafx.h"
#include "tier0/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vcprojconvert.h"
#include "utlvector.h"

//-----------------------------------------------------------------------------
// Purpose:  constructor
//-----------------------------------------------------------------------------
CVCProjConvert::CVCProjConvert()
{
	m_bProjectLoaded = false;
	m_bIs2010 = false;
}

//-----------------------------------------------------------------------------
// Purpose:  destructor
//-----------------------------------------------------------------------------
CVCProjConvert::~CVCProjConvert()
{

}

//-----------------------------------------------------------------------------
// Purpose: load up a project and parse it
//-----------------------------------------------------------------------------
bool CVCProjConvert::LoadProject( const char *project )
{
	TiXmlDocument doc( project );
	if ( !doc.LoadFile() )
		return false;

	// VC2010 Update
	if ( Q_strstr( project, ".vcxproj" ) )
		m_bIs2010 = true;

	TiXmlNode *pStart;
	if ( m_bIs2010 )
		pStart = doc.FirstChild("Project");
	else
		pStart = doc.FirstChild("VisualStudioProject");

	if ( !pStart )
		return false;

	TiXmlHandle docHandle(pStart);

	ExtractProjectName( docHandle );
	if ( !m_Name.IsValid() )
	{
		Msg( "Failed to extract project name\n" );
		return false;
	}
	char baseDir[ MAX_PATH ];
	Q_ExtractFilePath( project, baseDir, sizeof(baseDir) );
	Q_StripTrailingSlash( baseDir );
	m_BaseDir = baseDir;

	ExtractConfigurations( docHandle );
	if ( m_Configurations.Count() == 0 )
	{
		Msg( "Failed to find any configurations to load\n" );
		return false;
	}

	ExtractFiles( docHandle );

	m_bProjectLoaded = true;
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: returns the number of different configurations loaded
//-----------------------------------------------------------------------------
int CVCProjConvert::GetNumConfigurations()
{
	Assert( m_bProjectLoaded );
	return m_Configurations.Count();

}

//-----------------------------------------------------------------------------
// Purpose: returns the index of a config with this name, -1 on err
//-----------------------------------------------------------------------------
int CVCProjConvert::FindConfiguration( CUtlSymbol name )
{
	if ( !name.IsValid() )
	{
		return -1;
	}
	
	for ( int i = 0; i < m_Configurations.Count(); i++ )
	{
		if ( m_Configurations[i].GetName() == name )
		{
			return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: returns the config object at this index
//-----------------------------------------------------------------------------
CVCProjConvert::CConfiguration & CVCProjConvert::GetConfiguration( int i )
{
	Assert( m_bProjectLoaded );
	Assert( m_Configurations.IsValidIndex(i) );
	return m_Configurations[i];
}

//-----------------------------------------------------------------------------
// Purpose: extracts the project name from the loaded vcproj
//-----------------------------------------------------------------------------
bool CVCProjConvert::ExtractProjectName( TiXmlHandle &hDoc )
{
	if ( m_bIs2010 )
	{
		TiXmlElement *pProp = hDoc.FirstChild("PropertyGroup").ToElement();
		for ( pProp; pProp; pProp = pProp->NextSiblingElement() )
		{
			TiXmlNode *pName = pProp->FirstChild("ProjectName");
			if ( pName )
			{
				m_Name = pName->ToElement()->GetText();
				return true;
			}
		}
	}
	else
	{
		m_Name = hDoc.Element()->Attribute("Name");
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: extracts the list of configuration names from the vcproj
//-----------------------------------------------------------------------------
bool CVCProjConvert::ExtractConfigurations( TiXmlHandle &hDoc )
{
	m_Configurations.RemoveAll();

	if ( m_bIs2010 )
	{
		TiXmlElement *pItemGroup = hDoc.FirstChild( "ItemGroup" ).ToElement();
		for ( pItemGroup; pItemGroup; pItemGroup = pItemGroup->NextSiblingElement() )
		{
			const char *label = pItemGroup->Attribute("Label");

			if ( label && !Q_stricmp( label, "ProjectConfigurations" ) )
			{
				TiXmlNode *pConfig = pItemGroup->FirstChild("ProjectConfiguration");
				for ( pConfig; pConfig; pConfig = pConfig->NextSiblingElement() )
				{
					int newIndex = m_Configurations.AddToTail();
					CConfiguration & config = m_Configurations[newIndex];
					config.SetName( pConfig->ToElement()->Attribute("Include") );
					ExtractIncludes( hDoc, config );
				}

				return true;
			}
		}
	}
	else
	{
		TiXmlElement *pConfig = hDoc.FirstChild("Configurations").FirstChild("Configuration").ToElement();
		for ( pConfig; pConfig; pConfig = pConfig->NextSiblingElement() )
		{
			int newIndex = m_Configurations.AddToTail();
			CConfiguration & config = m_Configurations[newIndex];
			config.SetName( pConfig->Attribute("Name") );
			TiXmlHandle hConfig( pConfig );
			ExtractIncludes( hConfig, config );
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: extracts the list of defines and includes used for this config
//-----------------------------------------------------------------------------
bool CVCProjConvert::ExtractIncludes( TiXmlHandle &hDoc, CConfiguration & config )
{
	config.ResetDefines();
	config.ResetIncludes();

	if ( m_bIs2010 )
	{
		TiXmlElement *pDefGroup = hDoc.FirstChild("ItemDefinitionGroup").ToElement();
		for ( pDefGroup; pDefGroup; pDefGroup = pDefGroup->NextSiblingElement() )
		{
			const char *cond = pDefGroup->Attribute("Condition");
			if ( cond && Q_stristr( cond, config.GetName().String() ) )
				break;
		}

		if ( !pDefGroup )
			return false;

		TiXmlNode *pCompile = pDefGroup->FirstChild("ClCompile");
		for ( pCompile; pCompile; pCompile = pCompile->ToElement()->NextSiblingElement() )
		{
			TiXmlNode *pPrePro = pCompile->FirstChild("PreprocessorDefinitions");
			TiXmlNode *pAddInc = pCompile->FirstChild("AdditionalIncludeDirectories");
			if ( pPrePro && pAddInc )
			{
				CUtlSymbol defines = pPrePro->ToElement()->GetText();
				char *str = (char *)_alloca( Q_strlen( defines.String() ) + 1 );
				Q_strcpy( str, defines.String() );
				// Expanded tokenize with , or ; both are accepted by VS
				char *delim = NULL;
				char *try1 = strchr( str, ';' );
				char *try2 = strchr( str, ',' );
				char *curpos = str;
				while ( try1 || try2 )
				{
					delim = ( (try1 && !try2) || (try1 && try2 && try1 < try2) ) ? try1 : try2;
					*delim = 0;
					delim++;
					if ( Q_stricmp( curpos, "WIN32" ) && Q_stricmp( curpos, "_WIN32" )  &&  
							Q_stricmp( curpos, "_WINDOWS") && Q_stricmp( curpos, "WINDOWS") &&
							curpos[0] != '%' ) // don't add WIN32 defines
					{
						config.AddDefine( curpos );
					}
					curpos = delim;
					try1 = strchr( delim, ';' );
					try2 = strchr( delim, ',' );
				}
				if ( Q_stricmp( curpos, "WIN32" ) && Q_stricmp( curpos, "_WIN32" )  &&  
						Q_stricmp( curpos, "_WINDOWS") && Q_stricmp( curpos, "WINDOWS") &&
						curpos[0] != '%' ) // don't add WIN32 defines
				{
					config.AddDefine( curpos );
				}		

				CUtlSymbol includes = pAddInc->ToElement()->GetText();
				char *str2 = (char *)_alloca( Q_strlen( includes.String() ) + 1 );
				Assert( str2 );
				Q_strcpy( str2, includes.String() );
				// Expanded tokenize with , or ; both are accepted by VS
				delim = NULL;
				try1 = strchr( str2, ';' );
				try2 = strchr( str2, ',' );
				curpos = str2;
				while ( try1 || try2 )
				{
					delim = ( (try1 && !try2) || (try1 && try2 && try1 < try2) ) ? try1 : try2;
					*delim = 0;
					delim++;
					if ( curpos[0] != '%' )
					{
						Q_FixSlashes( curpos );
						char fullPath[ MAX_PATH ];
						Q_snprintf( fullPath, sizeof(fullPath), "%s/%s", m_BaseDir.String(), curpos );
						Q_StripTrailingSlash( fullPath );
						config.AddInclude( fullPath );
					}

					curpos = delim;
					try1 = strchr( delim, ';' );
					try2 = strchr( delim, ',' );
				}

				if ( curpos[0] != '%' )
				{
					Q_FixSlashes( curpos );
					char fullPath[ MAX_PATH ];
					Q_snprintf( fullPath, sizeof(fullPath), "%s/%s", m_BaseDir.String(), curpos );
					Q_StripTrailingSlash( fullPath );
					config.AddInclude( fullPath );
				}

				return true;
			}
		}
	}
	else
	{
		// We are passed a handle to the current config to make things easy
		TiXmlElement *pTool = hDoc.FirstChild("Tool").ToElement();
		for ( pTool; pTool; pTool = pTool->NextSiblingElement() )
		{
			if ( !Q_stricmp( pTool->Attribute("Name"), "VCCLCompilerTool" ) )
			{
				CUtlSymbol defines = pTool->Attribute("PreprocessorDefinitions");
				char *str = (char *)_alloca( Q_strlen( defines.String() ) + 1 );
				Q_strcpy( str, defines.String() );
				// Expanded tokenize with , or ; both are accepted by VS
				char *delim = NULL;
				char *try1 = strchr( str, ';' );
				char *try2 = strchr( str, ',' );
				char *curpos = str;
				while ( try1 || try2 )
				{
					delim = ( (try1 && !try2) || (try1 && try2 && try1 < try2) ) ? try1 : try2;
					*delim = 0;
					delim++;
					if ( Q_stricmp( curpos, "WIN32" ) && Q_stricmp( curpos, "_WIN32" )  &&  
							Q_stricmp( curpos, "_WINDOWS") && Q_stricmp( curpos, "WINDOWS") ) // don't add WIN32 defines
					{
						config.AddDefine( curpos );
					}

					curpos = delim;
					try1 = strchr( delim, ';' );
					try2 = strchr( delim, ',' );
				}
				if ( Q_stricmp( curpos, "WIN32" ) && Q_stricmp( curpos, "_WIN32" )  &&  
						Q_stricmp( curpos, "_WINDOWS") && Q_stricmp( curpos, "WINDOWS") ) // don't add WIN32 defines
				{
					config.AddDefine( curpos );
				}		

				CUtlSymbol includes = pTool->Attribute("AdditionalIncludeDirectories");
				char *str2 = (char *)_alloca( Q_strlen( includes.String() ) + 1 );
				Assert( str2 );
				Q_strcpy( str2, includes.String() );
				// Expanded tokenize with , or ; both are accepted by VS
				delim = NULL;
				try1 = strchr( str2, ';' );
				try2 = strchr( str2, ',' );
				curpos = str2;
				while ( try1 || try2 )
				{
					delim = ( (try1 && !try2) || (try1 && try2 && try1 < try2) ) ? try1 : try2;
					*delim = 0;
					delim++;
					Q_FixSlashes( curpos );
					char fullPath[ MAX_PATH ];
					Q_snprintf( fullPath, sizeof(fullPath), "%s/%s", m_BaseDir.String(), curpos );
					Q_StripTrailingSlash( fullPath );
					config.AddInclude( fullPath );

					curpos = delim;
					try1 = strchr( delim, ';' );
					try2 = strchr( delim, ',' );
				}

				Q_FixSlashes( curpos );
				char fullPath[ MAX_PATH ];
				Q_snprintf( fullPath, sizeof(fullPath), "%s/%s", m_BaseDir.String(), curpos );
				Q_StripTrailingSlash( fullPath );
				config.AddInclude( fullPath );
				return true;
			}
		} // End Tool loop
	} // End m_bIs2010

	return true;
}

void CVCProjConvert::RecursivelyAddFiles( TiXmlElement * pFilter, TiXmlHandle & hDoc )
{
	for ( pFilter; pFilter; pFilter = pFilter->NextSiblingElement() )
	{
		TiXmlNode * pChildFilter = pFilter->FirstChild("Filter");

		if ( pChildFilter )
		{
			RecursivelyAddFiles(pChildFilter->ToElement(), hDoc);
		}

		TiXmlNode *pFile = pFilter->FirstChild("File");
		for ( pFile; pFile; pFile = pFile->NextSibling() )
		{
			CUtlSymbol fileName = pFile->ToElement()->Attribute("RelativePath");
			if ( fileName.IsValid() )
			{
				char fixedFileName[ MAX_PATH ];
				Q_strncpy( fixedFileName, fileName.String(), sizeof(fixedFileName) );
				printf( fixedFileName );
				printf( "\n" );
				if ( fixedFileName[0] == '.' && fixedFileName[1] == '\\' )
				{
					Q_memmove( fixedFileName, fixedFileName+2, sizeof(fixedFileName)-2 );
				}

				Q_FixSlashes( fixedFileName );
				FindFileCaseInsensitive( fixedFileName, sizeof(fixedFileName) );

				TiXmlNode *pExclude = pFile->FirstChild("FileConfiguration");
				const char *pExcluded = pExclude ? pExclude->ToElement()->Attribute("ExcludedFromBuild") : NULL;
				CConfiguration::FileType_e type = GetFileType( fileName.String() );
				CConfiguration::CFileEntry fileEntry( fixedFileName, type );
				for ( int i = 0; i < m_Configurations.Count(); i++ ) // add the file to all configs
				{
					CConfiguration & config = m_Configurations[i];
					// Don't add this file if it is excluded from this config's build
					if ( pExclude && pExcluded && Q_stristr(pExclude->ToElement()->Attribute("Name"), config.GetName().String()) && Q_stricmp(pExcluded, "true") )
						continue;

					config.InsertFile( fileEntry );
				}
			}
		} // file for
	} //filter for
}

//-----------------------------------------------------------------------------
// Purpose: walks the file elements in the vcproj and inserts them into configs
//-----------------------------------------------------------------------------
bool CVCProjConvert::ExtractFiles( TiXmlHandle &hDoc  )
{
	Assert( m_Configurations.Count() ); // some configs must be loaded first

	if ( m_bIs2010 )
	{
		// *.vcxproj
		// <ItemGroup>
		//   <ClCompile Include="RelPath" />
		// </ItemGroup>
		// <ItemGroup>
		//   <ClInclude Include="RelPath" />
		// </ItemGroup>

		// Get ClCompile files
		TiXmlElement *pDefGroup = hDoc.FirstChild("ItemGroup").ToElement();
		for ( pDefGroup; pDefGroup; pDefGroup = pDefGroup->NextSiblingElement() )
		{
			if ( pDefGroup->FirstChild("ClCompile") && pDefGroup->FirstChild("ClCompile")->ToElement()->Attribute("Include") )
				break;
		}

		if ( !pDefGroup )
			return false;

		TiXmlNode *pFile = pDefGroup->FirstChild( "ClCompile" );
		for ( pFile; pFile; pFile = pFile->NextSibling() )
		{
			CUtlSymbol fileName = pFile->ToElement()->Attribute("Include");
			if ( fileName.IsValid() )
			{
				char fixedFileName[ MAX_PATH ];
				Q_strncpy( fixedFileName, fileName.String(), sizeof(fixedFileName) );
				if ( fixedFileName[0] == '.' && fixedFileName[1] == '\\' )
				{
					Q_memmove( fixedFileName, fixedFileName+2, sizeof(fixedFileName)-2 );
				}

				Q_FixSlashes( fixedFileName );
				FindFileCaseInsensitive( fixedFileName, sizeof(fixedFileName) );

				TiXmlNode *pExclude = pFile->FirstChild("ExcludedFromBuild");
				CConfiguration::FileType_e type = GetFileType( fileName.String() );
				CConfiguration::CFileEntry fileEntry( fixedFileName, type );
				for ( int i = 0; i < m_Configurations.Count(); i++ ) // add the file to all configs
				{
					CConfiguration & config = m_Configurations[i];
					// Don't add this file if it is excluded from this config's build
					if ( pExclude && Q_stricmp( pExclude->ToElement()->GetText(), "true" ) && Q_stristr( pExclude->ToElement()->Attribute("Condition"), config.GetName().String() ) )
						continue;

					config.InsertFile( fileEntry );
				}
			}
		}


		// Get ClInclude files
		pDefGroup = hDoc.FirstChild("ItemGroup").ToElement();
		for ( pDefGroup; pDefGroup; pDefGroup = pDefGroup->NextSiblingElement() )
		{
			if ( pDefGroup->FirstChild("ClInclude") && pDefGroup->FirstChild("ClInclude")->ToElement()->Attribute("Include") )
				break;
		}

		if ( !pDefGroup )
			return false;

		pFile = pDefGroup->FirstChild( "ClInclude" );
		for ( pFile; pFile; pFile = pFile->NextSibling() )
		{
			CUtlSymbol fileName = pFile->ToElement()->Attribute("Include");
			if ( fileName.IsValid() )
			{
				char fixedFileName[ MAX_PATH ];
				Q_strncpy( fixedFileName, fileName.String(), sizeof(fixedFileName) );
				if ( fixedFileName[0] == '.' && fixedFileName[1] == '\\' )
				{
					Q_memmove( fixedFileName, fixedFileName+2, sizeof(fixedFileName)-2 );
				}

				Q_FixSlashes( fixedFileName );
				FindFileCaseInsensitive( fixedFileName, sizeof(fixedFileName) );

				TiXmlNode *pExclude = pFile->FirstChild("ExcludedFromBuild");
				CConfiguration::FileType_e type = GetFileType( fileName.String() );
				CConfiguration::CFileEntry fileEntry( fixedFileName, type );
				for ( int i = 0; i < m_Configurations.Count(); i++ ) // add the file to all configs
				{
					CConfiguration & config = m_Configurations[i];
					// Don't add this file if it is excluded from this config's build
					if ( pExclude && Q_stricmp( pExclude->ToElement()->GetText(), "true" ) && Q_stristr( pExclude->ToElement()->Attribute("Condition"), config.GetName().String() ) )
						continue;

					config.InsertFile( fileEntry );
				}
			}
		}
	}
	else
	{
		TiXmlElement *pFilter = hDoc.FirstChild("Files").FirstChild("Filter").ToElement();

		RecursivelyAddFiles( pFilter, hDoc );
	}

	return true;
}



#ifdef _LINUX
static char fileName[MAX_PATH];
int CheckName(const struct dirent *dir)
{
        return !strcasecmp( dir->d_name, fileName );
}

const char *findFileInDirCaseInsensitive(const char *file)
{
        const char *dirSep = strrchr(file,'/');
        if( !dirSep )
        {
                dirSep=strrchr(file,'\\');
                if( !dirSep )
                {
                        return NULL;
                }
        }

        char *dirName = static_cast<char *>( alloca( ( dirSep - file ) +1 ) );
        if( !dirName )
                return NULL;

		Q_strncpy( dirName, file, dirSep - file );
        dirName[ dirSep - file ] = '\0';

        struct dirent **namelist;
        int n;

		Q_strncpy( fileName, dirSep + 1, MAX_PATH );


        n = scandir( dirName , &namelist, CheckName, alphasort );

        if( n > 0 )
        {
                while( n > 1 )
                {
                        free( namelist[n] ); // free the malloc'd strings
                        n--;
                }

                Q_snprintf( fileName, sizeof( fileName ), "%s/%s", dirName, namelist[0]->d_name );
                return fileName;
        }
        else
        {
                // last ditch attempt, just return the lower case version!
                Q_strncpy( fileName, file, sizeof(fileName) );
                Q_strlower( fileName );
                return fileName;
        }
}
#endif

void CVCProjConvert::FindFileCaseInsensitive( char *fileName, int fileNameSize )
{
	char filePath[ MAX_PATH ];

	Q_snprintf( filePath, sizeof(filePath), "%s/%s", m_BaseDir.String(), fileName ); 

	struct _stat buf;
	if ( _stat( filePath, &buf ) == 0)
	{
		return; // found the filename directly
	}

#ifdef _LINUX
	const char *realName = findFileInDirCaseInsensitive( filePath );
	if ( realName )
	{
		Q_strncpy( fileName, realName+strlen(m_BaseDir.String())+1, fileNameSize );
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: extracts the generic type of a file being loaded
//-----------------------------------------------------------------------------
CVCProjConvert::CConfiguration::FileType_e CVCProjConvert::GetFileType( const char *fileName )
{
	CConfiguration::FileType_e type = CConfiguration::FILE_TYPE_UNKNOWN_E;
	char ext[10];
	Q_ExtractFileExtension( fileName, ext, sizeof(ext) );
	if ( !Q_stricmp( ext, "lib" ) )
	{
		type = CConfiguration::FILE_LIBRARY;
	}
	else if ( !Q_stricmp( ext, "h" ) )
	{
		type = CConfiguration::FILE_HEADER;
	}
	else if ( !Q_stricmp( ext, "hh" ) )
	{
		type = CConfiguration::FILE_HEADER;
	}
	else if ( !Q_stricmp( ext, "hpp" ) )
	{
		type = CConfiguration::FILE_HEADER;
	}
	else if ( !Q_stricmp( ext, "cpp" ) )
	{
		type = CConfiguration::FILE_SOURCE;
	}
	else if ( !Q_stricmp( ext, "c" ) )
	{
		type = CConfiguration::FILE_SOURCE;
	}
	else if ( !Q_stricmp( ext, "cc" ) )
	{
		type = CConfiguration::FILE_SOURCE;
	}

	return type;
}
