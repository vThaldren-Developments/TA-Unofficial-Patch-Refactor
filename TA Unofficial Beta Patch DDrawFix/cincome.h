#ifndef cincomeH
#define cincomeH


#define EnergyBar 1
#define MetalBar 2

#define PlayerHight 30
#define PlayerWidth 210

class CIncome
{
  private:
    LPDIRECTDRAWSURFACE lpIncomeSurf;
    unsigned int BlitState;
    int BackgroundType;
    int ShowAllyIncome();
    void PaintStoragebar(int posx, int posy, int Player, int Type);
    LPVOID SurfaceMemory;
    int lPitch;
    void DrawText(char *String, int posx, int posy, char Color);
    int ShowAllIncome();
	void ShowMyViewIncome (int posx, int posy);
    void ShowPlayerIncome(int Player, int posx, int posy);
    int DrawStorageText(float Storage, int posx, int posy);
    void DrawPlayerRect(int posx, int posy, char Color);
    void FillRect(char Color);
    bool First;
    int posX;
    int posY;
	int CursorX;
	int CursorY;
    void ReadPos();
    void WritePos();
    void CorrectPos();
	bool inRect(int x, int y);
    unsigned char GetPlayerColor(int Player);

	int PlayerDotColors[10];
    bool StartedInRect;
  public:
    CIncome(BOOL VidMem);
    ~CIncome();
    void BlitIncome(LPDIRECTDRAWSURFACE DestSurf);
    void Set(int BGType);
    bool Message(HWND WinProchWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	bool IsShow (RECT * Rect_p);
};

#endif

