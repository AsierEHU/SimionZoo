#include "stdafx.h"
#include "states-and-actions.h"
#include "noise.h"
#include "vfa-actor.h"
#include "vfa.h"
#include "vfa-policy.h"
#include "actor.h"
#include "experiment.h"
#include "globals.h"
#include "parameters-xml-helper.h"


CSingleOutputVFAPolicyLearner::CSingleOutputVFAPolicyLearner(tinyxml2::XMLElement* pParameters) : CParamObject(pParameters)
{
	m_pPolicy = new CSingleOutputVFAPolicy(pParameters->FirstChildElement("POLICY"));
}

CSingleOutputVFAPolicyLearner* CSingleOutputVFAPolicyLearner::getInstance(tinyxml2::XMLElement* pParameters)
{
	if (pParameters)
	{
		if (!strcmp(pParameters->Name(), "CACLA"))
			return new CCACLALearner(pParameters);
		if (!strcmp(pParameters->Name(), "Regular-Gradient"))
			return new CRegularPolicyGradientLearner(pParameters);
	}
	return 0;
}

CVFAActor::CVFAActor(tinyxml2::XMLElement* pParameters): CParamObject(pParameters)
{
	m_numOutputs = XMLParameters::countChildren(pParameters,"OUTPUTS");

	tinyxml2::XMLElement* pOutputs = pParameters->FirstChildElement("OUTPUTS");

	m_pPolicyLearners = new CSingleOutputVFAPolicyLearner*[m_numOutputs];

	tinyxml2::XMLElement* pOutput= pOutputs->FirstChildElement();
	for (int i = 0; i<m_numOutputs; i++)
	{
		//////////////hemen VFAActor motako haurrak hartu behar die bakarrik!!!
		m_pPolicyLearners[i] = CSingleOutputVFAPolicyLearner::getInstance(pOutput);
		pOutput = pOutput->NextSiblingElement();
	}

	if (pParameters->FirstChildElement("LOAD"))
		loadPolicy(pParameters->FirstChildElement("LOAD")->Value());

}

CVFAActor::~CVFAActor()
{
	if (m_pParameters->FirstChildElement("SAVE"))
		savePolicy(m_pParameters->FirstChildElement("SAVE")->Value());

	for (int i = 0; i<m_numOutputs; i++)
	{
		delete m_pPolicyLearners[i];
	}

	delete [] m_pPolicyLearners;

}

void CVFAActor::selectAction(CState *s, CAction *a)
{
	for (int i = 0; i<m_numOutputs; i++)
	{
		//each uni-dimensional policy sets its own action's value
		m_pPolicyLearners[i]->getPolicy()->selectAction(s, a);
	}
}

void CVFAActor::updatePolicy(CState* s, CAction* a, CState* s_p, double r, double td)
{
	if (!g_pExperiment->isEvaluationEpisode())
		for (int i = 0; i<m_numOutputs; i++)
		{
			//each uni-dimensional policy sets its own action's value
			m_pPolicyLearners[i]->updatePolicy(s, a, s_p, r, td);
		}
}


double CVFAActor::getProbability(CState* s, CAction* a)
{
	double actionProb = 1.0;
	double actionDist = 0.0;
	double mahalanobisDist = 0.0;
	double varProd = 1.0;
	double noiseWidth;
	double var_i; //action's i-th dimension's variance
	double a_width;
	double output;

	//http://en.wikipedia.org/wiki/Multivariate_normal_distribution

	//if there is no correlation:
	//f(x,mu)= exp(-0.5 * (x-mu)^T * (x-mu)/var(x))/ sqrt(2*pi^k* prod var(x))

	for (int i = 0; i<m_numOutputs; i++)
	{
		output = m_pPolicyLearners[i]->getPolicy()->getVFA()->getValue(s, a);

		var_i = std::max(0.000000001, m_pPolicyLearners[i]->getPolicy()->getExpNoise()->getSigma());
		a_width = 0.5*(a->getMax(i) - a->getMin(i));
		noiseWidth = fabs((a->getValue(i) - output) / a_width);

		if (noiseWidth != 0.0)
		{
			//actionDist= (m_pExpNoise[i]->getLastValue()/noiseWidth)*3.0; //because we are using sigma=1 /3 to map values into [-1,1]
			mahalanobisDist += noiseWidth*noiseWidth / (var_i*var_i);// Variance=1.0 , otherwise it should be  / (var_i*var_i);
			varProd = varProd* var_i*var_i;
		}
	}

	if (mahalanobisDist == 0.0) return 1.0;

	double root = sqrt(pow((2 * M_PI), (double)m_numOutputs)*varProd);
	double expo = exp(-0.5*mahalanobisDist);

	return std::min(1.0, expo / root);
}


void CVFAActor::savePolicy(const char* pFilename)
{
	FILE* pFile;

	if (!strcmp(pFilename, "NONE") || pFilename == 0 || pFilename[0] == 0) return;

	printf("Saving Policy (\"%s\")...", pFilename);

	fopen_s(&pFile, pFilename, "wb");
	if (pFile)
	{
		for (int i = 0; i<m_numOutputs; i++)
		{
			m_pPolicyLearners[i]->getPolicy()->getVFA()->save(pFile);
		}
		fclose(pFile);
	}

}

void CVFAActor::loadPolicy(const char* pFilename)
{
	FILE* pFile;

	if (!strcmp(pFilename, "NONE") || pFilename == 0 || pFilename[0] == 0) return;

	printf("Loading Policy (\"%s\")...", pFilename);

	fopen_s(&pFile, pFilename, "rb");
	if (pFile)
	{
		for (int i = 0; i<m_numOutputs; i++)
		{
			m_pPolicyLearners[i]->getPolicy()->getVFA()->load(pFile);
		}
		//assert(numWeights==numWeightsRead);
		fclose(pFile);
		printf("OK\n");
	}
	else printf("FAILED\n");
}