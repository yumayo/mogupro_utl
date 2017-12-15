#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <sstream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip> 
#include <direct.h>
#include <cstdlib>
#include <algorithm>
#include <shlwapi.h>
#include <regex>
#pragma comment(lib, "shlwapi.lib")
using namespace std;
bool isDirectory( std::string fullPath )
{
    return PathIsDirectory( fullPath.c_str( ) );
}
bool isFile( std::string fullPath )
{
    return PathFileExists( fullPath.c_str( ) );
}
bool writeFile( string relativePath, string file )
{
    if ( isFile( relativePath ) )
    {
        cout << relativePath << endl;
        cout << "すでにファイルが存在します。上書きしますか？[Y/n]" << endl;
        string s;
        cin >> s;
        if ( ( s == "Y" ) || ( s == "y" ) )
        {

        }
        else
        {
            cout << "上書きしませんでした。" << endl;
            return false;
        }
    }
    std::ofstream ofs( relativePath );
    if ( !ofs ) {
        cerr << "file write error: " << relativePath << '\n';
        return false;
    }
    cout << relativePath << endl;
    cout << file << endl;
    ofs << file;
    ofs.close( );
}
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
vector<string> split( const string &s, char delim )
{
    vector<string> elems;
    stringstream ss( s );
    string item;
    while ( getline( ss, item, delim ) ) {
        if ( !item.empty( ) ) {
            elems.push_back( item );
        }
    }
    return elems;
}
void errorExit( std::vector<std::string> comment )
{
    for ( auto o : comment )
    {
        cout << o << endl;
    }
    cout << "3秒後、自動的に終了します。" << endl;
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
}
bool makeDirectory( std::string fullPath )
{
    if ( _mkdir( fullPath.c_str( ) ) )
    {
        cout << fullPath << " ディレクトリの作成に失敗しました。" << endl;
        return false;
    }
    else
    {
        cout << fullPath << " を作成しました。" << endl;
        return true;
    }
}
std::string createHeaderOnly( std::vector<std::string> namespaces, std::string className )
{
    std::string t;
    t += "#pragma once\n";
    for ( auto& o : namespaces )
    {
        t += "namespace " + o + "\n{\n";
    }
    std::string classTemplate =
        "class _TEMPLATE_CLASS_\n"
        "{\n"
        "public:\n"
        "    _TEMPLATE_CLASS_( )\n"
        "    {\n"
        "    \n"
        "    }\n"
        "    ~_TEMPLATE_CLASS_( )\n"
        "    {\n"
        "    \n"
        "    }\n"
        "};\n";
    replace( &classTemplate, "_TEMPLATE_CLASS_", className );
    t += classTemplate;
    for ( int i = 0; i < namespaces.size( ); ++i )
    {
        t += "}\n";
    }
    return t;
}
std::string createHeader( std::vector<std::string> namespaces, std::string className )
{
    std::string t;
    t += "#pragma once\n";
    for ( auto& o : namespaces )
    {
        t += "namespace " + o + "\n{\n";
    }
    std::string classTemplate =
        "class _TEMPLATE_CLASS_\n"
        "{\n"
        "public:\n"
        "    _TEMPLATE_CLASS_( );\n"
        "    ~_TEMPLATE_CLASS_( );\n"
        "};\n";
    replace( &classTemplate, "_TEMPLATE_CLASS_", className );
    t += classTemplate;
    for ( int i = 0; i < namespaces.size( ); ++i )
    {
        t += "}\n";
    }
    return t;
}
std::string createSoruce( std::vector<std::string> namespaces, std::string className )
{
    std::string t;
    t += "#include <";
    for ( auto& o : namespaces )
    {
        t += o + "/";
    }
    t += className + ".h>\n";
    for ( auto& o : namespaces )
    {
        t += "namespace " + o + "\n{\n";
    }
    std::string classTemplate =
        "_TEMPLATE_CLASS_::_TEMPLATE_CLASS_( )\n"
        "{\n"
        "\n"
        "}\n"
        "_TEMPLATE_CLASS_::~_TEMPLATE_CLASS_( )\n"
        "{\n"
        "\n"
        "}\n";
    replace( &classTemplate, "_TEMPLATE_CLASS_", className );
    t += classTemplate;
    for ( int i = 0; i < namespaces.size( ); ++i )
    {
        t += "}\n";
    }
    return t;
}
std::string currentDir;
void headerOnly( std::string folderName, std::vector<std::string> splited, std::string className )
{
    std::string headerDir;
    for ( auto& o : splited )
    {
        headerDir += "\\" + o;
        std::string dir = currentDir + "\\include" + headerDir;
        if ( !isDirectory( dir ) )
        {
            makeDirectory( dir );
        }
    }
    std::string header = currentDir + "\\include" + headerDir + "\\" + className + ".h";
    writeFile( header, createHeaderOnly( splited, className ) );
}
void headerSource( std::string folderName, std::vector<std::string> splited, std::string className )
{
    std::string headerDir;
    for ( auto& o : splited )
    {
        headerDir += "\\" + o;
        std::string dir = currentDir + "\\include" + headerDir;
        if ( !isDirectory( dir ) )
        {
            makeDirectory( dir );
        }
    }
    std::string sourceDir;
    for ( auto& o : splited )
    {
        sourceDir += "\\" + o;
        std::string dir = currentDir + "\\src" + sourceDir;
        if ( !isDirectory( dir ) )
        {
            makeDirectory( dir );
        }
    }
    std::string header = currentDir + "\\include" + headerDir + "\\" + className + ".h";
    writeFile( header, createHeader( splited, className ) );
    std::string source = currentDir + "\\src" + sourceDir + "\\" + className + ".cpp";
    writeFile( source, createSoruce( splited, className ) );
}
int main( )
{
    TCHAR crDir[MAX_PATH + 1];
    GetCurrentDirectoryA( MAX_PATH + 1, crDir );
    currentDir = crDir;

    if ( !PathIsDirectory( std::string( currentDir + "\\assets" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\include" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\resources" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\src" ).c_str( ) ) ||
         !PathIsDirectory( std::string( currentDir + "\\vc2017" ).c_str( ) ) )
    {
        errorExit( {
            "cinderのファイル構成がある場所ではありません。",
        } );
        return 1;
    }

    string folderName;
    cout << "フォルダ階層を入力してください。[hoge/fuga/piyo]" << endl;
    cin >> folderName;
    replace( &folderName, "/", "\\" );
    auto splited = split( folderName, '\\' );
    for ( int i = 0; i < splited.size( ); ++i )
    {
        if ( splited[i].empty( ) )
        {
            splited.erase( splited.begin( ) + i );
            --i;
        }
    }

    string className;
    cout << "クラス名を入力してください。" << endl;
    cin >> className;

    std::string value;
    cout << "どのファイル構成にしますか？[h/hcpp]" << endl;
    cin >> value;
    
    if ( value == "h" )
    {
        headerOnly( folderName, splited, className );
    }
    else if ( value == "hcpp" )
    {
        headerSource( folderName, splited, className );
    }
    else
    {
        errorExit( {
            "構成名が不正です。" ,
        } );
        return 1;
    }

    std::cout << "3秒後、自動的に終了します。" << std::endl;
    std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

    return 0;
}