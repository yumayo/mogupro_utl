//
// http://qiita.com/episteme/items/0e3c2ee8a8c03780f01e
// 上記のサイトより

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <map>
class cSearchSystem
{
    std::string mWindowsNotationRoot;
    std::string mUnixNotationRoot;
    std::vector<std::string> mWindowsNotationFiles;
    std::vector<std::string> mWindowsNotationDirectorys;
    std::vector<std::string> mUnixNotationFiles;
    std::vector<std::string> mUnixNotationDirectorys;
public:
    void search( std::string const& path )
    {
        mWindowsNotationRoot = path;
        replace( &mWindowsNotationRoot, "/", "\\" );
        searchPath( path );
    }
    std::vector<std::string> const& windowsNotationFiles( )
    {
        return mWindowsNotationFiles;
    }
    std::vector<std::string> const& windowsNotationDirectorys( )
    {
        return mWindowsNotationDirectorys;
    }
    std::vector<std::string> const& unixNotationFiles( )
    {
        return mUnixNotationFiles;
    }
    std::vector<std::string> const& unixNotationDirectorys( )
    {
        return mUnixNotationDirectorys;
    }
private:
    std::wstring multi_to_wide_capi( std::string const& src )
    {
        std::size_t converted { };
        std::vector<wchar_t> dest( src.size( ), L'\0' );
        if ( ::_mbstowcs_s_l( &converted, dest.data( ), dest.size( ), src.data( ), _TRUNCATE, ::_create_locale( LC_ALL, "jpn" ) ) != 0 ) {
            throw std::system_error { errno, std::system_category( ) };
        }
        return std::wstring( dest.begin( ), dest.end( ) );
    }
    std::wstring multi_to_wide_winapi( std::string const& src )
    {
        auto const dest_size = ::MultiByteToWideChar( CP_ACP, 0U, src.data( ), -1, nullptr, 0U );
        std::vector<wchar_t> dest( dest_size, L'\0' );
        if ( ::MultiByteToWideChar( CP_ACP, 0U, src.data( ), -1, dest.data( ), dest.size( ) ) == 0 ) {
            throw std::system_error { static_cast<int>( ::GetLastError( ) ), std::system_category( ) };
        }
        return std::wstring( dest.begin( ), dest.end( ) );
    }
    std::string wide_to_multi_capi( std::wstring const& src )
    {
        std::size_t converted { };
        std::vector<char> dest( src.size( ) * sizeof( wchar_t ) + 1, '\0' );
        if ( ::_wcstombs_s_l( &converted, dest.data( ), dest.size( ), src.data( ), _TRUNCATE, ::_create_locale( LC_ALL, "jpn" ) ) != 0 ) {
            throw std::system_error { errno, std::system_category( ) };
        }
        return std::string( dest.begin( ), dest.end( ) );
    }
    std::string wide_to_multi_winapi( std::wstring const& src )
    {
        auto const dest_size = ::WideCharToMultiByte( CP_ACP, 0U, src.data( ), -1, nullptr, 0, nullptr, nullptr );
        std::vector<char> dest( dest_size, '\0' );
        if ( ::WideCharToMultiByte( CP_ACP, 0U, src.data( ), -1, dest.data( ), dest.size( ), nullptr, nullptr ) == 0 ) {
            throw std::system_error { static_cast<int>( ::GetLastError( ) ), std::system_category( ) };
        }
        return std::string( dest.begin( ), dest.end( ) );
    }
    std::string wide_to_utf8_cppapi( std::wstring const& src )
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes( src );
    }
    std::string wide_to_utf8_winapi( std::wstring const& src )
    {
        auto const dest_size = ::WideCharToMultiByte( CP_UTF8, 0U, src.data( ), -1, nullptr, 0, nullptr, nullptr );
        std::vector<char> dest( dest_size, '\0' );
        if ( ::WideCharToMultiByte( CP_UTF8, 0U, src.data( ), -1, dest.data( ), dest.size( ), nullptr, nullptr ) == 0 ) {
            throw std::system_error { static_cast<int>( ::GetLastError( ) ), std::system_category( ) };
        }
        return std::string( dest.begin( ), dest.end( ) );
    }
    std::wstring utf8_to_wide_cppapi( std::string const& src )
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes( src );
    }
    std::wstring utf8_to_wide_winapi( std::string const& src )
    {
        auto const dest_size = ::MultiByteToWideChar( CP_UTF8, 0U, src.data( ), -1, nullptr, 0U );
        std::vector<wchar_t> dest( dest_size, L'\0' );
        if ( ::MultiByteToWideChar( CP_UTF8, 0U, src.data( ), -1, dest.data( ), dest.size( ) ) == 0 ) {
            throw std::system_error { static_cast<int>( ::GetLastError( ) ), std::system_category( ) };
        }
        return std::wstring( dest.begin( ), dest.end( ) );
    }
    std::string multi_to_utf8_cppapi( std::string const& src )
    {
        auto const wide = multi_to_wide_capi( src );
        return wide_to_utf8_cppapi( wide );
    }
    std::string multi_to_utf8_winapi( std::string const& src )
    {
        auto const wide = multi_to_wide_winapi( src );
        return wide_to_utf8_winapi( wide );
    }
    std::string utf8_to_multi_cppapi( std::string const& src )
    {
        auto const wide = utf8_to_wide_cppapi( src );
        return wide_to_multi_capi( wide );
    }
    std::string utf8_to_multi_winapi( std::string const& src )
    {
        auto const wide = utf8_to_wide_winapi( src );
        return wide_to_multi_winapi( wide );
    }
    void replace( std::string* src, std::string prev, std::string write )
    {
        std::string::size_type  pos( src->find( prev ) );
        while ( pos != std::string::npos )
        {
            src->replace( pos, prev.length( ), write );
            pos = src->find( prev, pos + write.length( ) );
        }
    }
    void searchDirectory( std::tr2::sys::path const& path )
    {
        std::string src = path.string( ).substr( mWindowsNotationRoot.size( ) + 1 );
        src = multi_to_utf8_winapi( src );
        mWindowsNotationDirectorys.emplace_back( src );
        replace( &src, "\\", "/" );
        mUnixNotationDirectorys.emplace_back( src );
        searchPath( path );
    }
    void searchFile( std::tr2::sys::path const& path )
    {
        std::string src = path.string( ).substr( mWindowsNotationRoot.size( ) + 1 );
        src = multi_to_utf8_winapi( src );
        mWindowsNotationFiles.emplace_back( src );
        replace( &src, "\\", "/" );
        mUnixNotationFiles.emplace_back( src );
    }
    void searchPath( std::tr2::sys::path const& path )
    {
        std::tr2::sys::path p( path ); // 列挙の起点
        std::for_each( std::tr2::sys::directory_iterator( p ), std::tr2::sys::directory_iterator( ), [ this ] ( std::tr2::sys::path const& p )
        {
            // ファイルなら...
            if ( std::tr2::sys::is_regular_file( p ) )
            {
                searchFile( p );
            }
        } );

        p = std::tr2::sys::path( path ); // 列挙の起点
        std::for_each( std::tr2::sys::directory_iterator( p ), std::tr2::sys::directory_iterator( ), [ this ] ( std::tr2::sys::path const& p )
        {
            // ディレクトリなら...
            if ( std::tr2::sys::is_directory( p ) )
            {
                searchDirectory( p );
            }
        } );
    }
};