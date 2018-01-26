#include "utility.h"
#include <iostream>
#include <sstream>
#include <Windows.h>
#include <random>
#include <iomanip> 
#include <direct.h>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <algorithm>
using namespace std;
std::string eventHPP = "#pragma once\n";
std::string requestHPP = "#pragma once\n";
std::string responseHPP = "#pragma once\n";
std::vector<std::string> eventClass;
std::vector<std::string> eventEnum;
std::vector<std::string> requestClass;
std::vector<std::string> requestEnum;
std::vector<std::string> responseClass;
std::vector<std::string> responseEnum;
std::string templateIncludeFile;
std::string templateSourceFile;
void packetWriteIncludeFile( std::string packetType, std::string className, std::string enumName )
{
    std::string append = "#include <Network/Packet/" + packetType + "/" + className + ".h>\n";
    if ( packetType == "Event" )
    {
        eventHPP += append;
    }
    else if ( packetType == "Request" )
    {
        requestHPP += append;
    }
    else if ( packetType == "Response" )
    {
        responseHPP += append;
    }

    std::string writeFullPath = getCurrentDirectory( ) + "\\include\\Network\\Packet\\" + packetType + "\\" + className + ".h";
    cout << "ファイル名: " << writeFullPath << endl;
    if ( isFile( writeFullPath.c_str( ) ) )
    {
        cout << "ファイルはすでにありました。" << endl;
    }
    else
    {
        std::string writableFile = templateIncludeFile;
        replace( &writableFile, "_PACKET_TYPE_", packetType );
        replace( &writableFile, "_TEMPLATE_CLASS_", className );
        replace( &writableFile, "_TEMPLATE_ENUM_", enumName );

        cout << writableFile << endl;
        writableFile = convertLF( writableFile );
        writeFileOverwrite( writeFullPath, writableFile );
    }
}
void packetWriteSourceFile( std::string packetType, std::string className, std::string enumName )
{
    std::string writeFullPath = getCurrentDirectory( ) + "\\src\\Network\\Packet\\" + packetType + "\\" + className + ".cpp";
    cout << "ファイル名: " << writeFullPath << endl;
    if ( isFile( writeFullPath.c_str( ) ) )
    {
        cout << "ファイルはすでにありました。" << endl;
    }
    else
    {
        std::string writableFile = templateSourceFile;
        replace( &writableFile, "_PACKET_TYPE_", packetType );
        replace( &writableFile, "_TEMPLATE_CLASS_", className );
        replace( &writableFile, "_TEMPLATE_ENUM_", enumName );

        cout << writableFile << endl;
        writableFile = convertLF( writableFile );
        writeFileOverwrite( writeFullPath, writableFile );
    }
}
std::string enumToClassName( std::string enumName )
{
    std::string className = "c";
    auto parts = split( enumName, '_' );
    for ( auto& o : parts )
    {
        std::transform( o.begin( ), o.end( ), o.begin( ), tolower );
        std::transform( o.begin( ), ++o.begin( ), o.begin( ), toupper );
        className += o;
    }
    return className;
}
void packet( std::string packetType, std::string className, std::string enumName )
{
    cout << "packetType: " << packetType << endl;
    cout << "enumName  : " << enumName << endl;
    cout << "className : " << className << endl;

    packetWriteIncludeFile( packetType, className, enumName );
    packetWriteSourceFile( packetType, className, enumName );
}
void lineReader( std::string line )
{
	if ( findComment( line ) ) return;

    auto eventPos = line.find( "EVE_" );
    if ( eventPos != line.npos )
    {
        auto commaPos = line.find( "," );
        string enumName = line.substr( eventPos, commaPos - eventPos );
        eventEnum.emplace_back( enumName );
        string className = enumToClassName( enumName );
        eventClass.emplace_back( className );
        packet( "Event", className, enumName );
        return;
    }
    auto requestPos = line.find( "REQ_" );
    if ( requestPos != line.npos )
    {
        auto commaPos = line.find( "," );
        string enumName = line.substr( requestPos, commaPos - requestPos );
        requestEnum.emplace_back( enumName );
        string className = enumToClassName( enumName );
        requestClass.emplace_back( className );
        packet( "Request", className, enumName );
        return;
    }
    auto responsePos = line.find( "RES_" );
    if ( responsePos != line.npos )
    {
        auto commaPos = line.find( "," );
        string enumName = line.substr( responsePos, commaPos - responsePos );
        responseEnum.emplace_back( enumName );
        string className = enumToClassName( enumName );
        responseClass.emplace_back( className );
        packet( "Response", className, enumName );
        return;
    }
}
void analysisPacketId( )
{
    std::string packetIdFullPath = getCurrentDirectory( ) + "\\include\\Network\\Packet\\PacketId.h";
    string packetIDfile;
    if ( !loadFile( packetIdFullPath, &packetIDfile ) )
    {
        errorExit( {
            packetIdFullPath,
            "が見つかりませんでした。"
        } );
    }
    packetIDfile = convertLF( packetIDfile );
    stringstream ss( packetIDfile );
    string line;
    std::vector<std::string> lines;
    while ( std::getline( ss, line ) )
    {
        lines.emplace_back( line );
        std::cout << line << std::endl;
    }
    for ( auto& o : lines )
    {
        lineReader( o );
    }
}
void hppReplace( std::string type, std::string hpp )
{
    std::string fullPath = getCurrentDirectory( ) + "\\include\\Network\\Packet\\_TYPE_.hpp";
    replace( &fullPath, "_TYPE_", type );
    hpp = convertLF( hpp );
    writeFileOverwrite( fullPath, hpp );
    cout << "\"" << fullPath << "\"を再定義しました。" << endl;
}
std::vector<string> beginEndSpliter( std::string pullPath )
{
    std::string file;
    loadFile( pullPath, &file );
    file = convertLF( file );

    stringstream ss( file );
    string line;
    string beforeBegin;
    string afterEnd;
    bool begin = false;
    bool end = false;
    while ( std::getline( ss, line ) )
    {
        if ( begin )
        {
            if ( line.find( "PACKET_TEMPLATE_END" ) != line.npos )
            {
                end = true;
            }
        }
        if ( !begin )
        {
            beforeBegin += line + "\n";
            if ( line.find( "PACKET_TEMPLATE_BEGIN" ) != line.npos )
            {
                begin = true;
            }
        }
        if ( end )
        {
            afterEnd += line + "\n";
        }
    }
    return { beforeBegin, afterEnd };
}
void udpManagerHeaderReplace( )
{
	auto fileFullPath = getCurrentDirectory( ) + "\\include\\Network\\cUDPManager.h";
	auto file = beginEndSpliter( fileFullPath );

	string tp;
	loadFile( getCurrentDirectory( ) + "\\pmake\\MANAGER.h", &tp );
	tp = convertLF( tp );

	string source = file[0];
	for ( int i = 0; i < eventEnum.size( ); ++i )
	{
		auto tyType = "Event";
		auto tyEnum = eventEnum[i];
		auto tyValue = eventClass[i].substr( 1 );
		auto t = tp;
		replace( &t, "_PACKET_TYPE_", tyType );
		replace( &t, "_TEMPLATE_VALUE_", tyValue );
		source += t;
	}
	for ( int i = 0; i < requestEnum.size( ); ++i )
	{
		auto tyType = "Request";
		auto tyEnum = requestEnum[i];
		auto tyValue = requestClass[i].substr( 1 );
		auto t = tp;
		replace( &t, "_PACKET_TYPE_", tyType );
		replace( &t, "_TEMPLATE_VALUE_", tyValue );
		source += t;
	}
	for ( int i = 0; i < responseEnum.size( ); ++i )
	{
		auto tyType = "Response";
		auto tyEnum = responseEnum[i];
		auto tyValue = responseClass[i].substr( 1 );
		auto t = tp;
		replace( &t, "_PACKET_TYPE_", tyType );
		replace( &t, "_TEMPLATE_VALUE_", tyValue );
		source += t;
	}
	source += file[1];

	source = convertLF( source );
	if ( writeFileOverwrite( fileFullPath, source ) )
	{
		cout << fileFullPath << std::endl;
		cout << "の\"begin\"から\"end\"までを書き換えました。" << std::endl;
	}
}
void udpManagerSoruceReplace( )
{
	string fileFullPath = getCurrentDirectory( ) + "\\src\\Network\\cUDPManager.cpp";
	auto file = beginEndSpliter( fileFullPath );

	string tp;
	loadFile( getCurrentDirectory( ) + "\\pmake\\MANAGER.cpp", &tp );
	tp = convertLF( tp );

	string source = file[0];
	for ( int i = 0; i < eventEnum.size( ); ++i )
	{
		auto tyType = "Event";
		auto tyEnum = eventEnum[i];
		auto tyValue = eventClass[i].substr( 1 );
		auto t = tp;
		replace( &t, "_TEMPLATE_ENUM_", tyEnum );
		replace( &t, "_PACKET_TYPE_", tyType );
		replace( &t, "_TEMPLATE_VALUE_", tyValue );
		source += t;
	}
	for ( int i = 0; i < requestEnum.size( ); ++i )
	{
		auto tyType = "Request";
		auto tyEnum = requestEnum[i];
		auto tyValue = requestClass[i].substr( 1 );
		auto t = tp;
		replace( &t, "_TEMPLATE_ENUM_", tyEnum );
		replace( &t, "_PACKET_TYPE_", tyType );
		replace( &t, "_TEMPLATE_VALUE_", tyValue );
		source += t;
	}
	for ( int i = 0; i < responseEnum.size( ); ++i )
	{
		auto tyType = "Response";
		auto tyEnum = responseEnum[i];
		auto tyValue = responseClass[i].substr( 1 );
		auto t = tp;
		replace( &t, "_TEMPLATE_ENUM_", tyEnum );
		replace( &t, "_PACKET_TYPE_", tyType );
		replace( &t, "_TEMPLATE_VALUE_", tyValue );
		source += t;
	}
	source += file[1];

	source = convertLF( source );
	if ( writeFileOverwrite( fileFullPath, source ) )
	{
		cout << fileFullPath << std::endl;
		cout << "の\"begin\"から\"end\"までを書き換えました。" << std::endl;
	}
}
int main( int argv, char* argc [ ] )
{
    if ( !isDirectory( getCurrentDirectory( ) + "\\assets" ) ||
         !isDirectory( getCurrentDirectory( ) + "\\include" ) ||
         !isDirectory( getCurrentDirectory( ) + "\\resources" ) ||
         !isDirectory( getCurrentDirectory( ) + "\\src" ) ||
         !isDirectory( getCurrentDirectory( ) + "\\vc2017" ) )
    {
        errorExit( {
            "cinderのフォルダ構成が見つかりませんでした。",
        } );
    }

    if ( !isDirectory( getCurrentDirectory( ) + "\\pmake" ) )
    {
        errorExit( {
            "pmakeフォルダが見つかりませんでした。",
        } );
    }

    std::vector<std::string> folderCheck =
    {
        getCurrentDirectory( ) + "\\include\\Network",
        getCurrentDirectory( ) + "\\include\\Network\\Packet",
        getCurrentDirectory( ) + "\\include\\Network\\Packet\\Event",
        getCurrentDirectory( ) + "\\include\\Network\\Packet\\Request",
        getCurrentDirectory( ) + "\\include\\Network\\Packet\\Response",
        getCurrentDirectory( ) + "\\src\\Network",
        getCurrentDirectory( ) + "\\src\\Network\\Packet",
        getCurrentDirectory( ) + "\\src\\Network\\Packet\\Event",
        getCurrentDirectory( ) + "\\src\\Network\\Packet\\Request",
        getCurrentDirectory( ) + "\\src\\Network\\Packet\\Response",
    };

    for ( auto& o : folderCheck )
    {
        if ( !isDirectory( o ) )
        {
            makeDirectory( o );
        }
    }

    if ( !loadFile( getCurrentDirectory( ) + "\\pmake\\TEMPLATE.h", &templateIncludeFile ) )
    {
        errorExit( {
            getCurrentDirectory( ) + "\\pmake\\TEMPLATE.h",
            "が見つかりませんでした。"
        } );
    }
    convertLF( templateIncludeFile );
    if ( !loadFile( getCurrentDirectory( ) + "\\pmake\\TEMPLATE.cpp", &templateSourceFile ) )
    {
        errorExit( {
            getCurrentDirectory( ) + "\\pmake\\TEMPLATE.cpp",
            "が見つかりませんでした。"
        } );
    }
    convertLF( templateSourceFile );

    // PacketId.hからパケット一覧を解析します。
    analysisPacketId( );

    // Event.hppなどの定義まとめを置き換える。
    hppReplace( "Event", eventHPP );
    hppReplace( "Request", requestHPP );
    hppReplace( "Response", responseHPP );

    // cUdpManager.hを書き換える。
    udpManagerHeaderReplace( );
	udpManagerSoruceReplace( );

    std::cout << "3秒後、自動的に終了します。" << std::endl;
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    return 0;
}