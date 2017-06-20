#pragma once

#include "app.h"
#define MAX_PATH_SIZE 1024

class CRenderer;
class IInputHandler;

class RLSimionApp : public CSimionApp
{
	CRenderer *m_pRenderer = 0;
	IInputHandler *m_pInputHandler = 0;
	void initRenderer(string sceneFile);
	void updateScene(CState* s);

	char m_directory[MAX_PATH_SIZE];
public:

	RLSimionApp(CConfigNode* pParameters);
	virtual ~RLSimionApp();

	void getInputFiles(CFilePathList& filePathList);
	void getOutputFiles(CFilePathList& filePathList);
	void setOutputDirectory(const char*);
	char* getOutputDirectory();

	void run();
};