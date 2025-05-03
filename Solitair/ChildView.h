// ChildView.h : interface of the CChildView class  
//  

#pragma once  
#include <Gdiplus.h>  

using namespace Gdiplus;  

// Enumeration for card suits  
enum SUIT { HEARTS = 0, DIAMONDS, SPADES, CLUBS };  

// Structure to define card values  
typedef struct  
{  
   unsigned short shown : 1;       // True if the card is shown  
   unsigned short suit : 2;       // Card suit (HEARTS, DIAMONDS, etc.)  
   unsigned short cardnumber : 4; // Card number (1 to 13)  
} CARDTYPE;  

// Structure to define card resources  
typedef struct  
{  
   CARDTYPE id;       // Card type (suit and number)  
   Image* img;        // Pointer to the card image  
   RECT r;            // Rectangle defining the card's position on the screen  
} CARDRESOURES;  

// Structure to store undo record information  
typedef struct  
{  
   CARDRESOURES m_card7stacks[7][18]; // 7 stacks of cards  
   CARDRESOURES m_cardaces[4][13];    // 4 stacks for aces  
   CARDRESOURES m_closedpack[24];     // Stack of closed cards  
   CARDRESOURES m_openpack[24];       // Stack of open cards  
   int m_numbercard7stacks[7];        // Number of cards in each of the 7 stacks  
   int m_numbercardaces[4];           // Number of cards in each ace stack  
   int m_numberclosedpack;            // Number of cards in the closed stack  
   int m_numberopenpack;              // Number of cards in the open stack  
} UNDORECORD;  

typedef struct
{
	time_t	m_datetime;				// date time time game was wone
	int		m_besttime;				// time game was wone in
} BESTTIMERECORD;

// CChildView class definition  
class CChildView : public CWnd  
{  
// Construction  
public:  
   CChildView(); // Constructor  

// Attributes  
public:  
   ULONG_PTR gdiplusToken; // GDI+ token for managing resources  
   CARDRESOURES m_cardresources[53]; // Array of card resources (52 cards + 1 extra)  
   Image* m_cardback;                // Image for the back of the cards  
   int m_cardpack[52];               // Array representing the unshuffled pack of cards  
   int m_shuffledcards[52];          // Array representing the shuffled cards  
   CARDRESOURES m_card7stacks[7][18]; // 7 stacks of cards  
   CARDRESOURES m_cardaces[4][13];    // 4 stacks for aces  
   CARDRESOURES m_closedpack[24];     // Stack of closed cards  
   CARDRESOURES m_openpack[24];       // Stack of open cards  
   int m_numbercard7stacks[7];        // Number of cards in each of the 7 stacks  
   int m_numbercardaces[4];           // Number of cards in each ace stack  
   int m_numberclosedpack;            // Number of cards in the closed stack  
   int m_numberopenpack;              // Number of cards in the open stack  
   CARDRESOURES m_selectedcards[13];  // Group of selected cards  
   int m_numberofselectedcards;       // Number of cards in the selected group  
   BOOL m_bleftbutton;                // Indicates if the left mouse button is pressed  
   CPoint m_dragpoint, m_newpoint;    // Points for tracking drag operations  
   // --- these varables are available during OnPaint
   int m_cardmargin;                  // Margin between cards, calculated during drawing  
   int	m_cardheight;				  // card height in screen units
   int	m_cardwidth;				  // card width in screen units
   Gdiplus::Font* m_cardfont;		  // card aces stack font for empty pack
   Pen* m_cardrectpen;				  // card pen to draw rectangle with
   SolidBrush* m_tablecolorbrush;	  // the card table brush
   int	m_screenheight;
   // -----------------------------------------------
   BOOL m_bFinnish;                   // Indicates if the game is in the finishing state  
   CString m_datadir;                 // Directory for card resources  
   int m_highscore;                   // Current high score  
   BOOL m_timerstarted;               // Indicates if the timer has started  
   time_t m_startgametimer;           // Start time of the game  
   BOOL m_newhighscore;               // Indicates if a new high score was achieved  
   BOOL m_gamefinnished;              // Indicates if the game has finished  
   int m_timetofinishgame;            // Time taken to finish the game  
   UNDORECORD m_undorec;              // Undo record for one-time undo functionality  
   BESTTIMERECORD	m_besttimes[10];  // The last 10 best times 
   int		m_newscoreindex;		  // index of new high score

// Operations  
public:  
   void LoadResources();                // Load card resources  
   void ShuffelCards();                 // Shuffle the cards  
   void SetupLayout();                  // Set up the layout of cards in stacks  
   CARDRESOURES GetCard7StackUnderPt(CPoint point); // Get the card in the 7-stack under a point  
   CARDRESOURES GetOpenCardUnderPt(CPoint point);   // Get the open card under a point  
   CARDRESOURES GetAcesCardUnderPt(CPoint point);   // Get the ace card under a point  
   int Get7StackWithImage(CARDRESOURES card);       // Get the stack number of a card in the 7-stack  
   int GetAceStackWithImage(CARDRESOURES card);     // Get the ace stack number of a card  
   BOOL GetOpenStackWithImage(CARDRESOURES card);   // Check if a card is on top of the open stack  
   int GetAcesStackUnderPt(CPoint point);           // Get the ace stack number under a point  
   int Get7StackUnderPt(CPoint point);              // Get the 7-stack number under a point  
   BOOL MoveSelectedCardsToPt(CPoint point);        // Move selected cards to a point  
   BOOL MoveSelectedCard();                         // Move a single selected card  
   int GetCountOfUnshown7StackCards();              // Get the count of unopened cards in the 7 stacks  
   BOOL GetSelectedCardsUnderPt(CPoint point);      // Check if selected cards are under a point  
   BOOL OpenStackOneMore(CPoint point);             // Open one more card in the stack  
   BOOL MoveCardToFinnish(RECT* r);                 // Move a card to the finish stack  
   void SaveUndoRecord();                           // Save the current state for undo  
   void LoadUndoRecord();                           // Load the saved undo state  
   void GetBestTimes();								// read the 10 best times
   void WriteBestTimes();							// write the 10 best times
   BOOL IsNewBestTimes(int bt, time_t tm, int *id);	// TRUE if new best time, adds any new best time into m_besttimes and saves them returns id of best time
   void	FinnishGame();								// game has finnished setup finnish game
   void DrawAcesPacks(Gdiplus::Graphics *graphics);
   void DrawClosedPack(Gdiplus::Graphics* graphics);
   void DrawOpenPack(Gdiplus::Graphics* graphics);
   void DrawStacksPacks(Gdiplus::Graphics* graphics);
   void DisplayTimer(Gdiplus::Graphics* graphics);

// Overrides  
protected:  
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs); // Override for window creation  

// Implementation  
public:  
   virtual ~CChildView(); // Destructor  

// Generated message map functions  
protected:  
   afx_msg void OnPaint();               // Handle paint events  
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point); // Handle left mouse button down  
   afx_msg void OnLButtonUp(UINT nFlags, CPoint point);   // Handle left mouse button up  
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);   // Handle mouse move events  
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags); // Handle key down events  
   afx_msg void OnTimer(UINT_PTR nIDEvent);              // Handle timer events  
   DECLARE_MESSAGE_MAP()  

public:  
   afx_msg void OnFileNewgame(); // Start a new game  
   afx_msg void OnEditUndo();    // Undo the last action  
   afx_msg void OnHelpBesttimes();
};
