#include "stdafx.h"
#include "experiment.h"
#include "parameters.h"
#include "logger.h"
#include "world.h"
#include "globals.h"
#include "stats.h"
#include "timer.h"
#include "app.h"

CExperimentTime& CExperimentTime::operator=(CExperimentTime& exp)
{
	m_step = exp.m_step;
	m_episodeIndex = exp.m_episodeIndex;

	return *this;
}

bool CExperimentTime::operator==(CExperimentTime& exp)
{
	return m_step == exp.m_step && m_episodeIndex == exp.m_episodeIndex;
}

double CExperiment::getExperimentProgress()
{
	double progress = ((double)m_step - 1 + (m_episodeIndex - 1)*m_numSteps)
		/ ((double)m_numSteps*m_totalNumEpisodes - 1);
	return progress;
}

double CExperiment::getEpisodeProgress()
{
	double progress = ((double)m_step - 1)	/ ((double)m_numSteps - 1);
	return progress;
}

const char* CExperiment::getProgressString()
{
	sprintf_s(m_progressMsg, MAX_PROGRESS_MSG_LEN, "Episode: %d/%d Step %d/%d (%.2f%%)"
		, getEpisodeIndex(), getTotalNumEpisodes()
		, getStep(), getNumSteps()
		, getExperimentProgress()*100.0);
	return m_progressMsg;
}


bool CExperiment::isEvaluationEpisode()
{
	if (m_evalFreq > 0)
	{
		return (m_episodeIndex - 1) % (m_evalFreq + 1) == 0;
	}
	return false;
}

void CExperiment::reset()
{
	m_trainingEpisodeIndex= 0; //[1..m_numTrainingEpisodes]
	m_evalEpisodeIndex= 0; //[1..1+m_numTrainingEpisodes/ evalFreq]
	m_episodeIndex= 0; //[1..g_numEpisodes]
	m_step= 0; //]1..g_numStepsPerEpisode]
	m_bTerminalState = false;
}


void CExperiment::nextStep()
{
	if (!m_bTerminalState && m_step<m_numSteps)
		m_step ++;

	else m_step = 0;
}

void CExperiment::nextEpisode()
{
	//reset the terminal state flag at the beginning of every episode
	m_bTerminalState = false;

	m_episodeIndex++;
	if (isEvaluationEpisode()) m_evalEpisodeIndex++;
	else m_trainingEpisodeIndex++;
}

bool CExperiment::isFirstEpisode()
{
	//this would result in two calls
	/*if (isEvaluationEpisode()) return m_evalEpisodeIndex == 1;
	return m_trainingEpisodeIndex == 1;*/
	//for now, we may assume that the first episode will be an evaluation one
	return m_episodeIndex == 1;
}

bool CExperiment::isLastEpisode()
{
	if (isEvaluationEpisode()) return m_evalEpisodeIndex == m_numEvaluationEpisodes;
	return m_trainingEpisodeIndex == m_numTrainingEpisodes;
}

CExperiment::~CExperiment()
{
	if (m_pProgressTimer) delete m_pProgressTimer;
}

CLASS_INIT(CExperiment)
{
	if (!pParameters) return;

	CONST_DOUBLE_VALUE(m_progUpdateFreq ,"Progress-Update-Freq", 0.5,"Progress update frequency (seconds)");
	CONST_INTEGER_VALUE(m_randomSeed ,"Random-Seed", 1,"Random seed used to generate random sequences of numbers");

	CONST_INTEGER_VALUE(m_numTrainingEpisodes, "Num-Episodes", 1,"Number of episodes");
	CONST_INTEGER_VALUE(m_evalFreq, "Eval-Freq", 0,"Evaluation frequency (in episodes)");
	
	if (m_evalFreq != 0)
	{
		m_numEvaluationEpisodes = 1 + m_numTrainingEpisodes / m_evalFreq;
		m_totalNumEpisodes = m_numTrainingEpisodes + m_numEvaluationEpisodes;
	}
	else
	{
		m_numEvaluationEpisodes = 0;
		m_totalNumEpisodes = m_numTrainingEpisodes;
	}
	CONST_DOUBLE_VALUE(m_episodeLength, "Episode-Length", 1.0, "Length of an episode (seconds)");
	setNumSteps((unsigned int)(m_episodeLength / CApp::get()->World.getDT()));
	reset();


	m_pProgressTimer = new CTimer();
	//QueryPerformanceFrequency((LARGE_INTEGER*)&m_counterFreq);
	srand(m_randomSeed);

	END_CLASS();
}

CExperiment::CExperiment()
{
	m_randomSeed = 0;
	m_numEvaluationEpisodes = 0;
	m_totalNumEpisodes = 0;
	m_numTrainingEpisodes = 0;
	m_evalFreq = 0;
	setNumSteps(0);
	m_pProgressTimer = 0;
}


void CExperiment::timestep(CState* s, CAction* a, CState* s_p, CReward* r)
{
	char msg[1024];


	double time = m_pProgressTimer->getElapsedTime();//(double)(currentCounter - m_lastProgressReportCounter) / (double)m_counterFreq;

	if (time>m_progUpdateFreq || (isLastStep() && isLastEpisode()))
	{
		sprintf_s(msg, 1024, "%f", CApp::get()->Experiment.getExperimentProgress()*100.0);
		CLogger::logMessage(Progress, msg);
		m_pProgressTimer->startTimer();
	}

	bool evalEpisode = isEvaluationEpisode();
	if (isFirstEpisode() && isFirstStep())
		CApp::get()->Logger.firstEpisode();

	unsigned int episodeIndex = getRelativeEpisodeIndex();
	if (isFirstStep())
		CApp::get()->Logger.firstStep();

	//update stats
	//output step-stats
	CApp::get()->Logger.timestep(s, a, s_p, r);

	if (isLastStep())
		CApp::get()->Logger.lastStep();

	if (isLastEpisode() && (isLastStep()))
		CApp::get()->Logger.lastEpisode();
}