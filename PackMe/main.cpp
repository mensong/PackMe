#include "PackMe.h"
#include "PackMeNamed.h"
#include <Windows.h>
#include <shlwapi.h>
#include "ArgumentMan.h"

void ListFilesA(const char* lpPath, std::vector<std::string>& vctDir, std::vector<std::string>& vctFiles, const char* filter = "*.*", bool bSubDir = true, bool bAppendPath = true)
{
	char szFind[MAX_PATH] = { 0 };
	StrCpyA(szFind, lpPath);
	StrCatA(szFind, "/");
	StrCatA(szFind, filter);

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = ::FindFirstFileA(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
		return;

	char szFile[MAX_PATH] = { 0 };
	while (TRUE)
	{
		szFile[0] = '\0';
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				if (bAppendPath)
				{
					StrCpyA(szFile, lpPath);
					StrCatA(szFile, "\\");
					StrCatA(szFile, FindFileData.cFileName);
				}
				else
				{
					StrCpyA(szFile, FindFileData.cFileName);
				}
				vctDir.push_back(szFile);

				if (bSubDir)
				{
					ListFilesA(szFile, vctDir, vctFiles, filter, bSubDir, bAppendPath);
				}
			}
		}
		else
		{
			if (bAppendPath)
			{
				StrCpyA(szFile, lpPath);
				StrCatA(szFile, "\\");
				StrCatA(szFile, FindFileData.cFileName);
			}
			else
			{
				StrCpyA(szFile, FindFileData.cFileName);
			}
			vctFiles.push_back(szFile);
		}

		if (!FindNextFileA(hFind, &FindFileData))
			break;
	}

	FindClose(hFind);
}

std::string GetDirFromPath(const std::string& sPath)
{
	int len = strlen(sPath.c_str());
	std::string path = sPath;
	char c = path[len - 1];
	if (c == '\\' || c == '/')
		path = path.substr(0, len - 1);

	size_t split1 = path.find_last_of('/');
	size_t split2 = path.find_last_of('\\');
	size_t split;
	if (split1 == std::string::npos && split2 != std::string::npos)
	{
		split = split2;
	}
	else if (split1 != std::string::npos && split2 == std::string::npos)
	{
		split = split1;
	}
	else if (split1 != std::string::npos && split2 != std::string::npos)
	{
		split = max(split1, split2);
	}
	else
	{
		return "";
	}
	
	path = path.substr(0, split);

	return path;
}

std::string GetFileNameFromPath(const std::string& sPath)
{
	int len = strlen(sPath.c_str());
	char c = sPath[len - 1];
	if (c == '\\' || c == '/')
	{
		return "";
	}

	std::string path = sPath;
	size_t split1 = path.find_last_of('/');
	size_t split2 = path.find_last_of('\\');
	size_t split;
	if (split1 == std::string::npos && split2 != std::string::npos)
	{
		split = split2;
	}
	else if (split1 != std::string::npos && split2 == std::string::npos)
	{
		split = split1;
	}
	else if (split1 != std::string::npos && split2 != std::string::npos)
	{
		split = max(split1, split2);
	}
	else
	{
		return sPath;
	}
	
	return path.substr(split + 1);
}

bool CreateMultipleDirectory(const char* szPath)
{
	std::string strDir(szPath);//���Ҫ������Ŀ¼�ַ���
	//ȷ����'\'��β�Դ������һ��Ŀ¼
	if (strDir[strDir.length() - 1] != '\\' )
	{
		strDir += '\\';
	}
	std::vector<std::string> vPath;//���ÿһ��Ŀ¼�ַ���
	std::string strTemp;//һ����ʱ����,���Ŀ¼�ַ���
	bool bSuccess = false;//�ɹ���־
	//����Ҫ�������ַ���
	for (int i = 0; i < strDir.length(); ++i)
	{
		if (strDir[i] != '\\')
		{//�����ǰ�ַ�����'\\'
			strTemp += strDir[i];
		}
		else
		{//�����ǰ�ַ���'\\'
			vPath.push_back(strTemp);//����ǰ����ַ������ӵ�������
			strTemp += '\\';
		}
	}

	//�������Ŀ¼������,����ÿ��Ŀ¼
	std::vector<std::string>::const_iterator vIter;
	for (vIter = vPath.begin(); vIter != vPath.end(); vIter++)
	{
		//���CreateDirectoryִ�гɹ�,����true,���򷵻�false
		bSuccess = CreateDirectoryA(vIter->c_str(), NULL) ? true : false;
	}

	return bSuccess;
}

bool StringReplaceA(std::string& strBase, const std::string& strSrc, const std::string& strDes)
{
	bool b = false;

	std::string::size_type pos = 0;
	std::string::size_type srcLen = strSrc.size();
	std::string::size_type desLen = strDes.size();
	pos = strBase.find(strSrc, pos);
	while ((pos != std::string::npos))
	{
		strBase.replace(pos, srcLen, strDes);
		pos = strBase.find(strSrc, (pos + desLen));
		b = true;
	}

	return b;
}

size_t MyReadFile(const char * path, char* outContent, size_t & in_outLen, size_t start=0)
{
	FILE *f = NULL;
	long sz;
	size_t nFileSize = 0;

	if (!path)
	{
		return 0;
	}

	f = fopen(path, "rb");
	if (!f)
	{
		return 0;
	}

	do
	{
		if (fseek(f, 0, SEEK_END) < 0)
		{
			break;
		}

		sz = ftell(f);
		if (sz < 0)
		{
			break;
		}
		nFileSize = (size_t)sz;

		if (fseek(f, start, SEEK_SET) < 0)
		{
			break;
		}

		size_t nToRead = in_outLen;
		if (nToRead == 0)
		{
			nToRead = (size_t)sz;
		}

		size_t nReaded = fread(outContent, 1, nToRead, f);
		if (nReaded > nToRead)
		{
			break;
		}

		in_outLen = nReaded;

	} while (0);

	fclose(f);

	return nFileSize;
}


void main(int argc, char** argv)
{	
	AM::CArgumentMan<> am;
	am.SetIgnoreCase(true);
	am.SetMultiValue(true);
	am.AddCmdFlag('-');
	am.AddCmdFlag('/');
	am.AddCmdFlag('\\');
	am.Parse(argc, argv);

	bool bRead = true;

	std::string sOutFile = am.GetArg("o");
	if (sOutFile.length() < 1)
	{
		printf("-o targetFile -f file1 [file2...] -d dir1 [dir2...]\n");
		return;
	}

	bool bBak = false;
	std::string sBak;
	if (am.Has(sBak, "bak"))
		bBak = true;

	std::vector<std::string> vctFiles = am.GetArgs("f");
	std::vector<std::string> vctDirs = am.GetArgs("d");
	std::vector<std::string> vctDir;
	for (int i = 0; i < vctDirs.size(); ++i)
	{
		ListFilesA(vctDirs[i].c_str(), vctDir, vctFiles);
	}
	
	if (vctFiles.size() > 0)
		bRead = false;
	else
		bRead = true;

	try
	{
		PackMeNamed pak(sOutFile.c_str(), bRead);
		if (!bRead)
		{//write
			if (pak.IsPacked())
			{
				printf("�Ѿ��������!\n");
				return;
			}

			//����
			if (bBak)
				CopyFileA(sOutFile.c_str(), (sOutFile+".bak").c_str(), FALSE);

			char* fileContent = new char[1024 * 1024 * 10];
			for (int i = 0; i < vctFiles.size(); ++i)
			{
				const char* pFileName = vctFiles[i].c_str();

				pak.BeginBlock(pFileName);
				//�ļ�������|�ļ���|�ļ�����
				//file name
				long fileNameLen = strlen(pFileName);
				pak.AppendBlockData((const char*)&fileNameLen, sizeof(fileNameLen));
				pak.AppendBlockData(pFileName, fileNameLen);
				//file content
				size_t len = 0;
				if (MyReadFile(pFileName, (char*)fileContent, len) == 0)
				{
					printf("��ȡ�ļ�ʧ��%s\n", pFileName);
				}
				else
				{
					printf("%s\n", pFileName);
				}
				pak.AppendBlockData(fileContent, len);

				pak.EndBlock();
			}
			delete[] fileContent;
		}
		else
		{//read
			long nCount = pak.IndexCount();
			for (int i = 0; i < nCount; ++i)
			{
				//��ȫ������
				long len = pak.GetDataLen(i);
				char* pData = new char[len];
				pak.ReadData(i, pData);
				char* p = pData;
				//�����ļ�������
				long fileNameLen = 0;
				p = PackMe::GetDataByType(p, fileNameLen);
				//�����ļ���
				char* fileName = new char[fileNameLen + 1];
				memcpy(fileName, p, fileNameLen);
				fileName[fileNameLen] = '\0';
				p = p + fileNameLen;
				//�����ļ���
				std::string sDir = GetDirFromPath(fileName);
				if (!sDir.empty())
				{//�����ļ���
					StringReplaceA(sDir, "/", "\\");
					CreateMultipleDirectory(sDir.c_str());
				}
				//�����ļ�����
				FILE* f = fopen(fileName, "wb+");
				if (f == NULL)
				{
					printf("д���ļ�ʧ��%s\n", fileName);
					continue;
				}
				fwrite(p, len - sizeof(fileNameLen) - fileNameLen, 1, f);
				fclose(f);

				printf("%s\n", fileName);

				delete[] fileName;
				delete[] pData;
			}
		}

		pak.Close();
	}
	catch (const std::exception& )
	{

	}	
	
}