#pragma once
#include <string>
#include <vector>
std::string getCurrentDirectory( );
bool writeFileOverwrite( std::string const & fullpath, std::string const & file );
bool writeFile( std::string const& fullpath, std::string const& file );
bool loadFile( std::string const& fullpath, std::string* dst );
bool copyFile( std::string const& src, std::string const& dst );
void replace( std::string* src, std::string const& prev, std::string const& write );
std::string toWindowsPath( std::string fullpath );
std::vector<std::string> getWindowsPathSplited( std::string const& fullpath );
std::string getDirectoryName( std::string path );
std::string getExtensionName( std::string const& path );
std::vector<std::string> split( std::string const& s, char delim );
std::vector<std::string> split( std::string const& s, std::string const& delim );
bool find( std::string const& dst, std::string const& str );
void errorExit( std::vector<std::string> comment );
bool isDirectory( std::string const& fullPath );
bool isFile( std::string const& fullPath );
bool makeDirectory( std::string const& fullPath );
std::string convertCRLF( std::string targetStr );
std::string convertLF( std::string targetStr );