#include "utility.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <direct.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

std::string getCurrentDirectory( )
{
    static std::string dir;
    if ( dir.empty( ) )
    {
        TCHAR crDir[MAX_PATH + 1];
        GetCurrentDirectoryA( MAX_PATH + 1, crDir );
        dir = crDir;
    }
    return dir;
}

bool writeFileOverwrite( std::string const & fullpath, std::string const & file )
{
    std::ofstream ofs( fullpath );
    if ( !ofs ) {
        std::cerr << "ファイルを書き込めませんでした。: " << fullpath << '\n';
        return false;
    }
    std::cout << fullpath << std::endl;
    std::cout << file << std::endl;
    ofs << file;
    ofs.close( );
    return true;
}

bool writeFile( std::string const& fullpath, std::string const& file )
{
    if ( isFile( fullpath ) )
    {
        std::cout << fullpath << std::endl;
        std::cout << "すでにファイルが存在します。上書きしますか？[Y/n]" << std::endl;
        std::string s;
        std::cin >> s;
        if ( ( s == "Y" ) || ( s == "y" ) )
        {
            // nothing
        }
        else
        {
            std::cout << "上書きしませんでした。" << std::endl;
            return false;
        }
    }
    return writeFileOverwrite( fullpath, file );
}

bool loadFile( std::string const & fullpath, std::string * dst )
{
    std::ifstream ifs( fullpath, std::ios::binary );
    if ( !ifs ) {
        std::cerr << "ファイルを開けませんでした。: " << fullpath << '\n';
        return false;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf( ) << std::flush;
    *dst = buffer.str( );
    return true;
}

bool copyFile( std::string const & srcFullpath, std::string const & dstFullpath )
{
    std::ifstream ifs( srcFullpath, std::ios::binary );
    if ( !ifs ) {
        std::cerr << "ファイルを開けませんでした。: " << srcFullpath << '\n';
        return false;
    }
    std::ofstream ofs( dstFullpath, std::ios::binary );
    if ( !ofs ) {
        std::cerr << "ファイルを開けませんでした。: " << dstFullpath << '\n';
        return false;
    }
    ofs << ifs.rdbuf( ) << std::flush;
    return true;
}

void replace( std::string * src, std::string const & prev, std::string const & write )
{
    std::string::size_type pos( src->find( prev ) );
    while ( pos != std::string::npos )
    {
        src->replace( pos, prev.length( ), write );
        pos = src->find( prev, pos + write.length( ) );
    }
}

std::string toWindowsPath( std::string fullpath )
{
    replace( &fullpath, "/", "\\" );
    return fullpath;
}

std::vector<std::string> getWindowsPathSplited( std::string const & fullpath )
{
    return split( fullpath, '\\' );
}

std::string getDirectoryName( std::string path )
{
    std::string::size_type end( path.rfind( '\\' ) );
    return ( end != std::string::npos ) ? path.substr( 0, end ).c_str( ) : std::string( );
}

std::string getExtensionName( std::string const & path )
{
    std::string::size_type pos( path.rfind( '.' ) );
    return ( pos != std::string::npos ) ? path.substr( pos + 1 ).c_str( ) : std::string( );
}

std::vector<std::string> split( std::string const & s, char delim )
{
    std::vector<std::string> elems;
    std::stringstream ss( s );
    std::string item;
    while ( getline( ss, item, delim ) ) {
        if ( !item.empty( ) ) {
            elems.push_back( item );
        }
    }
    return elems;
}

std::vector<std::string> split( std::string const & s, std::string const & delim )
{
    std::vector<std::string> result;
    std::string::size_type pos = 0;
    while ( pos != std::string::npos )
    {
        std::string::size_type p = s.find( delim, pos );
        if ( p == std::string::npos )
        {
            result.push_back( s.substr( pos ) );
            break;
        }
        else
        {
            result.push_back( s.substr( pos, p - pos ) );
        }
        pos = p + delim.size( );
    }
}

bool find( std::string const & dst, std::string const & str )
{
    return dst.find( str ) != dst.npos;
}

void errorExit( std::vector<std::string> comment )
{
    for ( auto o : comment )
    {
        std::cout << o << std::endl;
    }
    std::cout << "3秒後、自動的に終了します。" << std::endl;
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    exit( 1 );
}

bool isDirectory( std::string const & fullPath )
{
    return PathIsDirectory( fullPath.c_str( ) );
}

bool isFile( std::string const & fullPath )
{
    return PathFileExists( fullPath.c_str( ) );
}

bool makeDirectory( std::string const & fullPath )
{
    if ( _mkdir( fullPath.c_str( ) ) )
    {
        std::cout << "\"" << fullPath << "\"ディレクトリの作成に失敗しました。" << std::endl;
        return false;
    }
    else
    {
        std::cout << "\"" << fullPath << "\"を作成しました。" << std::endl;
        return true;
    }
}

std::string convertCRLF( std::string targetStr )
{
    replace( &targetStr, "\r", "\n" );
    replace( &targetStr, "\n", "\r\n" );
    return targetStr;
}

std::string convertLF( std::string targetStr )
{
    replace( &targetStr, "\r\n", "\n" );
    replace( &targetStr, "\r", "\n" );
    return targetStr;
}
