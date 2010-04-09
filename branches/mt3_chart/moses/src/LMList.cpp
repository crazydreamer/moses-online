// $Id$

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2006 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#include "LMList.h"
#include "Phrase.h"
#include "LanguageModelSingleFactor.h"
#include "ScoreComponentCollection.h"
#include "ScoreIndexManager.h"
#include "FactorCollection.h"
#include "StaticData.h"
#include "Util.h"

using namespace std;

namespace Moses
{

LMList::~LMList()
{
	RemoveAllInColl(m_coll);
}

void LMList::Add(LanguageModel *lm)
{
	m_coll.push_back(lm);
	m_maxNGramOrder = (lm->GetNGramOrder() > m_maxNGramOrder) ? lm->GetNGramOrder() : m_maxNGramOrder;

	const ScoreIndexManager &scoreMgr = StaticData::Instance().GetScoreIndexManager();
	size_t startInd = scoreMgr.GetBeginIndex(lm->GetScoreBookkeepingID())
				,endInd		= scoreMgr.GetEndIndex(lm->GetScoreBookkeepingID()) - 1;

	m_minInd = min(m_minInd, startInd);
	m_maxInd = max(m_maxInd, endInd);
}

void LMList::CalcScore(const Phrase &phrase
											 , float &retFullScore
											 , float &retNGramScore
											 , ScoreComponentCollection &breakdown
											 , ScoreComponentCollection *nGramOnly
											 , bool useNGramScore) const
{ 
	assert(phrase.GetNumTerminals() == phrase.GetSize());

	const_iterator lmIter;
	for (lmIter = begin(); lmIter != end(); ++lmIter)
	{
		const LanguageModel &lm = **lmIter;
		const float weightLM = lm.GetWeight();

		float fullScore, nGramScore;

		// do not process, if factors not defined yet (happens in partial translation options)
		if (!lm.Useable(phrase))
			continue;

		lm.CalcScore(phrase, fullScore, nGramScore);

		breakdown.Assign(&lm, useNGramScore? nGramScore : fullScore);  // I'm not sure why += doesn't work here- it should be 0.0 right?

		if (nGramOnly != NULL)
		{
			nGramOnly->Assign(&lm, nGramScore);
		}

		retFullScore   += fullScore * weightLM;
		retNGramScore	+= nGramScore * weightLM;
	}	
}

void LMList::CalcScore(const Phrase &phrase
										, ScoreComponentCollection &nGramOnly
										, ScoreComponentCollection *beginningBitsOnly) const
{
	assert(phrase.GetNumTerminals() == phrase.GetSize());

	const_iterator lmIter;
	for (lmIter = begin(); lmIter != end(); ++lmIter)
	{
		const LanguageModel &lm = **lmIter;

		// do not process, if factors not defined yet (happens in partial translation options)
		if (!lm.Useable(phrase))
			continue;

		float beginningScore, nGramScore;
		lm.CalcScore2(phrase, beginningScore, nGramScore);

		nGramOnly.PlusEquals(&lm, nGramScore);

		if (beginningBitsOnly)
			beginningBitsOnly->PlusEquals(&lm, beginningScore);

	}	

}

}
