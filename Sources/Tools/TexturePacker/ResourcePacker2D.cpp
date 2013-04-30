#include "TexturePacker/ResourcePacker2D.h"
#include "TexturePacker/DefinitionFile.h"
#include "TexturePacker/TexturePacker.h"
#include "TexturePacker/CommandLineParser.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Core/Core.h"
#include "Platform/SystemTimer.h"
#include "Utils/MD5.h"
#include "Utils/StringFormat.h"

#include <Magick++.h>
#include <magick/MagickCore.h>
#include <magick/property.h>

namespace DAVA
{

ResourcePacker2D::ResourcePacker2D()
{
	isLightmapsPacking = false;
	clearProcessDirectory = false;
}

String ResourcePacker2D::GetProcessFolderName()
{
	return "$process/";
}

void ResourcePacker2D::InitFolders(const FilePath & inputPath,const FilePath & outputPath)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());
    
	inputGfxDirectory = inputPath;
	outputGfxDirectory = outputPath;
	excludeDirectory = inputPath + "../";
}
    
void ResourcePacker2D::PackResources()
{
	Logger::Debug("Input: %s \nOutput: %s \nExclude: %s",
                  inputGfxDirectory.GetAbsolutePathname().c_str(),
                  outputGfxDirectory.GetAbsolutePathname().c_str(),
                  excludeDirectory.GetAbsolutePathname().c_str());
	
	isGfxModified = false;

    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
	std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);


	FilePath processDirectoryPath = excludeDirectory + GetProcessFolderName();
	if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
	}


	if (IsMD5ChangedDir(processDirectoryPath, outputGfxDirectory, gfxDirName + ".md5", true))
	{
		if (Core::Instance()->IsConsoleMode())
			printf("[Gfx not available or changed - performing full repack]\n");
		isGfxModified = true;
	
		// Remove whole output directory
		bool result = FileSystem::Instance()->DeleteDirectory(outputGfxDirectory);
		if (result)
		{
			Logger::Debug("Removed output directory: %s", outputGfxDirectory.GetAbsolutePathname().c_str());
		}
		if (!result && Core::Instance()->IsConsoleMode())
		{
			printf("[ERROR: Can't delete directory %s]\n", outputGfxDirectory.GetAbsolutePathname().c_str());
		}
	}

	RecursiveTreeWalk(inputGfxDirectory, outputGfxDirectory);

	// Put latest md5 after convertation
	IsMD5ChangedDir(processDirectoryPath, outputGfxDirectory, gfxDirName + ".md5", true);
}


bool ResourcePacker2D::IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & name, bool isRecursive)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());

	FilePath md5FileName = FilePath::CreateWithNewExtension(processDirectoryPath + name, ".md5");

    
	uint8 oldMD5Digest[16];
	uint8 newMD5Digest[16];
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);
	if (!file)
	{
		isChanged = true;		
	}else
	{
		int32 bytes = file->Read(oldMD5Digest, 16);
		DVASSERT(bytes == 16 && "We should always read 16 bytes from md5 file");
	}
	SafeRelease(file);


	MD5::ForDirectory(pathname, newMD5Digest, isRecursive);

	file = File::Create(md5FileName, File::CREATE | File::WRITE);
    
	int32 bytes = file->Write(newMD5Digest, 16);
	DVASSERT(bytes == 16 && "16 bytes should be always written for md5 file");
	SafeRelease(file);

	// if already changed return without compare
	if (isChanged)
		return true;

	for (int32 k = 0; k < 16; ++k)
		if (oldMD5Digest[k] != newMD5Digest[k])
			isChanged = true;

	return isChanged;
}


bool ResourcePacker2D::IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());

	FilePath md5FileName = FilePath::CreateWithNewExtension(processDirectoryPath + psdName, ".md5");

	uint8 oldMD5Digest[16];
	uint8 newMD5Digest[16];
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);
	if (!file)
	{
		isChanged = true;		
	}else
	{
		int32 bytes = file->Read(oldMD5Digest, 16);
		DVASSERT(bytes == 16 && "We should always read 16 bytes from md5 file");
	}
	SafeRelease(file);

		
	MD5::ForFile(pathname, newMD5Digest);
	
	file = File::Create(md5FileName, File::CREATE | File::WRITE);
	int32 bytes = file->Write(newMD5Digest, 16);
	DVASSERT(bytes == 16 && "16 bytes should be always written for md5 file");
	SafeRelease(file);

	// file->Write()

	for (int32 k = 0; k < 16; ++k)
		if (oldMD5Digest[k] != newMD5Digest[k])
			isChanged = true;
	
	return isChanged;
}

DefinitionFile * ResourcePacker2D::ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());
    
	int32 maxTextureSize = 1024;
	if (CommandLineParser::Instance()->IsFlagSet("--tsize2048"))
	{
		maxTextureSize = 2048;
	}
	
	// TODO: Check CRC32
	Vector<Magick::Image> layers;
	
    FilePath psdNameWithoutExtension(processDirectoryPath + psdName);
    psdNameWithoutExtension.TruncateExtension();
	
	try 
	{
		Magick::readImages(&layers, psdPathname.GetAbsolutePathname());
		
		if (layers.size() == 0)
		{
			Logger::Error("Number of layers is too low: %s", psdPathname.GetAbsolutePathname().c_str());
			return 0;
		}
		
		if (layers.size() == 1)
		{
			layers.push_back(layers[0]);
		}
		
		//Logger::Debug("psd file: %s wext: %s", psdPathname.c_str(), psdNameWithoutExtension.c_str());
		
		int width = (int)layers[0].columns();
		int height = (int)layers[0].rows();
		
		for(int k = 1; k < (int)layers.size(); ++k)
		{
			Magick::Image & currentLayer = layers[k];
			
			currentLayer.crop(Magick::Geometry(width,height, 0, 0));
			currentLayer.magick("PNG");
			FilePath outputFile = FilePath::CreateWithNewExtension(psdNameWithoutExtension, String(Format("%d.png", k - 1)));
			currentLayer.write(outputFile.GetAbsolutePathname());
		}
		
		
		DefinitionFile * defFile = new DefinitionFile;
		defFile->filename = FilePath::CreateWithNewExtension(psdNameWithoutExtension, ".txt");
		defFile->spriteWidth = width;
		defFile->spriteHeight = height;
		defFile->frameCount = (int)layers.size() -1;
		defFile->frameRects = new Rect2i[defFile->frameCount];
		
		for(int k = 1; k < (int)layers.size(); ++k)
		{
			Magick::Image & currentLayer = layers[k];
			Magick::Geometry bbox = currentLayer.page();
			int xOff = (int)bbox.xOff();
			if (bbox.xNegative())
				xOff = -xOff;
			int yOff = (int)bbox.yOff();
			if (bbox.yNegative())
				yOff = -yOff;
			
			defFile->frameRects[k - 1] = Rect2i(xOff, yOff, (int32)bbox.width(), (int32)bbox.height());
			
			//printf("Percent: %d Aspect: %d Greater: %d Less: %d\n", (int)bbox.percent(), (int)bbox.aspect(), (int)bbox.greater(), (int)bbox.less());
			
			if ((defFile->frameRects[k - 1].dx >= maxTextureSize) || (defFile->frameRects[k - 1].dy >= maxTextureSize))
			{
				
				printf("* WARNING * - frame of %s layer %d is bigger than maxTextureSize(%d) layer exportSize (%d x %d) FORCE REDUCE TO (%d x %d). Bewarned!!! Results not guaranteed!!!\n", psdName.c_str(), k - 1, maxTextureSize
					   , defFile->frameRects[k - 1].dx, defFile->frameRects[k - 1].dy, width, height);
				defFile->frameRects[k - 1].dx = width;
				defFile->frameRects[k - 1].dy = height;
			}
				
			
			
			if (CommandLineParser::Instance()->IsFlagSet("--add0pixel"))
			{
				
			}else if (CommandLineParser::Instance()->IsFlagSet("--add1pixel"))
			{
				defFile->frameRects[k - 1].dx++;
				defFile->frameRects[k - 1].dy++;
			}
			else if (CommandLineParser::Instance()->IsFlagSet("--add2pixel"))
			{
				defFile->frameRects[k - 1].dx+=2;
				defFile->frameRects[k - 1].dy+=2;
			}
			else if (CommandLineParser::Instance()->IsFlagSet("--add4pixel"))
			{
				defFile->frameRects[k - 1].dx+=4;
				defFile->frameRects[k - 1].dy+=4;
			}
			else if(CommandLineParser::Instance()->IsFlagSet("--add2sidepixel"))
			{
				defFile->frameRects[k - 1].dx+=2;
				defFile->frameRects[k - 1].dy+=2;
			}
			else
			{
				defFile->frameRects[k - 1].dx++;
				defFile->frameRects[k - 1].dy++;	
			}
		}
		
		return defFile;
	}
	catch( Magick::Exception &error_ )
    {
        printf("Caught exception: %s file: %s", error_.what(), psdPathname.GetAbsolutePathname().c_str());
		return 0;
    }
	return 0;
}

void ResourcePacker2D::ProcessFlags(const FilePath & flagsPathname)
{
	File * file = File::Create(flagsPathname, File::READ | File::OPEN);
	if (!file)
	{
		Logger::Error("Failed to open file: %s", flagsPathname.GetAbsolutePathname().c_str());
	}
	char flagsTmpBuffer[4096];
	int flagsSize = 0;
	while(!file->IsEof())
	{
		char c;
		int32 readSize = file->Read(&c, 1);
		if (readSize == 1)
		{
			flagsTmpBuffer[flagsSize++] = c;
		}	
	}
	flagsTmpBuffer[flagsSize++] = 0;
	
	currentFlags = flagsTmpBuffer;
	String flags = flagsTmpBuffer;
	
	const String & delims=" ";
	
	// Skip delims at beginning, find start of first token
	String::size_type lastPos = flags.find_first_not_of(delims, 0);
	// Find next delimiter @ end of token
	String::size_type pos     = flags.find_first_of(delims, lastPos);
	// output vector
	Vector<String> tokens;
	
	while (String::npos != pos || String::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(flags.substr(lastPos, pos - lastPos));
		// Skip delims.  Note the "not_of". this is beginning of token
		lastPos = flags.find_first_not_of(delims, pos);
		// Find next delimiter at end of token.
		pos     = flags.find_first_of(delims, lastPos);
	}
	
	if (CommandLineParser::Instance()->GetVerbose())
		for (int k = 0; k < (int) tokens.size(); ++k)
		{
			Logger::Debug("Token: %s", tokens[k].c_str());
		}

	if (Core::Instance()->IsConsoleMode())
	{
		for (int k = 0; k < (int) tokens.size(); ++k)
		{
			String sub = tokens[k].substr(0, 2);
			if (sub != "--")
				printf("\n[WARNING: flag %s incorrect]\n", tokens[k].c_str());
		}
	}
	
	CommandLineParser::Instance()->SetFlags(tokens);
	
	SafeRelease(file);
}


void ResourcePacker2D::RecursiveTreeWalk(const FilePath & inputPath, const FilePath & outputPath)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());
    
	uint64 packTime = SystemTimer::Instance()->AbsoluteMS();

	/* New $process folder structure */
	
	String dataSourceRelativePath = inputPath.GetRelativePathname(excludeDirectory);
	FilePath processDirectoryPath = excludeDirectory  + GetProcessFolderName() + dataSourceRelativePath;
	if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
	}

	if(clearProcessDirectory)
	{
		FileSystem::Instance()->DeleteDirectoryFiles(processDirectoryPath, false);
	}

	//String outputPath = outputPath;
	if (FileSystem::Instance()->CreateDirectory(outputPath) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", outputPath.c_str());
	}
	
	CommandLineParser::Instance()->ClearFlags();
	List<DefinitionFile *> definitionFileList;

	// Find flags and setup them
	FileList * fileList = new FileList(inputPath);
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (!fileList->IsDirectory(fi))
		{
			if (fileList->GetFilename(fi) == "flags.txt")
			{
				ProcessFlags(fileList->GetPathname(fi));
				break;
			}
		}
	}
	
	bool modified = isGfxModified;
	// Process all psd / png files

	if (IsMD5ChangedDir(processDirectoryPath, inputPath, "dir.md5", false))
	{
		modified = true;
		//if (Core::Instance()->IsConsoleMode())
		//	printf("[Directory changed - rebuild: %s]\n", inputGfxDirectory.c_str());
	}

	if (modified)
	{
		FileSystem::Instance()->DeleteDirectoryFiles(outputPath, false);
		
		for (int fi = 0; fi < fileList->GetCount(); ++fi)
		{
			if (!fileList->IsDirectory(fi))
			{
				FilePath fullname = fileList->GetPathname(fi);
				if (fullname.IsEqualToExtension(".psd"))
				{
                    //TODO: check if we need filename or pathname
					DefinitionFile * defFile = ProcessPSD(processDirectoryPath, fullname, fullname.GetFilename());
					definitionFileList.push_back(defFile);
				}
				else if(isLightmapsPacking && fullname.IsEqualToExtension(".png"))
				{
					DefinitionFile * defFile = new DefinitionFile();
					defFile->LoadPNG(fullname, processDirectoryPath);
					definitionFileList.push_back(defFile);
				}
				else if (fullname.IsEqualToExtension(".pngdef"))
				{
					DefinitionFile * defFile = new DefinitionFile();
					if (defFile->LoadPNGDef(fullname, processDirectoryPath))
					{
						definitionFileList.push_back(defFile);
					}
					else 
					{
						SafeDelete(defFile);
					}
				}
			}
		}

		// 
		if (definitionFileList.size() > 0 && modified)
		{
			TexturePacker packer;
			if(isLightmapsPacking)
			{
				packer.UseOnlySquareTextures();
				packer.SetMaxTextureSize(2048);
			}

			if (CommandLineParser::Instance()->IsFlagSet("--split"))
			{
				packer.PackToTexturesSeparate(excludeDirectory, outputPath, definitionFileList);
			}
			else
			{
				packer.PackToTextures(excludeDirectory, outputPath, definitionFileList);
			}
		}
	}	

	packTime = SystemTimer::Instance()->AbsoluteMS() - packTime;

	if (Core::Instance()->IsConsoleMode())
	{
		if (CommandLineParser::Instance()->IsExtendedOutput())
		{
			printf("[%d files packed with flags: %s]\n", (int)definitionFileList.size(), currentFlags.c_str());
		}
	
		String result = "[unchanged]";
		if (modified)
			result = "[REPACKED]";

		printf("[%s - %.2lf secs] - %s\n", inputPath.GetAbsolutePathname().c_str(), (float64)packTime / 1000.0f, result.c_str());
	}

	
	for (List<DefinitionFile*>::iterator it = definitionFileList.begin(); it != definitionFileList.end(); ++it)
	{
		DefinitionFile * file = *it;
		SafeDelete(file);
	}
	definitionFileList.clear();
	
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
			String filename = fileList->GetFilename(fi);
			if (!fileList->IsNavigationDirectory(fi) && (filename != "$process") && (filename != ".svn"))
			{
				if ((filename.size() > 0) && (filename[0] != '.'))
                {
                    FilePath input = inputPath + filename;
                    input.MakeDirectoryPathname();
                    
                    FilePath output = outputPath + filename;
                    output.MakeDirectoryPathname();
					RecursiveTreeWalk(input, output);
                }
			}
		}
	}
	
	SafeRelease(fileList);
}


};