
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include "cdrLang.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "cdrLang.cpp"
#include "debug.h"

#define maxsize 35
static ssize_t  __getline(char **lineptr, ssize_t *n, FILE *stream)
{
	ssize_t count=0;
	int buf;

	if(*lineptr == NULL) {
		*n=maxsize;
		*lineptr = (char*)malloc(*n);
	}

	if(( buf = fgetc(stream) ) == EOF ) {
		return -1;
	}

	do
	{
		if(buf=='\n') {    
			count += 1;
			break;
		}

		count++;

		*(*lineptr+count-1) = buf;
		*(*lineptr+count) = '\0';

		if(*n <= count)
			*lineptr = (char*)realloc(*lineptr,count*2);
		buf = fgetc(stream);
	} while( buf != EOF);

	return count;
}


cdrLang::cdrLang()
	: lang(LANG_ERR)
	, mLogFont(NULL)
	, mLogFont_ch(NULL)
	, mLogFont_other(NULL)
{
	db_info("cdrLang Constructor\n");
	mLangFileTable.clear();

	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_CN, std::string("/etc/res/lang/zh-CN.bin")) );
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_TW,		std::string("/etc/res/lang/zh-TW.bin")) );
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_EN,		std::string("/etc/res/lang/en.bin") ));
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_JPN,	std::string("/etc/res/lang/jpn.bin") ));
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_KR,		std::string("/etc/res/lang/korean.bin")) );
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_RS,		std::string("/etc/res/lang/russian.bin")) );
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_DE,		std::string("/etc/res/lang/german.bin") ));
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_FR,		std::string("/etc/res/lang/franch.bin") ));
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_IT,		std::string("/etc/res/lang/italian.bin") ));
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_ES,		std::string("/etc/res/lang/spanish.bin") ));
	mLangFileTable.insert(std::pair<LANGUAGE, std::string>(LANG_PL,		std::string("/etc/res/lang/polish.bin") ));
}


int cdrLang::initLabel(LANGUAGE language)
{
	std::string dataFile;
	int i = 1000;
    std::map<LANGUAGE, std::string>::iterator index;  
	
	db_msg("initLabel");

	index = mLangFileTable.find(language);
	if(index != mLangFileTable.end()) {
		dataFile = index->second;
	} else {
		db_error("invalid language %d\n", language);
		return -1;
	}

	db_msg("language data file is %s\n", dataFile.c_str());
	while(i--){

	}
		
	if(loadLabelFromLangFile(dataFile) < 0) {
		db_error("load label from %s failed\n", dataFile.c_str());
		return -1;
	}
	db_msg("by hero nothing initLabel end 2");
	if(loadLabelFromLangFile(dataFile) < 0) {
		db_error("load label from %s failed\n", dataFile.c_str());
		return -1;
	}
	db_msg("by hero nothing initLabel end");
	return 0;
}



int cdrLang::loadLabelFromLangFile(std::string langFile)
{
	FILE *fp;
	char *line = NULL;
	ssize_t len = 0;

	fp = fopen(langFile.c_str(), "r");
	if (fp == NULL) {
		db_error("open file %s failed\n", langFile.c_str());
		return -1;
	}
	db_msg("by hero nothing loadLabelFromLangFile 1");
	if(label.size() != 0) {
		db_msg("size is %zd\n", label.size());
		label.clear();
		db_msg("size is %zd\n", label.size());
	}
	while ((__getline(&line, &len, fp)) != -1) {
		//db_msg("Retrieved line of length %ld :\n", read);
		//db_msg("%s\n", line);
		label.insert(label.end(),std::string(line));
	}

	free(line);
	db_msg("size is %zd\n", label.size());
	fclose(fp);

	return 0;
}

const char* cdrLang::getLabel(unsigned int labelIndex)
{
	if(labelIndex < label.size()) {
		return (const char*)label[labelIndex].c_str();
	}
	else {
		db_msg("invalide label Index: %d, size is %zd\n", labelIndex, label.size());
		return NULL;
	}
}

