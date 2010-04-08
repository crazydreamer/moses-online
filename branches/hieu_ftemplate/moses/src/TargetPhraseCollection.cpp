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

#include <algorithm>
#include "TargetPhraseCollection.h"

using namespace std;

// helper for sort
struct CompareTargetPhrase
{
 	bool operator() (const TargetPhrase *a, const TargetPhrase *b)
  {
 		return a->GetFutureScore() > b->GetFutureScore();
 	}
};

void TargetPhraseCollection::Sort(size_t tableLimit)
{
  vector<TargetPhrase*>::iterator 
  	iterMiddle = (tableLimit == 0 || m_collection.size() < tableLimit) ?m_collection.end() : m_collection.begin() + tableLimit;
  
	//std::sort(m_collection.begin(), m_collection.end(), CompareTargetPhrase());
  std::nth_element(m_collection.begin(), iterMiddle, m_collection.end(), CompareTargetPhrase());
}

void TargetPhraseCollection::Prune(size_t tableLimit)
{
  vector<TargetPhrase*>::iterator 
  	iterMiddle = (tableLimit == 0 || m_collection.size() < tableLimit) ?m_collection.end() : m_collection.begin() + tableLimit;
  
	//std::sort(m_collection.begin(), m_collection.end(), CompareTargetPhrase());
  std::nth_element(m_collection.begin(), iterMiddle, m_collection.end(), CompareTargetPhrase());

	if (iterMiddle != m_collection.end())
	{
		vector<TargetPhrase*>::iterator iter;
		for (iter = iterMiddle; iter != m_collection.end() ; ++iter)
		{
			delete *iter;
		}
		m_collection.erase(iterMiddle, m_collection.end());
	}
}

// friend
ostream& operator<<(ostream& out, const TargetPhraseCollection &coll)
{
	TargetPhraseCollection::const_iterator iter;
	for (iter = coll.begin() ; iter != coll.end() ; ++iter)
		out << **iter << endl;
	return out;
}
