/***************************************************************************
                    
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <string>

#if defined(__APPLE__)
#	include <Carbon/Carbon.h>
#else
#	include <fcntl.h>
#endif

#ifdef _WIN32
#	include <io.h>
#	include <direct.h>
#	include <shlobj.h>
#	include "ADM_win32.h"
#else
#	include <unistd.h>
#endif

#include "ADM_default.h"


#undef fread
#undef fwrite
#undef fopen
#undef fclose
static void AddSeparator(char *path)
{
	if (path && (strlen(path) < strlen(ADM_SEPARATOR) || strncmp(path + strlen(path) - strlen(ADM_SEPARATOR), ADM_SEPARATOR, strlen(ADM_SEPARATOR)) != 0))
		strcat(path, ADM_SEPARATOR);
}
size_t ADM_fread (void *ptr, size_t size, size_t n, FILE *sstream)
{
	return fread(ptr,size,n,sstream);
}

size_t ADM_fwrite(const void *ptr, size_t size, size_t n, FILE *sstream)
{
	return fwrite(ptr,size,n,sstream);
}

/**
    \fn ADM_fileSize
    \brief return filesize, -1 on error
*/
int64_t ADM_fileSize(const char *file)
{
    FILE *f=ADM_fopen(file,"r");
    if(!f) return -1;
    fseeko(f,0,SEEK_END);
    int64_t v=ftello(f);
    fclose(f);
    return v;
}

int ADM_fclose(FILE *file)
{
	return fclose(file); 
}


#ifdef _WIN32
#define DIR _WDIR
#define dirent _wdirent
#define opendir _wopendir
#define readdir _wreaddir
#define closedir _wclosedir
#endif

/**
 * \fn ADM_PathSplit
 * \brief std::string version of the above
 * @param in
 * @param root
 * @param ext
 */
void            ADM_PathSplit(const std::string &in,std::string &root, std::string &ext)
{
  	char *full;
    std::string canonized;

	full = ADM_PathCanonize(in.c_str());
    canonized=std::string(full);
    delete [] full;full=NULL;
    
    size_t pos=canonized.find_last_of(".");
    
    
    // no "." ?
    if(pos==std::string::npos)
      {
        root=canonized;
        ext=std::string("");
        return;
      }
    
    // else split
    root=canonized.substr(0,pos);
    ext=canonized.substr(pos+1);
}
/**
    \fn ADM_copyFile
*/
uint8_t ADM_copyFile(const char *source, const char *target)
{
    FILE *fin=ADM_fopen(source,"rb");
    if(!fin)
    {
        ADM_error("Cannot open %s for reading\n",source);
        return false;
    }
    FILE *fout=ADM_fopen(target,"wb");
    if(!fout)
    {
        fclose(fin);
        ADM_error("Cannot open %s for writting\n",target);
        return false;
    }
    uint8_t buffer[1024];
    while(!feof(fin))
    {
        int r=(int)fread(buffer,1,1024,fin);
        fwrite(buffer,1,r,fout);
        if(r!=1024) break;
    }

    fclose(fin);
    fclose(fout);
    return true;
}


/**
 * 	\fn ADM_getRelativePath
 */
char *ADM_getRelativePath(const char *base0, const char *base1, const char *base2, const char *base3)
{
	char *result;
	int length = (int)strlen(base0) + 2;

	if (base1)
		length += (int)strlen(base1) + 1;

	if (base2)
		length += (int)strlen(base2) + 1;

	if (base3)
		length += (int)strlen(base3) + 1;

	result = (char *)new char[length];
	strcpy(result, base0);
	AddSeparator(result);

	if (base1)
	{
		if (strlen(base1))
		{
			strcat(result, base1);
			strcat(result, ADM_SEPARATOR);
		}

		if (base2)
		{
			if (strlen(base2))
			{
				strcat(result, base2);
				strcat(result, ADM_SEPARATOR);
			}

			if (base3 && strlen(base3))
			{
				strcat(result, base3);				
				strcat(result, ADM_SEPARATOR);
			}
		}
	}

	return result;
}


/**
 *  \fn ADM_getCustomDir
 *  \brief Get the directory where custom scripts are stored
 */
static std::string ADM_customdir;
const std::string ADM_getCustomDir(void)
{
    if (ADM_customdir.size())
        return ADM_customdir;

    const char *s = ADM_getHomeRelativePath("custom");

    if (ADM_mkdir(s))
        ADM_customdir = s;
    else
        ADM_warning("Cannot create custom directory (\"%s\").\n", s);

    delete [] s;
    s=NULL;

    return ADM_customdir;
}


/**
 *  \fn ADM_getJobDir
 *  \brief Get the directory where jobs are stored
 */
static std::string ADM_jobdir;
const std::string ADM_getJobDir(void)
{
    if (ADM_jobdir.size())
        return ADM_jobdir;

    const char *s = ADM_getHomeRelativePath("jobs");

    if (ADM_mkdir(s))
        ADM_jobdir = s;
    else
        ADM_warning("Cannot create jobs directory (\"%s\").\n", s);

    delete [] s;
    s=NULL;

    return ADM_jobdir;
}

/**
 *  \fn ADM_getUserPluginSettingsDir
 *  \brief returns the path to the user plugin setting directory
 */
static std::string ADM_userPluginSettings;
const std::string ADM_getUserPluginSettingsDir(void)
{
    if(ADM_userPluginSettings.size())
        return ADM_userPluginSettings;

    const char *s = ADM_getHomeRelativePath("pluginSettings");
    if (ADM_mkdir(s))
        ADM_userPluginSettings = s;
    else
        ADM_warning("Cannot create pluginSettings directory (\"%s\").\n", s);

    delete [] s;
    s=NULL;

    return ADM_userPluginSettings;
}


/**
    \fn isPortableMode
    \brief returns true if we are in portable mode
*/
bool isPortableMode(int argc, char *argv[])
{
	bool portableMode = false;
    std::string mySelf=argv[0];
    // if the name ends by "_portable.exe" => portable
    int match=(int)mySelf.find("portable");
    if(match!=-1)
    {
        ADM_info("Portable mode\n");
        return true;
    }

    for (int i = 0; i < argc; i++)
    {
            if (!strcmp(argv[i], "--portable"))
            {
                    portableMode = true;
                    break;
            }
    }

    return portableMode;
}

static std::string pluginDir;

/**
 * \fn ADM_setPluginDir
 */
void ADM_setPluginDir(std::string path)
{
    pluginDir = path;
}
/**
 * \fn ADM_getPluginDir
 */
std::string ADM_getPluginDir(const char *subfolder)
{
    if(!pluginDir.size())
    {
        const char *startDir =
#ifdef __APPLE__
        "../lib";
#else
        ADM_RELATIVE_LIB_DIR;
#endif
        char *p = ADM_getInstallRelativePath(startDir, ADM_PLUGIN_DIR);
        pluginDir = p;
        delete [] p;
        p = NULL;
    }
    if(subfolder)
        return pluginDir + subfolder;
    return pluginDir;
}

// EOF
