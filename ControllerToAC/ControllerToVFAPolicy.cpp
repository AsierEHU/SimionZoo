// ControllerToActorCritic.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../tinyxml2/tinyxml2.h"
#include "../RLSimion-Lib/world.h"
#include "../RLSimion-Lib/actor.h"
#include "../RLSimion-Lib/reward.h"
#include "../RLSimion-Lib/logger.h"
#include "../RLSimion-Lib/named-var-set.h"
#include "../RLSimion-Lib/vfa.h"
#include "../RLSimion-Lib/features.h"
#include "../RLSimion-Lib/globals.h"
#include "../RLSimion-Lib/experiment.h"
#include "../RLSimion-Lib/vfa-policy.h"


int main(int argc, char* argv[])
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLElement* pConfig;

	if (argc < 2)
	{
		printf("ERROR: a configuration file should be provided as an argument.");
		exit(-1);
	}


	//set app parameters and additional parameters if any, generated by Badger
	xmlDoc.LoadFile(argv[1]);

	if (xmlDoc.Error())
	{
		printf("Configuration error: %s\n", xmlDoc.ErrorName());
		exit(-1);
	}

	pConfig = xmlDoc.FirstChildElement("Controller-To-VFA-Policy");
	
	//INITIALISE WORLD -> STATE PROPERTIES
	RLSimion::g_pWorld = new CWorld(pConfig->FirstChildElement("World"));
//	RLSimion::g_pExperiment = new CExperiment(pConfig->FirstChildElement("Experiment"));

	//INTIALISE CONTROLLER: VIDAL, BOUKHEZZAR, ...
	tinyxml2::XMLElement* pControllerConfig = pConfig->FirstChildElement("Controller");
	CActor* pController = CActor::getInstance(pControllerConfig);

	tinyxml2::XMLElement* pVFAConfig = pConfig->FirstChildElement("Multi-Output-VFA");
	tinyxml2::XMLElement* pOutputConfig = pConfig->FirstChildElement("Output-File");

	if (!pVFAConfig || !RLSimion::g_pWorld || !pOutputConfig)
	{
		printf("Configuration error: Some component is missing in the config file\n");
		exit(-1);
	}

	CState *s = RLSimion::g_pWorld->getStateInstance();
	CAction *a = RLSimion::g_pWorld->getActionInstance();


	tinyxml2::XMLElement* pSingleOutputVFAConfig = pVFAConfig->FirstChildElement("Single-Output-VFA");
	//CONTROLLER -> ACTOR
	printf("\nSaving the weights of a VFA that approximates the controller...\n");
	FILE* pFile;
	unsigned int numWeights;
	unsigned int feature;
	double output;
	fopen_s(&pFile, pOutputConfig->GetText(), "wb");

	int i = 0;
	if (pFile)
	{
		CLinearVFA* pVFA;
		const char* pOutputAction;

		while (pSingleOutputVFAConfig)
		{

			pVFA = new CLinearVFA(pSingleOutputVFAConfig->FirstChildElement("Linear-VFA"));
			pOutputAction = pSingleOutputVFAConfig->FirstChildElement("Output-Action")->GetText();

			numWeights = pVFA->getNumWeights();
			fwrite(&numWeights, sizeof(unsigned int), 1, pFile);

			double *pWeights = new double[numWeights];

			for (feature = 0; feature < numWeights; feature++)
			{
				if (feature % 1000 == 0)
					printf("Output dim: %d      Progress: %2.2f%%\r", i + 1	,  100.0*((double)feature) / ((double)numWeights));
				pVFA->getFeatureStateAction(feature, s, a);
				pController->selectAction(s, a);
				output = a->getValue(i);
				pWeights[feature] = output;
			}
			fwrite(pWeights, sizeof(double), numWeights, pFile);
			delete[] pWeights;
			delete pVFA;
			i++;

			pSingleOutputVFAConfig = pSingleOutputVFAConfig->NextSiblingElement("Single-Output-VFA");
		}
		printf("\nDone\n");
		fclose(pFile);
	}
	else printf("ERROR: could not open output file\n");
	

	//CLEAN-UP
	delete a;
	delete s;
	delete RLSimion::g_pWorld;
	delete pController;
	//delete RLSimion::g_pExperiment;

}

