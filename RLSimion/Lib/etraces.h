#pragma once
#include "parameters.h"
#include "features.h"

class ConfigNode;

class ETraces : public FeatureList
{
	bool m_bUse;
	DOUBLE_PARAM m_threshold;
	DOUBLE_PARAM m_lambda;
	BOOL_PARAM m_bReplace;
public:
	ETraces(ConfigNode* pConfigNode);
	ETraces();
	virtual ~ETraces();

	//traces will be multiplied by factor*lambda
	//traces are automatically cleared if it's the first step of an episode
	void update(double factor = 1.0);

	void addFeatureList(FeatureList *inList, double factor = 1.0);

	double getLambda() { return m_lambda.get(); };
	void setLambda(double value) { m_lambda.set(value); }

	double getTreshold() { return m_threshold.get(); };
	void setTreshold(double value) { m_threshold.set(value); }

	bool getReplace() { return m_bReplace.get(); }
	void setReplace(bool value) { m_bReplace.set(value); }
};