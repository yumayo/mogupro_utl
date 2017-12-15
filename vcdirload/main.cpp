#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <sstream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip> 
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "cSearchSystem.h"
#include <direct.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
using namespace rapidxml;
using namespace std;
bool loadFile( string relativePath, string* dst )
{
    ifstream ifs( relativePath, ios::binary );
    if ( !ifs ) {
        cerr << "file open error: " << relativePath << '\n';
        return false;
    }
    stringstream buffer;
    buffer << ifs.rdbuf( ) << flush;
    *dst = buffer.str( );
    return true;
}
bool copyFile( string src, string dst )
{
    ifstream ifs( src, ios::binary );
    if ( !ifs ) {
        cerr << "file open error: " << src << '\n';
        return false;
    }
    ofstream ofs( dst, ios::binary );
    if ( !ofs ) {
        cerr << "file open error: " << dst << '\n';
        return false;
    }
    ofs << ifs.rdbuf( ) << flush;
    return true;
}
void replace( string* src, string prev, string write )
{
    string::size_type  Pos( src->find( prev ) );
    while ( Pos != string::npos )
    {
        src->replace( Pos, prev.length( ), write );
        Pos = src->find( prev, Pos + write.length( ) );
    }
}
std::string getDirectoryName( std::string path )
{
    std::string::size_type end( path.rfind( '\\' ) );
    return ( end != std::string::npos ) ? path.substr( 0, end ).c_str( ) : std::string( );
}
std::string getExtensionName( const std::string& path )
{
    std::string::size_type pos( path.rfind( '.' ) );
    return ( pos != std::string::npos ) ? path.substr( pos + 1 ).c_str( ) : std::string( );
}
std::string toHex( unsigned char value )
{
    std::stringstream ss;
    ss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << static_cast<unsigned int>( value );
    return ss.str( );
}
std::string createUniqueIdentifier( )
{
    std::string uniqueIdentifier;
    std::random_device rd;
    for ( int i = 0; i < 4; ++i ) uniqueIdentifier += toHex( rd( ) );
    uniqueIdentifier += "-";
    for ( int i = 0; i < 2; ++i ) uniqueIdentifier += toHex( rd( ) );
    uniqueIdentifier += "-";
    for ( int i = 0; i < 2; ++i ) uniqueIdentifier += toHex( rd( ) );
    uniqueIdentifier += "-";
    for ( int i = 0; i < 2; ++i ) uniqueIdentifier += toHex( rd( ) );
    uniqueIdentifier += "-";
    for ( int i = 0; i < 6; ++i ) uniqueIdentifier += toHex( rd( ) );
    return uniqueIdentifier;
}
std::string currentDir;
cSearchSystem includeFileSearch;
cSearchSystem srcFileSearch;
void xmlDraw( xml_node<> *base, int deep )
{
    for ( xml_node<> *node = base; node; node = node->next_sibling( ) )
    {
        for ( int i = 0; i < deep; ++i ) cout << "    ";
        cout << "ノード名: " << node->name( ) << std::endl;
        for ( xml_attribute<> *attr = node->first_attribute( ); attr; attr = attr->next_attribute( ) )
        {
            for ( int i = 0; i < deep; ++i ) cout << "    ";
            cout << "属性名: " << std::setw( 12 ) << std::setfill( ' ' ) << attr->name( );
            cout << "    属性値: " << std::setw( 12 ) << std::setfill( ' ' ) << attr->value( ) << endl;
        }
        xmlDraw( node->first_node( ), deep + 1 );
    }
}
bool sln( std::string path )
{
    return copyFile( "vcdirload\\TEMPLATE.sln", path + "\\game.sln" );
}
bool vcxprojFilters( std::string path )
{
    string file;
    if ( !loadFile( "vcdirload\\TEMPLATE.vcxproj.filters", &file ) ) return false;

    xml_document<> doc;
    try
    {
        doc.parse<0>( (char*)file.c_str( ) );
    }
    catch ( parse_error& err )
    {
        cout << err.what( ) << " " << err.where<char*>( );
        return false;
    }

    auto folderNode = doc.first_node( )->first_node( );
    auto compileNode = folderNode->next_sibling( );
    auto includeNode = compileNode->next_sibling( );

    for ( auto o : srcFileSearch.windowsNotationDirectorys( ) )
    {
        auto node = doc.allocate_node( node_type::node_element, "Filter" );
        folderNode->append_node( node );
        node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "Source Files\\" + o ).c_str( ) ) ) );
        node->append_node( doc.allocate_node( node_type::node_element, "UniqueIdentifier", doc.allocate_string( std::string( "{" + createUniqueIdentifier( ) + "}" ).c_str( ) ) ) );
    }
    for ( auto o : includeFileSearch.windowsNotationDirectorys( ) )
    {
        auto node = doc.allocate_node( node_type::node_element, "Filter" );
        folderNode->append_node( node );
        node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "Header Files\\" + o ).c_str( ) ) ) );
        node->append_node( doc.allocate_node( node_type::node_element, "UniqueIdentifier", doc.allocate_string( std::string( "{" + createUniqueIdentifier( ) + "}" ).c_str( ) ) ) );
    }
    for ( auto o : srcFileSearch.windowsNotationFiles( ) )
    {
        auto ext = getExtensionName( o );
        if ( ext == "h" || ext == "hpp" )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClInclude" );
            compileNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\src\\" + o ).c_str( ) ) ) );
            std::string dir = getDirectoryName( o ).empty( ) ? "Source Files" : "Source Files\\" + getDirectoryName( o );
            node->append_node( doc.allocate_node( node_type::node_element, "Filter", doc.allocate_string( dir.c_str( ) ) ) );
        }
        else if ( ext == "c" || ext == "cpp" )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClCompile" );
            compileNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\src\\" + o ).c_str( ) ) ) );
            std::string dir = getDirectoryName( o ).empty( ) ? "Source Files" : "Source Files\\" + getDirectoryName( o );
            node->append_node( doc.allocate_node( node_type::node_element, "Filter", doc.allocate_string( dir.c_str( ) ) ) );
        }
    }
    for ( auto o : includeFileSearch.windowsNotationFiles( ) )
    {
        auto ext = getExtensionName( o );
        if ( ext == "h" || ext == "hpp" )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClInclude" );
            includeNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\include\\" + o ).c_str( ) ) ) );
            std::string dir = getDirectoryName( o ).empty( ) ? "Header Files" : "Header Files\\" + getDirectoryName( o );
            node->append_node( doc.allocate_node( node_type::node_element, "Filter", doc.allocate_string( dir.c_str( ) ) ) );
        }
        else if ( ext == "c" || ext == "cpp" )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClCompile" );
            includeNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\include\\" + o ).c_str( ) ) ) );
            std::string dir = getDirectoryName( o ).empty( ) ? "Header Files" : "Header Files\\" + getDirectoryName( o );
            node->append_node( doc.allocate_node( node_type::node_element, "Filter", doc.allocate_string( dir.c_str( ) ) ) );
        }
    }

    xmlDraw( doc.first_node( ), 0 );
    cout << "====================================" << std::endl;

    std::ofstream file_stored( path + "\\game.vcxproj.filters" );
    file_stored << doc;
    file_stored.close( );
    doc.clear( );

    return true;
}
bool vcxproj( std::string path )
{
    string file;
    if ( !loadFile( "vcdirload\\TEMPLATE.vcxproj", &file ) ) return false;

    xml_document<> doc;
    try
    {
        doc.parse<0>( (char*)file.c_str( ) );
    }
    catch ( parse_error& err )
    {
        cout << err.what( ) << " " << err.where<char*>( );
        return false;
    }

    auto node = doc.first_node( )->first_node( );
    for ( int i = 0; i <= 23; ++i )
    {
        node = node->next_sibling( );
    }
    auto compileFileNode = node;
    auto includeFileNode = node->next_sibling( );

    for ( auto o : srcFileSearch.windowsNotationFiles( ) )
    {
        auto ext = getExtensionName( o );
        if ( ( ext == "h" ) || ( ext == "hpp" ) )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClInclude" );
            includeFileNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\src\\" + o ).c_str( ) ) ) );
        }
        else if ( ( ext == "c" ) || ( ext == "cpp" ) )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClCompile" );
            compileFileNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\src\\" + o ).c_str( ) ) ) );
        }
    }
    for ( auto o : includeFileSearch.windowsNotationFiles( ) )
    {
        auto ext = getExtensionName( o );
        if ( ext == "h" || ext == "hpp" )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClInclude" );
            includeFileNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\include\\" + o ).c_str( ) ) ) );
        }
        else if ( ext == "c" || ext == "cpp" )
        {
            auto node = doc.allocate_node( node_type::node_element, "ClCompile" );
            compileFileNode->append_node( node );
            node->append_attribute( doc.allocate_attribute( "Include", doc.allocate_string( std::string( "..\\include\\" + o ).c_str( ) ) ) );
        }
    }

    xmlDraw( doc.first_node( ), 0 );
    cout << "====================================" << std::endl;

    std::ofstream file_stored( path + "\\game.vcxproj" );
    file_stored << doc;
    file_stored.close( );
    doc.clear( );

    return true;
}
int main( int argv, char* argc [ ] )
{
    TCHAR crDir[MAX_PATH + 1];
    GetCurrentDirectoryA( MAX_PATH + 1, crDir );
    currentDir = crDir;

    if ( !PathIsDirectory( std::string( currentDir + "\\assets" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\include" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\resources" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\src" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\vc2017" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\vcdirload" ).c_str( ) ) )
    {
        std::cout << "cinderのファイル構成がある場所出ないか、";
        std::cout << "\"vcdirloadフォルダ\"が見つかりませんでした。" << std::endl;
        std::cout << "3秒後、自動的に終了します。" << std::endl;
        std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
        return 1;
    }

    includeFileSearch.search( currentDir + "\\include" );
    srcFileSearch.search( currentDir + "\\src" );

    if ( !PathFileExists( std::string( currentDir + "\\vc2017\\game.sln" ).c_str( ) ) )
    {
        sln( "vc2017" );
    }
    vcxproj( "vc2017" );
    vcxprojFilters( "vc2017" );

    std::cout << "3秒後、自動的に終了します。" << std::endl;
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
    return 0;
}