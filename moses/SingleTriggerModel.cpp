/* 
 * File:   SingleTriggerModel.cpp
 * Author: pmathur
 * 
 * Created on August 13, 2013, 5:21 PM
 */

#include "SingleTriggerModel.h"
#include "StaticData.h"
#include<sstream>
using namespace Moses;

inline int SingleTriggerModel::split_marker_perl(string str, string marker, vector<string> &array) {
	int found = str.find(marker), prev = 0;
	while (found != string::npos) // warning!
	{
		array.push_back(str.substr(prev, found - prev));
		prev = found + marker.length();
		found = str.find(marker, found + marker.length());
	}
	array.push_back(str.substr(prev));
	return array.size() - 1;
}

SingleTriggerModel::SingleTriggerModel() : StatelessFeatureFunction("SingleTriggerModel", 1) {
    m_sentence="";
}

SingleTriggerModel::~SingleTriggerModel() {
    m_sentence.resize(0);
    m_stm.empty();
}

void SingleTriggerModel::Read(const std::string filePath)
{
    // read the trigger model
    
    const StaticData& staticData = StaticData::Instance();
    util::FilePiece inFile(filePath.c_str(), staticData.GetVerboseLevel() >= 1 ? &std::cerr : NULL);
    size_t line_num = 0;
    while (true) {
        ++line_num;
        StringPiece line;
        try {
            line = inFile.ReadLine();
        } catch (util::EndOfFileException &e) {
            break;
        }

        util::TokenIter<util::MultiCharacter> pipes(line, util::MultiCharacter("|||"));
        StringPiece sourcePhraseString(GrabOrDie(pipes, filePath, line_num));
        StringPiece targetPhraseString(GrabOrDie(pipes, filePath, line_num));
        StringPiece scoreString(GrabOrDie(pipes, filePath, line_num));
        char* err_ind;
        float score = FloorScore(TransformScore(static_cast<float> (strtod(scoreString->data(), &err_ind))));
        if (err_ind == scoreString->data()) {
            stringstream strme;
            strme << "Bad number " << scoreString << " on line " << line_num;
            UserMessage::Add(strme.str());
            abort();
        }
        // Insertion in memory
        m_stm[std::pair<sourcePhraseString, targetPhraseString>]=score;
    }
}

// this function should be called before decoding of a sentence starts
void SingleTriggerModel::SetSentence(std::string sent)
{
    m_sentence = sent;
}

void SingleTriggerModel::Evaluate(const TargetPhrase& tp, ScoreComponentCollection* out) const {
    // for now assuming that we have m_sentence set already
    float score = 0.0;
    std::string t = "";
    size_t endpos = tp.GetSize();
    for (size_t pos = 0; pos < endpos; ++pos) {
        t = tp.GetWord(pos).GetFactor(0)->GetString();
        std::vector<string> array;
        int x = split_marker_perl(m_sentence, " ", array);
        for (int i = 0; i < x; i++) {
            if(m_stm.find(std::pair<array[i], t>) != m_stm.end())
            {
                score += m_stm[std::pair<array[i], t>];
            }
        }
    }
    out->PlusEquals(this, score);
}

void SingleTriggerModel::Evaluate(const PhraseBasedFeatureContext& context, ScoreComponentCollection* accumulator) const {
    const TargetPhrase& tp = context.GetTargetPhrase();
    Evaluate(tp, accumulator);
}

void SingleTriggerModel::EvaluateChart(const ChartBasedFeatureContext& context, ScoreComponentCollection* accumulator) const {
    const TargetPhrase& tp = context.GetTargetPhrase();
    Evaluate(tp, accumulator);
}