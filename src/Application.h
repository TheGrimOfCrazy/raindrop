
class Application 
{
	double oldTime;
	Screen *Game;
	
	struct {
		int Argc;
		char **Argv;
	} Args;

	enum
	{
		MODE_NULL,
		MODE_PLAY,
		MODE_CONVERT,
		MODE_GENCACHE,
		MODE_VSRGPREVIEW,
		MODE_STOPPREVIEW
	}RunMode;

	void ParseArgs();

	Directory InFile, OutFile;
	
	// VSRG-Specific
	enum {
		CONV_BMS,
		CONV_SM,
		CONV_OM
	} ConvertMode;

	int Measure;
	int difIndex;
	GString Author;

	bool Upscroll;

	void SetupPreviewMode();
	bool PollIPC();


public:

	Application(int argc, char *argv[]);

	void HandleInput(int32 key, KeyEventType state, bool isMouseInput);
	void HandleScrollInput(double xOff, double yOff);

	void Init();
	void Run();
	void Close();
};