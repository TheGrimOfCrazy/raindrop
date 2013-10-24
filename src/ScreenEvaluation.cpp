#include "Global.h"
#include "Game_Consts.h"
#include "GraphObject2D.h"
#include "BitmapFont.h"
#include "ScreenEvaluation.h"
#include "GameWindow.h"
#include "ImageLoader.h"
#include "Audio.h"
#include "FileManager.h"

VorbisStream *ScreenEvaluationMusic = NULL;

ScreenEvaluation::ScreenEvaluation(IScreen *Parent) :
	IScreen(Parent)
{
	Running = true;
	Font = NULL;
}

int32 ScreenEvaluation::CalculateScore()
{
	return Results.NumExcellents * 10000 +
		Results.NumPerfects * 8500 +
		Results.NumGreats * 4500 +
		Results.NumOK * 2000 +
		Results.NumBads * 1000;
		
}

void ScreenEvaluation::Init(EvaluationData _Data)
{
	if (!ScreenEvaluationMusic)
	{
		ScreenEvaluationMusic = new VorbisStream((FileManager::GetSkinPrefix() + "screenevaluationloop.ogg").c_str());
		ScreenEvaluationMusic->setLoop(true);
		MixerAddStream(ScreenEvaluationMusic);
	}

	ScreenEvaluationMusic->seek(0);
	ScreenEvaluationMusic->Start();


	Background.SetImage(ImageLoader::LoadSkin("MenuBackground.png"));
	if (!Font)
	{
		Font = new BitmapFont();
		Font->LoadSkinFontImage("font_screenevaluation.tga", glm::vec2(10, 20), glm::vec2(32, 32), glm::vec2(10,20), 32);
	}
	Results = _Data;

	char _Results[256];
	const char *Text ="Excellent: \n"
					  "Perfect:   \n"
					  "Great:     \n"
					  "Bad:       \n"
					  "NG:        \n"
					  "OK:        \n"
					  "Misses:    \n"
					  "\n"
					  "Max Combo: \n"
					  "Score:     \n";

	ResultsString = Text;

	sprintf(_Results, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n\n%d\n%d\n",
	Results.NumExcellents,
		Results.NumPerfects,
		Results.NumGreats,
		Results.NumBads,
		Results.NumNG,
		Results.NumOK,
		Results.NumMisses,
		Results.MaxCombo,
		CalculateScore());

	ResultsNumerical = _Results;
}


void ScreenEvaluation::HandleInput(int32 key, KeyEventType code, bool isMouseInput)
{
	if ((BindingsManager::TranslateKey(key) == KT_Escape || BindingsManager::TranslateKey(key) == KT_Select) && code == KE_Press)
		Running = false;
}

void ScreenEvaluation::Cleanup()
{
	ScreenEvaluationMusic->Stop();
	delete Font;
}

bool ScreenEvaluation::Run(double)
{
	Background.Render();
	if (Font)
	{
		Font->DisplayText(ResultsString.c_str(),        glm::vec2( ScreenWidth/2 - 110, ScreenHeight/2 - 100 ));
		Font->DisplayText(ResultsNumerical.c_str(),     glm::vec2( ScreenWidth/2, ScreenHeight/2 - 100 ));
		Font->DisplayText("results screen",			    glm::vec2( ScreenWidth/2 - 70, 0 ));
		Font->DisplayText("press space to continue...", glm::vec2( ScreenWidth/2 - 130, ScreenHeight*7/8 ));
	}
	return Running;
}